#pragma once

#include "OctGlobalDef.h"

#include <functional>


namespace OctGlobal
{
	constexpr std::uint16_t LED_INTENSITY_MIN = 0;
	constexpr std::uint16_t LED_INTENSITY_MAX = 100;
	constexpr std::uint16_t LED_INTENSITY_INIT = 0;
	constexpr std::uint16_t LED_INTENSITY_STEP = 5;

	constexpr std::uint16_t SLD_INTENSITY_MIN = 0;
	constexpr std::uint16_t SLD_INTENSITY_MAX = 1;
	constexpr std::uint16_t SLD_INTENSITY_INIT = 0;
	constexpr std::uint16_t SLD_INTENSITY_STEP = 1;

	constexpr std::uint16_t LED_RETINA_IR_INIT_VALUE = 5; //  30;
	constexpr std::uint16_t LED_RETINA_IR_BRIGHT_INIT_VALUE = 10; //  30;
	constexpr std::uint16_t LED_CORNEA_IR_INIT_VALUE = 5;// 30;
	constexpr std::uint16_t LED_KER_RING_INIT_VALUE = 35;//40;// 30;
	constexpr std::uint16_t LED_KER_FOCUS_INIT_VALUE = 25;// 40;// 90;
	constexpr std::uint16_t LED_FUNDUS_FLASH_INIT_VALUE = 60;
	constexpr std::uint16_t LED_PANNEL_INIT_VALUE = 0;
	constexpr std::uint16_t LED_WORKING_DOTS_INIT_VALUE = 15;// 2;// 10;
	constexpr std::uint16_t LED_SPLIT_FOCUS_INIT_VALUE = 60;// 20; // 80;
	constexpr std::uint16_t LED_EXT_FIXATION_INIT_VALUE = 2;
	constexpr std::uint16_t LED_OCT_SLD_INIT_VALUE = 0;

	constexpr std::uint16_t LED_PANNEL_VALUE_MAX = 20;
	constexpr std::uint16_t LED_PANNEL_VALUE_MIN = 0;

	constexpr const char* LED_RETINA_IR_NAME = "RetinaIR";
	constexpr const char* LED_RETINA_IR_BRIGHT_NAME = "RetinaIR_Bright";
	constexpr const char* LED_CORNEA_IR_NAME = "CorneaIR";
	constexpr const char* LED_KER_RING_NAME = "KeratoRing";
	constexpr const char* LED_KER_FOCUS_NAME = "KeratoFocus";
	constexpr const char* LED_FUNDUS_FLASH_NAME = "FundusFlash";
	constexpr const char* LED_SPLIT_FOCUS_NAME = "SplitFocus";
	constexpr const char* LED_EXT_FIXATION_NAME = "ExternalFixation";
	constexpr const char* LED_WORKING_DOTS_NAME = "WorkingDots";
	constexpr const char* LED_WORKING_DOT2_NAME = "WorkingDot2";
	constexpr const char* LED_OCT_SLD_NAME = "OctSLD";
	constexpr const char* LED_PANNEL_NAME = "Pannel";
	constexpr const char* LED_UNKNOWN_NAME = "UnknownLED";

	constexpr std::int32_t MOTOR_OCT_FOCUS_INIT_POS = 1400;
	constexpr std::int32_t MOTOR_REFERENCE_INIT_POS = 42000;
	constexpr std::int32_t MOTOR_POLARIZATION_INIT_POS = 19;
	constexpr std::int32_t MOTOR_FUNDUS_FOCUS_INIT_POS = 2015;
	constexpr std::int32_t MOTOR_OCT_SAMPLE_INIT_POS = 0;
	constexpr std::int32_t MOTOR_SPLIT_FOCUS_INIT_POS = 0;
	constexpr std::int32_t MOTOR_DIOPTER_SEL_INIT_POS = 0;
	constexpr std::int32_t MOTOR_PUPIL_MASK_INIT_POS = 1800;
	constexpr std::int32_t MOTOR_OCT_COMPENSATION_INIT_POS = 0;
	constexpr std::int32_t MOTOR_STAGE_X_INIT_POS = 0;
	constexpr std::int32_t MOTOR_STAGE_Y_INIT_POS = 0;
	constexpr std::int32_t MOTOR_STAGE_Z_INIT_POS = 0;

	constexpr std::int32_t MOTOR_OCT_FOCUS_INIT_ORIGIN = 1400;
	constexpr std::int32_t MOTOR_REFERENCE_INIT_ORIGIN = 42000;
	constexpr std::int32_t MOTOR_POLARIZATION_INIT_ORIGIN = 19;
	constexpr std::int32_t MOTOR_FUNDUS_FOCUS_INIT_ORIGIN = 2015;
	constexpr std::int32_t MOTOR_SPLIT_FOCUS_INIT_ORIGIN = 0;
	constexpr std::int32_t MOTOR_OCT_COMPENSATION_INIT_ORIGIN = 0;

	constexpr std::int32_t MOTOR_OCT_SAMPLE_INIT_MIRROR_IN = 0;
	constexpr std::int32_t MOTOR_OCT_SAMPLE_INIT_MIRROR_OUT = 0;
	constexpr std::int32_t MOTOR_DIOPTER_SEL_INIT_MINUS = 0;
	constexpr std::int32_t MOTOR_DIOPTER_SEL_INIT_ZERO = 0;
	constexpr std::int32_t MOTOR_DIOPTER_SEL_INIT_PLUS = 0;
	constexpr std::int32_t MOTOR_PUPIL_MASK_INIT_NORMAL = 1800;
	constexpr std::int32_t MOTOR_PUPIL_MASK_INIT_SMALL = 0;

	constexpr float MOTOR_OCT_FOCUS_MM_RANGE_MIN = -1.6f;
	constexpr float MOTOR_OCT_FOCUS_MM_RANGE_MAX = 7.5f;
	constexpr float MOTOR_REFERENCE_MM_RANGE_MIN = -3.0f;
	constexpr float MOTOR_REFERENCE_MM_RANGE_MAX = 65;
	constexpr float MOTOR_POLARIZATION_DEGREE_MIN = -3.6f;
	constexpr float MOTOR_POLARIZATION_DEGREE_MAX = 180;
	constexpr float MOTOR_FUNDUS_FOCUS_DEGREE_MIN = -60;
	constexpr float MOTOR_FUNDUS_FOCUS_DEGREE_MAX = 309;

	constexpr const char* MOTOR_OCT_FOCUS_NAME = "OctFocus";
	constexpr const char* MOTOR_REFERENCE_NAME = "Reference";
	constexpr const char* MOTOR_POLARIZATION_NAME = "Polarization";
	constexpr const char* MOTOR_FUNDUS_FOCUS_NAME = "FundusFocus";
	constexpr const char* MOTOR_OCT_SAMPLE_NAME = "OctSample";
	constexpr const char* MOTOR_SPLIT_FOCUS_NAME = "SplitFocus";
	constexpr const char* MOTOR_FUNDUS_DIOPTER_NAME = "FundusDiopter";
	constexpr const char* MOTOR_PUPIL_MASK_NAME = "PupilMask";
	constexpr const char* MOTOR_OCT_DIOPTER_NAME = "OctDiopter";
	constexpr const char* MOTOR_STAGE_X_NAME = "StageX";
	constexpr const char* MOTOR_STAGE_Y_NAME = "StageY";
	constexpr const char* MOTOR_STAGE_Z_NAME = "StageZ";
	constexpr const char* MOTOR_UNKNOWN_NAME = "UnknownMotor";

	constexpr const char* CAMERA_RETINA_NAME = "RetinaIR";
	constexpr const char* CAMERA_CORNEA_NAME = "CorneaIR";
	constexpr const char* CAMERA_FUNDUS_NAME = "FundusColor";

	constexpr int CAMERA_PRESET_SIZE = 3;
	constexpr float CAMERA_RETINA_INIT_AGAIN = 8.0f;
	constexpr float CAMERA_RETINA_INIT_AGAIN2 = 8.0f;
	constexpr float CAMERA_RETINA_INIT_AGAIN3 = 4.0f;
	constexpr float CAMERA_CORNEA_INIT_AGAIN = 2.0f;
	constexpr float CAMERA_COLOR_INIT_AGAIN = 4.0f;

	constexpr float CAMERA_RETINA_INIT_DGAIN = 2.0f;
	constexpr float CAMERA_RETINA_INIT_DGAIN2 = 1.0f;
	constexpr float CAMERA_RETINA_INIT_DGAIN3 = 1.0f;
	constexpr float CAMERA_CORNEA_INIT_DGAIN = 4.0f;
	constexpr float CAMERA_COLOR_INIT_DGAIN = 1.0f;

	constexpr float CAMERA_DGAIN_MIN = 1.0f;
	constexpr float CAMERA_DGAIN_MAX = 8.0f;
	constexpr float CAMERA_AGAIN_MIN = 1.0f;
	constexpr float CAMERA_AGAIN_MAX = 10.0f;

	constexpr double RESAMPLE_INIT_PARAM1 = 800.752136;
	constexpr double RESAMPLE_INIT_PARAM2 = 0.047172;
	constexpr double RESAMPLE_INIT_PARAM3 = -0.000003;
	constexpr double RESAMPLE_INIT_PARAM4 = 0.000000;

	constexpr double PHASE_SHIFT_INIT_PARAM1 = -9.2;
	constexpr double PHASE_SHIFT_INIT_PARAM2 = +2.3;
	constexpr double PHASE_SHIFT_INIT_PARAM3 = 0.0;

	constexpr int LIGHT_LED_VALUE_MIN = 0;
	constexpr int LIGHT_LED_VALUE_MAX = 100;

	constexpr int FLASH_LEVEL_MIN = 0;
	constexpr int FLASH_LEVEL_MAX = 100;
	constexpr float FLASH_SIZE_FACTOR_MIN = -100.0f;
	constexpr float FLASH_SIZE_FACTOR_MAX = +100.0f;

	constexpr int SLD_FAULT_DETECTION_ENABLED = 1;

	/*
	// Device Index
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int CORNEA_IR_CAMERA = 0;
	constexpr int RETINA_IR_CAMERA = 1;
	constexpr int NUMBER_OF_IR_CAMERAS = (RETINA_IR_CAMERA + 1);

	constexpr int RETINA_IR_LED = 0;
	constexpr int CORNEA_IR_LED = 1;
	constexpr int KER_RING_LED = 2;
	constexpr int KER_FOCUS_LED = 3;
	constexpr int FUNDUS_FLASH_LED = 4;
	constexpr int SPLIT_FOCUS_LED = 5;
	constexpr int EXT_FIXATION_LED = 6;
	constexpr int WORKING_DOTS_LED = 7;
	constexpr int OCT_SLD_LED = 8;
	constexpr int NUMBER_OF_LIGHT_LEDS = (OCT_SLD_LED + 1);

	constexpr int OCT_FOCUS_MOTOR = 0;
	constexpr int REFERENCE_MOTOR = 1;
	constexpr int POLARIZATION_MOTOR = 2;
	constexpr int FUNDUS_FOCUS_MOTOR = 3;
	constexpr int OCT_SAMPLE_MOTOR = 4;
	constexpr int SPLIT_FOCUS_MOTOR = 5;
	constexpr int DIOPTER_SEL_MOTOR = 6;
	constexpr int PUPIL_MASK_MOTOR = 7;
	constexpr int OCT_COMPENSATION_MOTOR = 8;
	constexpr int NUMBER_OF_STEP_MOTORS = (OCT_COMPENSATION_MOTOR + 1);
	*/

	enum class CameraType : unsigned char
	{
		CORNEA = 0x00,
		RETINA = 0x01, 
		FUNDUS = 0x02,
		NUMBER_OF_ITEMS = (FUNDUS + 1)
	};


	enum class LightLedType : unsigned short
	{
		RETINA_IR = 0,
		RETINA_IR_BRIGHT = 1,
		CORNEA_IR = 2,
		KER_RING = 3,
		KER_FOCUS = 4,
		FUNDUS_FLASH = 5,
		SPLIT_FOCUS = 6,
		EXT_FIXATION = 7,
		WORKING_DOTS = 8,
		WORKING_DOT2 = 9,
		OCT_SLD = 10,
		PANNEL = 11,

		NUMBER_OF_ITEMS = (OCT_SLD + 1)
	};

	enum class StepMotorType : unsigned char
	{
		OCT_FOCUS = 0x00,
		REFERENCE = 0x01,
		POLARIZATION = 0x02,
		FUNDUS_FOCUS = 0x03,
		OCT_SAMPLE = 0x04,
		SPLIT_FOCUS = 0x05,
		FUNDUS_DIOPTER = 0x06,
		PUPIL_MASK = 0x07,
		OCT_DIOPTER = 0x08,
		STAGE_X = 0x09,
		STAGE_Y = 0x0A,
		STAGE_Z = 0x0B,
		NUMBER_OF_ITEMS = (OCT_DIOPTER + 1)
	};

	enum class StageMotorType : unsigned char
	{
		STAGE_X = 0x09,
		STAGE_Y = 0x0A,
		STAGE_Z = 0x0B,
		NUMBER_OF_ITEMS = (STAGE_Z + 1)
	};

	enum class OctGrabberType : unsigned int
	{
		MIL_FRAME_GRABBER = 0x00,
		USB_LINE_CAMERA = 0x01,
		NUMBER_OF_ITEMS = (USB_LINE_CAMERA + 1)
	};

	enum class OctLineCameraMode : unsigned int
	{
		LINE_CAMERA_68K = 1,
		LINE_CAMERA_80K_TEST = 2,
		LINE_CAMERA_80K = 3,
		LINE_CAMERA_120K = 4, 
	};

	enum class RetinaTrackingSpeed : unsigned int
	{
		TRACK_CAMERA_15FPS = 1, 
		TRACK_CAMERA_30FPS = 2,
	};


	// Ir Camera Data Callback type
	////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(unsigned char*, unsigned int, unsigned int)> IrCameraFrameCallback;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int)> RetinaCameraImageCallback;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, bool, float, float, bool, float)> CorneaCameraImageCallback;


	// Color Camera Data Callback type
	/////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, unsigned int, unsigned int)> ColorCameraFrameCallback;
	typedef std::function<void(unsigned char*, unsigned int, unsigned int, unsigned int)> ColorCameraImageCallback;

	// Joystick Callback type
	//////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(void)> JoystickEventCallback;
	typedef std::function<void(bool)> EyeSideEventCallback;
	typedef std::function<void(void)> OptimizeKeyEventCallback;
	typedef std::function<void(void)> BacklightOnEventCallback;
	typedef std::function<void(void)> SldWarningEventCallback;
	typedef std::function<void(void)> ScannerFaultEventCallback;
}
