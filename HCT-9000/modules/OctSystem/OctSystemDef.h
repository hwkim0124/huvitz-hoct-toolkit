#pragma once

#include "OctGlobal2.h"
#include "DiscTestConfig.h"

using namespace OctGlobal;


#include <functional>


namespace OctSystem {

	constexpr char TOOLKIT_SW_VERSION[] = "1.3.9"; // "0.7.3";


	// Retina Tracking
	/////////////////////////////////////////////////////////////////////////////////////
	constexpr int RETINA_TRACK_FRAME_DELAY_TIME = 50;
	constexpr int RETINA_TRACK_FRAME_COUNT_TO_CANCEL = 300;
	constexpr int RETINA_TRACK_FRAME_COUNT_TO_RESET = 150;
	constexpr int RETINA_TRACK_RESET_COUNT_TO_CANCEL = 3;

	constexpr int RETINA_TARCK_TARGET_NEED_COUNT = 2;


	// Cornea Tracking
	/////////////////////////////////////////////////////////////////////////////////////
	constexpr float CORNEA_TRACK_STEREO_ZDIST_OFFSET = +3.8f;


	// Scanner 
	/////////////////////////////////////////////////////////////////////////////////////
	constexpr int SCANNER_GRAB_ERROR_COUNT_DELAY = 5;
	constexpr int SCANNER_GRAB_ERROR_COUNT_LIMIT = (SCANNER_GRAB_ERROR_COUNT_DELAY * 5);
	constexpr int SCANNER_GRAB_ERROR_DELAY_TIME = 50;

	constexpr int SCANNER_WAIT_PREVIEW_PROCESSED_COUNT_MAX = 500;
	constexpr int SCANNER_WAIT_PREVIEW_PROCESSED_DELAY = 20;
	constexpr int SCANNER_WAIT_MEASURE_PROCESSED_COUNT_MAX = 1000; // 500;
	constexpr int SCANNER_WAIT_MEASURE_PROCESSED_DELAY = 25; // 20;

	// Split Focus Alignment 
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int RETINA_AUTO_FOCUS_RETRY_MAX = 10;
	constexpr int RETINA_AUTO_FOCUS_ERROR_MAX = 3;
	constexpr float RETINA_AUTO_FOCUS_DIST_PER_DIOPT = 2.5f; // 10.0f;
	constexpr float RETINA_AUTO_FOCUS_ON_RANGE_IN_DIOPT = 0.25f;
	constexpr float RETINA_AUTO_FOCUS_DIOPT_STEP = 0.25f;

	// Scan Optimizer
	/////////////////////////////////////////////////////////////////////////////////////
	constexpr float AUTO_FOCUS_QINDEX_TO_COMPLETE = 9.5f;
	constexpr float AUTO_FOCUS_QINDEX_TO_SIGNAL = 1.0f; // 2.0f; // 3.5f;
	constexpr float AUTO_FOCUS_QINDEX_DIFF_TO_RETRACE = 0.15f;

	constexpr float AutoFocusQualitySteps[7] = {
		9.5f, 8.5f, 7.5f, 6.5f, 5.5f, 4.5f, 3.5f 
	};

	constexpr float AutoFocusDioptSteps[7] = {
		0.25f, 0.5f, 0.75f, 1.0f, 1.5f, 2.5f, 3.5f
	};
	constexpr float AUTO_FOCUS_DIOPT_STEP_MIN = AutoFocusDioptSteps[0];
	constexpr float AUTO_FOCUS_DIOPT_STEP_MAX = AutoFocusDioptSteps[6];
	constexpr float AUTO_FOCUS_DIOPT_STEP_FIRST = AutoFocusDioptSteps[3];
	constexpr int AUTO_FOCUS_DIOPT_STEP_SIZE = 7;

	constexpr int AUTO_FOCUS_PLUS_DIRECTION = +1;
	constexpr int AUTO_FOCUS_MINUS_DIRECTION = -1;


	// Auto Scan Measure
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int AUTO_SCAN_FOCUS_DELAY_COUNT = 9;
	constexpr int AUTO_SCAN_FOCUS_DELAY_TIME = 50;


	// Auto Reference
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int AutoReferPointSteps[7] = {
		// 50, 100, 150, 200, 250
		// 10, 20, 30, 40, 50
		// 10, 25, 50, 75, 100, 150, 200
		10, 25, 50, 75, 100, 150, 200
	};

	constexpr int AutoReferMoveSteps[7] = {
		// 25, 50, 150, 250, 500
		// 100, 200, 300, 400, 500
		// 25, 50, 100, 150, 200, 300, 500
		25, 50, 75, 100, 150, 300, 500
	};

	constexpr int AUTO_REFER_MOVE_STEP_MIN = AutoReferMoveSteps[0];
	constexpr int AUTO_REFER_MOVE_STEP_MAX = AutoReferMoveSteps[6];
	constexpr int AUTO_REFER_MOVE_STEP_SIZE = 7;
	constexpr int AUTO_REFER_MOVE_STEP_FIND = AUTO_REFER_MOVE_STEP_MAX * 3;

	constexpr int AUTO_REFER_CENTER_POINT = 256;
	constexpr int AUTO_REFER_CENTER_OFFSET = 8; // 5;

	constexpr int AUTO_REFER_UPPER_DIRECTION = +1;
	constexpr int AUTO_REFER_LOWER_DIRECTION = -1;


	// Auto Leveling
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr int AutoLevelPointSteps[5] = {
		50, 100, 150, 200, 250
	};

	constexpr int AutoLevelMoveSteps[5] = {
		25, 50, 150, 250, 500
	};

	constexpr int AUTO_LEVEL_MOVE_STEP_MIN = AutoLevelMoveSteps[0];
	constexpr int AUTO_LEVEL_MOVE_STEP_MAX = (AutoLevelMoveSteps[4] * 2);
	constexpr int AUTO_LEVEL_MOVE_STEP_SIZE = 5;

	constexpr int AUTO_LEVEL_CENTER_POINT = 256;
	constexpr int AUTO_LEVEL_CENTER_OFFSET = 5;

	constexpr float AUTO_POLAR_QINDEX_DIFF_TO_RETRACE = 0.05f;

	constexpr float AUTO_POLAR_MOVE_STEP_MAX = 30;
	constexpr float AUTO_POLAR_MOVE_STEP_FIRST = 15;
	constexpr float AUTO_POLAR_MOVE_STEP_MIN = 5;

	constexpr int AUTO_POLAR_UPPER_DIRECTION = +1;
	constexpr int AUTO_POLAR_LOWER_DIRECTION = -1;



	enum class AutoFocusPhase {
		INIT = 0, 
		NO_SIGNAL, 
		NO_SIGNAL_REVERSE,
		FIRST_STEP,
		FORWARD, 
		RETRACE, 
		COMPLETE, 
		CANCELED
	};

	enum class AutoFocusDirection {
		MINUS = 0,
		PLUS
	};

	enum class AutoReferPhase {
		INIT = 0, 
		NO_SIGNAL, 
		NO_SIGNAL_REVERSE, 
		FORWARD, 
		RETRACE, 
		CENTER,
		COMPLETE, 
		CANCELED
	};

	enum class AutoLevelPhase {
		INIT = 0,
		NO_SIGNAL,
		NO_SIGNAL_REVERSE,
		FORWARD,
		RETRACE,
		COMPLETE,
		CANCELED
	};

	enum class AutoPolarPhase {
		INIT = 0, 
		NO_SIGNAL, 
		NO_SIGNAL_REVERSE, 
		FIRST_STEP, 
		FORWARD, 
		RETRACE, 
		COMPLETE, 
		CANCELED
	};

	enum class CorneaAlignTarget {
		CENTER = 0, 
		CENTER_LEFT, 
		CENTER_RIGHT
	};
}


#ifdef __OCTSYSTEM_DLL
#define OCTSYSTEM_DLL_API		__declspec(dllexport)
#else
#define OCTSYSTEM_DLL_API		__declspec(dllimport)
#endif