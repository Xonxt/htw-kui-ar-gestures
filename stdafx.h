// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

struct YCbCrBounds {
	int Y_MIN = -1, Y_MAX = -1;
	int Cr_MIN = -1, Cr_MAX = -1;
	int Cb_MIN = -1, Cb_MAX = -1;
};

struct HsvBounds {
	int H_MIN = -1, H_MAX = -1;
	int S_MIN = -1, S_MAX = -1;
	int V_MIN = -1, V_MAX = -1;
};

// define several skin-segmentation methods
enum SkinSegmMethod {
	SKIN_SEGMENT_ADAPTIVE = 0,
	SKIN_SEGMENT_YCRCB = 1,
	SKIN_SEGMENT_HSV = 2
};

// TODO: reference additional headers your program requires here
