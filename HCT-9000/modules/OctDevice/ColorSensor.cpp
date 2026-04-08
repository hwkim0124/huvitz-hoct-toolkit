#include "stdafx.h"
#include "ColorSensor.h"
#include "UsbPort.h"

#include "kac12040_bsp.h"

#include "CppUtil2.h"

using namespace OctDevice;
using namespace CppUtil;
using namespace std;


extern defect_map_t	SensorDefectMap;


struct ColorSensor::ColorSensorImpl
{
	UsbPort usbPort;

	ColorSensorImpl()
	{
	}
};


ColorSensor::ColorSensor() :
	d_ptr(make_unique<ColorSensorImpl>())
{
}


OctDevice::ColorSensor::ColorSensor(MainBoard * board) :
	d_ptr(make_unique<ColorSensorImpl>())
{
}


ColorSensor::~ColorSensor()
{
}


bool OctDevice::ColorSensor::initialize(void)
{
	bool ret = getUsbPort().open(COLOR_SENSOR_USB_DESC);
	return ret;
}


void OctDevice::ColorSensor::release(void)
{
	getUsbPort().close();
	return;
}


int OctDevice::ColorSensor::camRegWrite(unsigned Addr, unsigned data)
{
	CmdRegW_t cmd;

	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.Data = data;
	cmd.ChkSum = calcChkSum((unsigned *)(&cmd));

	if (!getUsbPort().send((unsigned char *)&cmd, sizeof(CmdRegW_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdRegW_t), &lenW, 0);
	if (FT_FAILED(ret)) return 1;
	*/
	else return 0;
}


int OctDevice::ColorSensor::camRegRead(unsigned Addr)
{
	CmdRegR_t cmd;
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.ChkSum = calcChkSum((unsigned *)(&cmd));

	if (!getUsbPort().send((unsigned char *)&cmd, sizeof(CmdRegR_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdRegR_t), &lenW, 0);
	if (FT_FAILED(ret))  return 1;
	*/
	else return 0;
}


int OctDevice::ColorSensor::cmdSensorRegRead(unsigned Addr)
{
	CmdSensorRegR_t cmd;
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.ChkSum = calcChkSum((unsigned *)(&cmd));

	if (!getUsbPort().send((unsigned char *)&cmd, sizeof(CmdSensorRegR_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSensorRegR_t), &lenW, 0);
	if (FT_FAILED(ret))  return 1;
	*/
	else return 0;
}


int OctDevice::ColorSensor::cmdSensorRegWrite(unsigned Addr, unsigned data)
{
	CmdSensorRegW_t cmd;
	cmd.PktId = 0;
	cmd.Addr = Addr;
	cmd.Data = data;
	cmd.ChkSum = calcChkSum((unsigned *)(&cmd));

	if (!getUsbPort().send((unsigned char *)&cmd, sizeof(CmdSensorRegW_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdSensorRegW_t), &lenW, 0);
	if (FT_FAILED(ret)) return 1;
	*/
	else return 0;
}


int OctDevice::ColorSensor::cmdFlushImagePipe(unsigned FlushOption)
{
	CmdFlushImgPipe_t cmd;

	cmd.PktId = 0;
	cmd.FlushOption = FlushOption | 0x3;
	cmd.ChkSum = calcChkSum((unsigned *)(&cmd));

	if (!getUsbPort().send((unsigned char *)&cmd, sizeof(CmdFlushImgPipe_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	// send command to endpoint 0x02
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmd, sizeof(CmdFlushImgPipe_t), &lenW, 0);
	if (FT_FAILED(ret)) return 1;
	*/
	else return 0;
}


unsigned int OctDevice::ColorSensor::calcChkSum(unsigned int * cmd)
{
	unsigned int i;
	unsigned int chksum = 0;
	for (i = 0; i < (cmd[2] + 1); i++)
	{
		chksum += cmd[1 + i];
	}
	return chksum;
}


int OctDevice::ColorSensor::checkRxPacket(unsigned int * buf, int len)
{
	int i;
	unsigned int chksum;
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


int OctDevice::ColorSensor::sensorRegRead(unsigned int addr, unsigned int * RdData)
{

	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[256];

	cmdSensorRegRead(addr);

	if (!getUsbPort().receive((unsigned char*)buf, 28)) {
		return 1;
	}
	lenRead = 28;
	/*
	FT_STATUS ret;
	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 28, &lenRead, 0);
	if (FT_FAILED(ret)) {
		printf("Sensor RegRead FT_ReadPipe() error : ret =%d\n ", ret);
		return 2;
	}
	*/

	if (lenRead != 28) {
		printf("Sensor RegRead FT_ReadPipe() invalid lenRead=%d\n ", lenRead);
		return 2;
	}

	lRet = checkRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		printf("%d th rx packet error=%d\n", addr, lRet);
		return 3;
	}
	*RdData = buf[4];
	return 0;
}


int OctDevice::ColorSensor::fpgaRegRead(unsigned int addr, unsigned int * RdData)
{
	int lRet;
	unsigned long lenRead = 0;
	unsigned  buf[256];

	camRegRead(addr);

	if (!getUsbPort().receive((unsigned char*)buf, 28)) {
		return 1;
	}
	lenRead = 28;
	/*
	FT_STATUS ret;
	ret = FT_ReadPipe(h_ft, 0x82, (UCHAR *)buf, 28, &lenRead, 0);
	if (FT_FAILED(ret)) {
	TRACE(_T("FPGA RegRead FT_ReadPipe() error : ret =%d\n"), ret);
	return 2;
	}
	*/

	if (lenRead != 28) {
		TRACE(_T("FPGA RegRead FT_ReadPipe() invalid lenRead=%d\n"), lenRead);
		return 2;
	}

	lRet = checkRxPacket(buf, (lenRead) / 4);
	if (lRet) {
		TRACE(_T("%d th rx packet error=%d\n"), lRet);
		return 3;
	}
	*RdData = buf[4];
	return 0;
}


bool OctDevice::ColorSensor::readFrame(unsigned char * data, unsigned int size)
{
	unsigned char epid = 0x83;
	if (getUsbPort().receive(epid, data, size)) {
		return true;
	}
	DebugOut2() << "readFrame() failed!, epid = " << epid;
	return false;
}


void OctDevice::ColorSensor::cmdUpdateStart(void)
{
	cmdSensorRegWrite(SENSOR_STATE_SEL_REG, CONFIG_STATE);
	return;
}


void OctDevice::ColorSensor::cmdUpdateClose(void)
{
	unsigned sensor_feedback_reg = 0;
	cmdSensorRegWrite(SENSOR_STATE_SEL_REG, IDLE_STATE);
	Sleep(60);//mandatory to wait sensor exit to IDLE_STATE:
	cmdSensorRegWrite(SENSOR_STATE_SEL_REG, SLAVE_INT_STATE);
	sensorRegRead(0x0719, &sensor_feedback_reg);
	return;
}


int OctDevice::ColorSensor::cmdFrameRequest(void)
{
	CmdFrameRequest_t cmdFrameRequest;
	cmdFrameRequest.PktId = 0;
	cmdFrameRequest.ChkSum = calcChkSum((unsigned *)(&cmdFrameRequest));

	if (!getUsbPort().send((unsigned char *)&cmdFrameRequest, sizeof(CmdFrameRequest_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdFrameRequest, sizeof(CmdFrameRequest_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdSetROI(const CameraROI_t & param)
{
	CmdSetROI_t cmdSetROI;
	cmdSetROI.PktId = 0;
	cmdSetROI.CameraROI = param;
	cmdSetROI.ChkSum = calcChkSum((unsigned int*)(&cmdSetROI));

	if (!getUsbPort().send((unsigned char *)&cmdSetROI, sizeof(CmdSetROI_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdSetROI, sizeof(CmdSetROI_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 6;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdCaptureParam(const CaptureParam_t & param)
{
	CmdCaptureParam_t cmdCaptureParam;
	cmdCaptureParam.PktId = 0;
	cmdCaptureParam.CaptureParam = param;
	cmdCaptureParam.ChkSum = calcChkSum((unsigned int*)(&cmdCaptureParam));

	if (!getUsbPort().send((unsigned char *)&cmdCaptureParam, sizeof(CmdCaptureParam_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdCaptureParam, sizeof(CmdCaptureParam_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdTrigExposure(const TrigExposureParam_t & param)
{
	CmdTrigExposureParam_t cmdTrigExposureParam;
	cmdTrigExposureParam.PktId = 0;
	cmdTrigExposureParam.TrigExposureParam = param;
	cmdTrigExposureParam.ChkSum = calcChkSum((unsigned int*)(&cmdTrigExposureParam));

	if (!getUsbPort().send((unsigned char *)&cmdTrigExposureParam, sizeof(CmdTrigExposureParam_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdTrigExposureParam, sizeof(CmdTrigExposureParam_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdStrobeParam(const StrbParam_t & param)
{
	CmdStrbParam_t cmdStrbParam;
	cmdStrbParam.PktId = 0;
	cmdStrbParam.StrbParam = param;
	cmdStrbParam.ChkSum = calcChkSum((unsigned int*)(&cmdStrbParam));

	if (!getUsbPort().send((unsigned char *)&cmdStrbParam, sizeof(CmdStrbParam_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdStrbParam, sizeof(CmdStrbParam_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdSensorGain(const SensorGain_t &param)
{
	CmdSensorGain_t cmdSensorGain;
	cmdSensorGain.PktId = 0;
	cmdSensorGain.SensorGain = param;
	cmdSensorGain.ChkSum = calcChkSum((unsigned int*)(&cmdSensorGain));
	
	if (!getUsbPort().send((unsigned char *)&cmdSensorGain, sizeof(CmdSensorGain_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdSensorGain, sizeof(CmdSensorGain_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdGrabStart(void)
{
	CmdGrabStart_t cmdGrabStart;
	cmdGrabStart.PktId = 0;
	cmdGrabStart.ChkSum = calcChkSum((unsigned *)(&cmdGrabStart));

	if (!getUsbPort().send((unsigned char *)&cmdGrabStart, sizeof(CmdGrabStart_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdGrabStart, sizeof(CmdGrabStart_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::cmdGrabStop(void)
{
	CmdGrabStop_t cmdGrabStop;
	cmdGrabStop.PktId = 0;
	cmdGrabStop.ChkSum = calcChkSum((unsigned *)(&cmdGrabStop));

	if (!getUsbPort().send((unsigned char *)&cmdGrabStop, sizeof(CmdGrabStop_t))) {
		return 1;
	}
	/*
	unsigned long lenW;
	FT_STATUS ret;
	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdGrabStop, sizeof(CmdGrabStop_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 1;
	}
	*/
	return 0;
}


int OctDevice::ColorSensor::setSensorMode(const SensorMode_t & param)
{
	unsigned cfg1;
	// cmdSensorRegWrite(SENSOR_STATE_SEL_REG, CONFIG_STATE);

	cfg1 = param.default_cfg1 | param.flip | param.ovelapped_int_en | param.shutter_mode;
	setSensorConfig1(cfg1);
	Sleep(100);//abnormaly workaround,
	return 0;
}


int OctDevice::ColorSensor::setSensorConfig1(unsigned config1)
{
	cmdSensorRegWrite(SENSOR_CONFIG1_REG, config1);
	return 0;
}


int OctDevice::ColorSensor::setROI(const CameraROI_t & param)
{
	if (param.decimationRatio == 0)
	{
		if (param.XROI_width % 32) return 1; //invalid XROI_WIDTH
		if (param.YROI_height % 2) return 2; //invalid YROI_HEIGHT
	}
	else if (param.decimationRatio == 4)
	{
		if (param.XROI_width % 64) return 1; //invalid XROI_WIDTH
		if (param.YROI_height % 4) return 2;
	}
	else if (param.decimationRatio == 9)
	{
		if (param.XROI_width % 96) return 1; //invalid XROI_WIDTH
		if (param.YROI_height % 6) return 2;
	}
	else return 3;

	if (param.XROI_start % 8) return 4; //should be 8n
	if (param.YROI_start % 8) return 5; //should be 8n

	

	cmdSetROI(param);
	/*
	FT_STATUS ret;
	unsigned long lenW;
	CmdSetROI_t cmdSetROI;
	cmdSetROI.PktId = 0;
	cmdSetROI.CameraROI = param;
	cmdSetROI.ChkSum = calcChkSum((unsigned int*)(&cmdSetROI));

	ret = FT_WritePipe(h_ft, 0x02, (unsigned char *)&cmdSetROI, sizeof(CmdSetROI_t), &lenW, 0);
	if (FT_FAILED(ret)) {
		return 6;
	}
	*/

	/*
	cmdSensorRegWrite(SENSOR_STATE_SEL_REG, IDLE_STATE);
	Sleep(60);//mandatory to wait sensor exit to IDLE_STATE:
	cmdSensorRegWrite(SENSOR_STATE_SEL_REG, SLAVE_INT_STATE);
	*/
	return 0;
}


bool OctDevice::ColorSensor::saveDefectPixels(const std::vector<std::pair<unsigned int, unsigned int>>& pixels)
{
	defect_map_t defectMap;
	defectMap.defect_size = (unsigned int) pixels.size();
	for (unsigned int i = 0; i < defectMap.defect_size; i++) {
		defectMap.defect_pixel[i].x = pixels[i].first;
		defectMap.defect_pixel[i].y = pixels[i].second;
		defectMap.defect_pixel[i].defect_type = 0; // i % 4; ///arbitrary ...
	}
	defectMap.chksum = CalcDefectMapChksum(defectMap.defect_size, &(defectMap.defect_pixel[0]));

	LogD() << "Defect pixels saved to camera, size: " << defectMap.defect_size;

	setUsbHandle(getUsbPort().getHandle());
	auto ret = SaveDefectMapToCam(defectMap.defect_size, &defectMap);
	if (ret) {
		LogW() << "Save defect pixels to camera failed, return: " << ret;
		return false;
	}
	return true;
}


bool OctDevice::ColorSensor::loadDefectPixels(std::vector<std::pair<unsigned int, unsigned int>>& pixels)
{
	defect_map_t defectMap;
	setUsbHandle(getUsbPort().getHandle());
	auto ret = GetDefectMap(&defectMap);
	if (ret) {
		LogW() << "Load defect pixels from camera failed, return: " << ret;
		return false;
	}

	LogD() << "Defect pixels loaded from camera, size: " << defectMap.defect_size;

	pixels.clear();
	for (unsigned int i = 0; i < defectMap.defect_size; i++) {
		pixels.push_back({ defectMap.defect_pixel[i].x, defectMap.defect_pixel[i].y });
	}
	return true;
}


ColorSensor::ColorSensorImpl & OctDevice::ColorSensor::getImpl(void) const
{
	return *d_ptr;
}


UsbPort & OctDevice::ColorSensor::getUsbPort(void) const
{
	return getImpl().usbPort;
}
