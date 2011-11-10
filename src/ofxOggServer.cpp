#include "ofxOggServer.h"
#include "ofxOggConnection.h"

ofxOggServer* ofxOggServer::instance_ = NULL;

ofxOggServer::ofxOggServer() 
:server_port(0)
,server_host("*")
,sock(NULL)
,reactor(NULL)
,address(NULL)
,yuv_y(NULL)
,yuv_u(NULL)
,yuv_v(NULL)
{
	instance_ = this;
	ofAddListener(ofEvents.setup, this, &ofxOggServer::onSetup);
}

ofxOggServer::~ofxOggServer() {
	if(sock != NULL) {
		delete sock;
	}
	if(reactor != NULL) {
		delete reactor;
	}
	if(address != NULL) {
		delete address; 
	}
	if(yuv_y != NULL) {
		free(yuv_y);
	}
	if(yuv_u != NULL) {
		free(yuv_u);
	}
	if(yuv_v != NULL) {
		free(yuv_v);
	}
}

void ofxOggServer::onSetup(ofEventArgs& ev) {
	setupOgg(ofGetWidth(),ofGetHeight(), 3);
}


ofxOggServer& ofxOggServer::instance() {
	return *instance_;
}

void ofxOggServer::setupServer(int port) {
	server_port = port;
}

void ofxOggServer::setupServer(string host, int port) {
	server_host = host;
	server_port = port;
}

bool ofxOggServer::start() {
	if(sock != NULL) {
		printf("! error starting ogg server, already created\n");
		return false;
	}
	if(server_host == "*") {	
		IPAddress wildcard_address;
		address = new SocketAddress(wildcard_address, server_port);
	}
	else {
		address = new SocketAddress(server_host, server_port);		
	}

	sock = new ServerSocket(*address);
	reactor = new SocketReactor();
	acceptor = new ofxOggAcceptor<ofxOggConnection>(*sock, *reactor, this);
	ofxOggAcceptor<ofxOggConnection> acc(*sock, *reactor, this);

	send_thread.start(send_handler);	
	reactor_thread = new Thread();
	reactor_thread->start(*reactor);
	return true;
}

void ofxOggServer::addClient(ofxOggConnection* client) {
	clients.push_back(client);
}

void ofxOggServer::removeClient(ofxOggConnection* client) {
	mutex.lock();
	vector<ofxOggConnection*>::iterator it = std::find(clients.begin(), clients.end(), client);
	if(it != clients.end()) {
		printf("! removing clients.\n");
		clients.erase(it);
	}
	else {
		printf("! error while trying to find a connection object.\n");
	}
	mutex.unlock();
}

void ofxOggServer::setupOgg(int w, int h, int bytesPerPixel) {
	width = w;
	height = h;
	bpp = bytesPerPixel;
	
	th_info_init(&theora_info);    
	theora_info.frame_width = ((w + 15) >>4)<<4; // why (?)
    theora_info.frame_height = ((h + 15)>>4)<<4; // why (?)
    theora_info.pic_width = w;
    theora_info.pic_height = h;
    theora_info.pic_x = 0;
    theora_info.pic_y = 0;
	theora_info.colorspace = TH_CS_UNSPECIFIED;
    theora_info.pixel_fmt = TH_PF_420;
    theora_info.fps_numerator = 35;
    theora_info.fps_denominator = 1;
    theora_info.aspect_numerator = 1;
    theora_info.aspect_denominator = 1;
	theora_info.target_bitrate = 800000;
	theora_info.keyframe_granule_shift = 1;
	//theora_info.quality = 6;
	
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
			header_buffer.storeBytes(header_page.header, header_page.header_len);
			header_buffer.storeBytes(header_page.body, header_page.body_len);
		}
	}

	// rest of headers... before creating a new page.
	while (ogg_stream_flush(&ogg_stream, &header_page) > 0) {
		header_buffer.storeBytes(header_page.header, header_page.header_len);
		header_buffer.storeBytes(header_page.body, header_page.body_len);
	}
	
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

	out_planes[0] = ycbcr[0].data;
	out_planes[1] = ycbcr[1].data;
	out_planes[2] = ycbcr[2].data;

	out_strides[0] = ycbcr[0].stride;
	out_strides[1] = ycbcr[1].stride;
	out_strides[2] = ycbcr[2].stride;

	line_size = w * bpp;	
}

OggBuffer ofxOggServer::getOggHeaderBuffer() {
	return header_buffer;
}


void ofxOggServer::addFrame() {
	// grab pixels from screen
	grab_image.grabScreen(0,0,width,height);
	unsigned char* pixels = grab_image.getPixels();
	
	// convert
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

	OggBuffer new_buffer;
	while(th_encode_packetout(theora_context, last, &oggpacket) > 0) {
		ogg_stream_packetin(&ogg_stream, &oggpacket);
		while(ogg_stream_pageout(&ogg_stream, &oggpage)) {
			new_buffer.storeBytes(oggpage.header, oggpage.header_len);
			new_buffer.storeBytes(oggpage.body, oggpage.body_len); 
		}
		//printf("add frame...\n");
	}

	send_handler.addBuffer(new_buffer);	
}

vector<ofxOggConnection*>& ofxOggServer::getClients() {
	return clients;
}


