#include "ofxOggConnection.h"
#include "ofxOggServer.h"
#include "ofxOggSendHandler.h"
#include <sstream>

ofxOggConnection::ofxOggConnection(StreamSocket rSocket, SocketReactor& rReactor) 
:sock(rSocket)
,reactor(rReactor)
,ogg_server(NULL)
{
	
	// add read event
	reactor.addEventHandler(
		 sock
		,NObserver<ofxOggConnection, ReadableNotification>(
					*this
					,&ofxOggConnection::onReadable
		)
	);
	
	
	// add shutdown event.
	reactor.addEventHandler(
		 sock
		,NObserver<ofxOggConnection, ShutdownNotification>(
					*this
					,&ofxOggConnection::onShutdown
		)
	);
	printf("new client...\n");
}

ofxOggConnection::~ofxOggConnection() {
	reactor.removeEventHandler(sock,NObserver<ofxOggConnection, ReadableNotification>(*this,&ofxOggConnection::onReadable));
	reactor.removeEventHandler(sock,NObserver<ofxOggConnection, ShutdownNotification>(*this,&ofxOggConnection::onShutdown));
}


void ofxOggConnection::setup(ofxOggServer* server) {
	ogg_server = server;
}

void ofxOggConnection::onReadable(const AutoPtr<ReadableNotification>& pNotif) {
	char buf[1024];
	int read = sock.receiveBytes(buf, 1024);
	if(read > 0) {
		read_buffer.storeBytes(buf, read);
		parseHTTPRequest();
	}
	else {
		ofxOggServer::instance().removeClient(this);
		delete this;
	}
}

void ofxOggConnection::parseHTTPRequest() {
	bool complete_header = false;
	int num_bytes = read_buffer.getNumBytesStored();
	if(num_bytes > 4) {
		for(int i = 0; i < num_bytes; ++i) {
			if(read_buffer.buffer[i] == '\r' 
				&& read_buffer.buffer[i+1] == '\n'
				&& read_buffer.buffer[i+2] == '\r'
				&& read_buffer.buffer[i+3] == '\n'
			)
			{
				complete_header = true;
				break;
			}
		}
	}
	
	if(complete_header) {
		// send response.
		sendHTTPResponse();
		sendOggHeader();
		deque<OggBuffer> prebuffers = ofxOggSendHandler::instance().getPreBuffers();
		deque<OggBuffer>::iterator it = prebuffers.begin();
		int num = 0;
		while(it != prebuffers.end()) {
			++num;
			send(*it);
			++it;
		}
		ofxOggServer::instance().addClient(this); 
	}
}

void ofxOggConnection::sendOggHeader() {
	send(ofxOggServer::instance().getOggHeaderBuffer());
}


void ofxOggConnection::sendHTTPResponse() {
	std::stringstream ss;
	ss	<< 	"HTTP/1.0 200 OK\r\n"
		<<	"Content-Type: application/ogg\r\n"
		<< "Connection: Keep-Alive\r\n"
		<< "Cache-Control: no-cache\r\n"
		<< "\r\n";
			
	string http_header		= ss.str();
	OggBuffer http_header_buffer;
	http_header_buffer.storeString(http_header);
	send(http_header_buffer);

}


void ofxOggConnection::send(OggBuffer buf) {
	int num_bytes = buf.getNumBytesStored();
	int bytes_sent = 0;
	int left = num_bytes;
	while(left > 0) {
		int done = sock.sendBytes(buf.getPtr()+bytes_sent, num_bytes);
		if(done == -1) {
			ofxOggServer::instance().removeClient(this);
			delete this;
			return;
		}
		left -= done;
		bytes_sent += done;
	}
}


void ofxOggConnection::onShutdown(const AutoPtr<ShutdownNotification>& pNotif) {
	ofxOggServer::instance().removeClient(this);
	delete this;
}