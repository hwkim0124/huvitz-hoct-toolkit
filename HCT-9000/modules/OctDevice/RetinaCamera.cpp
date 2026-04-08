#include "stdafx.h"
#include "RetinaCamera.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct RetinaCamera::RetinaCameraImpl
{
	RetinaCameraImpl()
	{
	}
};


RetinaCamera::RetinaCamera() :
	d_ptr(make_unique<RetinaCameraImpl>()), IrCamera()
{
}


OctDevice::RetinaCamera::RetinaCamera(MainBoard * board) :
	d_ptr(make_unique<RetinaCameraImpl>()), IrCamera(board, CameraType::RETINA)
{
}


RetinaCamera::~RetinaCamera()
{
}


bool OctDevice::RetinaCamera::initialize(void)
{
	if (!IrCamera::initialize()) {
		LogE() << "Ir retina camera init failed!";
		return false;
	}
	loadSystemParameters();
	setExposureTime(false);
	return true;
}


void OctDevice::RetinaCamera::loadSystemParameters(void)
{
	if (!isInitiated()) {
		return;
	}

	uint8_t data = SystemConfig::retinaCameraAgain();
	float gain;
	float a_gain, d_gain;

	if (data > 8) {
		gain = (float)(data - 8);
		if (gain == 1.0f) {
			gain += 0.25f;
		}
		else if (gain == 2.0f) {
			gain += 0.5f;
		}
		else if (gain == 4.0f) {
			gain += 1.0f;
		}
		else if (gain == 8.0f) {
			gain += 2.0f;
		}
	}
	else {
		gain = (float)data;
	}
	setAnalogGain(gain, false);
	a_gain = gain;

	data = SystemConfig::retinaCameraDgain();
	gain = (float)(data >> 5);
	gain += (float)((data & 0x1F) * exp2(-5));

	setDigitalGain(gain, false);
	d_gain = gain;

	// LogD() << "Retina ir camera, a-gain: " << a_gain << ", d-gain: " << d_gain;
	return;
}


void OctDevice::RetinaCamera::saveSystemParameters(void)
{
	if (!isInitiated()) {
		return;
	}

	float gain = getAnalogGain();
	SystemConfig::retinaCameraAgain(true, getAnalogGainData(gain));

	gain = getDigitalGain();
	SystemConfig::retinaCameraDgain(true, getDigitalGainData(gain));

	SystemConfig::setModified(true);
	return;
}


bool OctDevice::RetinaCamera::applyCameraPreset(int preset)
{
	CameraSettings* dset = SystemProfile::getCameraSettings();
	CameraItem* item = dset->getCameraItem(getType());
	if (item != nullptr) {
		setAnalogGain(item->analogGain(preset));
		setDigitalGain(item->digitalGain(preset));

		LogD() << "Retina camera preset: " << preset << ", a-gain: " << item->analogGain(preset) << ", d-gain: " << item->digitalGain(preset);
		return true;
	}
	return false;
}


RetinaCamera::RetinaCameraImpl & OctDevice::RetinaCamera::getImpl(void) const
{
	return *d_ptr;
}
