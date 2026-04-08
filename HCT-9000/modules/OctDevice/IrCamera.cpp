#include "stdafx.h"
#include "IrCamera.h"
#include "MainBoard.h"
#include "UsbComm.h"
#include "UsbPort.h"

#include <atomic>
#include <functional>
#include <mutex>

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct IrCamera::IrCameraImpl
{
	MainBoard *board;
	IrCameraId camId;
	std::uint8_t epid;
	bool initiated;
	unsigned int errorCount;
	unsigned int frameCount;

	uint8_t buffer[IR_CAMERA_FRAME_DATA_SIZE];
	uint32_t frameWidth;
	uint32_t frameHeight;
	OctFrameImage descript;

	atomic<bool> playing;
	thread worker;

	float analogGain;
	float digitalGain;
	uint16_t exposureTime;

	CameraType type;
	IrCameraFrameCallback* callback;
	mutex mutexLock;

	IrCameraImpl() : board(nullptr), playing(false), initiated(false), errorCount(0), frameCount(0),
					callback(nullptr), 
					analogGain(IR_CAMERA_AGAIN_VALUE1), 
					digitalGain(IR_CAMERA_DGAIN_MIN), exposureTime(IR_CAMERA_EXPOSURE_TIME1),
					buffer { 0 }, frameWidth(IR_CAMERA_FRAME_WIDTH), frameHeight(IR_CAMERA_FRAME_HEIGHT)
	{
	}
};


IrCamera::IrCamera() :
	d_ptr(make_unique<IrCameraImpl>())
{
}


OctDevice::IrCamera::IrCamera(MainBoard * board, CameraType type) :
	d_ptr(make_unique<IrCameraImpl>())
{
	getImpl().type = type;
	getImpl().board = board;
}


IrCamera::~IrCamera()
{
}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::IrCamera::IrCamera(IrCamera && rhs) = default;
IrCamera & OctDevice::IrCamera::operator=(IrCamera && rhs) = default;


bool OctDevice::IrCamera::initialize(void)
{
	if (getMainBoard()->isNoCamera()) {
		return true;
	}

	if (getImpl().board == nullptr) {
		return false;
	}

	// getImpl().board = board;
	IrCameraId camId = (getImpl().type == CameraType::CORNEA ? IrCameraId::CORNEA : IrCameraId::RETINA);
	getImpl().camId = camId;
	getImpl().epid = (camId == IrCameraId::CORNEA ? USB_IR1_PID : USB_IR2_PID);
	getImpl().callback = nullptr;
	getImpl().initiated = true;

	UsbComm& usbComm = getImpl().board->getUsbComm();
	bool res = usbComm.IrCameraControl(static_cast<unsigned char>(getImpl().camId), CTRL_OFF);
	return res;
}


bool OctDevice::IrCamera::isInitiated(void) const
{
	return getImpl().initiated;
}


void OctDevice::IrCamera::setCallback(IrCameraFrameCallback* callback)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	getImpl().callback = callback;
	return;
}


bool OctDevice::IrCamera::isPlaying(void) const
{
	return getImpl().playing;
}


bool OctDevice::IrCamera::play(void)
{
	if (!isInitiated()) {
		return false;
	}


	// if (isPlaying()) {
		pause();
	// }


	UsbComm& usbComm = getImpl().board->getUsbComm();
	bool ret = usbComm.IrCameraControl(static_cast<unsigned char>(getImpl().camId), CTRL_ON);

	getImpl().playing = true;
	getImpl().worker = thread{ &IrCamera::acquireCameraData, this };
	getImpl().errorCount = 0;
	getImpl().frameCount = 0;
	return ret;
}


void OctDevice::IrCamera::pause(void)
{
	if (!isInitiated()) {
		return ;
	}

	// if (isPlaying()) {
		getImpl().playing = false;
		if (getImpl().worker.joinable()) {
			getImpl().worker.join();
		}

		UsbComm& usbComm = getImpl().board->getUsbComm();
		usbComm.IrCameraControl(static_cast<unsigned char>(getImpl().camId), CTRL_OFF);
	// }
	return;
}


int OctDevice::IrCamera::getErrorCount(void)
{
	return getImpl().errorCount;
}

int OctDevice::IrCamera::getFrameCount(void)
{
	return getImpl().frameCount;
}


bool OctDevice::IrCamera::setAnalogGain(float gain, bool control)
{
	if (!isInitiated()) {
		return false;
	}

	getImpl().analogGain = gain;

	if (control) {
		unsigned char data = getAnalogGainData(gain);
		
		UsbComm& usbComm = getImpl().board->getUsbComm();
		bool ret = usbComm.IrCameraAnalogGain(static_cast<unsigned char>(getImpl().camId), (uint8_t)data);
		return ret;
	}
	else {
		return true;
	}
}


bool OctDevice::IrCamera::setDigitalGain(float gain, bool control)
{
	if (!isInitiated()) {
		return false;
	}

	gain = (gain < IR_CAMERA_DGAIN_MIN ? IR_CAMERA_DGAIN_MIN : gain);
	gain = (gain > IR_CAMERA_DGAIN_MAX ? IR_CAMERA_DGAIN_MAX : gain);
	getImpl().digitalGain = gain;

	if (control) {
		// dgain = x + y * 2^-5
		// bit (7:5) = x
		// bit (4:0) = y
		unsigned char x, y;
		x = static_cast<unsigned char>(std::floor(gain));
		y = static_cast<unsigned char>((gain - x) * 32);
		unsigned char data = (x << 5) + y;

		UsbComm& usbComm = getImpl().board->getUsbComm();
		bool ret = usbComm.IrCameraDigitalGain(static_cast<unsigned char>(getImpl().camId), data);
		return ret;
	}
	else {
		return true;
	}
}


float OctDevice::IrCamera::getAnalogGain(void) const
{
	return getImpl().analogGain;
}


float OctDevice::IrCamera::getDigitalGain(void) const
{
	return getImpl().digitalGain;
}


bool OctDevice::IrCamera::setExposureTime(bool highSpeed)
{
	getImpl().exposureTime = (highSpeed ? IR_CAMERA_EXPOSURE_TIME1 : IR_CAMERA_EXPOSURE_TIME2);
	UsbComm& usbComm = getImpl().board->getUsbComm();
	bool ret = usbComm.IrCameraExposureTime(static_cast<unsigned char>(getImpl().camId), getImpl().exposureTime);
	return ret;
}


bool OctDevice::IrCamera::isHighSpeedMode(void)
{
	return (getImpl().exposureTime == IR_CAMERA_EXPOSURE_TIME1);
}


std::uint8_t OctDevice::IrCamera::getAnalogGainData(float gain) const
{
	// bit (7:5) : ignore
	// bit (4)   : second stage (0:1x), (1:1.25x)
	// bit (3:0) : first stage (1:1x), (2:2x), (4:4x), (8:8x
	unsigned char first = 1;
	unsigned char second = 0;
	float value = (float)gain;

	if (value == 1.0f) {
		first = 1;  second = 0;
	}
	else if (value == 1.25f) {
		first = 1;  second = 16;
	}
	else if (value == 2.0f) {
		first = 2;  second = 0;
	}
	else if (value == 2.5f) {
		first = 2;  second = 16;
	}
	else if (value == 4.0f) {
		first = 4;  second = 0;
	}
	else if (value == 5.0f) {
		first = 4;  second = 16;
	}
	else if (value == 8.0f) {
		first = 8;  second = 0;
	}
	else if (value == 10.0f) {
		first = 8;  second = 16;
	}
	else {
		// throw std::runtime_error("Invalid analog gain value");
	}

	unsigned char data = first + second;
	return data;
}


std::uint8_t OctDevice::IrCamera::getDigitalGainData(float gain) const
{
	unsigned char x, y;
	x = static_cast<unsigned char>(std::floor(gain));
	y = static_cast<unsigned char>((gain - x) * 32);
	unsigned char data = (x << 5) + y;

	return data;
}


CameraType OctDevice::IrCamera::getType(void) const
{
	return getImpl().type;
}

std::string OctDevice::IrCamera::getCameraName(void) const
{
	auto type = getType();
	if (type == CameraType::CORNEA) {
		return "Cornea Ir";
	}
	else if (type == CameraType::RETINA) {
		return "Retina Ir";
	}
	return std::string();
}


std::uint8_t * OctDevice::IrCamera::getBuffer(void) const
{
	return getImpl().buffer;
}


std::uint32_t OctDevice::IrCamera::getFrameWidth(void) const
{
	return getImpl().frameWidth;
}


std::uint32_t OctDevice::IrCamera::getFrameHeight(void) const
{
	return getImpl().frameHeight;
}


std::uint32_t OctDevice::IrCamera::getFrameSize(void) const
{
	return getFrameWidth() * getFrameHeight();
}


bool OctDevice::IrCamera::captureFrame(std::uint8_t * frame) const
{
	if (!frame) {
		return false;
	}

	unique_lock<mutex> lock(getImpl().mutexLock);
	memcpy(frame, getImpl().buffer, getFrameSize());
	return true;
}


bool OctDevice::IrCamera::captureFrame(CppUtil::CvImage& image) const
{
	uint8_t frame[IR_CAMERA_FRAME_DATA_SIZE];
	if (captureFrame(frame)) {
		image.fromBitsData(frame, getFrameWidth(), getFrameHeight());
		return true;
	}
	return false;
}


bool OctDevice::IrCamera::captureFrame(OctFrameImage & image) const
{
	static uint8_t frame[IR_CAMERA_FRAME_DATA_SIZE];
	if (captureFrame(frame)) {
		image.setup(frame, getFrameWidth(), getFrameHeight());
		return true;
	}
	return false;
}


bool OctDevice::IrCamera::loadConfig(OctConfig::CameraSettings * dset)
{
	CameraItem* item = dset->getCameraItem(getType());
	if (item != nullptr) {
		setAnalogGain(item->analogGain());
		setDigitalGain(item->digitalGain());
		// LogD() << "retina a-gain: " << item->analogGain() << ", d-gain: " << item->digitalGain();
		return true;
	}
	return false;
}


bool OctDevice::IrCamera::saveConfig(OctConfig::CameraSettings * dset)
{
	CameraItem* item = dset->getCameraItem(getType());
	if (item != nullptr) {
		item->analogGain() = getAnalogGain();
		item->digitalGain() = getDigitalGain();
		return true;
	}
	return false;
}


IrCamera::IrCameraImpl & OctDevice::IrCamera::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::IrCamera::getMainBoard(void) const
{
	return getImpl().board;
}


void OctDevice::IrCamera::acquireCameraData(void)
{
	unsigned char* buff = getImpl().buffer;
	uint32_t width = getImpl().frameWidth;
	uint32_t height = getImpl().frameHeight;
	uint32_t size = (width * height);
	uint8_t epid = getImpl().epid;

	UsbComm& usbComm = getImpl().board->getUsbComm();

	while (true)
	{
		if (!isPlaying()) {
			break;
		}

		bool result = false;
		{
			unique_lock<mutex> lock(getImpl().mutexLock);
			result = usbComm.readStream(getImpl().type, buff, size);
		}

		if (result) {
			if (isValidFrameHeader(buff)) {
				getImpl().errorCount = 0;
				getImpl().frameCount = 1;
			}
			else {
				if (getImpl().errorCount == 0) {
					LogE() << getCameraName() << ", fame header invalid!";
					getImpl().errorCount = 1;
				}
			}

			{
				unique_lock<mutex> lock(getImpl().mutexLock);
				if (getImpl().callback != nullptr) {
					(*getImpl().callback)(buff, width, height);
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else {
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}

		/*
		if (usbComm.readStream(getImpl().type, buff, size)) {
			{
				unique_lock<mutex> lock(getImpl().mutexLock);
				if (getImpl().callback != nullptr) {
					(*getImpl().callback)(buff, width, height);
				}
			}
			getImpl().errorCount = 0;
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		else {
			*/
			/*
			if (!recoverFromUsbError()) {
				break;
			}
			*/
			/*
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		*/
	}
	return;
}


bool OctDevice::IrCamera::isValidFrameHeader(unsigned char* buff)
{
	auto data1 = buff[0];
	auto data2 = buff[1];

	if (data1 == 0xff && (data2 == 0x00 || data2 == 0x02)) {
		unsigned char* p = buff;
		int period = 16;
		for (unsigned int idx = 0; idx < getImpl().frameHeight; idx += period) {
			auto headTag1 = p[0];
			auto headTag2 = p[1];
			unsigned short* lineId = (unsigned short*)&p[2];
			// LogD() << "Line index: " << *lineId << ", head tags: " << (int)headTag1 << ", " << (int)headTag2;
			if (idx != *lineId) {
				return false;
			}
			p += (getImpl().frameWidth * period);
		}
		return true;
		/*
		unsigned short* lineId = (unsigned short*)&buff[2];
		if (*lineId == 0) {
			return true;
		}
		*/
	}
	return false;
}


bool OctDevice::IrCamera::recoverFromUsbError(void)
{
	if (getImpl().errorCount == 0) {
		UsbComm& usbComm = getImpl().board->getUsbComm();
		usbComm.TestCommand(getImpl().epid);
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		usbComm.getUsbPort()->abortPipe(getImpl().epid);
		usbComm.IrCameraControl(static_cast<unsigned char>(getImpl().camId), CTRL_ON);
		getImpl().errorCount++;

		CString cstr;
		cstr.Format(_T("Recovering from usb error (cam=%x)"), getImpl().epid);
		AfxMessageBox(cstr);
		return true;
	}
	return false;
}




