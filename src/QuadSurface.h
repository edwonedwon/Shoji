#pragma once

#include "ofxOpenCv.h"
#include "ofxQuadWarp.h"

#include "ShojiDefines.h"

class QuadSurface
{
	public:

		void setup();
		void draw();
		void update( ofxCvContourFinder & a_contour );

		ofPoint GetCenterPos() const;

		ofxQuadWarp & GetQuadWarp() { return m_warper; }

	private:
		void sortPoints();

	private:

		ofVec3f m_pos[SHOJI_NUM_BLOBS];
		ofxQuadWarp m_warper;


};