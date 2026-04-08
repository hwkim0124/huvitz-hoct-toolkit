#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


#include <string>

namespace RetFocus
{
	constexpr bool DEBUG_OUT = false;

	// Split Focus Detection 
	////////////////////////////////////////////////////////////////////////////////////////
	constexpr int CAMERA_IMAGE_WIDTH = 640; // 644;
	constexpr int CAMERA_IMAGE_HEIGHT = 480;
	constexpr int CAMERA_IMAGE_CENTER_X = (CAMERA_IMAGE_WIDTH / 2);
	constexpr int CAMERA_IMAGE_CENTER_Y = (CAMERA_IMAGE_HEIGHT / 2);

	constexpr int SPLIT_KERNEL_WIDTH = 9; // 13; // 19; // 5 // 23;// 24;
	constexpr int SPLIT_KERNEL_HEIGHT = 17; // 21; // 23; // 23;
	constexpr int SPLIT_FOCUS_WIDTH = SPLIT_KERNEL_WIDTH; // 35;
	constexpr int SPLIT_FOCUS_HEIGHT = SPLIT_KERNEL_HEIGHT; // 25;
	constexpr int SPLIT_KERNEL_OFFSET = 3;

	constexpr int SPLIT_CENTER_X = CAMERA_IMAGE_CENTER_X;
	constexpr int SPLIT_CENTER_X_MIN = CAMERA_IMAGE_CENTER_X - 75; // 35;
	constexpr int SPLIT_CENTER_X_MAX = CAMERA_IMAGE_CENTER_X + 75; // 35;
	constexpr int SPLIT_CENTER_Y = 122;
	constexpr int SPLIT_CENTER_Y_MIN = CAMERA_IMAGE_CENTER_Y - 150;
	constexpr int SPLIT_CENTER_Y_MAX = CAMERA_IMAGE_CENTER_Y + 150; // -90;

	constexpr int SPLIT_CONVS_RANGE = 90; // 70;
	constexpr int SPLIT_CONVS_X_STEP = 1; // 1;
	constexpr int SPLIT_CONVS_Y_STEP = 1; // 1;

	constexpr int SPLIT_PEAK_MIN = 55; // 45; // 75;
	constexpr int SPLIT_PEAK_MAX = 190; // 210;
	constexpr int SPLIT_FWHM_MIN = 5;
	constexpr int SPLIT_FWHM_MAX = 50; // 25;
	constexpr float SPLIT_SNR_MIN = 1.5f;

	constexpr float SPLIT_CENTER_OFFSET_DIFF_MAX = 25.0f;


	// Auto Split Focus
	/////////////////////////////////////////////////////////////////////////////////////////
	constexpr int AUTO_SPLIT_FOCUS_EMPTY_ERROR_MAX = 3; // 5; // 3;
	constexpr int AUTO_SPLIT_FOCUS_CLOSE_ERROR_MAX = 4; // 5; // 15; // 10; // 3;
	constexpr int AUTO_SPLIT_FOCUS_TIMES_MAX = 40; // 30; // 50; // 150;

	constexpr float AUTO_SPLIT_DIOPT_RANGE_TO_TARGET = 0.25f; // 0.50f;// 0.75f; // 1.0f; // 0.25f;
	constexpr float AUTO_SPLIT_OFFSET_PER_DIOPT = 1.0f; // 1.5f; // 2.5f;
	constexpr float AUTO_SPLIT_MOVE_STEP_IN_DIOPT = 0.25f;
	constexpr float AUTO_SPLIT_OFFSET_INVALID = 999.0f;
	constexpr float AUTO_SPLIT_OFFSET_CLOSED = 0.5f;
	constexpr float AUTO_SPLIT_DIOPT_DISTANT = 3.5f;

	constexpr int AUTO_SPLIT_FOCUS_DELAY_TO_ALIGN = 2;


	// Retina Feature 
	/////////////////////////////////////////////////////////////////////////////////////////
	constexpr int RETINA_FEAT_MEAN_MIN = 25; // 45; // 60;
	constexpr int RETINA_FEAT_STDEV_MIN = 5; // 25;
	constexpr int RETINA_FEAT_THRESHOLD_MIN = 35; // 45; // 75; // 140;
	constexpr int RETINA_FEAT_THRESHOLD_MAX = 240; // 230; // 180; // 140;
	constexpr float RETINA_FEAT_THRESHOLD_RATIO = 2.0f;

	constexpr int RETINA_FEAT_KERNEL_SIZE = 13; // 15;
	constexpr int RETINA_FEAT_SCAN_RADIUS = 200; // 210;

	constexpr int RETINA_FEAT_SCAN_SIZE_MIN = 600; // 250;
	constexpr int RETINA_FEAT_SCAN_SIZE_MAX = 4900;; // 9000;
	constexpr float RETINA_FEAT_SCAN_RATIO_MIN = 0.60f;
	constexpr float RETINA_FEAT_SCAN_ASPECT_MIN = 0.5f;
	constexpr float RETINA_FEAT_SCAN_ASPECT_MAX = 2.5f; // 5.0f;

	constexpr float RETINA_FEAT_SCAN_REGION_X1 = 0.25f;
	constexpr float RETINA_FEAT_SCAN_REGION_X2 = 0.75f;
	constexpr float RETINA_FEAT_SCAN_REGION_Y1 = 0.25f;
	constexpr float RETINA_FEAT_SCAN_REGION_Y2 = 0.75f;
	constexpr int RETINA_FEAT_SCAN_OFFSET_Y = 4;
	constexpr int RETINA_FEAT_SCAN_OFFSET_X = 1;
	constexpr int RETINA_FEAT_WIND_EXTENT = 25;
	constexpr int RETINA_FEAT_WIND_OFFSET = 3;

	constexpr int RETINA_FEAT_DISC_WIDTH_MIN = 15;
	constexpr int RETINA_FEAT_DISC_WIDTH_MAX = 95;
	constexpr int RETINA_FEAT_DISC_HEIGHT_MIN = 15;
	constexpr int RETINA_FEAT_DISC_HEIGHT_MAX = 95;
	constexpr float RETINA_FEAT_DISC_PIXELS_RATIO_MIN = 0.70f;


	// Auto Measure 
	///////////////////////////////////////////////////////////////////////////////////////
	constexpr int AUTO_LENS_FRONT_REFER_ADD = (-4500);
	constexpr float AUTO_LENS_FRONT_DIOPT_ADD = -7.0f;
	constexpr float AUTO_LENS_FRONT_DIOPT_MAX = AUTO_LENS_FRONT_DIOPT_ADD;
	constexpr int AUTO_LENS_BACK_REFER_ADD = (-4000);
	constexpr float AUTO_LENS_BACK_DIOPT_ADD = -7.0f;
	constexpr float AUTO_LENS_BACK_DIOPT_MAX = (AUTO_LENS_FRONT_DIOPT_MAX + AUTO_LENS_BACK_DIOPT_ADD);


	// Scan Optimizer 
	///////////////////////////////////////////////////////////////////////////////////////
	constexpr int OBTAIN_NEXT_PREVIEW_WAIT_DELAY = 5;
	constexpr int OBTAIN_NEXT_PREVIEW_RETRY_MAX = 100;
	constexpr int OBTAIN_NEXT_SPLIT_FOCUS_WAIT_DELAY = 5;
	constexpr int OBTAIN_NEXT_SPLIT_FOCUS_RETRAY_MAX = 100;

	constexpr float SCAN_AUTO_QINDEX_TO_SIGNAL = 1.5f;
	constexpr float SCAN_AUTO_QINDEX_TO_TARGET = 3.5f;
	constexpr float SCAN_AUTO_QINDEX_TO_TARGET_LENS_BACK = 1.5f;
	constexpr float SCAN_AUTO_QINDEX_TO_CONFIRM = 5.5f;
	constexpr float SCAN_AUTO_QINDEX_TO_COMPLETE = 9.5f;


	// Auto Reference 
	///////////////////////////////////////////////////////////////////////////////////////
	enum class AutoReferPhase {
		INIT = 0,
		NO_SIGNAL,
		NO_SIGNAL_REVERSE,
		CAPTURE,
		FORWARD,
		RETRACE,
		CENTER,
		COMPLETE,
		CANCELED
	};

	constexpr int AUTO_REFER_MOVE_STEP_SEARCH = 450; // 900;
	constexpr int AUTO_REFER_MOVE_STEP_SEARCH_FAST = 1200;
	constexpr int AUTO_REFER_MOVE_STEP_ALIGN = 150;
	constexpr int AUTO_REFER_MOVE_STEP_CENTER = 50; // 75;

	constexpr int AUTO_REFER_MOVE_DISTANCE_CAPTURE = 2500;
	constexpr float AUTO_REFER_MOVE_RATIO_TO_OFFSET = 3.5f;

	constexpr int AUTO_REFER_TO_FORWARD = 1;
	constexpr int AUTO_REFER_TO_BACKWARD = -1;

	constexpr int AUTO_REFER_MOMENTS_FORWARD = 5; // 3;
	constexpr int AUTO_REFER_MOMENTS_RETRACE = 5; // 3;
	constexpr int AUTO_REFER_RETRY_MAX_FORWARD = 9; // 5;
	constexpr int AUTO_REFER_RETRY_MAX_RETRACE = 9; // 5;
	constexpr int AUTO_REFER_RETRY_MAX_CENTER = 9; //  5;
	constexpr int AUTO_REFER_RETRY_MAX_CAPTURE = 5; // 3;// 5;
	constexpr int AUTO_REFER_CENTER_TRACK_MAX = 15; // 7;

	constexpr int AUTO_REFER_CENTER_LINE = 256;
	constexpr int AUTO_REFER_CENTER_OFFSET_MIN = -50;
	constexpr int AUTO_REFER_CENTER_OFFSET_MAX = 75;
	constexpr int AUTO_REFER_CENTER_LINE_SHIFT = (AUTO_REFER_CENTER_LINE + (AUTO_REFER_CENTER_OFFSET_MIN + AUTO_REFER_CENTER_OFFSET_MAX) / 2);
	constexpr int AUTO_REFER_VALID_POINT_MIN = (AUTO_REFER_CENTER_LINE - 100);
	constexpr int AUTO_REFER_VALID_POINT_MAX = (AUTO_REFER_CENTER_LINE + 150);
	constexpr int AUTO_REFER_CENTER_UPPER_LINE = 128; // 192; // 128;

	// Auto Focus 
	///////////////////////////////////////////////////////////////////////////////////////
	enum class AutoFocusPhase {
		INIT = 0,
		NO_SIGNAL,
		NO_SIGNAL_REVERSE,
		FIRST_GUESS,
		SEARCH,
		SEARCH_REVERSE,
		FORWARD,
		RETRACE,
		FORWARD2,
		RETRACE2,
		COMPLETE,
		CANCELED
	};

	constexpr int AUTO_FOCUS_TO_MINUS_DIOPT = -1;
	constexpr int AUTO_FOCUS_TO_PLUS_DIOPT = +1;

	constexpr float AUTO_FOCUS_PLUS_DIOPT_END = +30.0f;
	constexpr float AUTO_FOCUS_MINUS_DIOPT_END = -30.0f;
	constexpr float AUTO_FOCUS_PLUS_DIOPT_END_ANTERIOR = +15.0f;
	constexpr float AUTO_FOCUS_MINUS_DIOPT_END_ANTERIOR = -15.0f;

	constexpr float AUTO_FOCUS_MOVE_STEP_SEARCH = 3.0f; // 2.5f;
	constexpr float AUTO_FOCUS_MOVE_STEP_ALIGN = 1.5f; // 0.5f;
	constexpr float AUTO_FOCUS_MOVE_STEP_CONFIRM = 0.25f; // 0.5f;
	constexpr float AUTO_FOCUS_MOVE_STEP_GUESS = 1.5f;

	constexpr int AUTO_FOCUS_RETRY_MAX_FORWARD = 9; // 5;
	constexpr int AUTO_FOCUS_RETRY_MAX_RETRACE = 9; // 5;
	constexpr int AUTO_FOCUS_RETRY_MAX_FIRST_GUESS = 9; // 5; // 3;

	constexpr int AUTO_FOCUS_WAIT_TARGET_SLOWER = 5;
	constexpr int AUTO_FOCUS_WAIT_TARGET_NORMAL = 7;
	constexpr int AUTO_FOCUS_WAIT_TARGET_FASTER = 15;

	constexpr int AUTO_FOCUS_MOMENTS_FORWARD = 5; // 3;
	constexpr int AUTO_FOCUS_MOMENTS_RETRACE = 5; // 3;
	constexpr int AUTO_FOCUS_RESISTS_FORWARD = 3; // 7; // 3;
	constexpr int AUTO_FOCUS_RESISTS_RETRACE = 3; // 7; // 3;
	constexpr int AUTO_FOCUS_RESISTS_FORWARD2 = 5; // 1; // 3;
	constexpr int AUTO_FOCUS_RESISTS_RETRACE2 = 5; // 1; // 3;

	constexpr int AUTO_FOCUS_FORWARDS_LIMIT2 = 7;
	constexpr int AUTO_FOCUS_RETRACES_LIMIT2 = 7;


	// Auto Polarization 
	///////////////////////////////////////////////////////////////////////////////////////
	enum class AutoPolarPhase {
		INIT = 0,
		NO_SIGNAL,
		NO_SIGNAL_REVERSE,
		FIRST_GUESS,
		FORWARD,
		RETRACE,
		FORWARD2,
		RETRACE2,
		COMPLETE,
		CANCELED
	};

	constexpr int AUTO_POLAR_TO_MINUS_DEGREE = -1;
	constexpr int AUTO_POLAR_TO_PLUS_DEGREE = +1;

	constexpr float AUTO_POLAR_MOVE_STEP_SEARCH = 30.0f;
	constexpr float AUTO_POLAR_MOVE_STEP_ALIGN = 15.0f; // 5.0f;
	constexpr float AUTO_POLAR_MOVE_STEP_CONFIRM = 5.0f; // 5.0f;
	constexpr float AUTO_POLAR_MOVE_STEP_GUESS = 15.0f;

	constexpr int AUTO_POLAR_RETRY_MAX_FORWARD = 9;  // 5; // 5;
	constexpr int AUTO_POLAR_RETRY_MAX_RETRACE = 9;  // 5; // 5;
	constexpr int AUTO_POLAR_RETRY_MAX_FIRST_GUESS = 9; // 5; // 5; // 3;

	constexpr int AUTO_POLAR_WAIT_TARGET_SLOWER = 5;
	constexpr int AUTO_POLAR_WAIT_TARGET_NORMAL = 7;
	constexpr int AUTO_POLAR_WAIT_TARGET_FASTER = 15;

	constexpr int AUTO_POLAR_MOMENTS_FORWARD = 5; // 3;
	constexpr int AUTO_POLAR_MOMENTS_RETRACE = 5; // 3;
	constexpr int AUTO_POLAR_RESISTS_FORWARD = 3; // 7; // 3;
	constexpr int AUTO_POLAR_RESISTS_RETRACE = 3; // 7; // 3;
	constexpr int AUTO_POLAR_RESISTS_FORWARD2 = 3; // 1; // 7; // 3;
	constexpr int AUTO_POLAR_RESISTS_RETRACE2 = 3; // 1; // 7; // 3;

	constexpr int AUTO_POLAR_FORWARDS_LIMIT2 = 7;
	constexpr int AUTO_POLAR_RETRACES_LIMIT2 = 7;
};


#ifdef __RETFOCUS_DLL
#define RETFOCUS_DLL_API		__declspec(dllexport)
#else
#define RETFOCUS_DLL_API		__declspec(dllimport)
#endif
