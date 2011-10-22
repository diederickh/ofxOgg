#ifndef OGGHTTPWORKERH
#define OGGHTTPWORKERH

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "Poco/Thread.h"
#include <vector>
#include <map>
 

extern "C" {
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}

//#include <Poco/Net/ServerSocket.h>
//#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include <vector>
#include <map>
//#include <iostream>
//#include <iterator>
//#include <sstream>
#include "IOBuffer.h"

using namespace std;
using Poco::Runnable;
using Poco::Thread;
using Poco::Mutex;
//using Poco::Net::ServerSocket;


class OggHTTPClient;

class OggHTTPWorker : public Runnable {
public:
	OggHTTPWorker();
	~OggHTTPWorker();
	void setup(int port);
	virtual void run();
	void shutdown();
	void sendToClients(IOBuffer page);
	void removeClient(OggHTTPClient* client);
//	ogg_page* header_page;
	IOBuffer header_buffer;
	ogg_page header_page;	
private:

	Mutex mutex;
	int server_socket;
	struct sockaddr_in server_addr;
//	ServerSocket* sock;
	int port;
//	vector<OggHTTPClient*> clients;
//	map<Thread*, OggHTTPClient*> client_threads;
	map<int, OggHTTPClient*> clients;
};

#endif