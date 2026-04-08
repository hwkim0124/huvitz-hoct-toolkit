#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace SigChain {

	constexpr bool DEBUG_OUT = false;


	// Frame Buffer configuration.
	///////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int FRAME_DATA_LINE_SIZE = LINE_CAMERA_CCD_PIXELS;
	constexpr int FRAME_DATA_LINES_MAX = BSCAN_LATERAL_SIZE_MAX;
	constexpr int FRAME_DATA_BUFFER_SIZE = (FRAME_DATA_LINE_SIZE * FRAME_DATA_LINES_MAX);

	constexpr int FRAME_DATA_PREVIEW_WIDTH = FRAME_DATA_LINE_SIZE;
	constexpr int FRAME_DATA_PREVIEW_HEIGHT = 512;
	constexpr int FRAME_DATA_PREVIEW_DEPTH = 1;

	constexpr int FRAME_DATA_ENFACE_WIDTH = FRAME_DATA_LINE_SIZE;
	constexpr int FRAME_DATA_ENFACE_HEIGHT = 256;
	constexpr int FRAME_DATA_ENFACE_DEPTH = 64;

	constexpr int FRAME_DATA_MEASURE_WIDTH = FRAME_DATA_LINE_SIZE;
	constexpr int FRAME_DATA_MEASURE_HEIGHT = 1024;
	constexpr int FRAME_DATA_MEASURE_DEPTH = 1;

	// constexpr int FRAME_BUFFER_LIST_SIZE_MAX = 256; // 128; // 64; // 16; // 10;


	// Default Task plans
	////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int TASK_PLAN_DATA_WIDTH_PREVIEW = LINE_CAMERA_CCD_PIXELS;
	constexpr int TASK_PLAN_DATA_HEIGHT_PREVIEW = PATTERN_PREVIEW_ASCAN_POINTS;

	constexpr int TASK_PLAN_DATA_WIDTH_MEASURE1 = LINE_CAMERA_CCD_PIXELS;
	constexpr int TASK_PLAN_DATA_HEIGHT_MEASURE1 = PATTERN_ASCAN_POINTS1;
	constexpr int TASK_PLAN_DATA_WIDTH_MEASURE2 = LINE_CAMERA_CCD_PIXELS;
	constexpr int TASK_PLAN_DATA_HEIGHT_MEASURE2 = PATTERN_ASCAN_POINTS2;
	constexpr int TASK_PLAN_DATA_WIDTH_MEASURE3 = LINE_CAMERA_CCD_PIXELS;
	constexpr int TASK_PLAN_DATA_HEIGHT_MEASURE3 = PATTERN_ASCAN_POINTS3;

	constexpr int TASK_PLAN_DATA_WIDTH_ENFACE = LINE_CAMERA_CCD_PIXELS;
	constexpr int TASK_PLAN_DATA_HEIGHT_ENFACE = PATTERN_ENFACE_ASCAN_POINTS;


	// FFT transform configuration.
	/////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float FFT_OUTPUT_REAL_VALUE_MIN = -2048.0f;
	constexpr float FFT_OUTPUT_REAL_VALUE_MAX = +2048.0f;
	constexpr float FFT_OUTPUT_IMAG_VALUE_MIN = -2048.0f;
	constexpr float FFT_OUTPUT_IMAG_VALUE_MAX = +2048.0f;

	constexpr int FFT_OUTPUT_LINE_SIZE_MAX = FRAME_DATA_LINES_MAX;
	constexpr int FFT_OUTPUT_LINES_NUM = (FRAME_DATA_LINE_SIZE / 2 + 1);
	constexpr int FFT_OUTPUT_BUFFER_SIZE = (FFT_OUTPUT_LINES_NUM * FFT_OUTPUT_LINE_SIZE_MAX);

	constexpr int FFT_IMAGE_ROW_SIZE_FOR_ENFACE = 768;
	constexpr int FFT_IMAGE_ROW_SIZE_FOR_PREVIEW = 768; // 512;
	constexpr int FFT_IMAGE_ROW_SIZE_FOR_MEASURE = 768;
	constexpr int FFT_IMAGE_ROW_SIZE = FFT_IMAGE_ROW_SIZE_FOR_MEASURE;
	constexpr int FFT_IMAGE_ROW_OFFSET = 24; // (FFT_OUTPUT_LINES_NUM - FFT_IMAGE_ROW_SIZE - 24);
	constexpr int FFT_IMAGE_ROW_OFFSET_FOR_ACA_LINE_PREVIEW = 128;// (FFT_IMAGE_ROW_SIZE_FOR_MEASURE + FFT_IMAGE_ROW_OFFSET - FFT_IMAGE_ROW_SIZE_FOR_PREVIEW);
	constexpr int FFT_IMAGE_ROW_START = FFT_IMAGE_ROW_OFFSET;

	constexpr int FFT_IMAGE_WIDTH_MAX = BSCAN_LATERAL_SIZE_MAX;
	constexpr int FFT_IMAGE_HEIGHT_MAX = FFT_IMAGE_ROW_SIZE_FOR_MEASURE;
	constexpr int FFT_IMAGE_BUFFER_SIZE = (FFT_IMAGE_WIDTH_MAX * FFT_IMAGE_HEIGHT_MAX);


	// Magnitude Histogram for adaptive gray scaling. 
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr int MAGNITUDE_SCAN_LINES_NUM = 64;
	constexpr int MAGNITUDE_HISTOGRAM_BINS = 8192; // 2048; 
	constexpr int MAGNITUDE_HISTOGRAM_TAGS = 4;
	constexpr int MAGNITUDE_HISTOGRAM_SIZE = (MAGNITUDE_HISTOGRAM_BINS + MAGNITUDE_HISTOGRAM_TAGS);	
	constexpr int MAGNITUDE_VALUE_DIVIDER = 20; // 40;
	constexpr float MAGNITUDE_LOG_SCALER = 10000.0f;

	constexpr int MAGNITUDE_BLOCK_ITEMS = 5; //  4;
	constexpr int MAGNITUDE_BLOCK_BUFFER_SIZE = (MAGNITUDE_BLOCK_ITEMS*MAGNITUDE_SCAN_LINES_NUM);

	constexpr int MAGNITUDE_EXCEPT_SIDE_RETINA = 8; // 12;
	constexpr int MAGNITUDE_START_LINES_RETINA = (MAGNITUDE_EXCEPT_SIDE_RETINA);
	constexpr int MAGNITUDE_CLOSE_LINES_RETINA = (MAGNITUDE_SCAN_LINES_NUM - MAGNITUDE_EXCEPT_SIDE_RETINA - 1);
	constexpr int MAGNITUDE_EXCEPT_SIDE_CORNEA = 8; // 12;
	constexpr int MAGNITUDE_START_LINES_CORNEA = (MAGNITUDE_EXCEPT_SIDE_CORNEA);
	constexpr int MAGNITUDE_CLOSE_LINES_CORNEA = (MAGNITUDE_SCAN_LINES_NUM - MAGNITUDE_EXCEPT_SIDE_CORNEA - 1);
	constexpr int MAGNITUDE_EXCEPT_HALF_CENTER = 4; //  8; // (3);
	constexpr int MAGNITUDE_START_CENTER_CORNEA = (MAGNITUDE_SCAN_LINES_NUM / 2 - MAGNITUDE_EXCEPT_HALF_CENTER);
	constexpr int MAGNITUDE_CLOSE_CENTER_CORNEA = (MAGNITUDE_SCAN_LINES_NUM / 2 + MAGNITUDE_EXCEPT_HALF_CENTER);
	constexpr int MAGNITUDE_START_CENTER_RETINA = (MAGNITUDE_SCAN_LINES_NUM / 2 - MAGNITUDE_EXCEPT_HALF_CENTER / 2);
	constexpr int MAGNITUDE_CLOSE_CENTER_RETINA = (MAGNITUDE_SCAN_LINES_NUM / 2 + MAGNITUDE_EXCEPT_HALF_CENTER / 2);

	constexpr float MAGNITUDE_SNR_BASE = 20.0f;
	constexpr float MAGNITUDE_SNR_MIN = 12.0f;
	constexpr float MAGNITUDE_SNR_MAX = 41.0f; 

	constexpr int MAGNITUDE_BACKGROUND_MAX = 640; // 480;

	constexpr float IMAGE_QUALITY_SNR_MIN = 14.0f; // 20.0f;
	constexpr float IMAGE_QUALITY_SNR_MAX = 49.0f; // 41.0f;
	constexpr float IMAGE_QUALITY_SNR_MIN_NORMAL = 23.0f; // 14.0f;
	constexpr float IMAGE_QUALITY_SNR_MAX_NORMAL = 59.0f; // 52.0f;
	constexpr float IMAGE_QUALITY_SNR_MIN_FASTER = 23.0f; // 14.0f;
	constexpr float IMAGE_QUALITY_SNR_MAX_FASTER = 58.0f; // 57.0f; // 51.0f;
	constexpr float IMAGE_QUALITY_SNR_MIN_FASTEST = 23.0f; // 14.0f;
	constexpr float IMAGE_QUALITY_SNR_MAX_FASTEST = 56.0f; // 53.0f; // 45.0f;
	constexpr float IMAGE_QUALITY_SNR_MIN_CORNEA_RATIO = 1.0f; 
	constexpr float IMAGE_QUALITY_SNR_MAX_CORNEA_RATIO = 0.75f; 

	constexpr float IMAGE_QUALITY_INDEX_MIN = 0.0f;
	constexpr float IMAGE_QUALITY_INDEX_MAX = 10.0f;
	constexpr float IMAGE_QUALITY_INDEX_SIZE = (IMAGE_QUALITY_INDEX_MAX - IMAGE_QUALITY_INDEX_MIN);
	 
	constexpr float GRAY_SCALE_GROUND_SIZE_RATIO = 0.25f; // 25th percentile, background floor.  // 0.45f;
	constexpr float GRAY_SCALE_SIGNAL_SIZE_RATIO = 0.005f; // 99th percentile, bright signal. // 0.01f;  // 0.08f; // 0.001f; 
	constexpr float GRAY_SCALE_GROUND_SIZE_RATIO_RETINA = 0.25f; // 99th percentile, bright signal. // 0.01f;  // 0.08f; // 0.001f; 
	constexpr float GRAY_SCALE_GROUND_SIZE_RATIO_CORNEA = 0.25f; // 99th percentile, bright signal. // 0.01f;  // 0.08f; // 0.001f; 
	constexpr float GRAY_SCALE_SIGNAL_SIZE_RATIO_RETINA = 0.005f; // 99th percentile, bright signal. // 0.01f;  // 0.08f; // 0.001f; 
	constexpr float GRAY_SCALE_SIGNAL_SIZE_RATIO_CORNEA = 0.005f; // 99th percentile, bright signal. // 0.01f;  // 0.08f; // 0.001f; 

	constexpr float GRAY_SCALE_LOG_RANGE_RETINA_NORMAL = 1.4f; // 2.0f; // 1.8f;
	constexpr float GRAY_SCALE_LOG_RANGE_CORNEA_NORMAL = 1.2f; // 1.6f; // 2.0f; // 1.8f;
	constexpr float GRAY_SCALE_LOG_RANGE_RETINA_FASTER = 1.3f; // 1.8f; // 1.6f;
	constexpr float GRAY_SCALE_LOG_RANGE_CORNEA_FASTER = 1.1f; // 1.5f; // 1.8f; // 1.6f;
	constexpr float GRAY_SCALE_LOG_RANGE_RETINA_FASTEST = 1.1f; // 1.6f; // 1.4f;
	constexpr float GRAY_SCALE_LOG_RANGE_CORNEA_FASTEST = 1.0f; // 1.4f; // 1.6f; // 1.4f;
	constexpr float GRAY_SCALE_LOG_RANGE_LOWER_OFFSET = 0.0f; 
	constexpr float GRAY_SCALE_LOG_RANGE_UPPER_OFFSET = 0.25f; 

	constexpr float GRAY_SCALE_LOG_RANGE_MIN = 1.4f; 
	constexpr float GRAY_SCALE_LOG_VALUE_MIN = 2.0f; 
	constexpr float GRAY_SCALE_LOG_VALUE_MAX = 4.5f;
	constexpr float GRAY_SCALE_LOG_VALUE_RANGE = (GRAY_SCALE_LOG_VALUE_MAX - GRAY_SCALE_LOG_VALUE_MIN);


	// Frame Image Buffer configuration.
	//////////////////////////////////////////////////////////////////////////////////////////
	constexpr int FRAME_IMAGE_BUFFER_SIZE = FFT_IMAGE_BUFFER_SIZE;

	constexpr int IMAGE_BUFFER_LIST_SIZE_FOR_PREVIEW = 10; // 8;
	constexpr int IMAGE_BUFFER_LIST_SIZE_FOR_MEASURE = 64; // 32; // 8;
	constexpr int IMAGE_BUFFER_LIST_SIZE_FOR_ENFACE = 16; // 8;

}

#ifdef __SIGCHAIN_DLL
#define SIGCHAIN_DLL_API		__declspec(dllexport)
#else
#define SIGCHAIN_DLL_API		__declspec(dllimport)
#endif