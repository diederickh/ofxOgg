#ifndef ofxOggSendHandlerH
#define ofxOggSendHandlerH

#include <Poco/Thread.h>
#include <Poco/Runnable.h>
#include "OggBuffer.h"
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
	void addBuffer(OggBuffer buf);
	deque<OggBuffer> getPreBuffers();
	static ofxOggSendHandler& instance();
private:
	Mutex mutex;
	deque<OggBuffer> prebuffers;
	vector<OggBuffer> buffers;
	
	static ofxOggSendHandler* instance_;
};

#endif