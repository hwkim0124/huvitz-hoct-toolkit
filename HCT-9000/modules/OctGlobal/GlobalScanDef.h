#pragma once

#include "OctGlobalDef.h"

#include <functional>


namespace OctGlobal
{
	constexpr int LINE_CAMERA_CCD_PIXELS = 2048;
	constexpr unsigned short LINE_CAMERA_PIXEL_VALUE_MAX = 4096;

	constexpr int BSCAN_LATERAL_SIZE_MAX = 1024; // 2048;


	// Scan Image Output
	////////////////////////////////////////////////////////////////////////////////////////////////////////




	// Spectrometer Calibration
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float SPECTRO_CALIB_BSCAN_RANGE = 6.0f;
	constexpr int SPECTRO_CALIB_BSCAN_WIDTH = 1024;

	constexpr int SPECTRO_CALIB_FRAME_DATA_WIDTH = (LINE_CAMERA_CCD_PIXELS / 2 + 1); // padded (SPECTRO_CALIB_BSCAN_WIDTH + 1);
	constexpr int SPECTRO_CALIB_FRAME_DATA_HEIGHT = (LINE_CAMERA_CCD_PIXELS / 2 + 1);
	constexpr int SPECTRO_CALIB_FRAME_DATA_SIZE = (SPECTRO_CALIB_FRAME_DATA_WIDTH*SPECTRO_CALIB_FRAME_DATA_HEIGHT);

	constexpr unsigned int SPECTRO_CALIB_INTENSITY_MAX = 20480;// 10240; // 8192;// 10240;
	

	// Frame Grabber DCF file path
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	// constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_NORMAL = L"oct_hw_trigger_160929_13k.dcf";
	// constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_FASTER = L"oct_hw_trigger_160929_23k.dcf";
	// constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_FASTEST = L"oct_hw_trigger_160929_68k.dcf";
	constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_NORMAL = L"oct_hw_trigger_171110_13k.dcf";
	constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_FASTER = L"oct_hw_trigger_171110_26k.dcf";
	constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_FASTEST = L"oct_hw_trigger_171110_68k.dcf";

	constexpr wchar_t* DCF_FILE_NAME_TO_SPEED_CUSTOM = L"oct_hw_trigger_171110_cst.dcf";

	// Trigger Step by Scan speed. 
	//////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float TRIGGER_TIME_STEP_AT_NORMAL_SPEED = 78.0f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTER_SPEED = 38.6f; //  42.0;
	constexpr float TRIGGER_TIME_STEP_AT_FASTEST_SPEED = 14.8f; // 16.0; // 15.0;
	constexpr float TRIGGER_TIME_STEP_AT_CUSTOM_SPEED = 14.8f; //  16.0; // 15.0;

	constexpr float TRIGGER_TIME_STEP_AT_NORMAL_SPEED_USB3 = 66.6f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTER_SPEED_USB3 = 38.4f; //  14.8f; // 38.4f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTEST_SPEED_USB3 = 14.8f; //  8.4f; // 14.8f;

	constexpr float TRIGGER_TIME_STEP_AT_NORMAL_SPEED_068K = 66.6f;
	constexpr float TRIGGER_TIME_STEP_AT_NORMAL_SPEED_080K = 43.5f;
	constexpr float TRIGGER_TIME_STEP_AT_NORMAL_SPEED_120K = 43.5f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTER_SPEED_068K = 38.4f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTER_SPEED_080K = 25.0f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTER_SPEED_120K = 25.0f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTEST_SPEED_068K = 14.8f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTEST_SPEED_080K = 12.5f;
	constexpr float TRIGGER_TIME_STEP_AT_FASTEST_SPEED_120K = 9.4f;

	constexpr float EXPOSURE_TIME_AT_NORMAL_SPEED_USB3 = 64.0f;
	constexpr float EXPOSURE_TIME_AT_FASTER_SPEED_USB3 = 37.0f; // 13.7f; // 37.0f;
	constexpr float EXPOSURE_TIME_AT_FASTEST_SPEED_USB3 = 13.7f; // 7.4f; // 13.7f;

	constexpr float EXPOSURE_TIME_AT_NORMAL_SPEED_068K = 64.0f; // 7.4f; // 13.7f;
	constexpr float EXPOSURE_TIME_AT_NORMAL_SPEED_080K = 42.5f;
	constexpr float EXPOSURE_TIME_AT_NORMAL_SPEED_120K = 42.5f;
	constexpr float EXPOSURE_TIME_AT_FASTER_SPEED_068K = 37.0f; // 7.4f; // 13.7f;
	constexpr float EXPOSURE_TIME_AT_FASTER_SPEED_080K = 24.0f;
	constexpr float EXPOSURE_TIME_AT_FASTER_SPEED_120K = 24.0f;
	constexpr float EXPOSURE_TIME_AT_FASTEST_SPEED_068K = 13.7f; // 7.4f; // 13.7f;
	constexpr float EXPOSURE_TIME_AT_FASTEST_SPEED_080K = 11.5f;
	constexpr float EXPOSURE_TIME_AT_FASTEST_SPEED_120K = 8.4f;

	constexpr int TRIGGER_FORE_PADDING_POINTS_AT_FASTEST = 64; // 96; // 64; // 31;		// 500us // 16
	constexpr int TRIGGER_FORE_PADDING_POINTS_AT_FASTER = 32; // 48; // 32; // 15;		// 500us // 42
	constexpr int TRIGGER_FORE_PADDING_POINTS_AT_NORMAL = 18; // 24; // 18; // 9;		// 500us // 78

	constexpr int TRIGGER_POST_PADDING_POINTS_AT_FASTEST = 32; // 2;		// 500us // 16
	constexpr int TRIGGER_POST_PADDING_POINTS_AT_FASTER = 16; // 2;		// 500us // 42
	constexpr int TRIGGER_POST_PADDING_POINTS_AT_NORMAL = 12; // 2;		// 500us // 78


	// Enface Image 
	//////////////////////////////////////////////////////////////////////////////////////////
	constexpr int ENFACE_IMAGE_DEPTH_START = 0; // 64;
	constexpr int ENFACE_IMAGE_DEPTH_CLOSE = 768; // (512 - 1);


	// Scan Imaging Callback types
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(unsigned short*, unsigned int, unsigned int, unsigned int)> GrabPreviewBufferCallback;
	typedef std::function<void(unsigned short*, unsigned int, unsigned int, unsigned int)> GrabMeasureBufferCallback;
	typedef std::function<void(unsigned short*, unsigned int, unsigned int, unsigned int)> GrabEnfaceBufferCallback;


	// Scan Processing Callback types
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, float, unsigned int)> PreviewImageCallback;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, float, float, unsigned int, unsigned int)> PreviewImageCallback2;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, float, float, unsigned int, unsigned int)> PreviewCorneaCallback;

	typedef std::function<void(unsigned short*, unsigned int, unsigned int)> SpectrumDataCallback;
	typedef std::function<void(float*, unsigned int, unsigned int)> ResampleDataCallback;
	typedef std::function<void(float*, unsigned int, unsigned int)> IntensityDataCallback;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int)> EnfaceImageCallback;

	typedef std::function<void(unsigned char*, unsigned int, unsigned int, float, float, int, int)> PreviewImageCompletedEvent ;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int)> EnfaceImageCompletedEvent;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, float, float, int, int, unsigned char*)> MeasureImageCompletedEvent;


	// Scan, Fundus Measure Callback types 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(bool)> FundusImageAcquiredCallback;
	typedef std::function<void(bool)> FundusImageCompletedCallback;
	typedef std::function<void(bool)> ScanPatternAcquiredCallback;
	typedef std::function<void(bool)> ScanPatternCompletedCallback;


	// Scan Operation Callback types
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(bool, float, float)> AutoFocusCompleteCallback;
	typedef std::function<void(bool, float, float)> AutoPolarizationCompleteCallback;
	typedef std::function<void(bool, int, int)> AutoReferenceCompleteCallback;
	typedef std::function<void(bool, int, int)> AutoLevelCompleteCallback;

	typedef std::function<void(bool)> AutoOptimizeCompletedCallback;
	typedef std::function<void(bool)> AutoPositionCompletedCallback;
	typedef std::function<void(bool, int, int, float)> AutoMeasureCompletedCallback;

	typedef std::function<void(bool, int, float)> AutoCorneaFocusCompletedCallback;
	typedef std::function<void(bool, int, float)> AutoRetinaFocusCompletedCallback;
	typedef std::function<void(bool, int, float)> AutoLensFrontFocusCompletedCallback;
	typedef std::function<void(bool, int, float)> AutoLensBackFocusCompletedCallback;
}