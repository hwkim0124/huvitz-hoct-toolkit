#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace OctPattern {

	constexpr int TRIGGER_FORE_PADDING_POINTS = 96; // 72; // 48; // 24; //  8; // 5;
	constexpr int TRIGGER_POST_PADDING_POINTS = 8; // 2; // 5;
	constexpr float TRIGGER_FORE_PADDING_RATIO = 0.1f;
	constexpr float TRIGGER_POST_PADDING_RATIO = 0.1f;

	constexpr float PATTERN_SCAN_RANGE_X_MIN = 0.0f;// 2.0f;
	constexpr float PATTERN_SCAN_RANGE_X_MAX = 12.0f;// 12.0f;
	constexpr float PATTERN_SCAN_RANGE_Y_MIN = 0.0f;// 2.0f;
	constexpr float PATTERN_SCAN_RANGE_Y_MAX = 9.0f;// 9.0f;
	constexpr float PATTERN_SCAN_ANGLE_MIN = -180.0f;
	constexpr float PATTERN_SCAN_ANGLE_MAX = 179.0f;

	constexpr int PATTERN_SCAN_POINTS_MIN = 128;
	constexpr int PATTERN_SCAN_POINTS_MAX = 2048;
	constexpr int PATTERN_SCAN_LINES_MIN = 1;
	constexpr int PATTERN_SCAN_LINES_MAX = 512; //  256;

	constexpr float PATTERN_SCAN_CENTER_X = 0.0f;
	constexpr float PATTERN_SCAN_CENTER_Y = 0.0f;


	constexpr int PATTERN_PREVIEW_LINE_NUM_POINTS = 1024; // 512;


	// Line Trace id (Index of Trajectory)
	/////////////////////////////////////////////////////////////////////////////////////
	constexpr int TRACE_ID_START = 0;
	constexpr int TRACE_ID_CLOSE = 31;
	constexpr int TRACE_ID_MAX_LINES = (TRACE_ID_CLOSE + 1);
	constexpr int TRACE_ID_MAX_REPEATS = (TRACE_ID_MAX_LINES / 2);

	constexpr int TRACE_ID_PREVIEW_START = 0;
	constexpr int TRACE_ID_PREVIEW_POINT = 0;
	constexpr int TRACE_ID_PREVIEW_LINE = 0;
	constexpr int TRACE_ID_PREVIEW_CUBE = 0;
	constexpr int TRACE_ID_PREVIEW_HIDDEN = 2;
	constexpr int TRACE_ID_PREVIEW_LINE_HD = 4;

	constexpr int TRACE_ID_ENFACE_START = 6; // 4;
	constexpr int TRACE_ID_ENFACE_CUBE = 6; // 4;

	constexpr int TRACE_ID_MEASURE_START = 0;
	constexpr int TRACE_ID_MEASURE_LINE = 0;
	constexpr int TRACE_ID_MEASURE_CIRCLE = 0;
	constexpr int TRACE_ID_MEASURE_CROSS = 0;
	constexpr int TRACE_ID_MEASURE_RADIAL = 0;
	constexpr int TRACE_ID_MEASURE_RASTER = 0;
	constexpr int TRACE_ID_MEASURE_CUBE = 0;

	// Pattern Frame
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int PATTERN_FRAMES_ENFACE_MAX = (TRACE_ID_MAX_LINES - TRACE_ID_ENFACE_START);
	constexpr int PATTERN_FRAMES_MEASURE_MAX = (TRACE_ID_MAX_LINES - TRACE_ID_MEASURE_START);

}


#ifdef __OCTPATTERN_DLL
#define OCTPATTERN_DLL_API		__declspec(dllexport)
#else
#define OCTPATTERN_DLL_API		__declspec(dllimport)
#endif