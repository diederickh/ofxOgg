#ifndef OFXOGGSERVERH
#define OFXOGGSERVERH

#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "Poco/Runnable.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/NObserver.h"
#include "Poco/AutoPtr.h"


#include "ofxOggConnection.h"
#include "ofxOggSendHandler.h"
#include <vector>
#include <string>



extern "C" {
	#include "external/theora/theoraenc.h"
	#include "external/ogg/ogg.h"
	#include "external/libswscale/swscale.h"
	#include "vpx_image.h"
}

using Poco::Net::ReadableNotification;
using Poco::NObserver;
using Poco::Thread;
using Poco::Mutex;
using Poco::Runnable;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::Net::SocketAddress;
using Poco::Net::SocketReactor;
using Poco::Net::SocketAcceptor;
using Poco::Net::IPAddress;
using std::string;
using std::vector;



template<typename T>
class ofxOggAcceptor;

class ofxOggServer {
public:

	ofxOggServer();
	~ofxOggServer();
	void setupServer(int port);
	void setupServer(string host, int port);
	void setupOgg(int w, int h, int bpp);
	bool start();
	void addClient(ofxOggConnection* client);
	void removeClient(ofxOggConnection* client);
	static ofxOggServer& instance();
	IOBuffer getOggHeaderBuffer();
	void addFrame(unsigned char* pixels);
	vector<ofxOggConnection*>& getClients();
	typedef vector<ofxOggConnection*>::iterator iterator;

private:
	void initOgg();
	
	int server_port;
	string server_host;
	ServerSocket* sock;
	SocketReactor* reactor;
	SocketAddress* address;
	Mutex mutex;
	SocketAcceptor<ofxOggConnection>* acceptor;
	Thread* thread;
	Thread send_thread;
	ofxOggSendHandler send_handler;
	vector<ofxOggConnection*> clients;
	//deque<IOBuffer> prebuffers;
	static ofxOggServer* instance_;
	
	int width;
	int height;
	int bpp;
	
	// ogg & theora
	// ================================================================
	th_info theora_info;	
	th_enc_ctx* theora_context;
	ogg_stream_state ogg_stream;
	ogg_packet header_packet;
	ogg_page header_page;
	
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


//extern ofxOggServer* ofxOggServer::instance = NULL;


template<typename T>
class ofxOggAcceptor : public SocketAcceptor<T> {
public:
	ofxOggAcceptor(ServerSocket& socket, SocketReactor& reactor, ofxOggServer* oggServer)
		:SocketAcceptor<T>(socket,reactor)
		,ogg_server(oggServer)
	{
	}
	ofxOggServer* ogg_server;
protected:
	/*
	virtual ServiceHandler* createServiceHandler(StreamSocket& socket)
	{
		return new ServiceHandler(socket, *_pReactor);
	}
	*/
	//virtual T* createServiceHandler(StreamSocket& socket) {
	
//	virtual T* createServiceHandler(StreamSocket& sock) {
//		//return new ofxOggConnection(sock, *(reactor()));
//		//return new ofxOggConnection(sock, *(SocketAcceptor<T>::reactor()));
//		//return new ofxOggConnection(sock, *(SocketAcceptor<T>::reactor()), ogg_server);
//	
//		printf("< new connection.\n");
//		T* obj = new T(sock, *(SocketAcceptor<T>::reactor()));
//		obj->setup(ogg_server);
//		ogg_server->addClient(obj);
//		return obj;
//	
//	}

};


#endif