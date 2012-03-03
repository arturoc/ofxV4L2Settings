#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
	gui.setup("panel","settings.xml",0,0);
	settings.setup("/dev/video0");
	map<string,ofxV4L2Settings::Control>::iterator it;
	for(it=settings.controls.begin();it!=settings.controls.end();it++){
		cout << it->first << " " << it->second.parameter << endl;
		gui.add(new ofxIntSlider(it->second.parameter.getName(),it->second.parameter,it->second.minimum,it->second.maximum));
	}

	grabber.initGrabber(640,480);
}

//--------------------------------------------------------------
void testApp::update(){
	grabber.update();
}

//--------------------------------------------------------------
void testApp::draw(){
	grabber.draw(200,0);
	gui.draw();
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
