#include "stdafx.h"
#include "OctSampleMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct OctSampleMotor::OctSampleMotorImpl
{
	int32_t posMirrorIn;
	int32_t posMirrorOut;

	OctSampleMotorImpl() : posMirrorIn(0), posMirrorOut(0)
	{

	}
};


OctSampleMotor::OctSampleMotor() :
	d_ptr(make_unique<OctSampleMotorImpl>()), StepMotor()
{
}


OctDevice::OctSampleMotor::OctSampleMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<OctSampleMotorImpl>()), StepMotor(board, type)
{
}


OctSampleMotor::~OctSampleMotor()
{
}


OctSampleMotor::OctSampleMotor::OctSampleMotor(OctSampleMotor && rhs) = default;
OctSampleMotor & OctSampleMotor::OctSampleMotor::operator=(OctSampleMotor && rhs) = default;


OctDevice::OctSampleMotor::OctSampleMotor(const OctSampleMotor & rhs)
	: d_ptr(make_unique<OctSampleMotorImpl>(*rhs.d_ptr))
{
}


OctSampleMotor & OctDevice::OctSampleMotor::operator=(const OctSampleMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::OctSampleMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfMirrorIn(MOTOR_OCT_SAMPLE_INIT_MIRROR_IN);
		// setPositionOfMirrorOut(MOTOR_OCT_SAMPLE_INIT_MIRROR_OUT);
		loadSystemParameters();
		updatePositionToMirrorIn();
		return true;
	}
	return false;
}


bool OctDevice::OctSampleMotor::updatePositionToMirrorIn(void)
{
	int pos = getPositionOfMirrorIn();
	return StepMotor::updatePosition(pos);
}


bool OctDevice::OctSampleMotor::updatePositionToMirrorOut(void)
{
	int pos = getPositionOfMirrorOut();
	return StepMotor::updatePosition(pos);
}

bool OctDevice::OctSampleMotor::isPositionAtMirrorIn(void)
{
	int posIn = getPositionOfMirrorIn();
	int posCurr = getPosition();
	if (abs(posIn - posCurr) <= 1) {
		return true;
	}
	return false;
}

bool OctDevice::OctSampleMotor::isPositionAtMirrorOut(void)
{
	int posOut = getPositionOfMirrorOut();
	int posCurr = getPosition();
	if (abs(posOut - posCurr) <= 1) {
		return true;
	}
	return false;
}


int OctDevice::OctSampleMotor::getPositionOfMirrorIn(void) const
{
	return getImpl().posMirrorIn;
	// return SystemConfig::quickReturnMirrorPos(false);
}


int OctDevice::OctSampleMotor::getPositionOfMirrorOut(void) const
{
	return getImpl().posMirrorOut;
	// return SystemConfig::quickReturnMirrorPos(true);
}


void OctDevice::OctSampleMotor::setPositionOfMirrorIn(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	getImpl().posMirrorIn = pos;
	// SystemConfig::quickReturnMirrorPos(false, true, pos);
	return;
}


void OctDevice::OctSampleMotor::setPositionOfMirrorOut(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	getImpl().posMirrorOut = pos;
	// SystemConfig::quickReturnMirrorPos(true, true, pos);
	return;
}


void OctDevice::OctSampleMotor::setCurrentPositionAsMirrorIn(void)
{
	int pos = getPosition();
	setPositionOfMirrorIn(pos);
	return;
}


void OctDevice::OctSampleMotor::setCurrentPositionAsMirrorOut(void)
{
	int pos = getPosition();
	setPositionOfMirrorOut(pos);
	return;
}


void OctDevice::OctSampleMotor::loadSystemParameters(void)
{
	getImpl().posMirrorIn = SystemConfig::quickReturnMirrorPos(false);
	getImpl().posMirrorOut = SystemConfig::quickReturnMirrorPos(true);
	return;
}


void OctDevice::OctSampleMotor::saveSystemParameters(void)
{
	int pos = getImpl().posMirrorIn;
	SystemConfig::quickReturnMirrorPos(false, true, pos);
	pos = getImpl().posMirrorOut;
	SystemConfig::quickReturnMirrorPos(true, true, pos);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::OctSampleMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfMirrorIn(item->getSetupPosition(0));
			setPositionOfMirrorOut(item->getSetupPosition(1));
			return true;
		}
	}
	return false;
}


bool OctDevice::OctSampleMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfMirrorIn());
			item->setSetupPosition(1, getPositionOfMirrorOut());
			return true;
		}
	}
	return false;
}


OctSampleMotor::OctSampleMotorImpl & OctDevice::OctSampleMotor::getImpl(void) const
{
	return *d_ptr;
}
