#include "stdafx.h"
#include "CorneaCamera.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct CorneaCamera::CorneaCameraImpl
{
	CorneaCameraImpl()
	{
	}
};


CorneaCamera::CorneaCamera() :
	d_ptr(make_unique<CorneaCameraImpl>()), IrCamera()
{
}


CorneaCamera::CorneaCamera(MainBoard * board) :
	d_ptr(make_unique<CorneaCameraImpl>()), IrCamera(board, CameraType::CORNEA)
{
}


CorneaCamera::~CorneaCamera()
{
}


bool OctDevice::CorneaCamera::initialize(void)
{
	if (!IrCamera::initialize()) {
		LogE() << "Ir cornea camera init failed!";
		return false;
	}
	loadSystemParameters();
	return true;
}


void OctDevice::CorneaCamera::loadSystemParameters(void)
{
	if (!isInitiated()) {
		return;
	}

	uint8_t data = SystemConfig::corneaCameraAgain();
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

	data = SystemConfig::corneaCameraDgain();
	gain = (float)(data >> 5);
	gain += (float)((data & 0x1F) * exp2(-5));

	setDigitalGain(gain, false);
	d_gain = gain;

	// LogD() << "Cornea ir camera, a-gain: " << a_gain << ", d-gain: " << d_gain;
	return;
}


void OctDevice::CorneaCamera::saveSystemParameters(void)
{
	if (!isInitiated()) {
		return;
	}

	float gain = getAnalogGain();
	SystemConfig::corneaCameraAgain(true, getAnalogGainData(gain));

	gain = getDigitalGain();
	SystemConfig::corneaCameraDgain(true, getDigitalGainData(gain));

	SystemConfig::setModified(true);
	return;
}


CorneaCamera::CorneaCameraImpl & OctDevice::CorneaCamera::getImpl(void) const
{
	return *d_ptr;
}
