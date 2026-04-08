#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


#include <string>

namespace KerFocus
{
	constexpr bool DEBUG_OUT = false;

	// KeratoImage
	///////////////////////////////////////////////////////////////////////////////////////
	constexpr int CAMERA_IMAGE_WIDTH = 640; // 644;
	constexpr int CAMERA_IMAGE_HEIGHT = 480;
	constexpr int CAMERA_IMAGE_CENTER_X = (CAMERA_IMAGE_WIDTH / 2);
	constexpr int CAMERA_IMAGE_CENTER_Y = (CAMERA_IMAGE_HEIGHT / 2);

	constexpr float SAMPLE_IMAGE_SIZE_RATIO = (1.0f);
	constexpr int SAMPLE_IMAGE_WIDTH = (int)(CAMERA_IMAGE_WIDTH*SAMPLE_IMAGE_SIZE_RATIO);
	constexpr int SAMPLE_IMAGE_HEIGHT = (int)(CAMERA_IMAGE_HEIGHT*SAMPLE_IMAGE_SIZE_RATIO);
	constexpr int SAMPLE_IMAGE_CENTER_X = (SAMPLE_IMAGE_WIDTH / 2);
	constexpr int SAMPLE_IMAGE_CENTER_Y = (SAMPLE_IMAGE_HEIGHT / 2);

	constexpr int MARGIN_TOP = 20;
	constexpr int MARGIN_LEFT = 20;
	constexpr int MARGIN_RIGHT = 20;
	constexpr int MARGIN_BOTTOM = 20;

	constexpr int ROW_START = MARGIN_TOP;
	constexpr int ROW_END = (SAMPLE_IMAGE_HEIGHT - MARGIN_BOTTOM);
	constexpr int COL_START = MARGIN_LEFT;
	constexpr int COL_END = (SAMPLE_IMAGE_WIDTH - MARGIN_RIGHT);

	constexpr int ROW_STEP_HIST = 4;
	constexpr int COL_STEP_HIST = 4;
	constexpr int HISTOGRAM_BINS = 256;

	constexpr float PUPIL_RATIO = (0.0625f / 2.5f);// 0.15f;
	constexpr float SPOTS_RATIO = 0.001f;

	constexpr int PUPIL_THRESHOLD_MIN = 10;
	constexpr int PUPIL_THRESHOLD_MAX = 25; // 55; // 90;
	constexpr int SPOTS_THRESHOLD_MIN = 160;
	constexpr int SPOTS_THRESHOLD_MAX = 210;// 230;

	constexpr int MODEL_EYE_PUPIL_SIZE_MIN = 400;
	constexpr int MODEL_EYE_PUPIL_THRESHOLD_MAX = 35;

	constexpr int ROW_STEP_SPOT = 2;
	constexpr int COL_STEP_SPOT = 1; // 2;

	constexpr int SPOT_WIDTH_MIN = 3;
	constexpr int SPOT_WIDTH_MAX = 30;
	constexpr int SPOT_HEIGHT_MIN = 3;
	constexpr int SPOT_HEIGHT_MAX = 30;
	constexpr int SPOT_WIDTH_STD = (SPOT_WIDTH_MAX - SPOT_WIDTH_MIN) / 2;
	constexpr int SPOT_HEIGHT_STD = (SPOT_HEIGHT_MAX - SPOT_HEIGHT_MIN) / 2;
	constexpr float SPOT_SIZE_RATIO_MAX = 1.5f;
	constexpr int NUM_KER_SPOTS_MIN = 3;

	constexpr int PUPIL_SPOT_DISTANCE_MAX = (SAMPLE_IMAGE_WIDTH / 2);
	constexpr int HORZ_PAIRS_DISTANCE_MAX = (SAMPLE_IMAGE_WIDTH / 2);
	constexpr int HORZ_PAIRS_DISTANCE_MIN = (40);
	constexpr int HORZ_PAIRS_TEST_SIZE = 7;
	constexpr int HORZ_PAIRS_COUNT_MAX = 3;

	constexpr int VERT_PAIRS_DISTANCE_MIN = (30);

	constexpr int NUM_MIRE_SPOTS_CENTER = 2;
	constexpr int NUM_MIRE_SPOTS_LEFT = 3;
	constexpr int NUM_MIRE_SPOTS_RIGHT = 3;
	constexpr int NUM_MIRE_SPOTS = (NUM_MIRE_SPOTS_CENTER + NUM_MIRE_SPOTS_LEFT + NUM_MIRE_SPOTS_RIGHT);
	constexpr int NUM_FOCUS_SPOTS = 2;

	constexpr int ELLIPSE_SPOTS_MIN = 5;

	// KeratoImage2 
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int DENOISE_FILTER_SIZE = ((int)(20 * SAMPLE_IMAGE_SIZE_RATIO));
	
	constexpr float PUPIL_THRESHOLD_TO_MEAN_RATIO = 0.5f;

	constexpr int SCAN_SPOT_BACK_SIZE = ((int)(16 * SAMPLE_IMAGE_SIZE_RATIO));
	constexpr int SCAN_SPOT_BACK_DISTANCE = SCAN_SPOT_BACK_SIZE;
	constexpr int SCAN_SPOT_HORZ_SIZE_MIN = ((int)(4 * SAMPLE_IMAGE_SIZE_RATIO));
	constexpr int SCAN_SPOT_HORZ_SIZE_MAX = ((int)(60 * SAMPLE_IMAGE_SIZE_RATIO));
	constexpr int SCAN_SPOT_THRESHOLD = 180;
	constexpr int SCAN_SPOT_RECT_OFFSET = 3;
	
	constexpr int KER_SPOT_THRESHOLD = ((int)(SCAN_SPOT_THRESHOLD*0.75));
	constexpr int KER_SPOT_SIZE_MIN = 3;
	constexpr int KER_SPOT_SIZE_MAX = 360;
	constexpr float KER_SPOT_ECCENT_MAX = 3.0f;
	constexpr int KER_SPOTS_OFFSET = 15;

	constexpr int KER_MIRE_SPOTS_NUM = 6;
	constexpr int KER_FOCUS_SPOTS_NUM = 2;
	constexpr int KER_ALL_SPOTS_NUM = (KER_MIRE_SPOTS_NUM + KER_FOCUS_SPOTS_NUM);


	// KeratoImage
	///////////////////////////////////////////////////////////////////////////////////////
	constexpr float SAMPLE_WIDTH_RATIO = (1.0f / 3.0f);
	constexpr float SAMPLE_HEIGHT_RATIO = (1.0f / 3.0f);

	constexpr int SAMPLE_WIDTH = ((int)(CAMERA_IMAGE_WIDTH*SAMPLE_WIDTH_RATIO));
	constexpr int SAMPLE_HEIGHT = ((int)(CAMERA_IMAGE_HEIGHT*SAMPLE_HEIGHT_RATIO));
	constexpr int NOISE_FILTER_WIDTH = ((int)(15* SAMPLE_WIDTH_RATIO));
	constexpr int NOISE_FILTER_HEIGHT = ((int)(15* SAMPLE_HEIGHT_RATIO));

	constexpr int OUTER_MASK_CENTER_X = ((int)(310 * SAMPLE_WIDTH_RATIO));
	constexpr int OUTER_MASK_CENTER_Y = ((int)(240 * SAMPLE_HEIGHT_RATIO));
	constexpr int OUTER_MASK_RADIUS = ((int)(240 * SAMPLE_WIDTH_RATIO));
	constexpr int INNER_MASK_RADIUS = ((int)(160 * SAMPLE_WIDTH_RATIO));
	constexpr int INNER_SCAN_RADIUS = ((int)(30 * SAMPLE_WIDTH_RATIO));
	constexpr int OUTER_SCAN_RADIUS = ((int)(160 * SAMPLE_WIDTH_RATIO));
	constexpr int SCAN_LINE_LENGTH = (OUTER_SCAN_RADIUS - INNER_SCAN_RADIUS);
	constexpr int SCAN_HALF_LENGTH = (SCAN_LINE_LENGTH / 2);

	constexpr int MEAN_TO_MODEL_EYE = 45; //  35;
	constexpr int THRESHOLD_TO_PUPIL = 55;
	constexpr int THRESHOLD_TO_MODEL_EYE = 128;
	constexpr int PUPIL_SIZE_MIN = ((int)(SAMPLE_WIDTH*SAMPLE_HEIGHT*0.12));	 

	constexpr int MIRE_SPOTS_NUM = 6;
	constexpr int FOCUS_SPOTS_NUM = 2;
	constexpr int KERATO_SPOTS_NUM = (MIRE_SPOTS_NUM + FOCUS_SPOTS_NUM);
	constexpr int MIRE_SPOTS_TO_ELLIPSE = 6;

	constexpr int SPOT_THRESHOLD = 160;		// after gaussian blurring. 
	constexpr int SPOT_WINDOW_WIDTH = ((int)(48*SAMPLE_WIDTH_RATIO));
	constexpr int SPOT_WINDOW_HEIGHT = ((int)(36*SAMPLE_HEIGHT_RATIO));
	
	constexpr int SPOT_SIZE_MIN = ((int)(9*9)); // ((int)(4 * 4 * SAMPLE_WIDTH_RATIO*SAMPLE_HEIGHT_RATIO));
	constexpr int SPOT_SIZE_MAX = ((int)(30*30)); // ((int)(36 * 36 * SAMPLE_WIDTH_RATIO*SAMPLE_HEIGHT_RATIO));
	constexpr int SAMPLE_SPOT_SIZE_MIN = ((int)(SPOT_SIZE_MIN * SAMPLE_WIDTH_RATIO * SAMPLE_HEIGHT_RATIO));
	constexpr int SAMPLE_SPOT_SIZE_MAX = ((int)(SPOT_SIZE_MAX * SAMPLE_WIDTH_RATIO * SAMPLE_HEIGHT_RATIO));


	// KeratoSetup
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr int KER_SETUP_DIOPTER_INDEX = (KER_SETUP_FOCUS_STEP_NUM / 2);
	constexpr float KER_SETUP_FOCUS_STEP_DISTANCE = 0.5;

};


#ifdef __KERFOCUS_DLL
#define KERFOCUS_DLL_API		__declspec(dllexport)
#else
#define KERFOCUS_DLL_API		__declspec(dllimport)
#endif