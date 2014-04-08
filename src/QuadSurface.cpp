#include "QuadSurface.h"

void QuadSurface::update( ofxCvContourFinder & a_contour )
{
	int num_points = SHOJI_NUM_BLOBS;

	for( int i=0; i< min(num_points,a_contour.nBlobs); i++)
    {
        ofPoint corner;
        corner.x = ofMap(a_contour.blobs[i].centroid.x, 0, VIDEO_WIDTH, 0, ofGetWidth());
        corner.y = ofMap(a_contour.blobs[i].centroid.y, 0, VIDEO_HEIGHT, 0, ofGetHeight());

		m_pos[i] = corner;
    }
}

void QuadSurface::setup()
{
}

void QuadSurface::draw()
{
	ofSetColor(0);

	for ( int i = 0; i < SHOJI_NUM_BLOBS; ++i )
	{
		ofCircle( m_pos[i], 10.0f );
	}
}
