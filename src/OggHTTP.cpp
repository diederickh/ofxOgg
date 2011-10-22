#include "OggHTTP.h"

OggHTTP::OggHTTP() {
}

OggHTTP::~OggHTTP() {
	worker.shutdown();
	thread.join();
}


void OggHTTP::setup(int w, int h, int bytesPerSecond, int nPort) {
	port = nPort;
	width = w;
	height = h;
	bps = bytesPerSecond;
	worker.setup(port);
	
	th_info_init(&theora_info);    
	theora_info.frame_width = ((w + 15) >>4)<<4; // why (?)
    theora_info.frame_height = ((h + 15)>>4)<<4; // why (?)
    theora_info.pic_width = w;
    theora_info.pic_height = h;
    theora_info.pic_x = 0;
    theora_info.pic_y = 0;
	theora_info.colorspace = TH_CS_UNSPECIFIED;
    theora_info.pixel_fmt = TH_PF_420;
    theora_info.fps_numerator = 15;
    theora_info.fps_denominator = 1;
    theora_info.aspect_numerator = 1;
    theora_info.aspect_denominator = 1;
//	theora_info.target_bitrate = 800000;
	theora_info.quality = 63;
	theora_info.keyframe_granule_shift = 0;
	
	// context to work with.
	// -----------------------
	theora_context = th_encode_alloc(&theora_info);  
	if(!theora_context) {
		printf("Error: cannot create context.\n");
		exit(1);
	}	
    th_info_clear(&theora_info);
	
	
	// Add obligatory headers
	// ---------------------
	th_comment comment;
	th_comment_init(&comment);
	th_comment_add(&comment, (char *)"openFrameworks");
	comment.vendor = (char *)"openFrameworks";
	
	
	ogg_stream_init(&ogg_stream, rand());
	while (th_encode_flushheader(theora_context, &comment, &header_packet) > 0) {
		ogg_stream_packetin(&ogg_stream, &header_packet);
		while (ogg_stream_pageout(&ogg_stream, &header_page)) {
			printf("header len: %d page len: %d\n", header_page.header_len, header_page.body_len);
			header_buffer.storeBytes(header_page.header, header_page.header_len);
			header_buffer.storeBytes(header_page.body, header_page.body_len);
		}
	}

	 
	// rest of headers... before creating a new page.
	while (ogg_stream_flush(&ogg_stream, &header_page) > 0) {
		header_buffer.storeBytes(header_page.header, header_page.header_len);
		header_buffer.storeBytes(header_page.body, header_page.body_len);
	}
	//header_buffer.printHex();
	
	worker.header_buffer = header_buffer;
	
	// Setup conversion objects
	// ------------------------
	yuv_w = w;
	yuv_h = h;

	yuv_w = (w + 15) & ~15;
	yuv_h = (h + 15) & ~15;
  
	ycbcr[0].width = yuv_w;
	ycbcr[0].height = yuv_h;
	ycbcr[0].stride = yuv_w;
	ycbcr[1].width = (yuv_w >> 1);
	ycbcr[1].stride = ycbcr[1].width;
	ycbcr[1].height = (yuv_h >> 1);
	ycbcr[2].width = ycbcr[1].width;
	ycbcr[2].stride = ycbcr[1].stride;
	ycbcr[2].height = ycbcr[1].height;
	
	
	// @todo free buffers
	ycbcr[0].data = yuv_y = (unsigned char*)malloc(ycbcr[0].stride * ycbcr[0].height);
	ycbcr[1].data = yuv_u = (unsigned char*)malloc(ycbcr[1].stride * ycbcr[1].height);
	ycbcr[2].data = yuv_v = (unsigned char*)malloc(ycbcr[2].stride * ycbcr[2].height);
	
	
	// context used to convert RGB->YUV
	convert_context = sws_getContext(
		w
		,h
		,PIX_FMT_RGB24
		,w
		,h
		,PIX_FMT_YUV420P
		,SWS_FAST_BILINEAR
		,NULL
		,NULL
		,NULL
	);

	if(convert_context == NULL) {
		printf("Error: cannot get convert context\n");
		exit(1);
	}

	in_image = vpx_img_alloc(NULL, VPX_IMG_FMT_RGB24, width, height, 0);
	//out_planes = {ycbcr[0].data, ycbcr[1].data, ycbcr[2].data};
	out_planes[0] = ycbcr[0].data;
	out_planes[1] = ycbcr[1].data;
	out_planes[2] = ycbcr[2].data;

//	int out_strides[3] = {ycbcr[0].stride, ycbcr[1].stride, ycbcr[2].stride};
	out_strides[0] = ycbcr[0].stride;
	out_strides[1] = ycbcr[1].stride;
	out_strides[2] = ycbcr[2].stride;

	line_size = w * 3;	

	printf("created ogg stream\n");
	
}

void OggHTTP::start() {
	worker.header_page = header_page;
	thread.start(worker);
}

void OggHTTP::sendToClients(unsigned char* pixels) {
	//return;
	/*
	IOBuffer test_buffer;
	string test_data = "testdata\n";
	test_buffer.storeString(test_data);
	worker.sendToClients(test_buffer);
	
	return;
	*/
	
	in_image = vpx_img_wrap(
		in_image
		,VPX_IMG_FMT_RGB24
		,width
		,height
		,0
		,pixels
	);
	
	int output_slice_h = sws_scale(
		 convert_context
		,in_image->planes
		,in_image->stride
		,0
		,height
		,out_planes
		,out_strides
	);	
	
	int enc_result = th_encode_ycbcr_in(theora_context, ycbcr);
	if(enc_result == TH_EFAULT) {
		fprintf(stderr, "error: could not encode frame\n");
		exit(1);
	}
	
	ogg_page oggpage;
	ogg_packet oggpacket;
	int last = 0;

	IOBuffer new_buffer;
	while(th_encode_packetout(theora_context, last, &oggpacket) > 0) {
		ogg_stream_packetin(&ogg_stream, &oggpacket);
		while(ogg_stream_pageout(&ogg_stream, &oggpage)) {
			new_buffer.storeBytes(oggpage.header, oggpage.header_len);
			new_buffer.storeBytes(oggpage.body, oggpage.body_len); 
		}
	}
	
	static int c = 0;
	c++;
	//printf("Add to queue, header len: %d  body_le:%d - %d	\n", oggpage->header_len, oggpage->body_len, c);
	//printf("Adding buffer: %d\n", new_buffer.getNumBytesStored());
	//ogg_stream_flush(&ogg_stream, &oggpage);
	worker.sendToClients(new_buffer);		
	//buffered_pages.push_back(oggpage);

}