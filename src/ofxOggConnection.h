#ifndef OFXOGGCONNECTIONH
#define OFXOGGCONNECTIONH

#include "Poco/Net/SocketReactor.h"
#include "Poco/Net/SocketAcceptor.h"
#include "Poco/Net/SocketNotification.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/NObserver.h"
#include "Poco/Exception.h"
#include "Poco/Thread.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/AutoPtr.h"

#include "Poco/StringTokenizer.h"
#include "Poco/String.h"
#include <iostream>
#include "IOBuffer.h"

using Poco::Net::SocketReactor;
using Poco::Net::SocketAcceptor;
using Poco::Net::ReadableNotification;
using Poco::Net::ShutdownNotification;
using Poco::Net::ServerSocket;
using Poco::Net::StreamSocket;
using Poco::NObserver;
using Poco::AutoPtr;
using Poco::Thread;
using Poco::StringTokenizer;
using Poco::toLowerInPlace;

using namespace std;
class ofxOggServer;


class ofxOggConnection {
public:
	ofxOggConnection(StreamSocket rSocket, SocketReactor& rReactor);

	~ofxOggConnection();
	void setup(ofxOggServer* oggServer);
	int write(IOBuffer& amfBuffer);
	void onReadable(const AutoPtr<ReadableNotification>& pNotif);
	void onShutdown(const AutoPtr<ShutdownNotification>& pNotif);
	void parseHTTPRequest();
	void sendHTTPResponse();
	void sendOggHeader();
	void send(IOBuffer buffer);

private:
	// ogg
	ofxOggServer* ogg_server;

	// network
	StreamSocket sock;
	SocketReactor& reactor;

	// buffers
	IOBuffer read_buffer;
};
#endif