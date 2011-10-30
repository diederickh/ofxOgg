#ifndef ofxOggSendHandlerH
#define ofxOggSendHandlerH

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include "IOBuffer.h"
#include <vector>
#include <deque>

using Poco::Runnable;
using Poco::Thread;
using Poco::Mutex;
using std::vector;
using std::deque;


class ofxOggSendHandler : public Runnable {
public:
	ofxOggSendHandler();
	~ofxOggSendHandler();
	virtual void run();
	void addBuffer(IOBuffer buf);
	deque<IOBuffer> getPreBuffers();
	static ofxOggSendHandler& instance();
private:
	Mutex mutex;
	deque<IOBuffer> prebuffers;
	vector<IOBuffer> buffers;
	
	static ofxOggSendHandler* instance_;
};

#endif