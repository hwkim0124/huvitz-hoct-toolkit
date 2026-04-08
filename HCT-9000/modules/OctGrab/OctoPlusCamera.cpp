#include "stdafx.h"
#include "OctoPlusCamera.h"
#include "OctoPlus.h"

#include "CppUtil2.h"
#include "OctDevice2.h"

#include <atomic>
#include <functional>
#include <mutex>

using namespace OctGrab;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct OctoPlusCamera::OctoPlusCameraImpl
{
};


OctoPlusCamera::OctoPlusCamera() :
	d_ptr(make_unique<OctoPlusCameraImpl>())
{
}


OctGrab::OctoPlusCamera::~OctoPlusCamera()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctGrab::OctoPlusCamera::OctoPlusCamera(OctoPlusCamera && rhs) = default;
OctoPlusCamera & OctGrab::OctoPlusCamera::operator=(OctoPlusCamera && rhs) = default;



bool OctGrab::OctoPlusCamera::openLineCamera(Usb3Grabber* grabber)
{
	if (!OctoPlus::isLibraryInitialized()) {
		if (!OctoPlus::initializeLibrary()) {
			GlobalLogger::error("Usb cmos camera init failed!");
			return false;
		}
	}

	if (!OctoPlus::openLineCamera()) {
		GlobalLogger::error("Usb cmos camera open failed!");
		return false;
	}
	else {
		OctoPlus::setTriggerMode(ETriggerMode::Mode_ExternalLineTrigger_ProgrammableExposure);
		// OctoPlus::setTriggerMode(ETriggerMode::Mode_InternalLineTrigger_ProgrammableExposure_ProgrammableLinePeriod);

		ostringstream msg;
		double exposure;
		OctoPlus::getExposureTime(&exposure);
		msg << "Octo plus exposure time: " << exposure;

		double period;
		OctoPlus::getLinePeriod(&period);
		msg << ", line period: " << period;

		double dgain;
		OctoPlus::getDigitalGain(&dgain);
		msg << ", digital gain: " << dgain;

		double again;
		OctoPlus::getAnalogGain(&again);
		msg << ", analog gain: " << again;
		GlobalLogger::info(msg.str());

		msg.str("");  msg.clear();
		msg << "Trigger mode: " << OctoPlus::getTriggerMode();
		GlobalLogger::info(msg.str());

		msg.str(""); msg.clear();
		msg << "Output mode: " << OctoPlus::getOutputMode();
		GlobalLogger::info(msg.str());

		msg.str(""); msg.clear();
		msg << "Buffer type: " << OctoPlus::getBufferType();
		GlobalLogger::info(msg.str());

		msg.str(""); msg.clear();
		msg << "Circular buffer: " << OctoPlus::isEnabledCircularBuffer();
		GlobalLogger::info(msg.str());

		OctoPlus::setupUsb3Grabber(grabber);
		GlobalLogger::info("Usb cmos camera init ... ok!");

		setGrabber(grabber);
		return true;
	}
}

bool OctGrab::OctoPlusCamera::closeLineCamera(void)
{
	if (OctoPlus::isLibraryInitialized()) {
		OctoPlus::closeLineCamera();
		OctoPlus::releaseLibrary();
	}
	return true;
}

bool OctGrab::OctoPlusCamera::startAcquisition(bool init)
{
	bool rets = OctoPlus::startAcquisition(init);
	return rets;
}

bool OctGrab::OctoPlusCamera::cancelAcquisition(void)
{
	OctoPlus::cancelAcquisition();
	return true;
}

bool OctGrab::OctoPlusCamera::updateFrameHeight(int height)
{
	if (OctoPlus::getImageHeight() == height) {
		return true;
	}
	if (OctoPlus::isLineCameraGrabbing()) {
		OctoPlus::cancelAcquisition();
	}

	int numOfBuffer = 0;
	if (!OctoPlus::updateImageParameters(height, numOfBuffer)) {
		return false;
	}
	return true;
}

bool OctGrab::OctoPlusCamera::setExposureToNormal(void)
{
	auto trigs = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Normal);
	auto expos = GlobalSettings::getOctUsbExposureTime(ScanSpeed::Normal);

	LogD() << "OctoPlus normal mode, trigs: " << trigs << ", expos: " << expos;

	if (!OctoPlus::setLinePeriod(trigs)) {
		return false;
	}

	if (GlobalMemory::isOctManualExposureTime()) {
		expos = GlobalMemory::getOctManualExposureTime(true);
		LogD() << "OctoPlus manual exposure time: " << expos;
	}

	if (!setExposureTime(expos)) {
		return false;
	}
	return true;
}

bool OctGrab::OctoPlusCamera::setExposureToFaster(void)
{
	auto trigs = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Faster);
	auto expos = GlobalSettings::getOctUsbExposureTime(ScanSpeed::Faster);

	LogD() << "OctoPlus faster mode, trigs: " << trigs << ", expos: " << expos;

	if (!OctoPlus::setLinePeriod(trigs)) {
		return false;
	}

	if (GlobalMemory::isOctManualExposureTime()) {
		expos = GlobalMemory::getOctManualExposureTime(true);
		LogD() << "Usb line camera manual exposure time: " << expos;
	}

	if (!setExposureTime(expos)) {
		return false;
	}
	return true;
}

bool OctGrab::OctoPlusCamera::setExposureToFastest(void)
{
	auto trigs = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Fastest);
	auto expos = GlobalSettings::getOctUsbExposureTime(ScanSpeed::Fastest);

	LogD() << "OctoPlus fastest mode, trigs: " << trigs << ", expos: " << expos;

	if (!OctoPlus::setLinePeriod(trigs)) {
		return false;
	}

	if (GlobalMemory::isOctManualExposureTime()) {
		expos = GlobalMemory::getOctManualExposureTime(true);
		LogD() << "Usb line camera manual exposure time: " << expos;
	}

	if (!setExposureTime(expos)) {
		return false;
	}
	return true;
}

bool OctGrab::OctoPlusCamera::setExposureTime(float expTime)
{
	if (!OctoPlus::setExposureTime(expTime)) {
		return false;
	}
	return true;
}

bool OctGrab::OctoPlusCamera::setAnalogGain(float gain)
{
	auto value = (gain == 2.0f ? EAnalogGain::AnalogGain_2_0 : (gain == 4.0f ? EAnalogGain::AnalogGain_4_0 : EAnalogGain::AnalogGain_1_0));
	if (!OctoPlus::setAnalogGain(value)) {
		return false;
	}
	return true;
}

float OctGrab::OctoPlusCamera::getExposureTime(void)
{
	double expTime;
	if (!OctoPlus::getExposureTime(&expTime)) {
		return 0.0f;
	}
	return (float)expTime;
}

float OctGrab::OctoPlusCamera::getAnalogGain(void)
{
	double gain;
	if (OctoPlus::getAnalogGain(&gain)) {
		return (float)gain;
	}
	return 0.0f;
}

bool OctGrab::OctoPlusCamera::acquirePreviewImages(std::vector<int> bufferIds)
{
	bool retv = OctoPlus::acquirePreviewImages(bufferIds);
	return retv;
}

bool OctGrab::OctoPlusCamera::acquireMeasureImages(std::vector<int> bufferIds)
{
	bool retv = OctoPlus::acquireMeasureImages(bufferIds);
	return retv;
}

bool OctGrab::OctoPlusCamera::acquireEnfaceImages(std::vector<int> bufferIds)
{
	bool retv = OctoPlus::acquireEnfaceImages(bufferIds);
	return retv;
}


OctoPlusCamera::OctoPlusCameraImpl & OctGrab::OctoPlusCamera::impl(void) const
{
	return *d_ptr;
}
