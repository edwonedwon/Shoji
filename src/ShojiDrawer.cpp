#include "ShojiDrawer.h"

void ShojiDrawer::draw( const QuadSurface & a_quadSurface )
{
	int width = ofGetScreenWidth();
	int height = ofGetScreenHeight();

	ofVec2f screenDimen = ofVec2f( width, height );

	const int numLines = 24;
	const int numSegments = 64;

	const float deltaY = float(height) / numLines;
	const float deltaX = float(width) / numSegments;

	const float time = ofGetElapsedTimef() * 0.1f;

	ofSetColor( 100 );
	ofSetLineWidth(20.0f);

	static float offset = 0.0f;
	float offset_goal = a_quadSurface.getCenterPos().x*0.001f;
	offset = ofLerp( offset, offset_goal, 0.03f );

	for ( int iLine = 0; iLine < numLines; ++iLine )
	{
		float yCenter = iLine * deltaY;

		for ( int iSeg = 0; iSeg < numSegments; ++iSeg )
		{
			float x_0 = deltaX * iSeg;
			float x_1 = deltaX * (iSeg+1);

			const float octave = 0.001f;
			const float deltaHeight = deltaY * 3.0f;

			float y_0 = yCenter + deltaHeight * ofNoise( offset + time + iLine, x_0*octave );
			float y_1 = yCenter + deltaHeight * ofNoise( offset + time + iLine, x_1*octave );

			ofLine( x_0, y_0, x_1, y_1 );
		}
	}

	ofSetLineWidth(1.0f);
}