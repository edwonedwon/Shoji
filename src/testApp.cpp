#include "testApp.h"

#include "ShojiDefines.h"

void testApp::setup()
{
    ofSetFrameRate( SHOJI_FPS );
    ofSetVerticalSync( true );
    ofEnableSmoothing();
    
    vidGrabber.setDeviceID(1);
    vidGrabber.setVerbose(true);
    vidGrabber.initGrabber(VIDEO_WIDTH,VIDEO_HEIGHT);
	vidGrabber.setDesiredFrameRate(SHOJI_FPS);


    colorImg.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayImage.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayBg.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);
	grayDiff.allocate(VIDEO_WIDTH,VIDEO_HEIGHT);

	bLearnBakground = true;
	threshold = 80;
    blobsTotal = 4;
    blobsTotalFloat = 4;
    minBlobSize = 5;
    
    // DEBUG GUI SWITCH
    debug = true;
    showIR = true;
    
    gui = new ofxUISuperCanvas("DEBUG");
    gui->addToggle("DEBUG", &debug);
    gui->addToggle("INFRARED VIDEO", &showIR);
    gui->addToggle("QUAD", &showQuad);
    gui->autoSizeToFitWidgets();
    
    // DEBUG GUI
    debugGUI = new ofxUISuperCanvas("BLOBS");
    debugGUI->addSpacer();
    debugGUI->addSlider("THRESHOLD", 0.0, 100, &thresholdFloat);
    debugGUI->addSlider("BLOBS", 0.0, 4, &blobsTotalFloat);
    debugGUI->addSlider("MIN BLOB SIZE", 0.1, 20, &minBlobSize);
    debugGUI->addToggle("FULLSCREEN", false);
    debugGUI->autoSizeToFitWidgets();
    ofAddListener(debugGUI->newGUIEvent, this, &testApp::guiEvent);
    debugGUI->loadSettings("settings.xml");

    setupQuadWarp();

	m_quadSurface.setup();
}

void testApp::setupQuadWarp()
{
    showQuad = true;
    
    img.loadImage( "quad_warp_kittens.png" );
    
    int x = ( ofGetWidth()  - img.width  ) * 0.5;   // center on screen.
    int y = ( ofGetHeight() - img.height ) * 0.5;   // center on screen.
    int w = img.width;
    int h = img.height;
    
    fbo.allocate( w, h );
    
    warper.setSourceRect( ofRectangle( 0, 0, w, h ) );              // this is the source rectangle which is the size of the image and located at ( 0, 0 )
    warper.setTopLeftCornerPosition( ofPoint( x, y ) );             // this is position of the quad warp corners, centering the image on the screen.
    warper.setTopRightCornerPosition( ofPoint( x + w, y ) );        // this is position of the quad warp corners, centering the image on the screen.
    warper.setBottomLeftCornerPosition( ofPoint( x, y + h ) );      // this is position of the quad warp corners, centering the image on the screen.
    warper.setBottomRightCornerPosition( ofPoint( x + w, y + h ) ); // this is position of the quad warp corners, centering the image on the screen.
    warper.setup();
}

// UPDATE --------------------------------------------------------------
void testApp::update()
{
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
    blobsTotal = (int)blobsTotalFloat;

	m_quadSurface.update( contourFinder );
                                                
    updateQuadWarp();
}

void testApp::updateQuadWarp()
{
    for( int i=0; i<contourFinder.nBlobs; i++)
    {
        ofPoint corner;
        corner.x = ofMap(contourFinder.blobs[i].centroid.x, 0, 320, 0, ofGetWidth());
        corner.y = ofMap(contourFinder.blobs[i].centroid.y, 0, 240, 0, ofGetHeight());
        warper.setCorner(corner, i);
    }
}

void testApp::debugUpdate()
{
}

// DRAW --------------------------------------------------------------
void testApp::draw()
{
    if (debug == true)
        debugDraw();
    else
        mainDraw();

	m_quadSurface.draw();
}

void testApp::mainDraw()
{
    // draw the video
    if (showIR == true)
    {
        ofSetColor(255, 255, 255);
        grayDiff.draw(0, 0, (float)ofGetWidth(), (float)ofGetHeight());
    }
    
    // set the circle parameters
    ofSetColor(255, 100, 50);
    ofFill();
    
    // draw the circle that tracks to the light
    for (int i = 0; i < contourFinder.nBlobs; i++)
    {
        float x = ofMap(contourFinder.blobs[i].centroid.x, 0, 320, 0, ofGetWidth());
        float y = ofMap(contourFinder.blobs[i].centroid.y, 0, 240, 0, ofGetHeight());
    
        ofCircle(x,y, 30);
    }
    
    // set the rectangle params
    ofSetColor(100,255,57);
    ofFill();
    
    if (showQuad)
        drawQuadWarp();
}

void testApp::drawQuadWarp()
{
    ofSetColor( ofColor :: white );
    
    //======================== draw image into fbo.
    
    fbo.begin();
    {
        img.draw( 0, 0 );
    }
    fbo.end();
    
    //======================== get our quad warp matrix.
    
    ofMatrix4x4 mat = warper.getMatrix();
    
    //======================== use the matrix to transform our fbo.
    
    glPushMatrix();
    glMultMatrixf( mat.getPtr() );
    {
        fbo.draw( 0, 0 );
    }
    glPopMatrix();
    

    //======================== draw quad warp ui.
    
    ofSetColor( ofColor :: magenta );
    warper.draw();
    
    //======================== info.
    
    ofSetColor( ofColor :: white );
    ofDrawBitmapString( "to warp the image, drag the corners of the image.", 20, 30 );
    ofDrawBitmapString( "press 's' to toggle quad warp UI. this will also disable quad warp interaction.", 20, 50 );

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
    
    if( key == 's' )
        warper.toggleShow();
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
