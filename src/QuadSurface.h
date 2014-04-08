#pragma once

#include "ofxOpenCv.h"

#include "ShojiDefines.h"

class QuadSurface
{
	public:

		void setup();
		void draw();
		void drawMask();

		void update( ofxCvContourFinder & a_contour );

		ofPoint getCenterPos() const;
		ofVec3f getPt( int a_index ) const { return m_pos[a_index]; }

	private:

		float calcSurfaceArea() const;
		void sortPoints();

	private:

		ofVec3f m_pos[SHOJI_NUM_BLOBS];

};