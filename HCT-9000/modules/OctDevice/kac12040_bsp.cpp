#include "stdafx.h"
#include"stdio.h"
#include "ftd3XX.h"
#include "kac12040_def.h"
#include "kac12040_bsp.h"
#include "CameraParam.h"


FT_HANDLE h_ft = nullptr;

CameraROI_t			CameraROI;
CaptureParam_t		CaptureParam;
TrigExposureParam_t TrigExposureParam;
StrbParam_t			StrbParam;
SensorGain_t		SensorGain;
SensorMode_t		SensorMode;
CompandingParam_t    CompandingParam;

CmdSetROI_t				CmdSetROI;
CmdCaptureParam_t		CmdCaptureParam;
CmdTrigExposureParam_t	CmdTrigExposureParam;
CmdStrbParam_t			CmdStrbParam;
CmdSensorGain_t			CmdSensorGain;
CmdGrabStart_t			CmdGrabStart;
CmdGrabStop_t			CmdGrabStop;
CmdFrameRequest_t		CmdFrameRequest;
CmdSensorRegW_t			CmdSensorRegW;
CmdSensorRegR_t			CmdSensorRegR;
CmdFlushImgPipe_t		CmdFlushImgPipe;
CmdDDR2Read_t			CmdDDR2Read;
CmdSPI_Flash_write_t	CmdSPI_flashWr;
CmdSPI_Flash_Read_t		CmdSPI_flashRd;
CmdSPI_Flash_OpStat_t	CmdSPI_Flash_OpStat;
CmdDefectMap_write_t    CmdDefectMap_Write;
CmdSPI_Flash_DirectRead_t CmdSPI_Flash_DirectRead;

defect_map_t			SensorDefectMap;
defect_map_t			LoadedDefectMap;


int CamRegWrite(unsigned Addr, unsigned data)
{
	CmdRegW_t cmd;
	FT_STATUS ret;
	unsigned long lenW;
	
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.Data = data;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	 ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdRegW_t), &lenW, 0);

	 if (FT_FAILED(ret)) return 1;
	 else return 0;
	
}

int CamRegRead(unsigned Addr)
{
	CmdRegR_t cmd;
	FT_STATUS ret;
	unsigned long lenW;
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));
	
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdRegR_t), &lenW, 0);

	if (FT_FAILED(ret))  return 1;
	else return 0;

	return true;
}

int CmdSensorRegRead(unsigned Addr)
{
	CmdSensorRegR_t cmd;
	FT_STATUS ret;
	unsigned long lenW;
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSensorRegR_t), &lenW, 0);

	if (FT_FAILED(ret))  return 1;
	else return 0;

	return true;
}

int CmdSensorRegWrite(unsigned Addr, unsigned data)
{
	CmdSensorRegW_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.Data = data;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSensorRegW_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}

int CmdFlushImagePipe(unsigned FlushOption)
{

	CmdFlushImgPipe_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.FlushOption = FlushOption|0x3;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdFlushImgPipe_t), &lenW, 0);
	if (FT_FAILED(ret)) return 1;
	else return 0;
	
}


void setUsbHandle(PVOID handle)
{
	h_ft = handle;
	return;
}


int init_camera(void)
{
	int iret;
	unsigned sensor_feedback_reg = 0;
	//ROI set
	
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, CONFIG_STATE);

	SetSensorMode(SHUTTER_MODE, FLIP_OPTION);
	
	iret=CamSetROI();
	if (iret) return 2;

	iret = CamSetCaptureParam();
	if (iret) return 3;

	iret = CamSetTrigExposure();
	if (iret) return 4;
	
	iret = CamStrbParam();
	if (iret) return 5;

	iret = CamSetSensorGain();
	if (iret) return 6;
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, IDLE_STATE);
	Sleep(60);//mandatory to wait sensor exit to IDLE_STATE:
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, SLAVE_INT_STATE);
	SensorRegRead(0x0719, &sensor_feedback_reg);

	return 0;
	
}


int CamSetROI(void)
{
	FT_STATUS ret;
	unsigned long lenW;

	if (DECIMATION_RATIO == 0)
	{
		if (XROI_WIDTH % 32) return 1; //invalid XROI_WIDTH
		if (YROI_HEIGHT % 2) return 2; //invalid YROI_HEIGHT
	}
	else if (DECIMATION_RATIO == 4)
	{
		if (XROI_WIDTH % 64) return 1; //invalid XROI_WIDTH
		if (YROI_HEIGHT % 4) return 2;
	}
	else if (DECIMATION_RATIO == 9)
	{
		if (XROI_WIDTH % 96) return 1; //invalid XROI_WIDTH
		if (YROI_HEIGHT % 6) return 2;
	}
	else return 3;

	if (XROI_START % 8) return 4; //should be 8n
	if (YROI_START % 8) return 5; //should be 8n

	CmdSetROI.PktId = 0;
	CmdSetROI.CameraROI.XROI_start = XROI_START;
	CmdSetROI.CameraROI.XROI_width = XROI_WIDTH;
	CmdSetROI.CameraROI.YROI_start = YROI_START;
	CmdSetROI.CameraROI.YROI_height = YROI_HEIGHT;
	CmdSetROI.CameraROI.decimationRatio = DECIMATION_RATIO;
	CmdSetROI.ChkSum = CalcChkSum((unsigned *)(&CmdSetROI));
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdSetROI, sizeof(CmdSetROI), &lenW, 0);

	if (FT_FAILED(ret)) return 6;
	else return 0;
}


int CamSetCaptureParam(void)
{
	FT_STATUS ret;
	unsigned long lenW;
	CmdCaptureParam.CaptureParam.CaptureFrameSize = CAPTURE_FRAME_SIZE;
	CmdCaptureParam.CaptureParam.CaptureMode = CAPTURE_FRAME_MODE;
	CmdCaptureParam.PktId = 0;
	CmdCaptureParam.ChkSum= CalcChkSum((unsigned *)(&CmdCaptureParam));

	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdCaptureParam, sizeof(CmdCaptureParam), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}


int CamSetTrigExposure(void)
{

	FT_STATUS ret;
	unsigned long lenW;
	
	CmdTrigExposureParam.TrigExposureParam.ExposureCnt = EXPOSURE_CNT_MS*100000;
	CmdTrigExposureParam.TrigExposureParam.ExposureOffCnt = EXPOSURE_OFF_CNT_MS* 100000;

	CmdTrigExposureParam.PktId = 0;
	CmdTrigExposureParam.ChkSum = CalcChkSum((unsigned *)(&CmdTrigExposureParam));

	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdTrigExposureParam, sizeof(CmdTrigExposureParam), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}

int CamStrbParam(void)
{

	FT_STATUS ret;
	unsigned long lenW;

	CmdStrbParam.StrbParam.StrbDelay = STRB_DELAY_CNT_MS * 390.625; //strb delay 1 cnt: 2.56us, 1ms=390.625*2.56us
	CmdStrbParam.StrbParam.StrbActTime = STRB_ACT_CNT_MS * 390.625;
	CmdStrbParam.StrbParam.StrbMode = STRB_PULSE_SHAPING; //STRB_PULSE_SHAPING,STRB_BYPASS
	CmdStrbParam.PktId = 0;
	CmdStrbParam.ChkSum = CalcChkSum((unsigned *)(&CmdStrbParam));

	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdStrbParam, sizeof(CmdStrbParam), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}


int CamSetSensorGain(void)
{
	FT_STATUS ret;
	unsigned long lenW;

	CmdSensorGain.SensorGain.Again = SENSOR_AGAIN;
	CmdSensorGain.SensorGain.Dgain = SENSOR_DGAIN;
	CmdSensorGain.PktId = 0;

	CmdSensorGain.ChkSum = CalcChkSum((unsigned *)(&CmdSensorGain));

	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdSensorGain, sizeof(CmdSensorGain), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}

int CamFrameRequest(void)
{
	FT_STATUS ret;
	unsigned long lenW;

	CmdFrameRequest.PktId = 0;
	CmdFrameRequest.ChkSum=CalcChkSum((unsigned *)(&CmdFrameRequest));
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdFrameRequest, sizeof(CmdFrameRequest), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}


int GetSensorID(unsigned int *SensorID)
{
	FT_STATUS ret;
	unsigned long lenW;
	unsigned RdDataLSB;
	unsigned RdDataMSB;
	unsigned rtn;

	rtn=SensorRegRead(SENSOR_CHIP_ID_MSB_REG, &RdDataMSB);
	if (rtn) return 1;
	rtn = SensorRegRead(SENSOR_CHIP_ID_LSB_REG, &RdDataLSB);
	if (rtn) return 2;
	*SensorID = (RdDataMSB<<16)|RdDataLSB;
	return 0;
}

int SensorRegRead(unsigned int addr,unsigned int *RdData)
{
	FT_STATUS ret;
	bool retBool;
	int lRet;
	int i, j;
	unsigned long lenRead = 0;
	unsigned  buf[256];
	
	CmdSensorRegRead(addr);
	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 28, &lenRead, 0);
	if (lenRead != 28) {
		
		printf("Sensor RegRead FT_ReadPipe() invalid lenRead=%d\n ", lenRead);
		return 1;
	}
	if (FT_FAILED(ret)) {
		
		printf("Sensor RegRead FT_ReadPipe() error : ret =%d\n ", ret);
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		
		printf("rx packet error=%d\n",lRet);
		return 3;
	}
	*RdData = buf[4];

	return 0;
}

int grab_start(void)
{
	FT_STATUS ret;
	unsigned long lenW;

	CmdGrabStart.PktId = 0;
	CmdGrabStart.ChkSum = CalcChkSum((unsigned *)(&CmdGrabStart));
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdGrabStart, sizeof(CmdGrabStart), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}

int grab_stop(void)
{
	FT_STATUS ret;
	unsigned long lenW;

	CmdGrabStop.PktId = 0;
	CmdGrabStop.ChkSum = CalcChkSum((unsigned *)(&CmdGrabStop));
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&CmdGrabStop, sizeof(CmdGrabStop), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}

unsigned CalcChkSum(unsigned *cmd)
{
	unsigned i;
	unsigned chksum = 0;
	for (i = 0; i < (cmd[2] +1); i++)
	{
		chksum += cmd[1 + i];
	}

	return chksum;
}


int CheckRxPacket(unsigned *buf, int len)
{
	int i;
	unsigned chksum;
	chksum = 0;
	if (buf[0] != 0xCC55AA33) { printf("invalid sop\n"); return 1; }
	if (buf[len - 1] != 0x00FF00FF) { printf("invalid eop\n"); return 2; }
	for (i = 0; i < len - 3; i++)
	{
		chksum += buf[i + 1];
	}
	if (chksum != buf[len - 2]) { printf("invalid chksum\n"); return 3; }
	return 0;
}

#define MFSO_MULTIPLIER 16

void SetSensorFLOTiming(void)
{
	unsigned fedge_ref;
	unsigned redge_ref;
	unsigned strb_width=1500;
	float sensor_read_out_time;
	unsigned cfg_data;
	
//	CmdSensorRegWrite(SENSOR_STATEL_SEL_REG, CONFIG_STATE);

	
	redge_ref = 0/MFSO_MULTIPLIER; //1500 line time
	//redge_ref = 10;
	//fedge_ref = (redge_ref + strb_width);
	fedge_ref = 0/MFSO_MULTIPLIER;
	//CmdSensorRegWrite(SENSOR_CONFIG2_REG, FACTORY_GCAL_ACT|MFSO_DELAY_MULT_X32| MFSO_FDREF_READOUT);
	cfg_data = FACTORY_GCAL_ACT | MFSO_DELAY_MULT_X16 | MFSO_RDREF_INT;
	CmdSensorRegWrite(SENSOR_CONFIG2_REG, cfg_data);
	CmdSensorRegWrite(SENSOR_FLO_EDGE_DLY_REG, (fedge_ref << 8) | redge_ref); //delay=RegValue*LineLength [15:8]: FLO falling edge delay, [7:0]: Rising edge delay 
//	CmdSensorRegWrite(SENSOR_STATEL_SEL_REG, SLAVE_INT_STATE);
	Sleep(100);
}

void SetSensorConfig1(unsigned config1)
{
//	CmdSensorRegWrite(SENSOR_STATEL_SEL_REG, CONFIG_STATE);
	CmdSensorRegWrite(SENSOR_CONFIG1_REG, config1);
//	CmdSensorRegWrite(SENSOR_STATEL_SEL_REG, SLAVE_INT_STATE);
}

void SetSensorMode(unsigned shutterMode, unsigned flip ) //shutterMode:ROLLING_SHUTTER/GLOBAL_SHUTTER, flip:NO_FLIP/HORIZONTAL_FLIP/VERTICAL_FLIP
{
	unsigned cfg1;
	
	SensorMode.shutter_mode = shutterMode;
	SensorMode.flip = flip;
	
	cfg1 = SensorMode.default_cfg1 | SensorMode.flip | SensorMode.ovelapped_int_en | SensorMode.shutter_mode;
	SetSensorConfig1(cfg1);
	Sleep(100);//abnormaly workaround,
}

void FlushImagePipe(void)
{
	grab_stop();
	CmdFlushImagePipe(MEM_WR_RESET | MEM_RD_RESET | FRAME_BUF_RESET);
}

int GetCameraVersion(float *version)
{
	unsigned rversion;
	FpgaRegRead(CAM_VERSION_REG,&rversion);
	*version = rversion / 100.0;//
	printf("camera version=%f\n", *version);
	return 0;
}



int FpgaRegRead(unsigned int addr, unsigned int *RdData)
{
	FT_STATUS ret;
	bool retBool;
	int lRet;
	int i, j;
	unsigned long lenRead = 0;
	unsigned  buf[256];

	CamRegRead(addr);
	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 28, &lenRead, 0);
	if (lenRead != 28) {

		printf("FPGA RegRead FT_ReadPipe() invalid lenRead=%d\n ", lenRead);
		return 1;
	}
	if (FT_FAILED(ret)) {

		printf("FPGA RegRead FT_ReadPipe() error : ret =%d\n ", ret);
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {

		printf("%d th rx packet error=%d\n", lRet);
		return 3;
	}
	*RdData = buf[4];

	return 0;
}



void SetCompandingMode(void)
{
	int i;
	int CompandingSlopeGainReg[4];
	//
	CompandingParam.SlopeSize	= COMPANDING_SLOPE_SIZE;
	CompandingParam.SlopeLen[0] = COMPANDING_SLOPE1_LEN;
	CompandingParam.SlopeLen[1] = COMPANDING_SLOPE2_LEN;
	CompandingParam.SlopeLen[2] = COMPANDING_SLOPE3_LEN;
	CompandingParam.SlopeLen[3] = COMPANDING_SLOPE4_LEN;
	CompandingParam.SlopeLen[4] = COMPANDING_SLOPE5_LEN;
	CompandingParam.SlopeLen[5] = COMPANDING_SLOPE6_LEN;
	
	CompandingParam.SlopeGain[0] = COMPANDING_SLOPE1_GAIN;
	CompandingParam.SlopeGain[1] = COMPANDING_SLOPE2_GAIN;
	CompandingParam.SlopeGain[2] = COMPANDING_SLOPE3_GAIN;
	CompandingParam.SlopeGain[3] = COMPANDING_SLOPE4_GAIN;
	CompandingParam.SlopeGain[4] = COMPANDING_SLOPE5_GAIN;
	CompandingParam.SlopeGain[5] = COMPANDING_SLOPE6_GAIN;
	CompandingParam.SlopeGain[6] = COMPANDING_SLOPE6_GAIN; 

	//if 6(0~5) slopes used, slopeGain[6] should be equal to slopeGain[5], if slope size N<=5, N+1 th slope len should be set to zero

	for (i = CompandingParam.SlopeSize; i < 6;i++)
	{
		CompandingParam.SlopeLen[i] = 0;
	}

	
	CompandingParam.SlopeGain[CompandingParam.SlopeSize] = CompandingParam.SlopeGain[CompandingParam.SlopeSize - 1];

	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, CONFIG_STATE);

	CmdSensorRegWrite(SENSOR_SLOPE1_LEN, CompandingParam.SlopeLen[0]);
	CmdSensorRegWrite(SENSOR_SLOPE2_LEN, CompandingParam.SlopeLen[1]);
	CmdSensorRegWrite(SENSOR_SLOPE3_LEN, CompandingParam.SlopeLen[2]);
	CmdSensorRegWrite(SENSOR_SLOPE4_LEN, CompandingParam.SlopeLen[3]);
	CmdSensorRegWrite(SENSOR_SLOPE5_LEN, CompandingParam.SlopeLen[4]);
	CmdSensorRegWrite(SENSOR_SLOPE6_LEN, CompandingParam.SlopeLen[5]);
	
	CompandingSlopeGainReg[0] = ((CompandingParam.SlopeGain[0]) << 7) | CompandingParam.SlopeGain[1];
	CompandingSlopeGainReg[1] = ((CompandingParam.SlopeGain[2]) << 7) | CompandingParam.SlopeGain[3];
	CompandingSlopeGainReg[2] = ((CompandingParam.SlopeGain[4]) << 7) | CompandingParam.SlopeGain[5];
	CompandingSlopeGainReg[3] = ((CompandingParam.SlopeGain[6]) << 7) | 31;

	CmdSensorRegWrite(SENSOR_SLOPE12_GAIN, CompandingSlopeGainReg[0]);
	CmdSensorRegWrite(SENSOR_SLOPE34_GAIN, CompandingSlopeGainReg[1]);
	CmdSensorRegWrite(SENSOR_SLOPE56_GAIN, CompandingSlopeGainReg[2]);
	CmdSensorRegWrite(SENSOR_SLOPE7_GAIN, CompandingSlopeGainReg[3]);
	CmdSensorRegWrite(SENSOR_CONFIG2_REG, FACTORY_GCAL_ACT|COMPAND_EN);


	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, IDLE_STATE);
	Sleep(60);//mandatory to wait sensor exit to IDLE_STATE:
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, SLAVE_INT_STATE);
	



}

void DisableCompanding(void)
{

	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, CONFIG_STATE);
	CmdSensorRegWrite(SENSOR_CONFIG2_REG, FACTORY_GCAL_ACT);
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, IDLE_STATE);
	Sleep(60);//mandatory to wait sensor exit to IDLE_STATE:
	CmdSensorRegWrite(SENSOR_STATE_SEL_REG, SLAVE_INT_STATE);

}
#define MAX_DATA_BYTE_SIZE 256
int CmdDDR2_Write(unsigned pkt_id, unsigned cam_mem_addr,unsigned cam_memc,unsigned size_bytes, unsigned *Data)
{
	unsigned pkt_buf[(MAX_DATA_BYTE_SIZE/4) + 16];
	unsigned data_word_size;
	const unsigned SOP = 0xCC55AA33;
	const unsigned EOP = 0x00FF00FF;
	

	FT_STATUS ret;
	unsigned long lenW;
	

	if ((size_bytes > MAX_DATA_BYTE_SIZE)||(size_bytes==0)) return 1;
	

	if (size_bytes % 4) data_word_size = (size_bytes / 4) + 1;
	else data_word_size = (size_bytes / 4);

	pkt_buf[0]	= SOP;
	pkt_buf[1]	= pkt_id; //pkt_id, 
	pkt_buf[2] = data_word_size+5;
	pkt_buf[3] = DDR2_WR_CMD_TYPE;
	pkt_buf[4] = cam_mem_addr;
	pkt_buf[5] = cam_memc;
	pkt_buf[6] = size_bytes;
	memcpy(&pkt_buf[7], Data, size_bytes);
	pkt_buf[7+data_word_size]=CalcChkSum(pkt_buf); //
	pkt_buf[7+data_word_size + 1] = EOP;  //total packet size in words:9+data_word_size
		
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)pkt_buf,(9+data_word_size)*4, &lenW, 0);
	if (FT_FAILED(ret)) return 2;
	return 0;
}

int CmdDDR2_Read(unsigned pkt_id, unsigned cam_mem_addr, unsigned cam_memc, unsigned size_bytes)
{
	CmdDDR2Read_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.Addr = cam_mem_addr;
	cmd.memc = cam_memc;
	cmd.data_size_bytes = size_bytes;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdDDR2Read_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;
}

int GetDefaultCmdAck(unsigned cmd_type)
{
	FT_STATUS ret;
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[256];


	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 24, &lenRead, 0);
	if (lenRead != 24) {

		return 1;
	}
	if (FT_FAILED(ret)) {
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		return 3;
	}
	if (buf[3] != cmd_type) return 4;

	return 0;
}


int DDR2_Memory_Write(unsigned cam_mem_addr, unsigned cam_memc, unsigned size_bytes, unsigned *Data)
{
	unsigned pkt_id=0;
	unsigned mem_addr;
	unsigned blk_size;
	unsigned *src_addr;
	int irtn;
	mem_addr = cam_mem_addr;
	src_addr = Data;

	while (mem_addr < (cam_mem_addr + size_bytes))
	{
		if ((cam_mem_addr + size_bytes - mem_addr) < MAX_DATA_BYTE_SIZE) blk_size = cam_mem_addr + size_bytes - mem_addr;
		else blk_size = MAX_DATA_BYTE_SIZE;
		irtn=CmdDDR2_Write(pkt_id++,mem_addr, cam_memc,blk_size,src_addr);
		if (irtn) return 1;
		irtn = GetDefaultCmdAck(DDR2_WR_CMD_TYPE);
		if (irtn) return 2;

		mem_addr += blk_size;
		src_addr += blk_size/4; //integer type 1 cnt: 4 address increment
	}

	return 0;

}

int DDR2_Memory_Read(unsigned cam_mem_addr, unsigned cam_memc, unsigned size_bytes, unsigned *Data)
{
	unsigned pkt_id = 0;
	unsigned mem_addr;
	unsigned blk_size;
	unsigned *dst_addr;
	int irtn;
	mem_addr = cam_mem_addr;
	dst_addr = Data;

	while (mem_addr < (cam_mem_addr + size_bytes))
	{
		if ((cam_mem_addr + size_bytes - mem_addr) < MAX_DATA_BYTE_SIZE) blk_size = cam_mem_addr + size_bytes - mem_addr;
		else blk_size = MAX_DATA_BYTE_SIZE;
		irtn = CmdDDR2_Read(pkt_id++, mem_addr, cam_memc, blk_size);
		if (irtn)
			return 1;
		irtn = Get_DDR2_read_response(blk_size, dst_addr);
		if (irtn)
			return 2;

		mem_addr += blk_size;
		dst_addr += blk_size/4;
	}

	return 0;

}

//Ack for minimal type command ack: DDR2 Write ack.... other CMDs for  needs basic cmd acknowledge...



int Get_DDR2_read_response(unsigned data_byte_size,unsigned *Data)
{
	FT_STATUS ret;
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[16+ MAX_DATA_BYTE_SIZE];
	unsigned aligned_word_size;

	if (data_byte_size % 4) aligned_word_size = (data_byte_size / 4) + 1;
	else aligned_word_size = (data_byte_size / 4);



	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, (aligned_word_size+6)*4, &lenRead, 0);
	if (lenRead != (aligned_word_size + 6) * 4) {
		return 1;
	}
	if (FT_FAILED(ret)) {
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		return 3;
	}
	if (buf[3] != DDR2_READ_RESP_CMD_TYPE) return 4;


	memcpy(Data, (unsigned  *)(buf + 4), data_byte_size);

	return 0;
}

int Get_SPI_Flash_Direct_read_response(unsigned data_byte_size, unsigned *Data)
{
	FT_STATUS ret;
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[16 + MAX_DATA_BYTE_SIZE];
	unsigned aligned_word_size;

	if (data_byte_size % 4) aligned_word_size = (data_byte_size / 4) + 1;
	else aligned_word_size = (data_byte_size / 4);



	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, (aligned_word_size + 6) * 4, &lenRead, 0);
	if (lenRead != (aligned_word_size + 6) * 4) {
		return 1;
	}
	if (FT_FAILED(ret)) {
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		return 3;
	}
	if (buf[3] != SPI_FLASH_DIRECT_READ_CMD_TYPE) return 4;


	memcpy(Data, (unsigned  *)(buf + 4), data_byte_size);

	return 0;
}


int ConfigFileDownload(char *src_buf, char *verify_buf,unsigned data_size_byte)
{
	int iRtn;

	iRtn = DownloadToCameraMemory(src_buf,verify_buf,data_size_byte);
	return iRtn;

}

int DownloadToCameraMemory(char *src_buf, char *verify_buf, unsigned data_size_byte)
{
	
	int iRtn;
	int i;
	
	iRtn = DDR2_Memory_Write(0, 0, data_size_byte, (unsigned int *)src_buf);
	if (iRtn) { printf("DDR2 memory write error=%d\r\n", iRtn); return 2; }

	iRtn = DDR2_Memory_Read(0, 0, data_size_byte, (unsigned int *)verify_buf);
	if (iRtn) {
		printf("DDR2 memory read error=%d\r\n", iRtn);
		return 3;
	}

	for (i = 0; i < data_size_byte / 4; i++)
	{
		if ((*(int *)(src_buf + i)) != (*(int *)(verify_buf + i))) { printf("download verify error\r\n");  return 4; }
	}
	printf("download ok\n");
	return 0;

}


int ProgramSPI_Flash(char *src_buf,char *verify_buf, int cam_mem_addr, int data_size_byte)
{
	int irtn;
	int i;
	
	irtn = Cmd_SPI_FlashWrite(0, data_size_byte);
	if (irtn) return 1;
	irtn=GetDefaultCmdAck(SPI_FLASH_WR_CMD_TYPE);
	if (irtn) return 2;

	CheckFlashStatus();

	irtn = Cmd_SPI_FlashRd(0x800000, data_size_byte);
	if (irtn) return 3;
	irtn = GetDefaultCmdAck(SPI_FLASH_RD_CMD_TYPE);
	if (irtn) return 4;

	printf("uploading flash data..\r\n");
	irtn = DDR2_Memory_Read(0, 0, data_size_byte, (unsigned int *)verify_buf);
	if (irtn) {
		printf("DDR2 memory read error=%d\r\n", irtn);
		return 3;
	}
	
	printf("flash verify....");
	for (i = 0; i < data_size_byte / 4; i++)
	{
		if ((*(int *)(src_buf + i)) != (*(int *)(verify_buf + i))) { printf("error\r\n");  return 4; }
	}
	printf("ok\r\n");
	return 0;




	return 0;

}

int Cmd_SPI_FlashWrite(unsigned src_Addr, unsigned data_size_bytes)
{
	CmdSPI_Flash_write_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.src_Addr = src_Addr;
	cmd.data_size_bytes = data_size_bytes;

	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSPI_Flash_write_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}

//dst addr: DDR2 0 memory addres for flash data load
int Cmd_SPI_FlashRd(unsigned dst_Addr, unsigned data_size_bytes)
{
	CmdSPI_Flash_Read_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.dst_Addr = dst_Addr;
	cmd.data_size_bytes = data_size_bytes;

	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSPI_Flash_Read_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}

int Cmd_SPI_FlashDirectRd(unsigned pktid,unsigned flash_rd_Addr, unsigned data_size_bytes)
{
	CmdSPI_Flash_DirectRead_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	if (data_size_bytes > 256) return 1;

	cmd.PktId = pktid;
	cmd.flash_rd_Addr = flash_rd_Addr;
	cmd.data_size_bytes = data_size_bytes;

	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSPI_Flash_DirectRead_t), &lenW, 0);

	if (FT_FAILED(ret)) return 2;
	else return 0;

}

int Cmd_Get_SPI_Flash_OpStatus(void)
{
	CmdSPI_Flash_OpStat_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSPI_Flash_OpStat_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	else return 0;

}

int Get_SPI_Flash_StatusCmdAck(unsigned *OpStatus, unsigned *ProgressCnt)
{
	FT_STATUS ret;
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[256];


	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 32, &lenRead, 0);
	if (lenRead != 32) {

		return 1;
	}
	if (FT_FAILED(ret)) {
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		return 3;
	}
	if (buf[3] != SPI_FLASH_OP_STATUS_CMD_TYPE) return 4;
	*OpStatus = buf[4];
	*ProgressCnt = buf[5];

	return 0;
}

int CheckFlashStatus(void)
{

	unsigned OpStatus;
	unsigned ProgressCnt;
	int irtn;

	Sleep(200);
	do {
		irtn = Cmd_Get_SPI_Flash_OpStatus();
		if (irtn) return 1;
		irtn = Get_SPI_Flash_StatusCmdAck(&OpStatus, &ProgressCnt);
		if (irtn) return 2;
		if(OpStatus==FLASH_ERASE_ST) printf("\rflash erasing..%d sec", ProgressCnt);
	} while (OpStatus==FLASH_ERASE_ST);

	printf("\r\nerase done.\r\n");

	do {
		irtn = Cmd_Get_SPI_Flash_OpStatus();
		if (irtn) return 3;
		irtn = Get_SPI_Flash_StatusCmdAck(&OpStatus, &ProgressCnt);
		if (irtn) return 4;
		if(OpStatus==FLASH_WRITE_ST) printf("\rflash programming %d%%", ProgressCnt);
	} while (OpStatus == FLASH_WRITE_ST);
	printf("\rflash programming 100%%\r\n");
	printf("\please wait...\r\n");
	return 0;
	
}


int SaveDefectMapToCam(int defect_size, defect_map_t* defect_map)
{
	int iRtn;
	int i;
	defect_map_t verifyMap;
	unsigned int defect_map_size;
	unsigned int chksum = 0;


	defect_map_size = (defect_map->defect_size) *12 + 8;// each defect map size:12 byte, defect_size 4byte, chksum 4 byte
	
	
	(defect_map->chksum) = CalcDefectMapChksum(defect_size, defect_map->defect_pixel);
	iRtn=DownloadToCameraMemory((char *)defect_map, (char *)(&verifyMap), defect_map_size);//download to camera DDR2 memory and upload to PC, then verify

	if (iRtn) return 1;
	iRtn=Cmd_DefectMap_Write(defect_map->defect_size, defect_map->chksum);
	if (iRtn) return 2;
	Sleep(1000);
	iRtn = GetCmdAckWstatus(DEFECT_MAP_WRITE_CMD_TYPE);
	if (iRtn) return 3;

	return 0;
}

unsigned int CalcDefectMapChksum(int defect_size, defect_pixel_t* defect_pixel)
{
	int i;
	unsigned int chksum=0;

	for (i = 0; i < defect_size; i++)
	{
		chksum = chksum + (defect_pixel[i]).defect_type + (defect_pixel[i]).x + (defect_pixel[i]).y;
	}

	return chksum;

}

int Cmd_DefectMap_Write(unsigned defect_size, unsigned defect_map_chksum)
{
	CmdDefectMap_write_t cmd;
	FT_STATUS ret;
	unsigned long lenW;

	cmd.PktId = 0;
	cmd.defect_size = defect_size;
	cmd.defect_chk_sum = defect_map_chksum;

	cmd.ChkSum = CalcChkSum((unsigned *)(&cmd));

	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdDefectMap_write_t), &lenW, 0);

	if (FT_FAILED(ret)) return 1;
	

	return 0;

}

//Get command acknowledge with status.28bytes expected
int GetCmdAckWstatus(unsigned cmd_type)
{
	FT_STATUS ret;
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[256];


	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 28, &lenRead, 0);
	if (lenRead != 28) {

		return 1;
	}
	if (FT_FAILED(ret)) {
		return 2;
	}

	lRet = CheckRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		return 3;
	}
	if (buf[3] != cmd_type) return 4;
	if (buf[4] != CMD_ACK_STATUS_OK) return 5;

	return 0;
}

int SPI_Flash_DirectRead(unsigned  flash_read_addr,unsigned size_bytes, unsigned *Data)
{
	unsigned pkt_id = 0;
	unsigned mem_addr;
	unsigned blk_size;
	unsigned *dst_addr;
	int irtn;
	mem_addr = flash_read_addr;
	dst_addr = Data;

	while (mem_addr < (flash_read_addr + size_bytes))
	{
		if ((flash_read_addr + size_bytes - mem_addr) < MAX_DATA_BYTE_SIZE) blk_size = flash_read_addr + size_bytes - mem_addr;
		else blk_size = MAX_DATA_BYTE_SIZE;

		irtn = Cmd_SPI_FlashDirectRd(pkt_id++,mem_addr, blk_size);
		
		if (irtn)
			return 1;
		
		irtn = Get_SPI_Flash_Direct_read_response(blk_size, dst_addr);
		if (irtn)
			return 2;

		mem_addr += blk_size;
		dst_addr += blk_size / 4;
	}

	return 0;
}

int GetDefectMap(defect_map_t* defect_map)
{
	unsigned buf[16];
	unsigned CalcChksum;
	int iRtn;

	iRtn=SPI_Flash_DirectRead(SPI_FLASH_DEFECT_MAP_BASE,8, buf);
	if (iRtn) return 1;
	defect_map->defect_size	= buf[0];
	defect_map->chksum		= buf[1];
	if ((defect_map->defect_size) > MAX_DEFECT_SIZE) return 2;

	iRtn = SPI_Flash_DirectRead(SPI_FLASH_DEFECT_MAP_BASE+8, (defect_map->defect_size)*12, (unsigned int *)(defect_map->defect_pixel));
	if (iRtn) return 3;

	CalcChksum=CalcDefectMapChksum(defect_map->defect_size, defect_map->defect_pixel);
	if (CalcChksum != (defect_map->chksum)) return 4;

	return 0;

}

