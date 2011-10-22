#ifndef OGGHTTPDH
#define OGGHTTPDH

//#include <Poco/Net/ServerSocket.h>
//#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <vector>
#include <map>
#include <iostream>
#include <iterator>
#include <sstream>
#include "IOBuffer.h"
#include "OggHTTPWorker.h"

extern "C" {
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}

using namespace std;

using std::map;
using std::vector;
//using Poco::Net::ServerSocket;
using Poco::Thread;
using Poco::Runnable;
//using Poco::Net::StreamSocket;
using Poco::Mutex;


class OggHTTP {
public:
	OggHTTP();
	~OggHTTP();
	void setup(int width, int height, int bps, int port);
	void start();
	void sendToClients(unsigned char* pixels);
	
private:
	OggHTTPWorker worker;
	Thread thread;
	
	th_info theora_info;	
	th_enc_ctx* theora_context;
	ogg_stream_state ogg_stream;
	ogg_packet header_packet;
	ogg_page header_page;
	
	int port;
	int width;
	int height;
	int bps;
		
	// used to encode to ogg
	// ================================================================
	th_ycbcr_buffer ycbcr;
	unsigned long yuv_w;
	unsigned long yuv_h;

	unsigned char *yuv_y; // @todo free this
	unsigned char *yuv_u; // @todo free this
	unsigned char *yuv_v; // @todo free this

	unsigned int x;
	unsigned int y;
	struct SwsContext* convert_context; // @todo free this
	vpx_image_t* in_image; // @todo free
	unsigned char* out_planes[3];
	int out_strides[3];
	int line_size ;
	
	IOBuffer header_buffer;
	// ================================================================	
};
#endif