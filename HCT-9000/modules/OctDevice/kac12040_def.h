#pragma once

#include "cameraParam.h"

//sensor register address
#define SENSOR_CHIP_ID_MSB_REG 0x4009
#define SENSOR_CHIP_ID_LSB_REG 0x4011
#define SENSOR_STATE_SEL_REG 0x4019
#define SENSOR_FLO_EDGE_DLY_REG 0x0211
#define SENSOR_CONFIG2_REG		0x01D9
#define SENSOR_CONFIG1_REG		0x01D1
#define SENSOR_FLOMSO_POL_REG		0x2D89

#define SENSOR_SLOPE12_GAIN 0x24E9
#define SENSOR_SLOPE34_GAIN 0x24F1
#define SENSOR_SLOPE56_GAIN 0x24F9
#define SENSOR_SLOPE7_GAIN  0x2501

#define SENSOR_SLOPE1_LEN   0x24B9
#define SENSOR_SLOPE2_LEN   0x24C1
#define SENSOR_SLOPE3_LEN   0x24C9
#define SENSOR_SLOPE4_LEN   0x24D1
#define SENSOR_SLOPE5_LEN   0x24D9
#define SENSOR_SLOPE6_LEN   0x24E1

//sensor state
#define NO_ACTION		0
#define STANDBY_STATE 	1
#define IDLE_STATE		2
#define RUNNING_STATE	3
#define SLAVE_INT_STATE 4
#define OTP_WRITE_STATE	5
#define OTP_READ_STATE	6
#define CONFIG_STATE	13

//config1 register definition
#define ROLLING_SHUTTER 0x0 //bit[1:0]
#define GLOBAL_SHUTTER  0x1//bit[1:0]
#define GLOBAL_RESET	0x2//bit[1:0]
#define DUAL_SCAN		(0x0<<2) //bit2
#define TRI_SCAN		(0x1<<2) //bit2
#define	FFR_EN			(0x1<<3) //bit3, fixed frame rate enable
#define OVERLAP_INT_EN	(0x1<<4) //bit4 . overlapped integegration enable
#define OVERLAP_INT_DISABLE 0
#define DUAL_VIDEO_EN	(0x1<<5) //bit5, when enabled, Frame A/B video alternates
#define FRAMEA_FIRST	(0x0<<6) //bit6, when dual video enabled, FRAME A first, when single mode, FRAME A only
#define FRAMEA_ONLY		(0x0<<6) //bit6, when dual video enabled, FRAME A first, when single mode, FRAME A only
#define FRAMEB_FIRST	(0x1<<6) //bit6, when dual video enabled, FRAME B first, when single mode, FRAME B only
#define FRAMEB_ONLY		(0x1<<6) //bit6, when dual video enabled, FRAME B first, when single mode, FRAME B only
#define VERTICAL_FLIP	(0x1<<7) //bit7, when set vertical flip
#define HORIZONTAL_FLIP	(0x1<<8) //bit8, when set horizontal flip
#define NO_FLIP			(0x0<<7)
#define MSO_EN			(0x1<<9) //bit9, mechanical shutter sync output enabled.
#define FLO_EN			(0x1<<10)//bit10, Flash output enable
#define FRAME_BLC_EN	(0x1<<11)//bit11, Frame BLC enable
#define RNC_EN			(0x1<<12)//bit12, Row noise correction enable
#define CNC_EN			(0x1<<13)//bit13, column noise correction enable
#define TEMP_SENSOR_EN	(0x1<<14)//bit14, temperature sensor enable, this mode adds one row of vertical blanking to the frame time
#define RSV_ALG			(0x0<<15)//bit15, this bit must be set to zero

//CONFIG2 register definition
#define TRIG_POL_HIGH		(0x0<<0) //bit 0: trigger polarity high active,
#define TRIG_POL_LOW		(0x1<<0) //bit 0: trigger polarity low active,
#define MFSO_RDREF_INT    	(0x0<<1) //bit 1: MSO/FSO rising edge delay reference 0:delay from start of integration phase
#define MFSO_RDREF_READOUT 	(0x1<<1) //bit 1: MSO/FSO rising edge delay reference 1:delay from start of readout phase
#define MFSO_FDREF_INT		(0x0<<2) //bit 2: MSO/FSO falling edge delay reference 0:delay from start of integration phase
#define MFSO_FDREF_READOUT	(0x1<<2) //bit 2: MSO/FSO falling edge delay reference 1:delay from start of readout phase
#define MFSO_DELAY_MULT_X1	(0x0<<4) //bit6~4: MSO/FLO delay width multiplier x1
#define MFSO_DELAY_MULT_X2	(0x1<<4) //bit6~4: MSO/FLO delay width multiplier x2
#define MFSO_DELAY_MULT_X4	(0x2<<4) //bit6~4: MSO/FLO delay width multiplier x4
#define MFSO_DELAY_MULT_X8	(0x3<<4) //bit6~4: MSO/FLO delay width multiplier x8
#define MFSO_DELAY_MULT_X16	(0x4<<4) //bit6~4: MSO/FLO delay width multiplier x16
#define MFSO_DELAY_MULT_X32	(0x5<<4) //bit6~4: MSO/FLO delay width multiplier x32
#define MFSO_DELAY_MULT_X64	(0x6<<4) //bit6~4: MSO/FLO delay width multiplier x64
#define MFSO_DELAY_MULT_X128 (0x7<<4) //bit6~4: MSO/FLO delay width multiplier x128
#define OUTPUT_REF_PXL_EN	(0x1<<7) //bit 7: output reference pixel enable
#define META_DATA_EN		(0x1<<8) //bit 8: embedded metadata data enable
#define FACTORY_GCAL_ACT 	(0x1<<10) //bit10: when set, factory gain calibration data used stored in OTP, Should always set.
#define COMPAND_EN			(0x1<<11) //bit 11: companding enable

#define KAC12040_SENSOR_ID 0x00110080


#define	FPGA_IMG_READ_CTRL_REG	 	0x00050
#define FPGA_FRAME_SIZE_REG		 	0x00054
#define FPGA_CMD_HSIZE_REG		 	0x00058
#define FPGA_LINE_DATAW_SIZE_REG 	0x0005C
#define FPGA_VSIZE_REG			 	0x00060
#define FPGA_HBLANK_REG			 	0x00064
#define FPGA_VBLANK_REG			 	0x00068
#define	FPGA_IMG_RD_CTRL_STATUS_REG 0x0006C
#define FPGA_FT601_RESET_REG		0x00018
#define CAM_VERSION_REG				0xFFFFF


#define FRAME_MODE_FIXED 0
#define FRAME_MODE_CONTINUOUS 1


//FPGA FLASH STRB  OUT definition
#define STRB_PULSE_SHAPING 1
#define STRB_BYPASS 0

//FLUSH image pipe FLUSH option
#define MEM_WR_RESET 	(0x1<<2)
#define MEM_RD_RESET 	(0x1<<3)
#define FRAME_BUF_RESET (0x1<<4)
#define MIG_RESET		(0x1<<5)


//
#define DDR2_WR_CMD_TYPE		0x60
#define DDR2_READ_RESP_CMD_TYPE 0x63
#define DDR2_WR_RESP_CMD_TYPE 0x61
#define SPI_FLASH_WR_CMD_TYPE 0x70
#define SPI_FLASH_RD_CMD_TYPE 0x71
#define SPI_FLASH_OP_STATUS_CMD_TYPE 0x72
#define DEFECT_MAP_WRITE_CMD_TYPE 0x80
#define SPI_FLASH_DIRECT_READ_CMD_TYPE 0x81

//cmd ACK status
#define CMD_ACK_STATUS_OK 0
#define CMD_ACK_STATUS_ERROR 1

#define MAX_DEFECT_SIZE 4096
#define SPI_FLASH_DEFECT_MAP_BASE 0x300000



#pragma pack(push, 1)

// camera command format
struct CmdRegW_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = 0;
	unsigned Addr;
	unsigned Data;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdFlushImgPipe_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 3;
	unsigned const CmdType = 0x50;
	unsigned FlushOption;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdRegR_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 3;
	unsigned const CmdType = 1;
	unsigned Addr;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};



struct CameraROI_t
{
	unsigned XROI_start;
	unsigned XROI_width;
	unsigned YROI_start;
	unsigned YROI_height;
	unsigned decimationRatio;
};

struct CaptureParam_t
{
	unsigned CaptureMode; //Fixed, or continuous frame
	unsigned CaptureFrameSize;//valid at Fixed mode
};

struct TrigExposureParam_t //total trig period:exposureCnt+ExposureOffCnt
{
	unsigned ExposureCnt;//1cnt:10ns
	unsigned ExposureOffCnt; //trig blank cnt
};

struct StrbParam_t
{
	unsigned StrbDelay;
	unsigned StrbActTime;
	unsigned StrbMode;
};

struct SensorGain_t
{
	unsigned  Again;
	float Dgain;
};

struct SensorMode_t
{
	//cfg1 register parameter
	unsigned shutter_mode = ROLLING_SHUTTER;
	unsigned ovelapped_int_en = OVERLAP_INT_DISABLE;
	unsigned flip = NO_FLIP;
	unsigned default_cfg1 = (CNC_EN | RNC_EN | FRAME_BLC_EN | FLO_EN | FRAMEB_ONLY | DUAL_SCAN);

};

struct CompandingParam_t
{

	unsigned SlopeLen[6];
	unsigned SlopeGain[7];
	unsigned SlopeSize; //#of slopes used , max 6
	unsigned CompandingEn; //1(enable),0(disable)

};

struct CmdSetROI_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 7;
	unsigned const CmdType = 0x10;
	CameraROI_t CameraROI;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdCaptureParam_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = 0x11;
	CaptureParam_t CaptureParam;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdTrigExposureParam_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = 0x12;
	TrigExposureParam_t TrigExposureParam;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdStrbParam_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 5;
	unsigned const CmdType = 0x14;
	StrbParam_t StrbParam;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSensorGain_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = 0x13;
	SensorGain_t SensorGain;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdGrabStart_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 2;
	unsigned const CmdType = 0x20;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdGrabStop_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 2;
	unsigned const CmdType = 0x21;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdFrameRequest_t
{
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 2;
	unsigned const CmdType = 0x30;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSensorRegW_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = 0x3;
	unsigned Addr;
	unsigned Data;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSensorRegR_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 3;
	unsigned const CmdType = 4;
	unsigned Addr;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdDDR2Read_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 5;
	unsigned const CmdType = 0x62;
	unsigned Addr;
	unsigned memc;
	unsigned data_size_bytes;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSPI_Flash_write_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = SPI_FLASH_WR_CMD_TYPE;
	unsigned src_Addr;
	unsigned data_size_bytes;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};


struct CmdSPI_Flash_Read_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = SPI_FLASH_RD_CMD_TYPE;
	unsigned dst_Addr;
	unsigned data_size_bytes;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSPI_Flash_DirectRead_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = SPI_FLASH_DIRECT_READ_CMD_TYPE;
	unsigned flash_rd_Addr;
	unsigned data_size_bytes; //max 256 bytes
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdSPI_Flash_OpStat_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 2;
	unsigned const CmdType = SPI_FLASH_OP_STATUS_CMD_TYPE;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct CmdDefectMap_write_t {
	const unsigned SOP = 0xCC55AA33;
	unsigned PktId;
	const unsigned PayloadLen = 4;
	unsigned const CmdType = DEFECT_MAP_WRITE_CMD_TYPE;
	unsigned defect_size;
	unsigned defect_chk_sum;
	unsigned ChkSum;
	const unsigned EOP = 0x00FF00FF;
};

struct defect_pixel_t {
	unsigned int x;
	unsigned int y;
	unsigned int defect_type;
};

struct defect_map_t
{
	unsigned int defect_size;//<MAX_DEFECT_SIZE
	unsigned int chksum;
	defect_pixel_t defect_pixel[MAX_DEFECT_SIZE];
};





struct rawgen_t {
	unsigned pixel_width;
	unsigned pixel_height;
	unsigned exposure_us;
	unsigned date;
	unsigned time;
	unsigned flash;
	unsigned imager_type;
	const unsigned cfa_pattern = 1;//BGGR
	const unsigned bit_depth = 10;
	unsigned byte_order;//unused
	unsigned const justification = 0; //?
	unsigned WB1 = 65536;//RED , gain=WB1/65536
	unsigned WB2 = 65536;//GREEN
	unsigned WB3 = 65536;//BLUE
	unsigned ISO;
	unsigned pixel_tpye = 0;//bayer
	unsigned timing_width; //entier frame width in pixel clocks
	unsigned timing_height; //entire frame height in lines
	unsigned reserved1;
	unsigned dark_level = 0;
	unsigned sensor_gain = 65536; //total system gain
	unsigned paddint_128[11]; //44bytes
#if DECIMATION_RATIO==0 
	unsigned img_data[XROI_WIDTH*YROI_HEIGHT * 2 / 4];
#else
	unsigned img_data[XROI_WIDTH*YROI_HEIGHT * 2 / 4 / DECIMATION_RATIO];
#endif

};


#pragma pack(pop)

/*
struct rawgen_t {
	unsigned pixel_width;
	unsigned pixel_height;
	unsigned exposure_us;
	unsigned date;
	unsigned time;
	unsigned flash;
	unsigned imager_type;
	const unsigned cfa_pattern = 1;//BGGR
	const unsigned bit_depth = 10;
	unsigned byte_order;//unused
	unsigned const justification = 0; //?
	unsigned WB1 = 65536;//RED , gain=WB1/65536
	unsigned WB2 = 65536;//GREEN
	unsigned WB3 = 65536;//BLUE
	unsigned ISO;
	unsigned pixel_tpye = 0;//bayer
	unsigned timing_width; //entier frame width in pixel clocks
	unsigned timing_height; //entire frame height in lines
	unsigned reserved1;
	unsigned dark_level = 0;
	unsigned sensor_gain = 65536; //total system gain
	unsigned paddint_128[11]; //44bytes
#if DECIMATION_RATIO==0 
	unsigned img_data[XROI_WIDTH*YROI_HEIGHT * 2 / 4];
#else
	unsigned img_data[XROI_WIDTH*YROI_HEIGHT * 2 / 4/DECIMATION_RATIO];
#endif

};
*/