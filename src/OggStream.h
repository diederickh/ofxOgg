#ifndef OGGSTREAMH
#define OGGSTREAMH

#include <sys/time.h>
#include <string>
#include "ofMain.h"

extern "C" {
	#include "external/theora/theoradec.h"
	#include "external/ogg/ogg.h"
	#include "external/vorbis/codec.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}
#include <iostream>
#include <sstream>
#include <map>

#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"

//#include "Roxlu.h"

/*

ogg_sync_state    oy;
ogg_page          og;
ogg_stream_state  vo;
ogg_stream_state  to;
th_info           ti;
th_comment        tc;
th_setup_info    *ts;
th_dec_ctx       *td;

*/

using namespace std;
using namespace Poco;
using namespace Poco::Net;

#define OGGSTREAM_TYPE_THEORA  1

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class OggTheoraData {
public:
	OggTheoraData();
	~OggTheoraData();
	
	th_info info;
	th_comment comment;
	th_setup_info *setup;
	th_dec_ctx* context;
	
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

class OggStreamData {
public:

	OggStreamData(int serialNumber);
	~OggStreamData();

	int serial;
	ogg_stream_state state;
	int packet_count;
	int type;
	bool headers_read;
	OggTheoraData theora;
	int header_count;
};

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class testApp;
class OggStream : public Poco::Runnable {
public:
	virtual void run();
	void setApp(testApp* theApp);
	OggStream();
	~OggStream();
	void setup(string host, int port, string url);
	void start();
	void send(void* data, int len);
	int receiveOgg();
//	void parseFromBuffer();
	
	// ogg related
	bool parseOgg();
	void queueOggPage(ogg_page* page);
	void printOggColorSpace();
	void printOggComments();
	double getOggVideoTime();
	void oggDecodeTheoraFrame(OggStreamData* stream, ogg_packet& packet);
	testApp* app;
	
	ofImage new_image;
	bool has_new_image;

	
private:
	map<int, OggStreamData*> streams;
	
	string host;
	int port;
//	IOBuffer buffer;
	string url;
	StreamSocket* sock;

	// ogg related stuff
	ogg_sync_state oy; // oy
	
	th_comment tc;
	th_info ti;
	th_pixel_fmt px_fmt;
	th_setup_info    *ts;
	th_dec_ctx       *td;
	
	int theora_p;			// theora pages
	int vorbis_p;			// vorbis pages
	int stateflag;
	vorbis_info      vi;
	vorbis_dsp_state vd;
	vorbis_block     vb;
	vorbis_comment   vc;
	ogg_page         og;
	ogg_stream_state vo;
	ogg_stream_state to;
	ogg_packet		 op;
	
	int pp_level_max;	
	int pp_level;
	int pp_inc;
	
	// single frame video buffering.
	int          videobuf_ready;
	ogg_int64_t  videobuf_granulepos;
	double       videobuf_time;
	
	int frames;
	
	// converting to RGB
	struct SwsContext* sws_context;
	
	

/*
ogg_sync_state   oy;
ogg_page         og;
ogg_stream_state vo;
ogg_stream_state to;
th_info      ti;
th_comment   tc;
th_dec_ctx       *td;
th_setup_info    *ts;
vorbis_info      vi;
vorbis_dsp_state vd;
vorbis_block     vb;
vorbis_comment   vc;
th_pixel_fmt     px_fmt;

*/
  
};

#endif