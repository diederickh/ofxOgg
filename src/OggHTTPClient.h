#ifndef OGGHTTPCLIENTH
#define OGGHTTPCLIENTH

//#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
//#include <vector>
//#include <map>
//#include <iostream>
//#include <iterator>
#include <sstream>
#include "IOBuffer.h"

extern "C" {
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}


using namespace std;
using Poco::Runnable;
using Poco::Thread;
using Poco::Mutex;
using Poco::Net::StreamSocket;
//using Poco::Net::ServerSocket;

class OggHTTPWorker;

class OggHTTPClient : public Runnable {
//class OggHTTPClient {
public:
	OggHTTPClient(OggHTTPWorker& server, int sockNum);
	~OggHTTPClient();
	virtual void run();
	void readFromSocket();
	void sendToClient(IOBuffer newbuf);
	void sendHTTPResponse();
	ogg_page header_page;
	int	sock;
private:

	bool			done;
	Mutex			mutex;
	OggHTTPWorker&	server;
//	StreamSocket	sock;
	IOBuffer		send_buffer;
	vector<char>	read_buffer;
	bool send_header;
};

#endif