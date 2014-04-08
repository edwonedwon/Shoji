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
	blobsTotal = SHOJI_NUM_BLOBS;
    minBlobSize = 5;
    
    // DEBUG GUI SWITCH
	m_drawMask = true;
    debug = true;
    
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
	bLearnBakground = true;
}

void testApp::setupGui()
{
	// camera
	gui = new ofxUISuperCanvas("Camera");
    gui->addToggle("Show Camera Debug", &debug);
	gui->addIntSlider("Select Camera ID", 0, vidGrabber.listDevices().size()-1, &m_camRequestedId );
    gui->autoSizeToFitWidgets();
    
    // blobs gui
    debugGUI = new ofxUISuperCanvas("Blobs");
    debugGUI->addSpacer();
	debugGUI->addToggle("Draw Quad Mask", &m_drawMask);
    debugGUI->addIntSlider("Threshold", 0, 100, &threshold);
	debugGUI->addIntSlider("Num Blobs", 0, 4, &blobsTotal);
    debugGUI->addSlider("Min Blob Size", 0.1, 20, &minBlobSize);
    debugGUI->addToggle("FullScreen", false);
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

	m_quadSurface.update( contourFinder );
}


void testApp::debugUpdate()
{
}

// DRAW --------------------------------------------------------------
void testApp::draw()
{
	m_shojiDrawer.draw( m_quadSurface );

	if (m_drawMask) 
		m_quadSurface.drawMask();

	if (debug == true)
	{
		m_quadSurface.draw();
		debugDraw();
	}
}

void testApp::debugDraw()
{
	const int debugWidth = VIDEO_WIDTH >> 1;
	const int debugHeight = VIDEO_HEIGHT >> 1;
	const int padding = 12;

    // draw the incoming, the grayscale, the bg and the thresholded difference
	ofSetHexColor(0xffffff);

	int i = 0;
	colorImg.draw(padding, padding + (i++) * (padding+debugHeight), debugWidth,debugHeight);
	ofDrawBitmapString( "Color Img", padding, (i) * (padding+debugHeight) );
	grayImage.draw(padding, padding + (i++) * (padding+debugHeight), debugWidth,debugHeight);
	ofDrawBitmapString( "Gray Img", padding, (i) * (padding+debugHeight) );
	grayBg.draw(padding, padding + (i++) * (padding+debugHeight), debugWidth,debugHeight);
	ofDrawBitmapString( "Gray BG", padding, (i) * (padding+debugHeight) );
	grayDiff.draw(padding, padding + (i++) * (padding+debugHeight), debugWidth,debugHeight);
	ofDrawBitmapString( "Gray Diff", padding, (i) * (padding+debugHeight) );
    
	// then draw the contours:

	const int windowContourWidth = VIDEO_WIDTH;
	const int windowContourHeight = VIDEO_HEIGHT;
	const int windowContourX = padding * 2 + debugWidth;
	const int windowContourY = padding;
    
	ofFill();
	ofSetHexColor(0x333333);
	ofRect(windowContourX,windowContourY, windowContourWidth, windowContourHeight);
	ofSetHexColor(0xffffff);
    
	// we could draw the whole contour finder
    //	contourFinder.draw(360,windowContourY);
    
	// or, instead we can draw each blob individually from the blobs vector,
	// this is how to get access to them:
    for (int i = 0; i < contourFinder.nBlobs; ++i)
	{
        contourFinder.blobs[i].draw(windowContourX,windowContourY);

		ofPoint circleCenter = ofPoint(
			 contourFinder.blobs[i].centroid.x + windowContourX,
             contourFinder.blobs[i].centroid.y + windowContourY
		);

		ofFill();
        ofSetColor(100,220,50);
        ofCircle(circleCenter, 8);
		ofSetColor(0);
		ofCircle(circleCenter, 2);
		
		// draw over the centroid if the blob is a hole
		ofSetColor(255);
        ofNoFill();
		if(contourFinder.blobs[i].hole){
			ofDrawBitmapString("hole",
                               contourFinder.blobs[i].boundingRect.getCenter().x + windowContourX,
                               contourFinder.blobs[i].boundingRect.getCenter().y + windowContourY);
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
	ofDrawBitmapString(reportStr.str(), windowContourX, windowContourY+windowContourHeight);
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
