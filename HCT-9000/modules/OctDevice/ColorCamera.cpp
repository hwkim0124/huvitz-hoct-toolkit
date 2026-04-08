#include "stdafx.h"
#include "ColorCamera.h"
#include "ColorSensor.h"
#include "MainBoard.h"

#include <atomic>
#include <functional>
#include <mutex>
#include <fstream>
#include <cstddef>
#include <direct.h>  // _mkdir
#include <io.h>      // _access
#include <thread>

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;

#ifdef __USE_SPINNAKER__
using namespace Spinnaker;
#endif

using namespace std;

namespace
{
	bool isFrameSizeSafe(size_t requested, size_t capacity, const char* label)
	{
		if (requested > capacity)
		{
			return false;
		}
		return true;
	}
}


struct ColorCamera::ColorCameraImpl
{
	MainBoard* board;
	ColorSensor sensor;
	bool initiated;
	bool usingLv4{ false };

	std::uint8_t frameBuffer[FUNDUS_FRAME_BUFFER_SIZE];
	std::uint8_t sonyFrameBuffer[FUNDUS_FILR_FRAME_BUFFER_SIZE];

	unsigned int sensorId;
	CvImage image;

	SensorMode_t sensorMode;
	CameraROI_t cameraROI;
	CaptureParam_t captureParam;
	TrigExposureParam_t trigExposureParam;
	StrbParam_t strbParam;
	SensorGain_t sensorGain;

#ifdef __USE_SPINNAKER__
	Spinnaker::SystemPtr systemPtr;
	Spinnaker::CameraList camList;
	Spinnaker::CameraPtr pCam;
	Spinnaker::ImagePtr rawImage;
	ImageProcessor processor;
#endif

	atomic<bool> liveMode;
	atomic<bool> originalMode;
	atomic<bool> acquireSuccess;
	thread worker;

	ColorCameraFrameCallback* frameCallback;
	ColorCameraImageCallback* imageCallback;
	mutex mutexLock;

#ifdef __USE_SPINNAKER__
	ColorCameraImpl() : initiated(false), liveMode(false), acquireSuccess(true), pCam(NULL), rawImage(NULL), systemPtr(NULL),
		frameCallback(nullptr), imageCallback(nullptr), board(nullptr)
	{
	}
#endif
};


OctDevice::ColorCamera::ColorCamera(MainBoard * board) :
	d_ptr(make_unique<ColorCameraImpl>())
{
	getImpl().board = board;
}


ColorCamera::~ColorCamera()
{
#ifdef __USE_SPINNAKER__
	getImpl().rawImage = NULL;
	getCamera() = NULL;
	getImpl().camList.Clear();
	getImpl().systemPtr = NULL;
#endif
}


bool OctDevice::ColorCamera::initialize(void)
{
	if (getMainBoard() == nullptr)
	{
		LogE() << "Color camera init failed: MainBoard is null";
		return false;
	}

	if (GlobalSettings::useFundusFILR_Enable()) {
		if (getMainBoard()->isNoColor()) {
			return true;
		}
#ifdef __USE_SPINNAKER__
		getImpl().systemPtr = System::GetInstance();
		getImpl().camList = getImpl().systemPtr->GetCameras();
		unsigned int numCameras = getImpl().camList.GetSize();

		if (numCameras == 0)
		{
			getImpl().camList.Clear();
			getImpl().systemPtr->ReleaseInstance();
			LogE() << "Color camera init failed! ";
			return false;
		}
		getImpl().pCam = getImpl().camList.GetByIndex(0);
		getImpl().pCam->Init();

		try
		{
			Spinnaker::GenApi::INodeMap& nodeMap = getImpl().pCam->GetNodeMap();
			auto firmwareNode = Spinnaker::GenApi::CStringPtr(nodeMap.GetNode("DeviceFirmwareVersion"));
			if (Spinnaker::GenApi::IsReadable(firmwareNode))
				LogI() << "FLIR camera firmware version: " << firmwareNode->GetValue();
			else
				LogE() << "Cannot read FLIR camera firmware version.";
		}
		catch (const std::exception& e)
		{
			LogE() << "Exception while reading FLIR firmware version: " << e.what();
		}

#endif
	}
	else {
		if (getMainBoard()->isNoColor()) {
			return true;
		}

		if (!getSensor().initialize()) {
			LogE() << "Color camera sensor init failed!";
			return false;
		}

		unsigned sid;
		if (!getSensorID(&sid)) {
			LogE() << "Color camera sensor id read failed!";
			return false;
		}

		if (sid != KAC12040_SENSOR_ID) {
			LogE() << "Color camera sensor id invalid!, sid: " << sid;
			return false;
		}

		getImpl().sensorId = sid;
	}

	setDefaultParameters();
	setDefaultParameters();

	getImpl().initiated = true;

	return true;
}


bool OctDevice::ColorCamera::isInitiated(void) const
{
	return getImpl().initiated;
}

void OctDevice::ColorCamera::setUsingLv4(bool enable)
{
	getImpl().usingLv4 = enable;
}

bool OctDevice::ColorCamera::isUsingLv4() const
{
	return getImpl().usingLv4;
}


void OctDevice::ColorCamera::setDefaultParametersForSingle()
{
#ifdef __USE_SPINNAKER__
	if (getCamera() == nullptr) {
		LogE() << "SetDefaultParametersForSingle: camera not available";
		return;
	}

	if (getCamera()->IsStreaming()) {
		getCamera()->EndAcquisition();
	}

	// OFF ALL auto settings
	if (!IsWritable(getCamera()->GainAuto)) {
		GlobalLogger::debug("GainAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->GainAuto.GetValue()));

		GlobalLogger::debug("Gain value is ReadOnly. Gain Value: " +
			std::to_string(getCamera()->Gain.GetValue()));
	}
	else {
		getCamera()->GainAuto.SetValue(GainAuto_Off);
	}

	if (!IsWritable(getCamera()->BalanceWhiteAuto)) {
		GlobalLogger::debug("BalanceWhiteAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->BalanceWhiteAuto.GetValue()));
	}
	else {
		getCamera()->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
	}

	if (!IsWritable(getCamera()->ExposureAuto)) {
		GlobalLogger::debug("ExposureAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->ExposureAuto.GetValue()));
	}
	else {
		getCamera()->ExposureAuto.SetValue(ExposureAuto_Off);
	}

	GlobalLogger::debug("Color Camera Set Indicator LED...ok!");

	// Set Indicator LED
	getCamera()->DeviceIndicatorMode.SetValue(DeviceIndicatorMode_Inactive);
	GlobalLogger::debug("Color Camera Off all auto settings...ok!");

	// Set Image Size
	getCamera()->BinningHorizontal.SetValue(1);
	getCamera()->BinningVertical.SetValue(1);

	getCamera()->Width.SetValue(4668);
	getCamera()->Height.SetValue(3648);
	getCamera()->OffsetX.SetValue(400);
	getCamera()->OffsetY.SetValue(0);
	GlobalLogger::debug("Color Camera Set ImageSize...ok!");


	// Set Parameter
	getCamera()->ReverseX.SetValue(false);
	getCamera()->ReverseY.SetValue(true);	// Horizontal Flip
	getCamera()->PixelFormat.SetValue(PixelFormat_BayerGB16);
	GlobalLogger::debug("Color Camera Set Parameter...ok!");

	// Set Exposure time

	getCamera()->ExposureTime.SetValue(20000);	// Unit = us
	GlobalLogger::debug("Color Camera Set ExposureTime...ok!");

	// Set Digital gain
	getCamera()->AdcBitDepth.SetValue(AdcBitDepth_Bit12);

	getCamera()->Gain.SetValue(1);

	GlobalLogger::debug("Color Camera Set Gain...ok!");

	// Set White Balnce 

	getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
	getCamera()->BalanceRatio.SetValue(1.58);
	getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
	getCamera()->BalanceRatio.SetValue(1.84);
	GlobalLogger::debug("Color Camera Set WhiteBalnce...ok!");

	// Set ISP Mode
	getCamera()->IspEnable.SetValue(false);
	getCamera()->GammaEnable.SetValue(false);
	GlobalLogger::debug("Color Camera Set ISP...ok!");

	// Set Acquisition mode SingleFrame
	getCamera()->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
	GlobalLogger::debug("Color Camera Set AcquisitionMode...ok!");

	std::this_thread::sleep_for(300ms);

#endif

	return;
}


void OctDevice::ColorCamera::setDefaultParametersForLive()
{
#ifdef __USE_SPINNAKER__
	if (getCamera() == nullptr) {
		LogE() << "SetDefaultParametersForLive: camera not available";
		return;
	}

	if (getCamera()->IsStreaming()) {
		getCamera()->EndAcquisition();
	}

	// OFF ALL auto settings
	if (!IsWritable(getCamera()->GainAuto)) {
		GlobalLogger::debug("GainAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->GainAuto.GetValue()));

		GlobalLogger::debug("Gain value is ReadOnly. Gain Value: " +
			std::to_string(getCamera()->Gain.GetValue()));
	}
	else {
		getCamera()->GainAuto.SetValue(GainAuto_Off);
	}

	if (!IsWritable(getCamera()->BalanceWhiteAuto)) {
		GlobalLogger::debug("BalanceWhiteAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->BalanceWhiteAuto.GetValue()));
	}
	else {
		getCamera()->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
	}

	if (!IsWritable(getCamera()->ExposureAuto)) {
		GlobalLogger::debug("ExposureAuto is ReadOnly. Current Mode: " +
			std::to_string(getCamera()->ExposureAuto.GetValue()));
	}
	else {
		getCamera()->ExposureAuto.SetValue(ExposureAuto_Off);
	}

	GlobalLogger::debug("Color Camera Set Indicator LED...ok!");

	// Set Indicator LED
	getCamera()->DeviceIndicatorMode.SetValue(DeviceIndicatorMode_Inactive);
	GlobalLogger::debug("Color Camera Off all auto settings...ok!");

	// Set Image Size
	getCamera()->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);
	getCamera()->BinningVerticalMode.SetIntValue(BinningVerticalMode_Average);
	getCamera()->BinningHorizontal.SetValue(4);
	getCamera()->BinningVertical.SetValue(4);

	getCamera()->Width.SetValue(1168);
	getCamera()->Height.SetValue(912);
	getCamera()->OffsetX.SetValue(100);
	getCamera()->OffsetY.SetValue(0);
	GlobalLogger::debug("Color Camera Set ImageSize...ok!");

	// Set Parameter
	getCamera()->ReverseX.SetValue(false);
	getCamera()->ReverseY.SetValue(false);	// Horizontal Flip
											//getCamera()->PixelFormat.SetValue(PixelFormat_BayerGB16);
	GlobalLogger::debug("Color Camera Set Parameter...ok!");

	// Set Exposure time

	getCamera()->ExposureTime.SetValue(20000);	// Unit = us
	GlobalLogger::debug("Color Camera Set ExposureTime...ok!");

	// Set Digital gain
	getCamera()->AdcBitDepth.SetValue(AdcBitDepth_Bit12);

	getCamera()->Gain.SetValue(1);

	GlobalLogger::debug("Color Camera Set Gain...ok!");

	// Set White Balnce 

	getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
	getCamera()->BalanceRatio.SetValue(1.58);
	getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
	getCamera()->BalanceRatio.SetValue(1.84);
	GlobalLogger::debug("Color Camera Set WhiteBalnce...ok!");

	// Set ISP Mode
	getCamera()->IspEnable.SetValue(false);
	getCamera()->GammaEnable.SetValue(false);
	GlobalLogger::debug("Color Camera Set ISP...ok!");

	// Set Acquisition mode SingleFrame
	getCamera()->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
	GlobalLogger::debug("Color Camera Set AcquisitionMode...ok!");

#endif
	return;
}


void OctDevice::ColorCamera::setDefaultParameters(bool update)
{
#ifdef __USE_SPINNAKER__

	if (GlobalSettings::useFundusFILR_Enable()) {
		for (int i = 0; i < 3; i++)
		{
			try {
				if (getCamera() == nullptr) {
					GlobalLogger::error("Color Camera pointer null during init");
					return;
				}

				if (!getCamera()->IsInitialized()) {
					getCamera()->Init();
					GlobalLogger::debug("Color Camera Is not Initialized!");
					std::this_thread::sleep_for(300ms);
				}

				// OFF ALL auto settings
				if (!IsWritable(getCamera()->GainAuto)) {
					GlobalLogger::debug("GainAuto is ReadOnly. Current Mode: " +
						std::to_string(getCamera()->GainAuto.GetValue()));

					GlobalLogger::debug("Gain value is ReadOnly. Gain Value: " +
						std::to_string(getCamera()->Gain.GetValue()));
				}
				else {
					getCamera()->GainAuto.SetValue(GainAuto_Off);
				}

				if (!IsWritable(getCamera()->BalanceWhiteAuto)) {
					GlobalLogger::debug("BalanceWhiteAuto is ReadOnly. Current Mode: " +
						std::to_string(getCamera()->BalanceWhiteAuto.GetValue()));
				}
				else {
					getCamera()->BalanceWhiteAuto.SetValue(BalanceWhiteAuto_Off);
				}

				if (!IsWritable(getCamera()->ExposureAuto)) {
					GlobalLogger::debug("ExposureAuto is ReadOnly. Current Mode: " +
						std::to_string(getCamera()->ExposureAuto.GetValue()));
				}
				else {
					getCamera()->ExposureAuto.SetValue(ExposureAuto_Off);
				}
				GlobalLogger::debug("Color Camera Off all auto settings...ok!");


				// Set Indicator LED
				getCamera()->DeviceIndicatorMode.SetValue(DeviceIndicatorMode_Inactive);
				GlobalLogger::debug("Color Camera Set Indicator LED...ok!");

				getCamera()->BinningHorizontal.SetValue(1);
				getCamera()->BinningVertical.SetValue(1);

				getCamera()->Width.SetValue(4668);
				getCamera()->Height.SetValue(3648);
				getCamera()->OffsetX.SetValue(400);
				getCamera()->OffsetY.SetValue(0);
				GlobalLogger::debug("Color Camera Set ImageSize...ok!");

				// Set Parameter
				getCamera()->ReverseX.SetValue(false);
				getCamera()->ReverseY.SetValue(true);	// Horizontal Flip
				getCamera()->PixelFormat.SetValue(PixelFormat_BayerGB16);
				GlobalLogger::debug("Color Camera Set Parameter...ok!");

				// Set Exposure time

				getCamera()->ExposureTime.SetValue(20000);	// Unit = us
				GlobalLogger::debug("Color Camera Set ExposureTime...ok!");

				// Set Digital gain
				getCamera()->AdcBitDepth.SetValue(AdcBitDepth_Bit12);

				getCamera()->Gain.SetValue(1);

				GlobalLogger::debug("Color Camera Set Gain...ok!");

				// Set White Balnce 

				getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Red);
				getCamera()->BalanceRatio.SetValue(1.58);
				getCamera()->BalanceRatioSelector.SetValue(BalanceRatioSelector_Blue);
				getCamera()->BalanceRatio.SetValue(1.84);
				GlobalLogger::debug("Color Camera Set WhiteBalnce...ok!");

				// Set ISP Mode
				getCamera()->IspEnable.SetValue(false);
				getCamera()->GammaEnable.SetValue(false);
				GlobalLogger::debug("Color Camera Set ISP...ok!");

				// Set Strobe Light
				getCamera()->AutoExposureLightingMode.SetIntValue(AutoExposureLightingMode_Frontlight);
				getCamera()->LineSelector.SetValue(LineSelector_Line2);
				getCamera()->LineMode.SetValue(LineMode_Output);
				GlobalLogger::debug("Color Camera Set StrobeLight...ok!");

				// Set Shutter Mode and Image Grab Test
				getCamera()->SensorShutterMode.SetValue(SensorShutterMode_Rolling);
				getCamera()->BeginAcquisition();
				getImpl().rawImage = getCamera()->GetNextImage();
				getCamera()->EndAcquisition();
				getCamera()->SensorShutterMode.SetValue(SensorShutterMode_GlobalReset);
				GlobalLogger::debug("Color Camera Grab Test...ok!");

				// Set Acquisition mode SingleFrame
				getCamera()->AcquisitionMode.SetValue(AcquisitionMode_SingleFrame);
				GlobalLogger::debug("Color Camera Set AcquisitionMode...ok!");

				GlobalLogger::info("Color Camera initialize...ok!");
				break;
			}
			catch (const std::exception& e)
			{
				auto elog = e.what();
				GlobalLogger::error("Exception occurred during ColorCamera init!");
				GlobalLogger::error(elog);

				getImpl().pCam = nullptr;
				getImpl().camList.Clear();

				getImpl().systemPtr->ReleaseInstance();
				getImpl().systemPtr = Spinnaker::System::GetInstance();

				std::this_thread::sleep_for(300ms);

				getImpl().camList = getImpl().systemPtr->GetCameras();
				unsigned int numCameras = getImpl().camList.GetSize();

				if (numCameras > 0) {
					getImpl().pCam = getImpl().camList.GetByIndex(0);
					try {
						getImpl().pCam->Init();
						GlobalLogger::info("Camera reinitialized successfully");
					}
					catch (...) {
						GlobalLogger::error("Re-init failed after hardware reset");
					}
				}
				else {
					GlobalLogger::error("No cameras detected after reset!");
				}
			}
		}
	}

	else
#endif	
	{
		CameraROI_t cameraROI;
		cameraROI.XROI_start = 0;
		cameraROI.XROI_width = 3840;
		cameraROI.YROI_start = 0;
		cameraROI.YROI_height = 3000;
		cameraROI.decimationRatio = 0; // 1;
		getImpl().cameraROI = cameraROI;

		CaptureParam_t captureParam;
		captureParam.CaptureFrameSize = 1;
		captureParam.CaptureMode = FRAME_MODE_FIXED;
		getImpl().captureParam = captureParam;

		TrigExposureParam_t trigExposureParam;
		trigExposureParam.ExposureCnt = 80 * 100000;
		trigExposureParam.ExposureOffCnt = 250 * 100000;
		getImpl().trigExposureParam = trigExposureParam;

		StrbParam_t strbParam;
		strbParam.StrbDelay = (unsigned int)(30 * 390.625); //strb delay 1 cnt: 2.56us, 1ms=390.625*2.56us
		strbParam.StrbActTime = (unsigned int)(20 * 390.625);
		strbParam.StrbMode = STRB_PULSE_SHAPING; //STRB_PULSE_SHAPING,STRB_BYPASS
		getImpl().strbParam = strbParam;

		SensorGain_t sensorGain;
		sensorGain.Again = (int)CAMERA_COLOR_INIT_AGAIN;
		sensorGain.Dgain = CAMERA_COLOR_INIT_DGAIN;
		getImpl().sensorGain = sensorGain;

		SensorMode_t sensorMode;
		sensorMode.shutter_mode = ROLLING_SHUTTER;
		sensorMode.flip = HORIZONTAL_FLIP; // NO_FLIP;
		getImpl().sensorMode = sensorMode;
	}

	if (update) {
		updateParameters();
	}
	return;
}



void OctDevice::ColorCamera::updateParameters(void)
{
	if (!isInitiated()) {
		return;
	}

	getSensor().cmdUpdateStart();

	getSensor().setROI(getImpl().cameraROI);
	getSensor().cmdCaptureParam(getImpl().captureParam);
	getSensor().cmdTrigExposure(getImpl().trigExposureParam);
	getSensor().cmdStrobeParam(getImpl().strbParam);
	getSensor().cmdSensorGain(getImpl().sensorGain);
	getSensor().setSensorMode(getImpl().sensorMode);
	getSensor().cmdUpdateClose();
	return;
}

void OctDevice::ColorCamera::setFrameCallback(ColorCameraFrameCallback * callback)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	getImpl().frameCallback = callback;
	return;
}


void OctDevice::ColorCamera::setImageCallback(ColorCameraImageCallback * callback)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	getImpl().imageCallback = callback;
	return;
}


void OctDevice::ColorCamera::takePicture(bool process)
{
	if (!isInitiated()) {
		return;
	}

	if (getImpl().liveMode) {
		return;
	}

	getImpl().cameraROI.decimationRatio = 0;
	getSensor().setROI(getImpl().cameraROI);

	getImpl().captureParam.CaptureFrameSize = 1;
	getImpl().captureParam.CaptureMode = FRAME_MODE_FIXED;
	getSensor().cmdCaptureParam(getImpl().captureParam);

	getImpl().acquireSuccess = true;
	getImpl().worker = thread{ &ColorCamera::acquireCameraData, this };

	if (getImpl().worker.joinable()) {
		getImpl().worker.join();
	}

	if (!getImpl().acquireSuccess.load()) {
		return;
	}

	if (process) {
		processImage();
	}
	return;
}


void OctDevice::ColorCamera::startLiveMode(void)
{
	if (!isInitiated()) {
		return;
	}
	if (getImpl().liveMode) {
		return;
	}

	getSensor().cmdUpdateStart();
	getImpl().cameraROI.decimationRatio = 4;
	getSensor().setROI(getImpl().cameraROI);

	getImpl().captureParam.CaptureFrameSize = 1;
	getImpl().captureParam.CaptureMode = FRAME_MODE_CONTINUOUS;
	getSensor().cmdCaptureParam(getImpl().captureParam);
	getSensor().cmdUpdateClose();

	getImpl().liveMode = true;
	getImpl().worker = thread{ &ColorCamera::acquireCameraData, this };
	return;
}


void OctDevice::ColorCamera::startOriginalMode(void)
{
	if (!isInitiated()) {
		return;
	}
	if (getImpl().liveMode) {
		return;
	}

	getImpl().originalMode = true;

	getSensor().cmdUpdateStart();
	getImpl().cameraROI.decimationRatio = 4;
	getSensor().setROI(getImpl().cameraROI);

	getImpl().captureParam.CaptureFrameSize = 1;
	getImpl().captureParam.CaptureMode = FRAME_MODE_CONTINUOUS;
	getSensor().cmdCaptureParam(getImpl().captureParam);
	getSensor().cmdUpdateClose();
	//getImpl().liveMode = true;
	acquireOriginalData();
	return;
}

void OctDevice::ColorCamera::pauseLiveMode(void)
{
	if (!isInitiated()) {
		return;
	}
	if (!getImpl().liveMode) {
		return;
	}

	getImpl().liveMode = false;
	if (getImpl().worker.joinable()) {
		getImpl().worker.join();
	}
	return;
}

void OctDevice::ColorCamera::pauseOriginalMode(void)
{
	if (!isInitiated()) {
		return;
	}
	getImpl().originalMode = false;

	return;
}

bool OctDevice::ColorCamera::isLiveMode(void) const
{
	return getImpl().liveMode;
}

bool OctDevice::ColorCamera::isOriginalMode(void) const
{
	return getImpl().originalMode;
}

bool OctDevice::ColorCamera::processImage(void)
{
	try {
		processCameraData();
		return true;
	}
	catch (const std::exception& e) {
		return false;
	}
	catch (...) {
		return false;
	}
}


bool OctDevice::ColorCamera::getSensorID(unsigned int * id)
{
	unsigned int RdDataLSB;
	unsigned int RdDataMSB;
	unsigned int rtn;

	rtn = getSensor().sensorRegRead(SENSOR_CHIP_ID_MSB_REG, &RdDataMSB);
	if (rtn) return false;
	rtn = getSensor().sensorRegRead(SENSOR_CHIP_ID_LSB_REG, &RdDataLSB);
	if (rtn) return false;
	*id = (RdDataMSB << 16) | RdDataLSB;
	return true;
}


int OctDevice::ColorCamera::getFrameWidth(void) const
{
	int width;

	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		width = getCamera()->Width.GetValue();
#endif
	}
	else {
		unsigned int decimationRatio = getImpl().cameraROI.decimationRatio;
		unsigned int XROI_width = getImpl().cameraROI.XROI_width;

		if (decimationRatio == 0) {
			width = XROI_width;
		}
		else if (decimationRatio == 4) {
			width = XROI_width / 2;
		}
		else if (decimationRatio == 9) {
			width = XROI_width / 3;
		}
		else {
			width = 0;
		}
	}

	return width;
}


int OctDevice::ColorCamera::getFrameHeight(void) const
{
	int height;

	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		height = getCamera()->Height.GetValue();
#endif
	}

	else {
		unsigned int decimationRatio = getImpl().cameraROI.decimationRatio;
		unsigned int YROI_height = getImpl().cameraROI.YROI_height;

		if (decimationRatio == 0) {
			height = YROI_height;
		}
		else if (decimationRatio == 4) {
			height = YROI_height / 2;
		}
		else if (decimationRatio == 9) {
			height = YROI_height / 3;
		}
		else {
			height = 0;
		}
	}
	return height;
}


int OctDevice::ColorCamera::getFrameSizeInBytes(void) const
{
	return getFrameWidth() * getFrameHeight() * sizeof(unsigned short);
}


int OctDevice::ColorCamera::getTotalFrames(void) const
{
	return getImpl().captureParam.CaptureFrameSize;
}


int OctDevice::ColorCamera::getImageWidth(void) const
{
	return getImpl().image.getWidth();
}


int OctDevice::ColorCamera::getImageHeight(void) const
{
	return getImpl().image.getHeight();
}


const std::uint8_t * OctDevice::ColorCamera::getFrameBuffer(void) const
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		return getImpl().sonyFrameBuffer;
	}
	else {
		return getImpl().frameBuffer;
	}
}


const std::uint8_t * OctDevice::ColorCamera::getImageBuffer(void) const
{
	return getImpl().image.getBitsData();
}


void OctDevice::ColorCamera::setROI_X_Start(unsigned int val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		val = val - (val % 8);
		val = val = (val < 0 ? 0 : val);
		val = val = (val > 5742 ? 5742 : val);
#ifdef __USE_SPINNAKER__
		getCamera()->OffsetX.SetValue(val);
#endif
	}

	else {
		val = val - (val % 8);
		val = (val < 0 ? 0 : val);
		val = (val > 4000 ? 4000 : val);
		getImpl().cameraROI.XROI_start = val;
	}
	return;
}


void OctDevice::ColorCamera::setROI_X_Width(unsigned int val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		val = val - (val % 8);
		val = val = (val < 0 ? 0 : val);
		val = val = (val > 5742 ? 5742 : val);
#ifdef __USE_SPINNAKER__
		getCamera()->Width.SetValue(val);
#endif
	}

	else {
		int ratio = getImpl().cameraROI.decimationRatio;
		if (ratio == 0) {
			val = val - (val % 32);
		}
		else if (ratio == 4) {
			val = val - (val % 64);
		}
		else if (ratio == 9) {
			val = val - (val % 96);
		}
		else {
			return;
		}
		val = (val < 0 ? 0 : val);
		val = (val > 4000 ? 4000 : val);
		getImpl().cameraROI.XROI_width = val;
	}
	return;
}


void OctDevice::ColorCamera::setROI_Y_Start(unsigned int val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		val = val - (val % 8);
		val = (val < 0 ? 0 : val);
		val = (val > 3648 ? 3648 : val);
#ifdef __USE_SPINNAKER__
		getCamera()->OffsetY.SetValue(val);
#endif
	}
	else {
		val = val - (val % 8);
		val = (val < 0 ? 0 : val);
		val = (val > 3016 ? 3016 : val);
		getImpl().cameraROI.YROI_start = val;
	}
	return;
}


void OctDevice::ColorCamera::setROI_Y_Height(unsigned int val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		val = val - (val % 8);
		val = (val < 0 ? 0 : val);
		val = (val > 3648 ? 3648 : val);
#ifdef __USE_SPINNAKER__
		getCamera()->Height.SetValue(val);
#endif
	}

	else {
		int ratio = getImpl().cameraROI.decimationRatio;
		if (ratio == 0) {
			val = val - (val % 2);
		}
		else if (ratio == 4) {
			val = val - (val % 4);
		}
		else if (ratio == 9) {
			val = val - (val % 6);
		}
		else {
			return;
		}
		val = (val < 0 ? 0 : val);
		val = (val > 3016 ? 3016 : val);
		getImpl().cameraROI.YROI_height = val;
	}
	return;
}


void OctDevice::ColorCamera::setROI_DecimationRatio(unsigned int val, bool setIdx)
{
	if (setIdx) {
		switch (val) {
		case 1:
			val = 4; break;
		case 2:
			val = 9; break;
		case 0:
		default:
			val = 1; break;
		}
	}
	getImpl().cameraROI.decimationRatio = val;
	return;
}


unsigned int OctDevice::ColorCamera::getROI_X_Start(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return getCamera()->OffsetX.GetValue();
#else
		return 0;
#endif
	}

	else {
		return getImpl().cameraROI.XROI_start;
	}
}

unsigned int OctDevice::ColorCamera::getROI_X_Width(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return getCamera()->Width.GetValue();
#else
		return 0;
#endif
	}

	else {
		return getImpl().cameraROI.XROI_width;
	}

}

unsigned int OctDevice::ColorCamera::getROI_Y_Start(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return getCamera()->OffsetY.GetValue();
#else
		return 0;
#endif
	}

	else {
		return getImpl().cameraROI.YROI_start;
	}

}

unsigned int OctDevice::ColorCamera::getROI_Y_Height(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return getCamera()->Height.GetValue();
#else
		return 0;
#endif
	}

	else {
		return getImpl().cameraROI.YROI_height;
	}

}


unsigned int OctDevice::ColorCamera::getROI_DecimationRatio(bool retIdx)
{
	if (retIdx) {
		unsigned int ratio = getImpl().cameraROI.decimationRatio;
		switch (ratio) {
		case 4:
			return 1;
		case 9:
			return 2;
		case 0:
		default:
			return 0;
		}
	}
	return getImpl().cameraROI.decimationRatio;
}


void OctDevice::ColorCamera::setAnalogGain(unsigned int val, bool setIdx)
{
	unsigned int again = val;
	if (setIdx) {
		switch (val) {
		case 1:
			again = 2; break;
		case 2:
			again = 4; break;
		case 3:
			again = 8; break;
		case 0:
		default:
			again = 1; break;
		}
	}
	getImpl().sensorGain.Again = again;

	return;
}


void OctDevice::ColorCamera::setDigitalGain(float val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		val = (val < 1.0f ? 1.0f : val);
		val = (val > 27.0f ? 27.0f : val);

#ifdef __USE_SPINNAKER__
		if (getCamera() != NULL) {
			getCamera()->Gain.SetValue(val);
		}
		else {
			LogE() << "setDigitalGain: camera not available";
		}
#endif
	}
	else {
		val = (val < 1.0f ? 1.0f : val);
		val = (val > 15.875f ? 15.875f : val);
		getImpl().sensorGain.Dgain = val;
	}

	return;
}

void OctDevice::ColorCamera::setDigitalIO(bool flag)
{
#ifdef __USE_SPINNAKER__
	if (GlobalSettings::useFundusFILR_Enable()) {
		if (getCamera() == nullptr) {
			LogE() << "setDigitalIO: camera not available";
			return;
		}

		if (!getCamera()->IsValid()) {
			getCamera()->Init();
		}
		if (getCamera()->IsStreaming()) {
			pauseLiveMode();

			LineModeEnums inOut;
			if (flag) {
				inOut = LineMode_Output;
			}
			else {
				inOut = LineMode_Input;
			}
			getCamera()->LineSelector.SetValue(LineSelector_Line2);
			getCamera()->LineMode.SetValue(inOut);
			startLiveMode();
		}

		LineModeEnums inOut;
		if (flag) {
			inOut = LineMode_Output;
		}
		else {
			inOut = LineMode_Input;
		}
		getCamera()->LineSelector.SetValue(LineSelector_Line2);
		getCamera()->LineMode.SetValue(inOut);
	}
#endif
}


unsigned int OctDevice::ColorCamera::getAnalogGain(bool retIdx)
{
	if (retIdx) {
		unsigned int again = getImpl().sensorGain.Again;
		switch (again) {
		case 2:
			return 1;
		case 4:
			return 2;
		case 8:
			return 3;
		case 0:
		default:
			return 0;
		}
	}
	return getImpl().sensorGain.Again;
}


float OctDevice::ColorCamera::getDigitalGain(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return getCamera()->Gain.GetValue();
#else
		return 0.0;
#endif
	}
	else {
		return getImpl().sensorGain.Dgain;
	}
}


void OctDevice::ColorCamera::setExposureCount(unsigned int val)
{
	getImpl().trigExposureParam.ExposureCnt = val * 100000;
	return;
}


void OctDevice::ColorCamera::setExposureOffCount(unsigned int val)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		if (getCamera() == nullptr) {
			LogE() << "setExposureOffCount: camera not available";
			return;
		}
		getCamera()->ExposureTime.SetValue(val * 10);
#endif
	}

	else {
		getImpl().trigExposureParam.ExposureOffCnt = val * 100000;
	}
	return;
}


unsigned int OctDevice::ColorCamera::getExposureCount(void)
{
	return (getImpl().trigExposureParam.ExposureCnt / 100000);
}


unsigned int OctDevice::ColorCamera::getExposureOffCount(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		return (getCamera()->ExposureTime.GetValue() / 10);
#else
		return 0;
#endif
	}

	else {
		return (getImpl().trigExposureParam.ExposureOffCnt / 100000);
	}
}


void OctDevice::ColorCamera::setStrobeDelay(unsigned int val)
{
	getImpl().strbParam.StrbDelay = (unsigned int)(val * 390.625); //strb delay 1 cnt: 2.56us, 1ms=390.625*2.56us
	return;
}


void OctDevice::ColorCamera::setStrobeActTime(unsigned int val)
{
	getImpl().strbParam.StrbActTime = (unsigned int)(val * 390.625); //strb delay 1 cnt: 2.56us, 1ms=390.625*2.56us
	return;
}


void OctDevice::ColorCamera::setStrobeMode(unsigned int val)
{
	getImpl().strbParam.StrbMode = val;
	return;
}


unsigned int OctDevice::ColorCamera::getStrobeDelay(void)
{
	return (unsigned int)(getImpl().strbParam.StrbDelay / 390.625 + 0.5);
}


unsigned int OctDevice::ColorCamera::getStrobeActTime(void)
{
	return (unsigned int)(getImpl().strbParam.StrbActTime / 390.625 + 0.5);
}


unsigned int OctDevice::ColorCamera::getStrobeMode(void)
{
	return getImpl().strbParam.StrbMode;
}


void OctDevice::ColorCamera::setShutterMode(unsigned int val, bool setIdx)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		unsigned int mode = val;
		if (setIdx) {
#ifdef __USE_SPINNAKER__
			if (getCamera() == nullptr) {
				LogE() << "setShutterMode: camera not available";
				return;
			}
			(val == 0 ? getCamera()->SensorShutterMode.SetValue(SensorShutterMode_Rolling) :
				getCamera()->SensorShutterMode.SetValue(SensorShutterMode_GlobalReset));
#endif
		}
	}

	else {
		unsigned int mode = val;
		if (setIdx) {
			mode = (val == 0 ? ROLLING_SHUTTER : GLOBAL_SHUTTER);
		}
		getImpl().sensorMode.shutter_mode = mode;
	}
	return;
}


void OctDevice::ColorCamera::setFlipMode(unsigned int val, bool setIdx)
{
	int mode = val;
	if (setIdx) {
		if (val == 1) {
			mode = VERTICAL_FLIP;
		}
		else if (val == 2) {
			mode = HORIZONTAL_FLIP;
		}
		else if (val == 3) {
			mode = (VERTICAL_FLIP | HORIZONTAL_FLIP);
		}
		else {
			mode = NO_FLIP;
		}
	}
	getImpl().sensorMode.flip = mode;
	return;
}


void OctDevice::ColorCamera::setFrameSize(unsigned int val, bool setIdx)
{
	if (setIdx) {
		switch (val) {
		case 1:
			val = 2; break;
		case 2:
			val = 3; break;
		case 3:
			val = 4; break;
		case 0:
		default:
			val = 1; break;
		}
	}
	getImpl().captureParam.CaptureFrameSize = val;
	return;
}


void OctDevice::ColorCamera::setCaptureMode(unsigned int val, bool setIdx)
{
	unsigned int mode = val;
	if (setIdx) {
		mode = (val == 0 ? FRAME_MODE_FIXED : FRAME_MODE_CONTINUOUS);
	}

	getImpl().captureParam.CaptureMode = mode;
	return;
}


unsigned int OctDevice::ColorCamera::getShutterMode(bool retIdx)
{
	if (retIdx) {
		if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
			if (getCamera() == nullptr) {
				LogE() << "getShutterMode: camera not available";
				return 0;
			}
			unsigned int mode = getCamera()->SensorShutterMode.GetValue();
			switch (mode) {
			case SensorShutterMode_GlobalReset:
				return 1;
			case SensorShutterMode_Rolling:
			default:
				return 0;
			}
#endif
		}

		else {
			unsigned int mode = getImpl().sensorMode.shutter_mode;
			switch (mode) {
			case GLOBAL_SHUTTER:
				return 1;
			case ROLLING_SHUTTER:
			default:
				return 0;
			}
		}
	}
	return getImpl().sensorMode.shutter_mode;
}


unsigned int OctDevice::ColorCamera::getFlipMode(bool retIdx)
{
	if (retIdx) {
		if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
			if (getCamera() == nullptr) {
				LogE() << "getFlipMode: camera not available";
				return 0;
			}
			unsigned int ReverseX = getCamera()->ReverseX.GetValue();
			unsigned int ReverseY = getCamera()->ReverseY.GetValue();
			if (ReverseX == true && ReverseY == false)
				return 1;
			else if (ReverseX == false && ReverseY == true)
				return 2;
			else if (ReverseX == true && ReverseY == true)
				return 3;
			else
#endif
				return 0;
		}
		else {
			unsigned int mode = getImpl().sensorMode.flip;
			switch (mode) {
			case VERTICAL_FLIP:
				return 1;
			case HORIZONTAL_FLIP:
				return 2;
			case (VERTICAL_FLIP | HORIZONTAL_FLIP):
				return 3;
			case NO_FLIP:
			default:
				return 0;
			}
		}
	}
	return getImpl().sensorMode.flip;
}


unsigned int OctDevice::ColorCamera::getFrameSize(bool retIdx)
{
	if (retIdx) {
		unsigned int size = getImpl().captureParam.CaptureFrameSize;
		switch (size) {
		case 2:
			return 1;
		case 3:
			return 2;
		case 4:
			return 3;
		case 1:
		default:
			return 0;
		}
	}
	return getImpl().captureParam.CaptureFrameSize;
}


unsigned int OctDevice::ColorCamera::getCaptureMode(bool retIdx)
{
	if (retIdx) {
		if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
			if (getCamera() == nullptr) {
				LogE() << "getCaptureMode: camera not available";
				return 0;
			}
			unsigned int mode = getCamera()->AcquisitionMode.GetValue();
			switch (mode) {
			case AcquisitionMode_Continuous:
				return 1;
			case AcquisitionMode_SingleFrame:
			default:
				return 0;
			}
#endif
		}

		else {
			unsigned int mode = getImpl().captureParam.CaptureMode;
			switch (mode) {
			case FRAME_MODE_CONTINUOUS:
				return 1;
			case FRAME_MODE_FIXED:
			default:
				return 0;
			}
		}
	}
	return getImpl().captureParam.CaptureMode;
}


bool OctDevice::ColorCamera::loadConfig(OctConfig::CameraSettings * dset)
{
	CameraItem* item = dset->getCameraItem(CameraType::FUNDUS);
	if (item != nullptr) {
		setAnalogGain((int)item->analogGain());
		setDigitalGain(item->digitalGain());
		updateParameters();
		return true;
	}
	return false;
}


bool OctDevice::ColorCamera::saveConfig(OctConfig::CameraSettings * dset)
{
	CameraItem* item = dset->getCameraItem(CameraType::FUNDUS);
	if (item != nullptr) {
		item->analogGain() = (float)getAnalogGain();
		item->digitalGain() = getDigitalGain();
		return true;
	}
	return false;
}


bool OctDevice::ColorCamera::loadDefectPixels(std::vector<std::pair<unsigned int, unsigned int>>& pixels)
{
	return getSensor().loadDefectPixels(pixels);
}


bool OctDevice::ColorCamera::saveDefectPixels(const std::vector<std::pair<unsigned int, unsigned int>>& pixels)
{
	return getSensor().saveDefectPixels(pixels);
}


ColorCamera::ColorCameraImpl & OctDevice::ColorCamera::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::ColorCamera::getMainBoard(void) const
{
	return getImpl().board;
}


ColorSensor & OctDevice::ColorCamera::getSensor(void) const
{
	return getImpl().sensor;
}

#ifdef __USE_SPINNAKER__
CameraPtr OctDevice::ColorCamera::getCamera(void) const
{
	return getImpl().pCam;
}
#endif

void OctDevice::ColorCamera::flushImagePipe(void)
{
	getSensor().cmdGrabStop();
	getSensor().cmdFlushImagePipe(MEM_WR_RESET | MEM_RD_RESET | FRAME_BUF_RESET);
	return;
}


bool OctDevice::ColorCamera::grabStart(void)
{
	if (!getSensor().cmdGrabStart()) {
		return true;
	}
	return false;
}


bool OctDevice::ColorCamera::grabStop(void)
{
	if (!getSensor().cmdGrabStop()) {
		return true;
	}
	return false;
}


void OctDevice::ColorCamera::acquireCameraData(void)
{
	try {
		uint32_t width = getFrameWidth();
		uint32_t height = getFrameHeight();
		uint32_t frameSize = getFrameSizeInBytes();

		uint32_t frameCount;
		uint32_t frameTotal = getTotalFrames();

		if (GlobalSettings::useFundusFILR_Enable()) {
			unsigned char* buffer = getImpl().sonyFrameBuffer;
			size_t capacity = sizeof(getImpl().sonyFrameBuffer);
			if (!isFrameSizeSafe(frameSize, capacity, "sonyFrameBuffer")) {
				getImpl().acquireSuccess = false;
				return;
			}

			if (!getImpl().liveMode)
			{
#ifdef __USE_SPINNAKER__
				if (getCamera() == nullptr) {
					getImpl().acquireSuccess = false;
					return;
				}

				getCamera()->BeginAcquisition();
				getImpl().rawImage = getCamera()->GetNextImage();

				ImageProcessor processor;
				processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_DIRECTIONAL_FILTER);

				getImpl().processor.SetColorProcessing(SPINNAKER_COLOR_PROCESSING_ALGORITHM_DIRECTIONAL_FILTER);

				if (getImpl().rawImage == nullptr || getImpl().rawImage->IsIncomplete()) {
					getCamera()->EndAcquisition();
					getImpl().rawImage = NULL;
					getImpl().acquireSuccess = false;
					return;
				}

				auto convertedImage = getImpl().processor.Convert(getImpl().rawImage, PixelFormat_BayerGB16);
				if (convertedImage == nullptr || convertedImage->GetBufferSize() < frameSize) {
					getCamera()->EndAcquisition();
					getImpl().rawImage = NULL;
					getImpl().acquireSuccess = false;
					return;
				}

				std::memcpy(buffer, convertedImage->GetData(), frameSize);
				getCamera()->EndAcquisition();
				getImpl().rawImage = NULL;
#endif
			}
			else
			{
#ifdef __USE_SPINNAKER__
				if (getCamera() == nullptr) {
					getImpl().acquireSuccess = false;
					return;
				}
				setDefaultParametersForLive();
#endif

				width = getFrameWidth();
				height = getFrameHeight();
				frameSize = getFrameSizeInBytes();

				frameCount = 0;

#ifdef __USE_SPINNAKER__
				getCamera()->BeginAcquisition();
#endif

				do {
					frameCount++;

#ifdef __USE_SPINNAKER__
					getImpl().rawImage = getCamera()->GetNextImage();
					if (getImpl().rawImage == nullptr || getImpl().rawImage->IsIncomplete()) {
						getImpl().acquireSuccess = false;
						break;
					}
					if (getImpl().rawImage->GetBufferSize() < frameSize) {
						getImpl().acquireSuccess = false;
						break;
					}
					std::memcpy(buffer, getImpl().rawImage->GetData(), frameSize);
#endif
					unique_lock<mutex> lock(getImpl().mutexLock);
					if (getImpl().frameCallback != nullptr) {
						(*getImpl().frameCallback)(buffer, width, height, frameCount, getFlipMode(true));
					}

				} while (getImpl().liveMode);

#ifdef __USE_SPINNAKER__
				getCamera()->EndAcquisition();
				setDefaultParametersForSingle();
#endif
			}
		}

		else {
			unsigned char* buffer = getImpl().frameBuffer;
			size_t capacity = sizeof(getImpl().frameBuffer);
			if (!isFrameSizeSafe(frameSize, capacity, "frameBuffer")) {
				getImpl().acquireSuccess = false;
				return;
			}

			flushImagePipe();
			grabStart();

			do
			{
				frameCount = 0;
				do
				{
					getSensor().cmdFrameRequest();
					std::this_thread::sleep_for(std::chrono::milliseconds(10));

					if (getSensor().readFrame(buffer, frameSize))
					{
						if (getImpl().liveMode) {
							if (getImpl().frameCallback != nullptr) {
								(*getImpl().frameCallback)(buffer, width, height, frameCount, getFlipMode(true));
							}
						}
					}
					frameCount++;

				} while (frameCount < frameTotal);

			} while (getImpl().liveMode);

			grabStop();
		}
	}
	catch (const std::exception& e) {
		getImpl().acquireSuccess = false;
		getImpl().liveMode = false;
#ifdef __USE_SPINNAKER__
		if (GlobalSettings::useFundusFILR_Enable()) {
			auto cam = getCamera();
			if (cam && cam->IsStreaming()) {
				cam->EndAcquisition();
			}
		}
#endif
	}
	return;
}



void OctDevice::ColorCamera::acquireOriginalData(void)
{
	uint32_t width = getFrameWidth();
	uint32_t height = getFrameHeight();
	uint32_t frameSize = getFrameSizeInBytes();

	uint32_t frameCount;
	uint32_t frameTotal = getTotalFrames();

	if (GlobalSettings::useFundusFILR_Enable()) {

		unsigned char* buffer = getImpl().sonyFrameBuffer;
		size_t capacity = sizeof(getImpl().sonyFrameBuffer);
		if (!isFrameSizeSafe(frameSize, capacity, "sonyFrameBuffer")) {
			return;
		}
#ifdef __USE_SPINNAKER__
		if (getCamera() == nullptr) {
			LogE() << "acquireOriginalData: camera not available";
			return;
		}
		getCamera()->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
		getCamera()->ReverseX.SetValue(false);
		getCamera()->ReverseY.SetValue(false);
		getCamera()->BinningHorizontalMode.SetValue(BinningHorizontalMode_Average);
		getCamera()->BinningVerticalMode.SetIntValue(BinningVerticalMode_Average);
		getCamera()->BinningHorizontal.SetValue(1);
		getCamera()->BinningVertical.SetValue(1);
		getCamera()->Width.SetValue(4668);
		getCamera()->Height.SetValue(3648);
		getCamera()->OffsetX.SetValue(400);
#endif
		width = getFrameWidth();
		height = getFrameHeight();
		frameSize = getFrameSizeInBytes();

		int t = clock();

#ifdef __USE_SPINNAKER__
		getCamera()->BeginAcquisition();

		getImpl().rawImage = getCamera()->GetNextImage();
		if (getImpl().rawImage == nullptr || getImpl().rawImage->IsIncomplete()) {
			LogE() << "acquireOriginalData: incomplete image";
			getCamera()->EndAcquisition();
			return;
		}
		if (getImpl().rawImage->GetBufferSize() < frameSize) {
			LogE() << "acquireOriginalData: buffer smaller than frameSize";
			getCamera()->EndAcquisition();
			return;
		}
		std::memcpy(buffer, getImpl().rawImage->GetData(), frameSize);
#endif

		t = clock() - t;

		if (getImpl().frameCallback != nullptr) {
			(*getImpl().frameCallback)(buffer, width, height, t, getFlipMode(true));
		}
#ifdef __USE_SPINNAKER__
		getCamera()->EndAcquisition();

		getCamera()->ReverseX.SetValue(false);
		getCamera()->ReverseY.SetValue(true);
		getCamera()->BinningHorizontal.SetValue(1);
		getCamera()->BinningVertical.SetValue(1);
		getCamera()->Width.SetValue(4668);
		getCamera()->Height.SetValue(3648);
		getCamera()->OffsetX.SetValue(400);
#endif
	}

	else {
		unsigned char* buffer = getImpl().frameBuffer;

		flushImagePipe();
		grabStart();
		frameCount = 0;
		do
		{
			getSensor().cmdFrameRequest();
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			if (getSensor().readFrame(buffer, frameSize))
			{
				// processCameraData(frameCount);
				if (getImpl().frameCallback != nullptr) {
					(*getImpl().frameCallback)(buffer, width, height, frameCount, getFlipMode(true));
				}
			}
			frameCount++;

		} while (frameCount < frameTotal);
		// delete[] buffer;
		grabStop();
	}
	return;
}

std::wstring OctDevice::ColorCamera::getTimestampedFilename(const std::wstring& baseName, const std::wstring& extension = L".tiff") {
	std::time_t now = std::time(nullptr);
	std::tm local_tm;
	localtime_s(&local_tm, &now);

	wchar_t timeStr[16];
	std::wcsftime(timeStr, sizeof(timeStr) / sizeof(wchar_t), L"%H%M%S", &local_tm);

	std::wstringstream ss;
	ss << baseName << L"_" << timeStr << extension;
	return ss.str();
}

void OctDevice::ColorCamera::processCameraData(int frameCount)
{
	//auto start_time = std::chrono::high_resolution_clock::now();

	int width = getFrameWidth();
	int height = getFrameHeight();

	// mkdir for roi.
	string roitemp_folderPath = "D:\\HCT_DATA_TEMP";
	
	if (_access(roitemp_folderPath.c_str(), 0) != 0) {
		_mkdir(roitemp_folderPath.c_str());
	}
	//ini call 

	if (GlobalSettings::useFundusFILR_Enable()) {
		std::uint8_t* buffer = getImpl().sonyFrameBuffer;

		{
			unique_lock<mutex> lock(getImpl().mutexLock);
			if (getImpl().frameCallback != nullptr) {
				try {
					(*getImpl().frameCallback)(buffer, width, height, frameCount, getFlipMode(true));
				}
				catch (const std::exception& e) {
				}
				catch (...) {
				}
			}
		}

		convertBayer16ToRGB16(buffer, width, height);

	}
	else {
		std::uint8_t* buffer = getImpl().frameBuffer;
		{
			unique_lock<mutex> lock(getImpl().mutexLock);
			if (getImpl().frameCallback != nullptr) {
				try {
					(*getImpl().frameCallback)(buffer, width, height, frameCount, getFlipMode(true));
				}
				catch (const std::exception& e) {
				}
				catch (...) {
				}
			}
		}

		convertBayer16ToRGB16(buffer, width, height);
	}

	float focusValue = 0.0f;
	int flashLevel = 0;
	int fixation = 0;

	std::ifstream ifs("fundus_info.txt");
	if (ifs.is_open()) {
		// first : focusValueIn, second : flashValueIn, third : fixation
		// fixation ( macular : 0, disc : 20 , custom : 99 )
		ifs >> focusValue >> flashLevel >> fixation;
		ifs.close();

		getImpl().image.RemoveCalibration(focusValue, flashLevel);
		if (GlobalSettings::useSaveTiff()){
			SystemProfile::saveConfigFile("D:\\HCT_DATA_TEMP\\HctConfig.xml");
			getImpl().image.saveFile_tiff("D:\\HCT_DATA_TEMP\\raw.tiff");
		}

		std::remove("fundus_info.txt");
	}
	else {
		// Save Rawdata TIFF Type file
		getImpl().image.saveFile_tiff("raw.tiff");
		return;
		//
	}



	// XXX wrap it around with optiona
	getImpl().image.denoise();

	if (GlobalSettings::useColorImageCorrection() && GlobalSettings::useWhiteBalance()) {
		float param1 = (float)GlobalSettings::whiteBalanceParameters()[0];
		float param2 = (float)GlobalSettings::whiteBalanceParameters()[1];
		getImpl().image.balanceColorByGrayWorld(param1, param2);
		DebugOut2() << "Color camera image awb applied";
	}

	if (GlobalSettings::useColorImageCorrection() && GlobalSettings::useFundusLevelCorrection()) {
		if (GlobalSettings::useFundusFILR_Enable()) {
			auto xs = GlobalSettings::levelCorrectionCoordsX_FILR();
			auto ys = GlobalSettings::levelCorrectionCoordsY_FILR();
			auto roi_pos0 = GlobalSettings::startOfFundusROI();
			auto roi_pos1 = GlobalSettings::closeOfFundusROI();
			getImpl().image.processRadialColorCorrection(xs, ys, roi_pos0, roi_pos1, GlobalSettings::useFundusFILR_Enable());
		}
		else {
			auto xs = GlobalSettings::levelCorrectionCoordsX();
			auto ys = GlobalSettings::levelCorrectionCoordsY();
			auto roi_pos0 = GlobalSettings::startOfFundusROI();
			auto roi_pos1 = GlobalSettings::closeOfFundusROI();
			getImpl().image.processRadialColorCorrection(xs, ys, roi_pos0, roi_pos1);
		}


		DebugOut2() << "Color camera image radial corrected";
	}

	//convertRGB16ToRGB8();

	if (GlobalSettings::useColorImageCorrection() && GlobalSettings::useFundusROI()) {
		auto center = GlobalSettings::centerOfFundusROI();
		auto radius = GlobalSettings::radiusOfFundusROI();
		getImpl().image.applyMaskROI(center, radius, true);
	}

	CvImage image_lv6;
	std::thread thread_fundusLv4;
	auto usingLv4 = getImpl().usingLv4;

	if (usingLv4) {
		if (!image_lv6.deepCopy(getImpl().image)) {
		}
		else {
			// thread 2: image_lv6 
			thread_fundusLv4 = std::thread(&ColorCamera::processImageLv4Optimized, this, std::ref(image_lv6), focusValue, fixation);
		}
	}

	// main thread (thread 1): getImpl().image

	if (GlobalSettings::useFundusFILR_Enable()) {
		const double scale_ratio = 0.0125;
		if (!getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0)) {
		}
		//image_lv6.convert16UC3To8UC3();
	}

	else {
		const double scale_ratio = 0.0625;
		if (!getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0)) {
		}
	}

	{
		unique_lock<mutex> lock(getImpl().mutexLock);
		if (getImpl().imageCallback != nullptr) {
			try {
				unsigned char* idata = const_cast<unsigned char*>(getImpl().image.getBitsData());
				width = getImpl().image.getWidth();
				height = getImpl().image.getHeight();
				(*getImpl().imageCallback)(idata, width, height, frameCount);
			}
			catch (const std::exception& e) {
			}
			catch (...) {
			}
		}
	}

	// 20250717 : hwajunlee
	if (GlobalSettings::useRemoveLight() && ( fixation != 20 ) ) { /* && fixation == 0) { */
		auto light = GlobalSettings::removeReflectionLight1();
		auto shadow = GlobalSettings::removeReflectionLight2();
		auto outer = GlobalSettings::removeReflectionLight3();
		getImpl().image.applyRemoveReflectionLight(light, shadow, outer, focusValue);
		//image_lv6.applyRemoveReflectionLight(light, shadow, outer, focusValue);
	}

//#if true
   #if false
	// Seperate Camera Module
	if (GlobalSettings::useFundusFILR_Enable()) {

		image_lv6.ApplyGammaCorrectEnhanceContrast(&image_lv6);
		// White Balance
		if (GlobalSettings::isMeasureFundusWhiteBalance()) {
			auto b = GlobalSettings::getMeasureFundusWhiteBalanceB();
			auto g = GlobalSettings::getMeasureFundusWhiteBalanceG();
			auto r = GlobalSettings::getMeasureFundusWhiteBalanceR();
			image_lv6.AdjustWhiteBalance(b, g, r);
		}
		//
		//Centeral Brightness
		if (GlobalSettings::isMeasureFundusCentralBrightness()) {
			image_lv6.ApplyRadialBrightnessMask(1.5f, 2.0f);
		}
		// Domain Transform	
		if (GlobalSettings::isMeasureFundusDomainTransform) {
			float sigma_s = GlobalSettings::getMeasureFundusBrightness();
			float sigma_r = GlobalSettings::getMeasureFundusBrightnessGamma();
			image_lv6.domainTransform(sigma_s, sigma_r);   // parameter
		}

		// 20250717 : hwajunlee
		//if (GlobalSettings::useRemoveLight()) { /* && fixation == 0) { */
		//	auto light = GlobalSettings::removeReflectionLight1();
		//	auto shadow = GlobalSettings::removeReflectionLight2();
		//	auto outer = GlobalSettings::removeReflectionLight3();
		//	image_lv6.applyRemoveReflectionLight(light, shadow, outer);
		//}

		wstring wEnfacePath9 = L"D:\\HCT_DATA_TEMP\\fundus_Lv4.jpg";
		CT2CA convert9(wEnfacePath9.c_str());
		string sEnfacePath9(convert9);

		image_lv6.saveFile(sEnfacePath9, true);
	}

#endif
	//
	// double msec = CppUtil::ClockTimer::elapsedMsec();
	// DebugOut2() << "Process color elapsed: " << msec;

	if (thread_fundusLv4.joinable()) {
		thread_fundusLv4.join();
	}
	
	/*
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

	std::ofstream log_file("D:\\processCameraData_execution_time.txt", std::ios_base::app);
	if (log_file.is_open()) {
		log_file << "processCameraData execution time: " << duration.count() << " ms" << std::endl;
		log_file.close();
	}
	*/

	return;
}


void OctDevice::ColorCamera::convertBayer16ToRGB16(unsigned char* data, unsigned int width, unsigned int height)
{
	CvImage::FlipMode flipMode;
	if (GlobalSettings::useFundusFILR_Enable()) {
#ifdef __USE_SPINNAKER__
		if (getCamera() == nullptr) {
			LogE() << "convertBayer16ToRGB16: camera not available";
			return;
		}
		unsigned int ReverseX = getCamera()->ReverseX.GetValue();
		unsigned int ReverseY = getCamera()->ReverseY.GetValue();
		if (ReverseX == true && ReverseY == false)
			flipMode = CvImage::FlipMode::VERTICAL;
		else if (ReverseX == false && ReverseY == true)
			flipMode = CvImage::FlipMode::HORIZONTAL;
		else if (ReverseX == true && ReverseY == true)
			flipMode = CvImage::FlipMode::BOTH;
		else
#endif
			flipMode = CvImage::FlipMode::NONE;
	}
	else {
		switch (getImpl().sensorMode.flip) {
		case NO_FLIP:
			flipMode = CvImage::FlipMode::NONE; break;
		case VERTICAL_FLIP:
			flipMode = CvImage::FlipMode::VERTICAL; break;
		case HORIZONTAL_FLIP:
			flipMode = CvImage::FlipMode::HORIZONTAL; break;
		case (HORIZONTAL_FLIP | VERTICAL_FLIP):
			flipMode = CvImage::FlipMode::BOTH; break;
		}
	}
	// Raw camera pixel is 10 bit data.
	// getImpl().image.fromBayer16s(data, width, height, flipMode, 1.0);
	// getImpl().image.fromBayer16s(data, width, height, flipMode, 0.25
	getImpl().image.fromBayer16s(data, width, height, flipMode, GlobalSettings::useFundusFILR_Enable());
	return;
}


bool OctDevice::ColorCamera::convertRGB16ToRGB8(void)
{
	if (GlobalSettings::useFundusFILR_Enable()) {
		const double scale_ratio = 0.0125;
		if (!getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0)) {
			return false;
		}
	}

	else {
		const double scale_ratio = 0.0625;
		if (!getImpl().image.convertRGB16ToRGB8(scale_ratio, 0.0)) {
			return false;
		}
	}
	return true;
}


void OctDevice::ColorCamera::processImageLv4(CppUtil::CvImage& image_lv6, float focusValue, int fixation)
{
	// 1. 16bit to 8bit convert 
	if (GlobalSettings::useFundusFILR_Enable()) {
		image_lv6.convert16UC3To8UC3();
	}

	// 2. remove light
	if (GlobalSettings::useRemoveLight() && (fixation != 20)) {
		auto light = GlobalSettings::removeReflectionLight1();
		auto shadow = GlobalSettings::removeReflectionLight2();
		auto outer = GlobalSettings::removeReflectionLight3();
		image_lv6.applyRemoveReflectionLight(light, shadow, outer, focusValue);
	}

	// 3. FILR camera
	if (GlobalSettings::useFundusFILR_Enable()) {
		image_lv6.ApplyGammaCorrectEnhanceContrast(&image_lv6);

		if (GlobalSettings::isMeasureFundusWhiteBalance()) {
			auto b = GlobalSettings::getMeasureFundusWhiteBalanceB();
			auto g = GlobalSettings::getMeasureFundusWhiteBalanceG();
			auto r = GlobalSettings::getMeasureFundusWhiteBalanceR();
			image_lv6.AdjustWhiteBalance(b, g, r);
		}

		if (GlobalSettings::isMeasureFundusCentralBrightness()) {
			image_lv6.ApplyRadialBrightnessMask(1.5f, 2.0f);
		}

		if (GlobalSettings::isMeasureFundusDomainTransform) {
			float sigma_s = GlobalSettings::getMeasureFundusBrightness();
			float sigma_r = GlobalSettings::getMeasureFundusBrightnessGamma();
			image_lv6.domainTransform(sigma_s, sigma_r);
		}

		// 4. save file
		wstring wEnfacePath9 = L"D:\\HCT_DATA_TEMP\\fundus_Lv4.jpg";
		CT2CA convert9(wEnfacePath9.c_str());
		string sEnfacePath9(convert9);
		image_lv6.saveFile(sEnfacePath9, true);
	}
}

void OctDevice::ColorCamera::processImageLv4Optimized(CppUtil::CvImage& image_lv6, float focusValue, int fixation)
{
	CppUtil::FundusProcessParams params;
	params.useFundusFILR = GlobalSettings::useFundusFILR_Enable();
	params.useRemoveLight = GlobalSettings::useRemoveLight();
	params.removeLight1 = GlobalSettings::removeReflectionLight1();
	params.removeLight2 = GlobalSettings::removeReflectionLight2();
	params.removeLight3 = GlobalSettings::removeReflectionLight3();
	params.measureWhiteBalance = GlobalSettings::isMeasureFundusWhiteBalance();
	params.whiteBalanceB = GlobalSettings::getMeasureFundusWhiteBalanceB();
	params.whiteBalanceG = GlobalSettings::getMeasureFundusWhiteBalanceG();
	params.whiteBalanceR = GlobalSettings::getMeasureFundusWhiteBalanceR();
	params.measureCentralBrightness = GlobalSettings::isMeasureFundusCentralBrightness();
	params.measureDomainTransform = GlobalSettings::isMeasureFundusDomainTransform;
	params.fundusbrightness = GlobalSettings::getMeasureFundusBrightness();
	params.fundusbrightnessgamma = GlobalSettings::getMeasureFundusBrightnessGamma();

	image_lv6.processImageLv4Optimized(params, focusValue, fixation);
}