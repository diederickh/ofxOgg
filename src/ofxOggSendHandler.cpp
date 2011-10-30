#include "ofxOggSendHandler.h"
#include "ofxOggServer.h"
#include "ofxOggConnection.h"

ofxOggSendHandler* ofxOggSendHandler::instance_ = NULL;

ofxOggSendHandler::ofxOggSendHandler() {
	instance_ = this;
}

ofxOggSendHandler::~ofxOggSendHandler() {
}

void ofxOggSendHandler::run() {
	while(true) {
		mutex.lock();
		vector<IOBuffer>::iterator it = buffers.begin();
		while(it != buffers.end()) {
			vector<ofxOggConnection*>& clients = ofxOggServer::instance().getClients();
			ofxOggServer::iterator client_it = clients.begin();
			while(client_it != clients.end()) {
				(*client_it)->send(*it);
				++client_it;
			}
			it = buffers.erase(it);
		}
		mutex.unlock();
	}
}

void ofxOggSendHandler::addBuffer(IOBuffer buf) {
	mutex.lock();
		buffers.push_back(buf);
		prebuffers.push_back(buf);
		if(prebuffers.size()>40) {
			prebuffers.erase(prebuffers.begin());
		}
	mutex.unlock();
}


deque<IOBuffer> ofxOggSendHandler::getPreBuffers() {
	return prebuffers;
}

ofxOggSendHandler& ofxOggSendHandler::instance() {
	return *instance_;
}