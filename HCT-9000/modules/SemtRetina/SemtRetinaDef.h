#pragma once


#include "OctGlobal2.h"

using namespace OctGlobal;


namespace RetSegm {
	class SegmLayer;
	class SegmImage;
}

namespace SemtRetina
{
	constexpr int CLASS_VITREOUS = 0;
	constexpr int CLASS_RNFL = 1;
	constexpr int CLASS_IPL_OPL = 2;
	constexpr int CLASS_ONL = 3;
	constexpr int CLASS_RPE = 4;
	constexpr int CLASS_CHOROID = 5;
	constexpr int CLASS_SCLERA = 6;
	constexpr int CLASS_DISC_HEAD = 7;
	constexpr int NUM_LAYER_CLASSES = 8;

	constexpr int SAMPLE_WIDTH_S = 384;
	constexpr int SAMPLE_WIDTH_M = 512;
	constexpr int SAMPLE_HEIGHT_S = 384; // 768;
	constexpr int SAMPLE_HEIGHT_M = 768;

	constexpr int COARSE_WIDTH_S = 384;
	constexpr int COARSE_WIDTH_M = 512;
	constexpr int COARSE_HEIGHT_S = 384; // 768;
	constexpr int COARSE_HEIGHT_M = 768;

	constexpr int MODEL_INPUT_WIDTH_S = 384;
	constexpr int MODEL_INPUT_WIDTH_M = 512;
	constexpr int MODEL_INPUT_HEIGHT_S = 384;
	constexpr int MODEL_INPUT_HEIGHT_M = 512;

	constexpr int COARSE_GUIDED_RADIUS_S = 3;
	constexpr int COARSE_GUIDED_RADIUS_M = 5;
	constexpr double COARSE_GUIDED_EPSILON_S = 0.05;
	constexpr double COARSE_GUIDED_EPSILON_M = 0.05;

	constexpr float COLUMN_OBJ_SN_RATIO_MIN = 18.0f;
	constexpr float COLUMN_VALID_RATIO_MIN = 0.45f;


	constexpr int MODEL_INPUT_WIDTH = 512;
	constexpr int MODEL_INPUT_HEIGHT = 768;
	constexpr int COARSE_IMAGE_WIDTH = 512;
	constexpr int COARSE_IMAGE_HEIGHT = 768;
	constexpr int COARSE_GUIDED_RADIUS = 5; // 3;
	constexpr double COARSE_GUIDED_EPSILON = 0.05;

	constexpr float HORIZ_BOUND_OBJ_SNR_MIN = 18.0f; // 24.0f;
	constexpr int HORIZ_BOUND_SMOOTH_SIZE = 5;  // 7;
	constexpr int INNER_BOUND_UPPER_MARGIN = 32;
	constexpr int INNER_BOUND_SMOOTH_SIZE = 21; // 31
	constexpr int OUTER_BOUND_LOWER_MARGIN = 12;
	constexpr int OUTER_BOUND_SMOOTH_SIZE = 35; // 51;

	constexpr int OPTIC_NERVE_HEAD_WIDTH_MIN = 18; // 24 // 32
	constexpr int OPTIC_NERVE_PERI_WIDTH_MIN = 8;
	constexpr int OPTIC_NERVE_HEAD_DEPTH_MIN = 24;
	constexpr float OPTIC_NERVE_HEAD_RATIO_MIN = 2.0f;

	constexpr int ILM_EDGE_KERNEL_ROWS = 15;
	constexpr int ILM_EDGE_KERNEL_COLS = 5;
	constexpr int ILM_PATH_THRESH_MIN = 45;
	constexpr int ILM_PATH_LOWER_RANGE_INIT = 24; // 12;
	constexpr int ILM_PATH_LOWER_RANGE_DISC = 24;
	constexpr int ILM_PATH_DELTA_INIT = 9;
	constexpr int ILM_PATH_DELTA_DISC = 15;
	constexpr int ILM_PATH_SMOOTH_SIZE = 7;
	constexpr int ILM_LINE_SMOOTH_SIZE = 5;
}


#ifdef __SEMTRETINA_DLL
#define SEMTRETINA_DLL_API		__declspec(dllexport)
#else
#define SEMTRETINA_DLL_API		__declspec(dllimport)
#endif