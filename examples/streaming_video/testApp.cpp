#include "testApp.h"
/**
 * This example demonstrates how you can stream your openFrameworks
 * application to a VLC or a webpage. 
 *
 * To view the video stream of this  application open Firefox or Chrome 
 * with the url:
 * http://localhost:1111 (or VLC).
 *
 * There still is a bug with playing the video in Chrome.
 */

//--------------------------------------------------------------
void testApp::setup(){
	ofBackground(33);
	
	// start server on port 1111
	ogg_server.setupServer("localhost", 1111);
	ogg_server.start();
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
	
	// server a new frame
	ogg_server.addFrame();
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){

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