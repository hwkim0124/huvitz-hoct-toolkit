#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace SigProc {

	// FBG Specification
	////////////////////////////////////////////////////////////////////////////////////////
	constexpr const char* FBG_UNKNOWN_NAME = "Unknown";
	constexpr const char* FBG_CLASS1_NAME = "3054-1";
	constexpr const char* FBG_CLASS2_NAME = "3054-2";
	constexpr const char* FBG_CLASS3_NAME = "3054-3";
	constexpr const char* FBG_CLASS4_NAME = "4160-1";
	constexpr const char* FBG_CLASS5_NAME = "4160-2";

	enum {
		FBG_CLASS1 = 0, FBG_CLASS2, FBG_CLASS3, FBG_CLASS4, FBG_CLASS5, NUMBER_OF_FBG_CLASSES = 5
	};

	constexpr int FBG_PEAK_WINDOW_SIZE = 50;
	constexpr int FBG_PEAK_THRESHOLD_MAX = (LINE_CAMERA_PIXEL_VALUE_MAX - 1);
	constexpr int FBG_PEAK_THRESHOLD_INIT = ((int)(FBG_PEAK_THRESHOLD_MAX*0.90f));

	constexpr float WAVELENGTH_CENTER_OF_SOURCE = 840.0f;
	constexpr int WAVELENGTH_FUNCTION_DEGREE = 3;
	constexpr int WAVELENGTH_FUNCTION_COEFFS = (WAVELENGTH_FUNCTION_DEGREE + 1);


	// Resampling Parameters 
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int RESAMPLE_DETECTOR_PIXELS = LINE_CAMERA_CCD_PIXELS;
	constexpr int RESAMPLE_ZERO_PADDING_SCALE = 1;
	constexpr int RESAMPLE_PADDED_PIXELS = (RESAMPLE_DETECTOR_PIXELS * RESAMPLE_ZERO_PADDING_SCALE);
	constexpr int RESAMPLE_WAVELENGTH_FUNCTION_DEGREE = 4;

	constexpr double RESAMPLE_WAVELENGTH_PARAM_1 = 800.752136;
	constexpr double RESAMPLE_WAVELENGTH_PARAM_2 = 0.047172;
	constexpr double RESAMPLE_WAVELENGTH_PARAM_3 = -0.000003;
	constexpr double RESAMPLE_WAVELENGTH_PARAM_4 = 0.000000;


	constexpr int RESAMPLE_KVALUE_START_INDEX = 1;
	constexpr int RESAMPLE_KVALUE_END_INDEX = (RESAMPLE_PADDED_PIXELS - 2);


	// Dispersion Compensation
	/////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int PHASE_SHIFT_PARAMETERS = 2;

	constexpr int PHASE_SHIFT_PARAMETER_SIZE = 3;
	constexpr double PHASE_SHIFT_PARAM1_SCALER = 1000000000.0;		// pow(10, 9)
	constexpr double PHASE_SHIFT_PARAM2_SCALER = 10000000000000.0;	// pow(10, 13)
	constexpr double PHASE_SHIFT_PARAM3_SCALER = 100000000000000000.0; // pow(10, 17)


	// Angiogram
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float ANGIO_DECOR_THRESHOLD_STDDEV_MULT = 0.5f; // 1.0f;
	constexpr int ANGIO_MIN_FRAMES_TO_CIRCULAR_DECOR = 3;
	constexpr int ANGIO_DECOR_CALC_Y_UPPER_MARGIN = 10;
	constexpr int ANGIO_DECOR_CALC_Y_LOWER_MARGIN = 10;
	constexpr float ANGIO_MOTION_LINE_THRESHOLD_MIN = 0.0f;
	constexpr float ANGIO_MOTION_LINE_THRESHOLD_MAX = 1.0f;
	constexpr float ANGIO_MOTION_LINE_THRESHOLD_TO_MEAN = 1.5f;
	constexpr float ANGIO_MOTION_LINE_OVER_POINTS_RATIO = 0.5f;
	constexpr int ANGIO_MOTION_FREE_STRIPE_HEIGHT_MIN = 5;
	constexpr int ANGIO_MOTION_CORRECT_REGION_MARGIN = 25;
	constexpr int ANGIO_MOTION_CORRECT_SHIFT_START = -20;
	constexpr int ANGIO_MOTION_CORRECT_SHIFT_CLOSE = +20;
	constexpr float ANGIO_MOTION_CORRECT_RATIO_MIN = 1.15f;
}


#ifdef __SIGPROC_DLL
#define SIGPROC_DLL_API		__declspec(dllexport)
#else
#define SIGPROC_DLL_API		__declspec(dllimport)
#endif