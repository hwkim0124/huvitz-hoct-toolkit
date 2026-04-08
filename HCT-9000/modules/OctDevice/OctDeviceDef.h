#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


#include <cstdint>
#include <functional>

namespace OctDevice {

	// Device Mode 
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr int DEV_MODE_NORMAL = 0x00;
	constexpr int DEV_MODE_NO_CAMERAS = 0x01;
	constexpr int DEV_MODE_NO_LEDS = 0x02;
	constexpr int DEV_MODE_NO_MOTORS = 0x04;
	constexpr int DEV_MODE_NO_OPENCL = 0x08;
	constexpr int DEV_MODE_NO_COLOR = 0x10;
	constexpr int DEV_MODE_NO_DEVICE = 0xFF;


	// Serial Port 
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr char* UART_PORT_NAME = "COM1";
	constexpr int UART_PORT_NUM = 1;
	constexpr std::uint32_t UART_PORT_BAUDRATE = 115200;
	constexpr char* UART_CMD_USB_RESET = "usb_reset\r";
	constexpr char* UART_CMD_FPGA_RESET = "fpga_reset\r";
	constexpr char* UART_CMD_COLOR_RESET = "fcam_reset\r";
	constexpr char* UART_CMD_SYSTEM_REBOOT = "sys_reset\r";
	constexpr char* UART_CMD_INIT_STATUS = "init_stat\r";
	constexpr char* UART_CMD_FTDI_RESET = "ft601_reset\r";
	constexpr char* UART_CMD_ACK_PREFIX = "ack_";

	constexpr int UART_INIT_STATUS_PROGRESS = 0;
	constexpr int UART_INIT_STATUS_ERROR = 1;
	constexpr int UART_INIT_STATUS_OK = 2;

	constexpr int HOCT_1_INIT_DONE_STATUS   =  0x007FFA7F;
	constexpr int HOCT_1A_INIT_DONE_STATUS  =  0x007FFA7F;
	constexpr int HOCT_1F_INIT_DONE_STATUS  =  0x007FFFFF;
	constexpr int HOCT_1FA_INIT_DONE_STATUS =  0x007FFFFF;
	constexpr int HFC_INIT_DONE_STATUS      =  0x006cf7c7;

	constexpr int UART_INIT_STATUS_RETRY_DELAY = 500;
	constexpr int UART_INIT_STATUS_TIMEOUT_WAIT = 5000;


	// USB Port Control
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr std::uint32_t FTDI_RESET_WAIT_TIME = 10000;
	constexpr std::uint32_t USB_RESET_WAIT_TIME = 1000;
	constexpr std::uint32_t USB_RESET_RETRY_DELAY = 1000;
	constexpr std::uint32_t USB_RESET_RETRY_MAX = 3;

	constexpr std::uint32_t USB_RESET_COLOR_WAIT_TIME = 10000;

	// Color Sensor 
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr char* COLOR_SENSOR_USB_DESC = "KAC12040";


	// Step Motors
	//////////////////////////////////////////////////////////////////////////////////////////
	constexpr int MOTOR_OCT_FOCUS_STEPS_PER_DIOPTER = 210;
	constexpr int MOTOR_FUNDUS_FOCUS_STEPS_PER_DIOPTER = 224;
	constexpr int MOTOR_POLARIZATION_STEPS_PER_DEGREE = 4;
	constexpr int MOTOR_SPLIT_FOCUS_STEPS_PER_DIOPTER = 100;

	constexpr float MOTOR_OCT_FOCUS_DIOPTER_MAX = +20.0f;
	constexpr float MOTOR_OCT_FOCUS_DIOPTER_MIN = -20.0f;

	constexpr int MOTOR_END_OF_RANGE_OFFSET = 0; // 3;
	constexpr int MOTOR_CENTER_OF_POSITION_OFFSET = 1; // 3;

	constexpr int MOTOR_CONTROL_WAIT_DELAY = 30;
	constexpr int MOTOR_CONTROL_WAIT_COUNT = 300;


	// Stage Motors 
	//////////////////////////////////////////////////////////////////////////////////////////
	constexpr int STAGE_END_OF_LIMIT_OFFSET = 1;

	constexpr int STAGE_X_LIMIT_RANGE_LOW = -30;
	constexpr int STAGE_X_LIMIT_RANGE_HIGH = +30;
	constexpr int STAGE_Y_LIMIT_RANGE_LOW = 0;
	constexpr int STAGE_Y_LIMIT_RANGE_HIGH = 48000;
	constexpr int STAGE_Z_LIMIT_RANGE_LOW = -30;
	constexpr int STAGE_Z_LIMIT_RANGE_HIGH = +30;


	// USB FTDI Descriptor 
	//////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char USB_READ_PID = 0x82;
	constexpr unsigned char USB_WRITE_PID = 0x02;
	constexpr unsigned char USB_IR1_PID = 0x83;
	constexpr unsigned char USB_IR2_PID = 0x84;
	constexpr unsigned char USB_EMPTY_PID = 0x00;
	constexpr unsigned long USB_TIMEOUT = 15000;

	constexpr char* USB_BOARD_DESC = "OCT_CTRL";
	constexpr std::uint32_t USB_BOARD_ADDR = 0x80000000;


	// USB Command Packet
	////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned int PACKET_DATA_SIZE_MIN = 1;
	constexpr unsigned int PACKET_DATA_SIZE_MAX = 256;
	constexpr unsigned char PACKET_CRC_CODE = '1';
	constexpr unsigned int PACKET_CRC_SIZE = 4;
	constexpr unsigned char PACKET_ID_START = 1;
	constexpr unsigned char PACKET_ID_END = 255;


	// Infrared Camera 
	/////////////////////////////////////////////////////////////////////////////////////////
	constexpr int IR_CAMERA_FRAME_WIDTH = 644;
	constexpr int IR_CAMERA_FRAME_HEIGHT = 480;
	constexpr int IR_CAMERA_FRAME_DATA_SIZE = (IR_CAMERA_FRAME_WIDTH * IR_CAMERA_FRAME_HEIGHT);
	constexpr int IR_CAMERA_FRAME_PADD_SIZE = 4;

	constexpr int IR_CAMERA_IMAGE_WIDTH = (IR_CAMERA_FRAME_WIDTH - IR_CAMERA_FRAME_PADD_SIZE);
	constexpr int IR_CAMERA_IMAGE_HEIGHT = IR_CAMERA_FRAME_HEIGHT;

	constexpr std::uint8_t IR_CAMERA_AGAIN_VALUE1 = 1;
	constexpr std::uint8_t IR_CAMERA_AGAIN_VALUE2 = 2;
	constexpr std::uint8_t IR_CAMERA_AGAIN_VALUE3 = 4;
	constexpr std::uint8_t IR_CAMERA_AGAIN_VALUE4 = 8;
	constexpr std::float_t IR_CAMERA_DGAIN_MIN = 0.0f;
	constexpr std::float_t IR_CAMERA_DGAIN_MAX = 8.0f;

	constexpr std::uint16_t IR_CAMERA_EXPOSURE_TIME1 = 0x03BC;
	constexpr std::uint16_t IR_CAMERA_EXPOSURE_TIME2 = 0x0706;

	// Control Value
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr unsigned char CTRL_OFF = 0x00;
	constexpr unsigned char CTRL_ON = 0x01;
	constexpr unsigned char TURN_OFF = 0x00;
	constexpr unsigned char TURN_ON = 0x01;

	constexpr unsigned char MIRROR_OUT = 0x00;
	constexpr unsigned char MIRROR_IN = 0x01;

	constexpr unsigned char FUNDUS_MODE = 0x00;
	constexpr unsigned char SCAN_MODE = 0x01;
	constexpr unsigned char BOTH_MODE = 0x02;

	constexpr unsigned char NO_DIOPT_LENS = 0x00;
	constexpr unsigned char MINUS_DIOPT_LENS = 0x01;
	constexpr unsigned char PLUS_DIOPT_LENS = 0x02;

	constexpr unsigned char NORMAL_PUPIL_MASK = 0x00;
	constexpr unsigned char SMALL_PUPIL_MASK = 0x01;


	// Trajectory Profile
	/////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float TRAJECT_TIME_STEP_IN_US = 42.0f; // 38.0f;  // 10^6, 26KHz
	constexpr short TRAJECT_TRIGGER_DELAY = 1;
	constexpr short TRAJECT_NUMBER_OF_REPEATS = 1;
	constexpr short TRAJECT_CAMERA_TRIGGER_COUNT = 1024;
	constexpr short TRAJECT_CAMERA_TRIGGER_INTERVAL = 1;
	constexpr short TRAJECT_TRIGGER_START_INDEX = 5;
	constexpr short TRAJECT_SAMPLE_SIZE = (TRAJECT_TRIGGER_START_INDEX + TRAJECT_CAMERA_TRIGGER_COUNT);
	constexpr short TRAJECT_SAMPLE_SIZE_MAX = 8192;

	constexpr int TRAJECT_NUMBER_OF_PROFILES = 128; //  32;


	// Galvanometer 
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int GALVANO_POSITION_FUNCTION_DEGREE = 5; // 8;
	constexpr int GALVANO_STEP_RANGE_MIN = -32767;
	constexpr int GALVANO_STEP_RANGE_MAX = +32767;
	constexpr int GALVANO_STEP_RANGE_SIZE = (GALVANO_STEP_RANGE_MAX - GALVANO_STEP_RANGE_MIN);
	constexpr int GALVANO_STEP_RANGE_HALF = GALVANO_STEP_RANGE_MAX;
	
	constexpr int GALVANO_TRACE_POSITION_BUFFER_SIZE = TRAJECT_SAMPLE_SIZE_MAX;

	constexpr int GALVANO_RESP_TIME_IN_US = 350;
	
	// SLD Parameters 
	//////////////////////////////////////////////////////////////////////////////////////////////
	constexpr std::uint8_t RMON_HIGH_CODE = 0;
	constexpr std::uint8_t RMON_LOW_CODE1 = 1;
	constexpr std::uint8_t RMON_LOW_CODE2 = 2;
	constexpr std::uint8_t RMON_RSI_CODE = 3;


	enum class IrCameraId : unsigned char
	{
		CORNEA = 0x00, 
		RETINA = 0x01
	};

	enum class PacketType : unsigned char
	{
		NREAD = 0x5A,
		NREAD_RESP_OK = 0x75,
		NREAD_RESP_ERR = 0x7A, 
		NWRITE = 0x33,
		NWRITE_R = 0x3C, 
		NWRITE_RESP = 0x7C, 
		MSG_CMD = 0xC5,
		MSG_RESP = 0xCC, 
		STATUS_OK = 0x01, 
		STATUS_ERR = 0x02
	};


	enum class ScanModeType : unsigned short
	{
		SCAN_MODE_X = 0x0000, 
		SCAN_MODE_Y = 0x0001, 
		SCAN_MODE_XY = 0x0003
	};


	enum class CommandType : unsigned short
	{
		GALVO_MOVEX = 0x0001,
		GALVO_MOVEY = 0x0002,
		GALVO_MOVEXY = 0x0003,
		GALVO_SLEWX = 0x0004,
		GALVO_SLEWY = 0x0005,
		GALVO_SLEWXY = 0x0006,
		GALVO_DMOVEX = 0x0007,
		GALVO_DMOVEY = 0x0008,
		GALVO_DMOVEXY = 0x0009,
		START_ACQ = 0x000E,
		START_ACQR = 0x0010,
		START_ACQRF = 0x0012,
		GALVO_DSLEWXY = 0x000F,
		FLASH = 0xF000,
		MOTJOG = 0x0100,
		MOTMOVE = 0x0101,
		MOTHOME = 0x0102,
		CORLENS = 0x0103,
		FLEDSOL = 0x0104,
		IRCAM = 0x0205,
		IRLED = 0x0206,
		INTFIXA = 0x0207,
		EXTFIXA = 0x0208,
		RNGELED = 0x0209,
		RNGFLED = 0x020A,
		PNL_LED = 0x020B,
		SLD_CTL = 0x020C,
		LCDFIXPARAM = 0x020E,
		LCDFIXA = 0x020F,

		SLD_POTENTIO = 0x0210,
		FLASH_SLD_POT = 0x0211,
		GET_SLD_POT = 0x0212,

		FPGA_UP = 0xF00F,
		UPDATE_CAL_ROM = 0xF033,
		RESET_CPU = 0xF355,
		UPDATE_DEV_STATUS = 0x0303,
		LOAD_SM_DEFAULT = 0x0105,
		TEST_CMD = 0x03FF,
		FLSH_WR = 0x03FF,
		QRTNM = 0x4000,
		SPLIT_FM = 0x4001,
		FDCS_LENS = 0x4002,
		PUPIL_MASK = 0x4003,
		COR740_LED = 0x4004,
		COR940_LED = 0x4005,
		FLASH_LED = 0x4006,
		FC_CAPTURE = 0x4007,
		SPFOCUS_LED = 0x4008,
		WDOT_LED = 0x4009,
		WDOT_LED2 = 0x400A,
		IRCAM_AGAIN = 0x4010,
		IRCAM_DGAIN = 0x4011,
		XZ_STOP = 0x4012,
		AF_XZMOVE = 0x4020,
		AF_XZPOS = 0x4021,
		CR_MOVE = 0x4022,
		CR_STOP = 0x4023,
		IRCAM_INT = 0x5000,
		UNKNOWN = 0xFFEF, 

		LOAD_CALIB = 0x4100,
		SAVE_CALIB = 0x4101,
		LOAD_CALIB2 = 0x4102,
		SAVE_CALIB2 = 0x4103, 

		SCAN_FAULT_PARAM = 0x5055,
	};

#define MAX_MOTOR_NUM 12
#define MAX_SLEW_RATE 43

#pragma pack(push, 1)

	// Command Message Structure
	////////////////////////////////////////////////////////////////////////////////////
	struct NREAD_REQ {
		std::uint8_t nread;
		std::uint8_t pkt_id;
		std::uint8_t addr[4];
		std::uint8_t rd_size;
		std::uint8_t crc[PACKET_CRC_SIZE];
	};

	struct ReadRequest {
		NREAD_REQ packet;
		std::uint16_t size;
	};

	struct NREAD_RES {
		std::uint8_t type;
		std::uint8_t pkt_id;
		std::uint8_t data[PACKET_DATA_SIZE_MAX + PACKET_CRC_SIZE];
	};

	struct ReadResponse {
		NREAD_RES packet;
		std::uint16_t size;
	};

	struct NWRITE_REQ {
		std::uint8_t nwrite;
		std::uint8_t pkt_id;
		std::uint8_t addr[4];
		std::uint8_t wr_size;
		std::uint8_t data[PACKET_DATA_SIZE_MAX+PACKET_CRC_SIZE];
	};

	struct WriteRequest {
		NWRITE_REQ packet;
		std::uint16_t size;
	};

	struct NWRITE_RES {
		std::uint8_t type;
		std::uint8_t pkt_id;
		std::uint8_t status;
		std::uint8_t crc[PACKET_CRC_SIZE];
	};

	struct WriteResponse {
		NWRITE_RES packet;
		std::uint16_t size;
	};

	struct MSG_CMD_RES {
		std::uint8_t type;
		std::uint8_t pkt_id;
		std::uint8_t status;
		std::uint8_t crc[PACKET_CRC_SIZE];
	};

	struct MSG_CMD_HEAD {
		std::uint8_t type;
		std::uint8_t pkt_id;
		std::uint8_t msg_len;
		std::uint16_t ctrl;
	};

	struct MSG_CMD {
		MSG_CMD_HEAD hdr;

		union {
			struct {
				short s1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} galv1 ;

			struct {
				short s1;
				short s2;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} galv2 ;

			struct {
				std::uint8_t c1;
				int n1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} motor ;

			struct {
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl0;

			struct {
				std::uint8_t c1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl1;

			struct {
				std::uint8_t c1;
				std::uint8_t c2;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl2;

			struct {
				std::uint8_t c1;
				std::float_t c2;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl3;

			struct {
				std::uint32_t n1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl4;

			struct {
				std::uint16_t s1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl5;

			struct {
				std::uint16_t s1;
				std::uint16_t s2;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl6;

			struct {
				std::uint8_t c1;
				std::uint16_t s1;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl7;

			struct {
				std::uint8_t c1;
				std::uint8_t c2;
				std::uint16_t s1;
				std::uint16_t s2;
				std::uint8_t c3;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl8;

			struct {
				std::uint8_t c1;
				std::uint8_t c2;
				std::uint8_t c3;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} ctrl9;

			struct {
				std::uint16_t s1;
				std::uint16_t s2;
				std::uint16_t s3;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} scan1;

			struct {
				std::uint16_t s1;
				std::uint16_t s2;
				std::uint16_t s3;
				std::uint16_t s4;
				std::uint16_t s5;
				std::uint8_t crc[PACKET_CRC_SIZE];
			} scan2;
		};
	};

	struct MsgCommand {
		MSG_CMD packet;
		std::uint16_t size;
	};



	// Mainboard Address Structure
	/////////////////////////////////////////////////////////////////////////////////////
	struct AddressTable {
		std::uint32_t initStat;
		std::uint32_t sysVer;
		std::uint32_t sysCfg;
		std::uint32_t trajProf;
		std::uint32_t adcBuffer;
		std::uint32_t downloadBase;
		std::uint32_t gpStatus;
		std::uint32_t motorInfo;
		std::uint32_t sysCal;
		std::uint32_t galvanoDynamic;
		std::uint32_t autoStageMotorInfo;
		std::uint32_t sysCal2;
		// std::uint32_t sldParameter;
	};


	// System Init Info Structure
	///////////////////////////////////////////////////////////////////////////////////
	struct SysInitStatus {
		std::uint16_t status;
		std::uint16_t forPadding;
		std::uint32_t devStatus;
		std::uint16_t errCode[20];
		std::uint16_t reserved[3];
	};

	typedef struct
	{
		std::uint16_t Mode; //0:Strobe mode, 1: continuous mode
		std::uint16_t StrbActiveTime; // actual time:16us* StrbActiveTime
		std::uint16_t StrbDelayTime; // Strb mode delay(16us*delay)
		std::uint16_t reserved;
	} StrbCfg_st;// 8byte

	typedef struct	//64bytes
	{
		std::uint16_t ADC_EN;
		std::uint16_t AutoSlew;
		std::uint32_t SlewRate;
		std::uint16_t UartDebugEn;
		std::uint16_t MotorWait[MAX_MOTOR_NUM];
		StrbCfg_st StrbCfg;
		std::uint16_t xz_stage_limit_overide;//XZ stage limit overide
		std::uint16_t StepMotorDiag; //Step motor diagnostic routine enable.
		std::uint16_t SLDWarningEn;
		std::uint16_t reserved[209];
	} SysCfg_st; //256 bytes


	// System Version Info Structure
	///////////////////////////////////////////////////////////////////////////////////
	struct SysVersionInfo {
		std::int8_t appVer[6];
		std::int8_t fpgaVer[6];
		std::int8_t reserved[20];
	};

	// Motor Info Structure
	///////////////////////////////////////////////////////////////////////////////////
	struct StepMotorInfo {
		std::int32_t cur_pos;
		std::int32_t max_speed;
		std::int32_t min_speed;
		std::int32_t acc_step;
		std::int32_t sm_pos_min;
		std::int32_t sm_pos_max;

		std::int32_t sm_pi_hit_ref_pos;
		std::int32_t sm_pi_hit_margin;
		std::int32_t sm_last_pi_hit_pos;
		std::int32_t sm_pi_hit_pos_error;

		std::int32_t reserved[12];
	};


	// Stage Motor Info Structure 
	////////////////////////////////////////////////////////////////////////////////////
	struct StageDcMotorInfo {
		std::int16_t enc_pos;
		std::int16_t center_pos;
		std::int16_t reserved[6];
	};

	struct StageStepMotorInfo {
		std::int32_t cur_pos;
		std::int16_t limit_sensor_state[2];
		std::int32_t sm_pos_min;
		std::int32_t sm_pos_max;

		std::int32_t sm_pi_hit_ref_pos;
		std::int32_t sm_pi_hit_margin;
		std::int32_t sm_last_pi_hit_pos;
		std::int32_t sm_pi_hit_pos_error;
	};

	struct AutoStageInfo {
		StageDcMotorInfo x_motor;
		StageDcMotorInfo z_motor;
		StageStepMotorInfo y_motor;
	};


	// Trajectory Profile Structure
	////////////////////////////////////////////////////////////////////////////////////
	struct TrajectoryProfileParams {
		float time_step_us;
		uint32_t trig_delay;
		uint16_t repeat_num;
		uint16_t sample_size;
		uint16_t cam_trig_cnt;
		uint16_t cam_trig_itv;
		uint16_t trig_st_index;
		uint16_t reserved[23];
	};

	struct TrajectoryProfile {
		TrajectoryProfileParams params;
		int16_t  posx[TRAJECT_SAMPLE_SIZE_MAX];
		int16_t  posy[TRAJECT_SAMPLE_SIZE_MAX];
	};


	// SLD Parameter Structure 
	//////////////////////////////////////////////////////////////////////////////////////
	struct SldParameters {
		uint32_t serialNum;
		uint16_t forwardCurr;
		uint16_t monitorCurr;
		uint16_t centerFreq;
		uint16_t waveBand;
		uint16_t rmonHighCode;
		uint16_t rmonLowCode1;
		uint16_t rmonLowCode2;
		uint16_t rmonRsiCode;
	};

	struct SldSensorData {
		float SLD_current;
		float IPD_current;
		uint32_t EPD_DN;
		float temp;
	};


	// General purpose register status 
	////////////////////////////////////////////////////////////////////////////////////////
	struct GP_Status_st {
		int32_t pktErrCode;
		SldSensorData sldSensorData;
		uint32_t intr_src;
		uint32_t pi_status;
		uint32_t upgradeStatus;
		uint32_t upgradeProgressCnt;
		uint32_t reserved[7];
	};

	struct Galvano_Dynamic_st {
		int16_t offsetX;
		int16_t offsetY;
	};

#pragma pack(pop)
}


#ifdef __OCTDEVICE_DLL
#define OCTDEVICE_DLL_API		__declspec(dllexport)
#else
#define OCTDEVICE_DLL_API		__declspec(dllimport)
#endif