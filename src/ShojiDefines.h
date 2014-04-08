#pragma once

const int SHOJI_FPS = 60;
const int SHOJI_NUM_BLOBS = 4;

const int VIDEO_WIDTH = 320;
const int VIDEO_HEIGHT = 240;

template<typename T>
void SWAP( T & a, T & b ) { T temp = a; a = b; b=(temp); }