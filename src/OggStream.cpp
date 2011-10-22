#include "OggStream.h"
//#include "Roxlu.h"
#include "testApp.h"

OggStreamData::OggStreamData(int serialNumber) 
:serial(serialNumber)
,packet_count(0)
,headers_read(false)
,header_count(0)
{
}
OggStreamData::~OggStreamData() {
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
OggTheoraData::OggTheoraData() 
:setup(0)
,context(0)

{
	th_info_init(&info);
	th_comment_init(&comment);
}

OggTheoraData::~OggTheoraData() {
	th_setup_free(setup);
	th_decode_free(context);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
void OggStream::setApp(testApp* p) {
	app = p;
}

OggStream::OggStream() 
:theora_p(0)
,vorbis_p(0)
,stateflag(0)
,pp_level(0)
,pp_level_max(0)
,videobuf_ready(0)
,videobuf_granulepos(-1)
,videobuf_time(0)
,frames(0)
,ts(NULL)
,td(NULL)
,app(NULL)
,has_new_image(false)
{
	new_image.setUseTexture(false);
	new_image.allocate(640,360,OF_IMAGE_COLOR);
}

OggStream::~OggStream() {
	th_info_clear(&ti);
    th_comment_clear(&tc);
}

void OggStream::setup(string sHost, int nPort, string sUrl) {
	
	// setup ogg stream.
	ogg_sync_init(&oy);
	
	// init vorbis.
	vorbis_info_init(&vi);
	vorbis_comment_init(&vc);
	
	// theora headers.
	th_comment_init(&tc);
	th_info_init(&ti);
	
	// setup socket
	url = sUrl;
	host = sHost;
	port = nPort;
	
	// create conversion context
	// context used to convert RGB->YUV
	int w = 640; // tmp
	int h = 360; // tmp
	sws_context = sws_getContext(
		 w
		,h
		,PIX_FMT_YUV420P
		,w
		,h
		,PIX_FMT_RGB24
		,SWS_FAST_BILINEAR
		,NULL
		,NULL
		,NULL
	);
	
}


void OggStream::start() {
		//receiveOgg();

}

void OggStream::run() {
	receiveOgg();
}

int OggStream::receiveOgg() {
	SocketAddress address(host, port);
	sock = new StreamSocket(address);

	stringstream rstr;
	rstr	<< "GET " << url << " HTTP/1.1\r\n"
			<< "HOST: " << host <<":" << port << "\r\n"
			<< "\r\n" << flush;

			
	string data = rstr.str();
	send((void*)data.c_str(), data.size());
	printf("receving ogg\n");


	char buf[512];
	int received = 0;
	received = sock->receiveBytes(buf, 512);
	// check if we got all the buffers
	int num_ends = 0;
	int read = 0;
	int i = 0;
	bool found = false;

	while(true) {
		if(buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
			i = i + 4;
			found = true;
			break;
		}
		if(i > received) {
			break;
		}
		++i;
	}
	if(found) {
		// check if have more data in the buffer.
		if(i < received) {
			printf("we got more data\n");
			int left = received - i;
			uint8_t* dest = (uint8_t*)ogg_sync_buffer(&oy,left);
			memcpy(dest, buf+i, left);
			ogg_sync_wrote(&oy,received);
			printf("to buffer: %d\n", left);

		}
		printf("---------- foud it -------------\n");
	}
	else {
		printf("nope");
	}
	printf("bytes read: %d bytes consumed: %d\n", received, i);
	
	//exit(1);
	//return;
	//buffer.printHex();
	//return;
	//string raw;
	//int consumed =  buffer.consumeUntil("\r\n", raw);
	//printf("consumed: %d\n", consumed);
	//string raw = buffer.consumeString(received);

	//printf("Response: ->%s<- read:%d \n\n", raw.c_str(), buffer.getNumBytesStored());
//	
	//printf("->%s<-\n",raw.c_str());
	//return;
	// parse HTTP header
	//buffer.reset();
	char tmp_buf[1024];
	while(true) {
			received = sock->receiveBytes(tmp_buf, 1024);
			printf("<< %d\n", received);
			uint8_t* dest = (uint8_t*)ogg_sync_buffer(&oy,received);
			memcpy(dest, tmp_buf, received);
			ogg_sync_wrote(&oy,received);
			printf("! read : %d\n", received);
	/*
		received = sock->receiveBytes(buffer.getStorePtr(), 4096);
		buffer.addNumBytesStored(received);		
		printf("< got total bytes: %d\n", buffer.getNumBytesStored());
*/
			parseOgg();
		/*
		if(buffer.getNumBytesStored() >= 322279) {
			break;
		}
		*/
	}
	/*
	if(bailout) {
		parseOgg();
	}
	//buffer.saveToFile(ofToDataPath("received.ogv"));
	*/
	return 1;
	//return buffer.getNumBytesStored();
}

/*
void OggStream::parseFromBuffer(IOBuffer& buf) {
	bool fake_download = true;
	if(!fake_download) {
		buffer = buf;
		parseOgg();
	}
	else {
		
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^ RETURNING HERE WHILE DEBUGGIN ^^^^^^^^^^^
		
		// simulate partial downloads...
		// --------------------------------
		int to_read = buf.getNumBytesStored();	
		unsigned char tmp_buf[512];
		while(to_read > 0)  {
			int from_buffer = buf.consumeBytes(tmp_buf, 512);
			uint8_t* dest = (uint8_t*)ogg_sync_buffer(&oy,from_buffer);
			memcpy(dest, tmp_buf, from_buffer);
			ogg_sync_wrote(&oy,from_buffer);
			printf("read:%d\n", from_buffer);
			fflush(stdout);
			
			//int num_read = buffer.consumeBytes(dest, 4096);
			//ogg_sync_wrote(&oy,num_read);
		
			//int read = buffer.storeBuffer(buf, 4096) ;
			//buffer.printHex(buffer.consumed, buffer.consumed+3);
			to_read -= from_buffer;
			if(to_read <= 0) {
				break;
			}
			if(!parseOgg()) {
				//buffer.resetConsumed();
			}
			printf("consumed pos: %d\n", from_buffer);
			//printf("copied: %d, read:%d to read:%d consumed:%d \n", buffer.getNumBytesStored(), read, to_read, buffer.consumed);
		}
	}
}
*/

bool OggStream::parseOgg() {
	printf("! parse ogg\n");
	OggStreamData* stream = 0;
	bool could_process = false;
	// get headers - startup info
	// -------------------------
	//while(!stateflag) {
		//printf("--------------------: body bytes=%d headerbytes=%d\n", oy.bodybytes, oy.headerbytes);
		// read data from buffer.
		/*
		uint8_t* dest = (uint8_t*)ogg_sync_buffer(&oy,4096);
		int num_read = buffer.consumeBytes(dest, 4096);
		ogg_sync_wrote(&oy,num_read);
		*/
		//printf("adding bytes read: %d\n", num_read);
		
		/*
		if(num_read == 0) {
			return false;
		//	printf("! nothing read\n");
			break;
		}
		printf("%c, %c, %c - num_read: %d ##########\n", dest[0], dest[1], dest[2], num_read);
	*/
		// read pageouts
		while(ogg_sync_pageout(&oy, &og) > 0) {
		
			could_process = true;
			int serial = ogg_page_serialno(&og);
			printf("! got serial: %d\n", serial);

			// beginning of new stream.
			if(ogg_page_bos(&og)) { 
				stream = new OggStreamData(serial);
				int init_result = ogg_stream_init(&stream->state, serial);
				if(init_result != 0) {
				//	printf("$ error init stream\n");
					exit(1);
				}
				streams[serial] = stream;
			}
			else {
				if(streams.find(serial) != streams.end()) {
					stream = streams[serial];
				}
				else {
				//	printf("$ error serial not found.\n");
				}				
			}
			
			// after this we can check for new packets with data
			int in_result = ogg_stream_pagein(&stream->state,&og);
			if(in_result == 0) {
			//	printf("!!!! Error with stream in\n");
			}
			
			// decode data from stream
			ogg_packet packet;
			while(true) {
				int pack_result = ogg_stream_packetout(&stream->state,&packet);
				if(pack_result == 0) {
				//	printf("! we need more data for packet\n.");
					break;
				}
				else if(pack_result == -1) {
					break;
				}
				else {
					if(!stream->headers_read) {
						printf("we can decode this packet: %d <--------\n", stream->serial);
						// check if the packet is a theora header.
						int ret = th_decode_headerin(
										 &stream->theora.info
										,&stream->theora.comment
										,&stream->theora.setup
										,&packet
						);
						if(ret == TH_ENOTFORMAT) {
						//	printf("!!!! error unknown format\n");
						}
						else if(ret > 0) {
							// start of header
							stream->type = OGGSTREAM_TYPE_THEORA;
						}
						else if(ret == 0) {
						//	printf("-------- header for %d\n", stream->serial);
							// we got header, not yet initialized.
							stream->theora.context = th_decode_alloc(
										&stream->theora.info
										,stream->theora.setup
							);
							if(stream->theora.context == NULL) {
							//	printf("!!!! error: cannot create theora context\n");
								exit(1);
							}
							stream->header_count++;
							if(stream->header_count == 3) {
								printf("))))))))) all theora headers read\n");
								stream->headers_read = true;
								break; // go out of page.
							}
							
						}
					}
					else {
						if(stream->type != OGGSTREAM_TYPE_THEORA) {
							break;
						}
						printf("YES HEADER READ\n");
						oggDecodeTheoraFrame(stream, packet);
					}
					
				}
				stream->packet_count++;
			}// while true..	
			
			
		} // end of page
//	}
	printf("return: %b\n", could_process);
	return could_process;
	// tmp printf streams.
	map<int,OggStreamData*>::iterator it = streams.begin();
	while(it != streams.end()) {
		printf("Serial foud: %d\n", it->first);
		++it;
	}
	return true;
		
}


void OggStream::oggDecodeTheoraFrame(OggStreamData* stream, ogg_packet& packet) {
	// decode video
	ogg_int64_t granulepos = -1;
	int r = th_decode_packetin(stream->theora.context, &packet, &granulepos);
	if(r != 0) {
		printf("err...\n");
		return;
	}
	
	th_ycbcr_buffer yuv;
	r = th_decode_ycbcr_out(stream->theora.context, yuv);
	if(r != 0) {
		printf("err..\n");
		return;
	}

	int i;
	int y_offset, uv_offset;
	th_decode_ycbcr_out(td,yuv);

	printf("! decoded width: %d height: %d stride: %d\n", yuv[0].width, yuv[0].height, yuv[0].stride);
	//printf("! size: %d\n", buffer.getNumBytesStored());
//	exit(1);
	// decode the frame.
	int w = yuv[0].width;
	int h = yuv[0].height;
	w = 640;
	h = 360;
	
	const int src_strides[3] = {yuv[0].stride, yuv[1].stride, yuv[2].stride};
	int out_strides[3] = {3*w,0,0};
	unsigned char* src_slices[3] = {yuv[0].data, yuv[1].data, yuv[2].data};
	vpx_image_t* dest_image = vpx_img_alloc(NULL, VPX_IMG_FMT_RGB24, w, h, 0);
	
	int output_slice_h = sws_scale(
		sws_context
		,src_slices
		,src_strides
		,0
		,h
		,dest_image->planes
		,out_strides
	);
	
	printf("stride 0: %d, 1: %d, 2: %d\n", out_strides[0], out_strides[1], out_strides[2]);
	printf("! outslice h: %d\n", output_slice_h);
	int size = w * h * 3;
	unsigned char* converted_pixels = new unsigned char[size];

	for(int i = 0; i < w; ++i) {
		for(int j = 0; j < h; ++j) {
			int dx = (j*w*3)+i*3;
			int dx1 = (j*out_strides[0])+i;
			int dx2 = (j*out_strides[1])+i; 
			int dx3 = (j*out_strides[2])+i;

			converted_pixels[dx] = dest_image->planes[0][dx];
			converted_pixels[dx+1] = dest_image->planes[0][dx+1];;
			converted_pixels[dx+2] = dest_image->planes[0][dx+2];;
		}
	}
	//app->setPixels(converted_pixels);
	ofImage ofimg;
	ofimg.setUseTexture(false);
	ofimg.allocate(w,h,OF_IMAGE_COLOR);
	ofimg.setFromPixels(converted_pixels, w, h, OF_IMAGE_COLOR);
	static int counter = 0;
	char imgname[512];
	//sprintf(imgname, "test-%02d.jpg", counter++);
	//ofimg.saveImage(imgname);
	new_image.setFromPixels(converted_pixels,w,h,OF_IMAGE_COLOR);
	has_new_image = true;
	vpx_img_free(dest_image);
}


void OggStream::queueOggPage(ogg_page* page) {

	if(theora_p) {
		ogg_stream_pagein(&to,page);
	}
	if(vorbis_p) {
		ogg_stream_pagein(&vo,page);
	}
}
	

void OggStream::send(void* data, int len) {
	printf("! to send: %d\n", len);
	int left = len;
	while(left > 0) {
		int done = sock->sendBytes(data, len);
		left -= done;
		printf(">> send: %d\n", done);
	}
}

// -----------------------------------------------------------------------------

// debug 
void OggStream::printOggColorSpace() {
    switch(ti.colorspace) { 
		case TH_CS_UNSPECIFIED: {
			printf("! encoder did not specify a color space\n");
			break;
		}
		
		case TH_CS_ITU_REC_470M: {
			printf("! encoder specified ITU Rec 470M (NTSC) color.\n");
			break;
		}
		
		case TH_CS_ITU_REC_470BG: {
			printf("! encoder specified ITU Rec 470BG (PAL) color.\n");
			break;
		}
		
		default: {
			printf("! warning: encoder specified unknown colorspace (%d).\n",ti.colorspace);
			break;
		}
	}
}

void OggStream::printOggComments() {

	 if(tc.comments){
		
		for(int i=0;i<tc.comments;i++){
		  if(tc.user_comments[i]){
			int len=tc.comment_lengths[i];
			unsigned char* value = new unsigned char[len+1];
			//value=malloc(len+1);
			memcpy(value,tc.user_comments[i],len);
			value[len]='\0';
			printf("\t%s\n", value);
			delete[] value;
		  }
		}
	  }
}

double OggStream::getOggVideoTime() {
	static ogg_int64_t last = 0;
	static ogg_int64_t up = 0;
	ogg_int64_t now;
	struct timeval tv;

	gettimeofday(&tv,0);
	now = tv.tv_sec*1000+tv.tv_usec/1000;
	return now;
}
