#pragma once

#include "ofxOpenCv.h"

#include "ShojiDefines.h"

class QuadSurface
{
	public:

		void setup();
		void draw();
		void update( ofxCvContourFinder & a_contour );

		ofPoint GetCenterPos() const;
		ofVec3f GetPt( int a_index ) const { return m_pos[a_index]; }

	private:
		void sortPoints();

	private:

		ofVec3f m_pos[SHOJI_NUM_BLOBS];

};