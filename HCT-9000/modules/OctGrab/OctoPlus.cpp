#include "stdafx.h"
#include "OctoPlus.h"
#include "OctoPlusUsb3_RegisterAddress.h"
#include "OctoPlusUsb3_Utility.h"

#include "CamCmosOctUsb3.h"
#include "Usb3Grabber.h"

#include "CppUtil2.h"
#include "OctDevice2.h"

#include <atomic>
#include <functional>
#include <mutex>

using namespace OctoPlus_USB_Register_Address;
using namespace OctGrab;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct OctoPlus::OctoPlusImpl
{
	bool initialized;
	atomic<bool> cameraOpened;
	atomic<bool> cameraWorking;
	atomic<bool> cameraStopping;

	size_t imageHeight = 1024;
	size_t numOfBuffer = 128;
	unsigned long imagesToAcquire = 0;
	Usb3Grabber* grabber;

	tCameraInfo CameraInfo;
	CAM_HANDLE hCamera;

	thread worker;

	OctoPlusImpl() : initialized(false), cameraOpened(false), grabber(nullptr),
		cameraStopping(false), cameraWorking(false), hCamera(NULL) {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<OctoPlus::OctoPlusImpl> OctoPlus::d_ptr(new OctoPlusImpl());


OctoPlus::OctoPlus()
{
}


OctoPlus::~OctoPlus()
{
}


bool OctGrab::OctoPlus::initializeLibrary(void)
{
	int nError = USB3_InitializeLibrary();
	if (nError != CAM_ERR_SUCCESS) {
		getImpl().initialized = false;
		LogD() << "USB3_InitializeLibrary Error: " << nError;
		LogD() << getErrorText(nError);
	}
	else {
		getImpl().initialized = true;
		LogD() << "Usb3 Cmos camera library initialized" << nError;
	}

	getImpl().cameraOpened = false;
	getImpl().cameraWorking = false;
	getImpl().cameraStopping = false;
	return getImpl().initialized;
}


void OctGrab::OctoPlus::releaseLibrary(void)
{
	if (getImpl().initialized) {
		int nError = USB3_TerminateLibrary();
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_TerminateLibrary Error: " << nError;
			LogD() << getErrorText(nError);
		}
	}
	return;
}


bool OctGrab::OctoPlus::isLibraryInitialized(void)
{
	return getImpl().initialized;
}


std::string OctGrab::OctoPlus::getErrorText(int errCode)
{
	char text[512];
	size_t size = sizeof(text);

	if (USB3_GetErrorText(errCode, text, &size) != CAM_ERR_SUCCESS) {
		text[0] = '\0';
	}
	return string(text);
}


void OctGrab::OctoPlus::setupUsb3Grabber(Usb3Grabber * grabber)
{
	getImpl().grabber = grabber;
	return;
}


bool OctGrab::OctoPlus::openLineCamera(void)
{
	getImpl().cameraOpened = false;
	getImpl().hCamera = NULL;

	unsigned long ulNbCameras;
	int nError = USB3_UpdateCameraList(&ulNbCameras);

	if (nError != CAM_ERR_SUCCESS) {
		LogD() << "USB3_UpdateCameraList Error: " << nError;
		LogD() << getErrorText(nError);
		return false;
	}
	if (ulNbCameras == 0) {
		LogD() << "Usb3 cmos camera not found";
		return false;
	}

	nError = USB3_GetCameraInfo(0, &getImpl().CameraInfo);
	if (nError != CAM_ERR_SUCCESS) {
		LogD() << "USB3_GetCameraInfo Error: " << nError;
		LogD() << getErrorText(nError);
		return false;
	}
	else {
		LogD() << "Usb3 cmos opened, id: " << getImpl().CameraInfo.pcID;
	}
	
	try {
		nError = USB3_OpenCamera(&getImpl().CameraInfo, &getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_OpenCamera Error: " << nError;
			LogD() << getErrorText(nError);
			return false;
		}

		getImpl().imageHeight = 0;
		getImpl().cameraOpened = true;

		writeRegister(OctoPlus_USB_Register_Address::cUl_Address_GPI_Format, 1);
		writeRegister(OctoPlus_USB_Register_Address::cUl_Address_Max_Trriger_Period, (int)(300/20.48));

		// Read device id
		{
			char buf[65] = { 0 };
			size_t sz = sizeof(buf) - 1;
			int err = USB3_ReadRegister(getImpl().hCamera, OctoPlus_USB_Register_Address::cUl_Address_Device_ID, buf, &sz);
			if (err == CAM_ERR_SUCCESS) {
				buf[sz] = '\0';
				LogI() << "Usb3 cmos camera Device ID: " << buf;
			}
			else {
				LogD() << "DeviceID read failed, addr: 0x" << hex << OctoPlus_USB_Register_Address::cUl_Address_Device_ID << dec << ", err: " << err;
			}
		}

		/*
		size_t iImageHeight = 100;
		size_t iNbOfBuffer = 10;
		printf("USB3_SetImageParameters\n");
		nError = USB3_SetImageParameters(getImpl().hCamera, iImageHeight, iNbOfBuffer);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_SetImageParameters Error: " << nError;
			LogD() << getErrorText(nError);
			return false;
		}
		*/
	}
	catch (...) {
		LogD() << "Unknown exception during opening Usb3 cmos camera";
	}
	return getImpl().cameraOpened;
}


void OctGrab::OctoPlus::closeLineCamera(void)
{
	if (getImpl().hCamera != NULL) {
		int nError = USB3_CloseCamera(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_CloseCamera Error: " << nError;
			LogD() << getErrorText(nError);
		}
	}
	return;
}


bool OctGrab::OctoPlus::isLineCameraOpened(void)
{
	return getImpl().cameraOpened;
}


bool OctGrab::OctoPlus::isLineCameraGrabbing(void)
{
	return isLineCameraWorking();
}


bool OctGrab::OctoPlus::startAcquisition(bool restart)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		if (!restart) {
			return true;
		}
		cancelAcquisition();
	}

	try {
		int nError = USB3_StartAcquisition(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_StartAcquisition Error: " << nError;
			LogD() << getErrorText(nError);
			cancelAcquisition();
			return false;
		}
		getImpl().cameraWorking = true;
	}
	catch (...) {
		LogD() << "Usb cmos camera : startAcquisition exception occurred!";
	}
	return true;
}


void OctGrab::OctoPlus::cancelAcquisition(void)
{
	if (!isLineCameraOpened()) {
		return;
	}
	if (!isLineCameraWorking()) {
		return;
	}
	
	getImpl().cameraWorking = false;

	try {
		int nError = USB3_AbortGetBuffer(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_AbortGetBuffer Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}

		nError = USB3_StopAcquisition(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_StopAcquisition Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}

		nError = USB3_FlushBuffers(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_FlushBuffers Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : cancelAcquisition exception occurred!";
	}
	return;
}


bool OctGrab::OctoPlus::acquireLineCameraImage(int count)
{
	int nError;
	tImageInfos ImageInfos;
	unsigned long imagesAcquired = 0;
	unsigned long imagesToAcquire = count;

	const unsigned long MAX_TIMEOUT_ACQ_IN_MS = 3000;
	
	try {
		while (imagesAcquired < imagesToAcquire) {
			nError = USB3_GetBuffer(getImpl().hCamera, &ImageInfos, MAX_TIMEOUT_ACQ_IN_MS);

			if (nError == CAM_ERR_SUCCESS) {
				imagesAcquired += 1;

				nError = USB3_RequeueBuffer(getImpl().hCamera, ImageInfos.hBuffer);
				if (nError != CAM_ERR_SUCCESS) {
				}
			}
			else {
				if (nError == CAM_ERR_TIMEOUT) {
					LogD() << "USB3_GetBuffer timeout!";
				}
				else {
					LogD() << "USB3_GetBuffer Error: " << nError;
				}
				LogD() << getErrorText(nError);
				break;
			}
		}

		if (imagesToAcquire == imagesAcquired) {
			return true;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : threadLineCameraImage exception occurred!";
	}
	return false;
}


bool OctGrab::OctoPlus::acquirePreviewImages(std::vector<int> bufferIds)
{
	int nError;
	tImageInfos ImageInfos;
	size_t imagesAcquired = 0;
	size_t imagesToAcquire = bufferIds.size();

	static int frame_count = 0;
	static unsigned __int64 timer_tick = GetTickCount64();
	const unsigned long MAX_TIMEOUT_ACQ_IN_MS = 5000;

	try {
		while (imagesAcquired < imagesToAcquire) {
			// auto t1 = GetTickCount64();
			nError = USB3_GetBuffer(getImpl().hCamera, &ImageInfos, MAX_TIMEOUT_ACQ_IN_MS);
			// auto t2 = GetTickCount64();
			// LogD() << "USB3_GetBuffer: " << (t2 - t1)/1000.0f;

			if (nError == CAM_ERR_SUCCESS) {
				if (getImpl().grabber != nullptr) {
					int buffId = bufferIds[imagesAcquired];
					auto buffer = (unsigned short*)ImageInfos.pDatas;
					auto dsize = ImageInfos.iImageSize;

					// auto t1 = GetTickCount64();
					getImpl().grabber->hookGrabPreviewBuffer(buffer, buffId, dsize);
					// auto t2 = GetTickCount64();
					// LogD() << "hookGrabPreviewBuffer: " << (t2 - t1) / 1000.0f;

					frame_count++;
					if (frame_count > 0 && frame_count % 100 == 0) {
						auto curr_tick = GetTickCount64();
						LogD() << "Usb camera 100 frames grabbed in " << (curr_tick - timer_tick) / 1000.0f << "ms";
						timer_tick = curr_tick;
						frame_count = 0;
					}
				}

				imagesAcquired += 1;

				// auto t1 = GetTickCount64();
				nError = USB3_RequeueBuffer(getImpl().hCamera, ImageInfos.hBuffer);
				// auto t2 = GetTickCount64();
				// LogD() << "USB3_RequeueBuffer: " << (t2 - t1) / 1000.0f;

				if (nError != CAM_ERR_SUCCESS) {
					LogD() << "USB3_RequeueBuffer Error: " << nError;
					LogD() << getErrorText(nError);
					break;
				}
			}
			else {
				LogD() << "USB3_GetBuffer Error: " << nError;
				LogD() << getErrorText(nError);
				break;
			}
		}

		if (imagesToAcquire == imagesAcquired) {
			return true;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : acquirePreviewImages exception occurred!";
	}
	return false;
}


bool OctGrab::OctoPlus::acquireMeasureImages(std::vector<int> bufferIds)
{
	int nError;
	tImageInfos ImageInfos;
	size_t imagesAcquired = 0;
	size_t imagesToAcquire = bufferIds.size();

	const unsigned long MAX_TIMEOUT_ACQ_IN_MS = 5000;

	try {
		while (imagesAcquired < imagesToAcquire) {
			nError = USB3_GetBuffer(getImpl().hCamera, &ImageInfos, MAX_TIMEOUT_ACQ_IN_MS);
			//double msec1 = CppUtil::ClockTimer::elapsedMsec();
			//LogD() << "Usb3GetBuffer elapsed: " << msec1;

			if (nError == CAM_ERR_SUCCESS) {
				if (getImpl().grabber != nullptr) {
					int buffId = bufferIds[imagesAcquired];
					auto buffer = (unsigned short*)ImageInfos.pDatas;
					auto dsize = ImageInfos.iImageSize;
					getImpl().grabber->hookGrabMeasureBuffer(buffer, buffId, dsize);
				}
				//double msec2 = CppUtil::ClockTimer::elapsedMsec();
				//LogD() << "HookGrabMeasureBuffer elapsed: " << msec2;

				imagesAcquired += 1;
				nError = USB3_RequeueBuffer(getImpl().hCamera, ImageInfos.hBuffer);
				if (nError != CAM_ERR_SUCCESS) {
					LogD() << "USB3_RequeueBuffer Error: " << nError;
					LogD() << getErrorText(nError);
					break;
				}

				//double msec3 = CppUtil::ClockTimer::elapsedMsec();
				//LogD() << "Usb3RequeueBuffer elapsed: " << msec3;

			}
			else {
				LogD() << "USB3_GetBuffer Error: " << nError;
				LogD() << getErrorText(nError);
				break;
			}
		}

		if (imagesToAcquire == imagesAcquired) {
			return true;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : acquireMeasureImages exception occurred!";
	}
	return false;
}


bool OctGrab::OctoPlus::acquireEnfaceImages(std::vector<int> bufferIds)
{
	int nError;
	tImageInfos ImageInfos;
	size_t imagesAcquired = 0;
	size_t imagesToAcquire = bufferIds.size();

	const unsigned long MAX_TIMEOUT_ACQ_IN_MS = 5000;

	try {
		while (imagesAcquired < imagesToAcquire) {
			nError = USB3_GetBuffer(getImpl().hCamera, &ImageInfos, MAX_TIMEOUT_ACQ_IN_MS);

			if (nError == CAM_ERR_SUCCESS) {
				if (getImpl().grabber != nullptr) {
					int buffId = bufferIds[imagesAcquired];
					auto buffer = (unsigned short*)ImageInfos.pDatas;
					auto dsize = ImageInfos.iImageSize;
					getImpl().grabber->hookGrabEnfaceBuffer(buffer, buffId, dsize);
				}

				imagesAcquired += 1;
				nError = USB3_RequeueBuffer(getImpl().hCamera, ImageInfos.hBuffer);
				if (nError != CAM_ERR_SUCCESS) {
					LogD() << "USB3_RequeueBuffer Error: " << nError;
					LogD() << getErrorText(nError);
					break;
				}
			}
			else {
				LogD() << "USB3_GetBuffer Error: " << nError;
				LogD() << getErrorText(nError);
				break;
			}
		}

		if (imagesToAcquire == imagesAcquired) {
			return true;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : acquireEnfaceImages exception occurred!";
	}
	return false;
}


void OctGrab::OctoPlus::threadLineCameraImage(const OctoPlus::OctoPlusImpl* impl)
{
	int nError;
	tImageInfos ImageInfos;
	unsigned long timeoutAcq = 3000;
	unsigned long imagesAcquired = 0;

	const unsigned long MAX_TIMEOUT_ACQ_IN_MS = 3000;

	try {
		while (true) {
			if (impl->cameraStopping) {
				break;
			}
			if (impl->imagesToAcquire == imagesAcquired) {
				break;
			}

			nError = USB3_GetBuffer(impl->hCamera, &ImageInfos, timeoutAcq);

			if (!impl->cameraStopping) {
				if (nError == CAM_ERR_SUCCESS) {
					imagesAcquired += 1;

					nError = USB3_RequeueBuffer(impl->hCamera, ImageInfos.hBuffer);
					if (nError != CAM_ERR_SUCCESS) {
					}
				}
				else {
					if (nError == CAM_ERR_TIMEOUT) {
						LogD() << "USB3_GetBuffer timeout!";
					}
					else {
						LogD() << "USB3_GetBuffer Error: " << nError;
					}
				}
			}
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : threadLineCameraImage exception occurred!";
	}
	return;
}


bool OctGrab::OctoPlus::startAcquisition2(unsigned long imagesToAcquire)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraWorking()) {
		cancelAcquisition2();
	}

	getImpl().worker = thread{ &OctoPlus::threadLineCameraImage, &getImpl() };
	getImpl().cameraWorking = true;

	try {
		int nError = USB3_StartAcquisition(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_StartAcquisition Error: " << nError;
			LogD() << getErrorText(nError);
			cancelAcquisition2();
			return false;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : startAcquisition exception occurred!";
	}
	return true;
}


void OctGrab::OctoPlus::cancelAcquisition2(void)
{
	if (!isLineCameraWorking()) {
		return;
	}

	getImpl().cameraStopping = true;
	if (getImpl().worker.joinable()) {
		getImpl().worker.join();
	}
	getImpl().cameraWorking = false;
	getImpl().cameraStopping = false;

	try {
		int nError = USB3_AbortGetBuffer(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_AbortGetBuffer Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}

		nError = USB3_StopAcquisition(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_StopAcquisition Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}

		nError = USB3_FlushBuffers(getImpl().hCamera);
		if (nError != CAM_ERR_SUCCESS) {
			LogD() << "USB3_FlushBuffers Error: " << nError;
			LogD() << getErrorText(nError);
			return;
		}
	}
	catch (...) {
		LogD() << "Usb cmos camera : cancelAcquisition exception occurred!";
	}
	return;
}


bool OctGrab::OctoPlus::isLineCameraWorking(void)
{
	return getImpl().cameraWorking;
}

int OctGrab::OctoPlus::getImageHeight(void)
{
	return (int)getImpl().imageHeight;
}


void OctGrab::OctoPlus::setImageHeight(size_t height)
{
	getImpl().imageHeight = height;
	return;
}

void OctGrab::OctoPlus::setNumberOfBuffers(size_t number)
{
	getImpl().numOfBuffer = number;
	return;
}

bool OctGrab::OctoPlus::setImageParameters(int imageHeight, int numOfBuffer)
{
	if (numOfBuffer == 0) {
		numOfBuffer = 64;
	}
	int nError = USB3_SetImageParameters(getImpl().hCamera, imageHeight, numOfBuffer);
	if (nError != CAM_ERR_SUCCESS) {
		LogD() << "USB3_SetImageParameters Error: " << nError;
		LogD() << getErrorText(nError);
		return false;
	}
	else {
		getImpl().imageHeight = imageHeight;
		getImpl().numOfBuffer = numOfBuffer;
		LogD() << "Usb cmos camera updated, height: " << getImpl().imageHeight << ", buffers: " << getImpl().numOfBuffer;
		USB3_FlushBuffers(getImpl().hCamera);
	}
	return true;
}


bool OctGrab::OctoPlus::updateImageParameters(int imageHeight, int numOfBuffer)
{
	bool ret = setImageParameters(imageHeight, numOfBuffer);
	return ret;
}


bool OctGrab::OctoPlus::setTriggerMode(ETriggerMode mode)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		return false;
	}

	auto value = getValueFromTriggerMode(mode);
	if (value < 0 || !writeRegister(cUl_Address_Trigger_Mode, value)) {
		LogD() << "OctoPlus::setTriggerMode() failed, mode: " << getStringTriggerMode(mode);
		return false;
	}
	LogD() << "OctoPlus::setTriggerMode(): " << getStringTriggerMode(mode);
	return true;
}


bool OctGrab::OctoPlus::getTriggerMode(ETriggerMode * mode)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Trigger_Mode, &value)) {
		LogD() << "OctoPlus::getTriggerMode() failed";
		return false;
	}

	*mode = getTriggerModeFromValue(value);
	LogD() << "OctoPlus::getTriggerMode(): " << getStringTriggerMode(*mode);
	return true;
}

bool OctGrab::OctoPlus::setBufferType(EBufferType type)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		return false;
	}

	auto value = getValueFromBufferType(type);
	if (value < 0 || !writeRegister(cUl_Address_Buffer_Type, value)) {
		LogD() << "OctoPlus::setBufferType() failed, type: " << getStringBufferType(type);
		return false;
	}
	LogD() << "OctoPlus::setBufferType(): " << getStringBufferType(type);
	return true;
}

bool OctGrab::OctoPlus::getBufferType(EBufferType * type)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Buffer_Type, &value)) {
		LogD() << "OctoPlus::getBufferType() failed";
		return false;
	}

	*type = getBufferTypeFromValue(value);
	LogD() << "OctoPlus::getBufferType(): " << getStringBufferType(*type);
	return true;
}


bool OctGrab::OctoPlus::setOutputMode(EOutputMode mode)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		return false;
	}

	auto value = getValueFromOutputMode(mode);
	if (value < 0 || !writeRegister(cUl_Address_Output_Mode, value)) {
		LogD() << "OctoPlus::setOutputMode() failed, mode: " << getStringOutputMode(mode);
		return false;
	}
	LogD() << "OctoPlus::setOutputMode(): " << getStringOutputMode(mode);
	return true;
}

bool OctGrab::OctoPlus::getOutputMode(EOutputMode * mode)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Output_Mode, &value)) {
		LogD() << "OctoPlus::getOutputMode() failed";
		return false;
	}

	*mode = getOutputModeFromValue(value);
	LogD() << "OctoPlus::getOutputMode(): " << getStringOutputMode(*mode);
	return true;
}

std::string OctGrab::OctoPlus::getTriggerMode(void)
{
	ETriggerMode mode = ETriggerMode::Mode_Trigger_Unknown;
	if (getTriggerMode(&mode)) {
	}
	return getStringTriggerMode(mode);
}

std::string OctGrab::OctoPlus::getBufferType(void)
{
	EBufferType type = EBufferType::Type_Unknown;
	if (getBufferType(&type)) {
	}
	return getStringBufferType(type);
}

std::string OctGrab::OctoPlus::getOutputMode(void)
{
	EOutputMode mode = EOutputMode::Mode_Pixel_Unknown;
	if (getOutputMode(&mode)) {
	}
	return getStringOutputMode(mode);
}

bool OctGrab::OctoPlus::setEnableCircularBuffer(bool enable)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		return false;
	}

	auto value = enable;
	if (!writeRegister(cUl_Address_Circular_Buffer, value)) {
		LogD() << "OctoPlus::setEnableCircularBuffer() failed";
		return false;
	}
	LogD() << "OctoPlus::setEnableCircularBuffer(): " << enable;
	return true;
}

bool OctGrab::OctoPlus::isEnabledCircularBuffer(void)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Circular_Buffer, &value)) {
		LogD() << "OctoPlus::isEnabledCircularBuffer() failed";
		return false;
	}

	LogD() << "OctoPlus::isEnabledCircularBuffer(): " << value;
	return true;
}

bool OctGrab::OctoPlus::setLinePeriod(double time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}
	if (isLineCameraGrabbing()) {
		return false;
	}

	int value = MicroSecToTimeStep(time_us);
	if (!writeRegister(cUl_Address_Line_Period, value)) {
		LogD() << "OctoPlus::setLinePeriod(), time_us: " << time_us << " failed";
		return false;
	}
	LogD() << "OctoPlus::setLinePeriod(), time_us: " << time_us;
	return true;
}

bool OctGrab::OctoPlus::getLinePeriod(double * time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Line_Period, &value)) {
		LogD() << "OctoPlus::getLinePeriod() failed";
		return false;
	}

	*time_us = TimeStepToMicroSec(value);
	LogD() << "OctoPlus::getLinePeriod(), time_us: " << *time_us;
	return true;
}

bool OctGrab::OctoPlus::getLinePeriodMin(double * time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Line_Period_Min, &value)) {
		LogD() << "OctoPlus::getLinePeriodMin() failed";
		return false;
	}

	*time_us = TimeStepToMicroSec(value);
	LogD() << "OctoPlus::getLinePeriodMin(), time_us: " << *time_us;
	return true;
}

bool OctGrab::OctoPlus::setExposureTime(double time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = MicroSecToTimeStep(time_us);
	if (!writeRegister(cUl_Address_Exposure_Time, value)) {
		LogD() << "OctoPlus::setExposureTime() failed, time_us: " << time_us;
		return false;
	}
	LogD() << "OctoPlus::setExposureTime(), time_us: " << time_us;
	return true;
}

bool OctGrab::OctoPlus::getExposureTime(double * time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Exposure_Time, &value)) {
		LogD() << "OctoPlus::getExposureTime() failed";
		return false;
	}

	*time_us = TimeStepToMicroSec(value);
	LogD() << "OctoPlus::getExposureTime(), time_us: " << *time_us;
	return true;
}

bool OctGrab::OctoPlus::getExposureTimeMin(double * time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Exposure_Time_Min, &value)) {
		LogD() << "OctoPlus::getExposureTimeMin() failed";
		return false;
	}

	*time_us = TimeStepToMicroSec(value);
	LogD() << "OctoPlus::getExposureTimeMin(), time_us: " << *time_us;
	return true;
}

bool OctGrab::OctoPlus::getExposureTimeMax(double * time_us)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Exposure_Time_Max, &value)) {
		LogD() << "OctoPlus::getExposureTimeMax() failed";
		return false;
	}

	*time_us = TimeStepToMicroSec(value);
	LogD() << "OctoPlus::getExposureTimeMax(), time_us: " << *time_us;
	return true;
}

bool OctGrab::OctoPlus::setAnalogGain(EAnalogGain gain)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = getValueFromAnalogGain(gain);
	if (value < 0 || !writeRegister(cUl_Address_Analog_Gain, value)) {
		LogD() << "OctoPlus::setAnalogGain() failed, value: " << value;
		return false;
	}
	LogD() << "OctoPlus::setAnalogGain(): " << getNumericAnalogGain(gain);
	return true;
}

bool OctGrab::OctoPlus::getAnalogGain(double * gain)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Analog_Gain, &value)) {
		LogD() << "OctoPlus::getAnalogGain() failed";
		return false;
	}

	auto mode = getAnalogGainFromValue(value);
	*gain = getNumericAnalogGain(mode);
	LogD() << "OctoPlus::getAnalogGain(): " << *gain;
	return true;
}

bool OctGrab::OctoPlus::setDigitalGain(double gain)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = DigitalGainValueToStep(gain);
	if (!writeRegister(cUl_Address_Digital_Gain, value)) {
		LogD() << "OctoPlus::setDigitalGain() failed, value: " << gain;
		return false;
	}
	LogD() << "OctoPlus::setDigitalGain(): " << gain;
	return true;
}

bool OctGrab::OctoPlus::getDigitalGain(double * gain)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	int value = 0;
	if (!readRegister(cUl_Address_Digital_Gain, &value)) {
		LogD() << "OctoPlus::getDigitalGain() failed";
		return false;
	}

	*gain = DigitalGainStepToValue(value);
	LogD() << "OctoPlus::getDigitalGain(): " << *gain;
	return true;
}

bool OctGrab::OctoPlus::writeRegister(unsigned long address, int value)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	size_t size = sizeof(int);
	auto ret = USB3_WriteRegister(getImpl().hCamera, address, &value, &size);
	if (ret != CAM_ERR_SUCCESS) {
		LogD() << "USB3_WriteRegister error: " << getErrorText(ret);
		return false;
	}
	return true;
}

bool OctGrab::OctoPlus::readRegister(unsigned long address, int * value)
{
	if (!isLineCameraOpened()) {
		return false;
	}

	size_t size = sizeof(int);
	auto ret = USB3_ReadRegister(getImpl().hCamera, address, value, &size);
	if (ret != CAM_ERR_SUCCESS) {
		LogD() << "USB3_ReadRegister error: " << getErrorText(ret);
		return false;
	}

	return true;
}

OctoPlus::OctoPlusImpl & OctGrab::OctoPlus::getImpl(void)
{
	return *d_ptr;
}
