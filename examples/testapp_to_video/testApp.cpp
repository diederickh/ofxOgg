#include "testApp.h"
/*

Example showing how to record an openFrameworks app to a ogg vorbis video
file. The quality is almost the same as your application and I got encoding
speeds of 60fps!

To play the videos you can drag&drop the file from 
bin/data/openFramweworksAppMovie-000#.ogv into chrome/firefox or with
VideoLan player.

*/


//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(33);
}

//--------------------------------------------------------------
void testApp::update(){

}

//--------------------------------------------------------------
void testApp::draw(){
	float t = ofGetElapsedTimef();
	glColor3f((1+sin(t*0.5))*0.5, (1+sin(t*1.5))*0.5, (1+sin(t*0.5))*0.05);	
	ofCircle(
		 ofGetWidth()*0.5 + cos(t * 0.5) * 100
		,ofGetHeight()*0.5
		,10 + (1+cos(t * 1.3)*0.5)*50
	);

	// add a new frame to the video.
	ogg.addFrame();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	if(key == ' ') {
		if(ogg.isRecordingEnabled()) {
			// stop recoding
			ogg.disableRecording();
		}
		else {
			ogg.enableRecording();
		}
	}
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 

}