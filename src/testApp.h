#pragma once

#include "ofMain.h"
#include "ofxUI.h"
#include "ofxOpenCv.h"
#include "ofxQuadWarp.h"

#include "QuadSurface.h"
#include "ShojiDrawer.h"

class testApp : public ofBaseApp
{
	public:
		void setup();
		void update();
		void draw();
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void debugUpdate();
        void debugDraw();
    
        void mainUpdate();
    
        void exit();
        void guiEvent(ofxUIEventArgs &e);
    
	private:
		void setupGui();
		void setupCamera( int a_id );

	private:

		// gui
		ofxUISuperCanvas *debugGUI;
        ofxUISuperCanvas *gui;

		int m_camId;
		int m_camRequestedId;

		QuadSurface m_quadSurface;
		ShojiDrawer m_shojiDrawer;

		ofVideoGrabber 		vidGrabber;

        ofxCvColorImage			colorImg;
        ofxCvGrayscaleImage 	grayImage;
		ofxCvGrayscaleImage 	grayBg;
		ofxCvGrayscaleImage 	grayDiff;

        ofxCvContourFinder 	contourFinder;

		bool				m_drawMask;
        bool                debug;
		int 				threshold;
        int                 blobsTotal;
		bool				bLearnBakground;
        float               minBlobSize;
};

