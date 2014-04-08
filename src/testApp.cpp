#include "testApp.h"

#include "ShojiDefines.h"

void testApp::setup()
{
    ofSetFrameRate( SHOJI_FPS );
    ofSetVerticalSync( true );
    ofEnableSmoothing();
    
	m_camId = 0;
	m_camRequestedId = m_camId;
	setupCamera( m_camId );

    colorImg.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayImage.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayBg.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayDiff.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);

	bLearnBakground = true;
	threshold = 80;
    blobsTotal = 4;
    minBlobSize = 5;
    
    // DEBUG GUI SWITCH
    debug = true;
    showIR = true;
    
    setupGui();

	m_quadSurface.setup();
}

void testApp::setupCamera( int a_id )
{
	if ( vidGrabber.isInitialized() )
	{
		vidGrabber.close();
	}

	vidGrabber.setDeviceID(a_id);
    vidGrabber.setVerbose(true);
	vidGrabber.setDesiredFrameRate(SHOJI_FPS);

    vidGrabber.initGrabber(VIDEO_WIDTH, VIDEO_HEIGHT);
}

void testApp::setupGui()
{
	gui = new ofxUISuperCanvas("DEBUG");
    gui->addToggle("DEBUG", &debug);
    gui->addToggle("INFRARED VIDEO", &showIR);
    gui->addToggle("QUAD", &showQuad);
	gui->addIntSlider("CAM ID", 0, vidGrabber.listDevices().size()-1, &m_camRequestedId );
    gui->autoSizeToFitWidgets();
    
    // DEBUG GUI
    debugGUI = new ofxUISuperCanvas("BLOBS");
    debugGUI->addSpacer();
    debugGUI->addSlider("THRESHOLD", 0.0, 100, &thresholdFloat);
	debugGUI->addIntSlider("BLOBS", 0, 4, &blobsTotal);
    debugGUI->addSlider("MIN BLOB SIZE", 0.1, 20, &minBlobSize);
    debugGUI->addToggle("FULLSCREEN", false);
    debugGUI->autoSizeToFitWidgets();
    ofAddListener(debugGUI->newGUIEvent, this, &testApp::guiEvent);
    debugGUI->loadSettings("settings.xml");
}

// UPDATE --------------------------------------------------------------
void testApp::update()
{
	if ( m_camRequestedId != m_camId )
	{
		setupCamera( m_camRequestedId );
		m_camId = m_camRequestedId;
	}

    mainUpdate();
}

void testApp::mainUpdate()
{
    ofBackground(255,255,255);
    
    bool bNewFrame = false;
    
        vidGrabber.update();
        bNewFrame = vidGrabber.isFrameNew();
        
    if (bNewFrame)
	{
        colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
        
        grayImage = colorImg;
        if (bLearnBakground == true){
            grayBg = grayImage;		// the = sign copys the pixels from grayImage into grayBg (operator overloading)
            bLearnBakground = false;
        }
        
        // take the abs value of the difference between background and incoming and then threshold:
        grayDiff.absDiff(grayBg, grayImage);
        grayDiff.threshold(threshold);
        
        // find contours which are between the size of 20 pixels and 1/3 the w*h pixels.
        // also, find holes is set to true so we will get interior contours as well....
        contourFinder.findContours(grayDiff, minBlobSize, (340*240)/3, blobsTotal, true);	// find holes
	}
    
    
    // GUI STUFF
    threshold = (int)thresholdFloat;

	m_quadSurface.update( contourFinder );
}


void testApp::debugUpdate()
{
}

// DRAW --------------------------------------------------------------
void testApp::draw()
{
    if (debug == true)
        debugDraw();

	m_shojiDrawer.draw( m_quadSurface );
	m_quadSurface.draw();
}

void testApp::debugDraw()
{
    // draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetHexColor(0xffffff);
	colorImg.draw(20,20);
	grayImage.draw(360,20);
	grayBg.draw(20,280);
	grayDiff.draw(360,280);
    
	// then draw the contours:
    
	ofFill();
	ofSetHexColor(0x333333);
	ofRect(360,540,320,240);
	ofSetHexColor(0xffffff);
    
	// we could draw the whole contour finder
    //	contourFinder.draw(360,540);
    
	// or, instead we can draw each blob individually from the blobs vector,
	// this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; i++){
        contourFinder.blobs[i].draw(360,540);
        ofSetColor(100,220,50);
        ofFill();
        ofCircle(
                 contourFinder.blobs[i].centroid.x + 360,
                 contourFinder.blobs[i].centroid.y + 540,
                 10);
		
		// draw over the centroid if the blob is a hole
		ofSetColor(255);
        ofNoFill();
		if(contourFinder.blobs[i].hole){
			ofDrawBitmapString("hole",
                               contourFinder.blobs[i].boundingRect.getCenter().x + 360,
                               contourFinder.blobs[i].boundingRect.getCenter().y + 540);
		}
    }
    
	// finally, a report:
	ofSetHexColor(0xffffff);
	stringstream reportStr;
	reportStr
    //                << "bg subtraction and blob detection" << endl
    //			  << "press ' ' to capture bg" << endl
    //			  << "threshold " << threshold << " (press: +/-)" << endl
    << "num blobs found " << contourFinder.nBlobs << ", fps: " << ofGetFrameRate();
	ofDrawBitmapString(reportStr.str(), 20, 600);
}

// GUI FUNCTIONS -----------------------------------------------

void testApp::exit()
{
    debugGUI->saveSettings("settings.xml");
    delete debugGUI;
}

void testApp::guiEvent (ofxUIEventArgs &e)
{
    if(e.getName() == "THRESHOLD")
    {
//        ofxUISlider *slider = e.getSlider();
//        ofBackground(slider->getScaledValue());
    }
    else if(e.getName() == "FULLSCREEN")
    {
        ofxUIToggle *toggle = e.getToggle();
        ofSetFullscreen(toggle->getValue());
    }
}

// END GUI FUNCTIONS -------------------------------------------

//--------------------------------------------------------------
void testApp::keyPressed(int key)
{
	switch (key){
		case ' ':
			bLearnBakground = true;
			break;
		case '+':
			threshold ++;
			if (threshold > 255) threshold = 255;
			break;
		case '-':
			threshold --;
			if (threshold < 0) threshold = 0;
			break;
	}
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
