#include "stdafx.h"
#include "PupilMaskMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct PupilMaskMotor::PupilMaskMotorImpl
{
	int32_t posNormal;
	int32_t posSmall;

	PupilMaskMotorImpl() : posNormal(0), posSmall(0)
	{

	}
};


PupilMaskMotor::PupilMaskMotor() :
	d_ptr(make_unique<PupilMaskMotorImpl>()), StepMotor()
{
}


OctDevice::PupilMaskMotor::PupilMaskMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<PupilMaskMotorImpl>()), StepMotor(board, type)
{
}


PupilMaskMotor::~PupilMaskMotor()
{
}


PupilMaskMotor::PupilMaskMotor::PupilMaskMotor(PupilMaskMotor && rhs) = default;
PupilMaskMotor & PupilMaskMotor::PupilMaskMotor::operator=(PupilMaskMotor && rhs) = default;


OctDevice::PupilMaskMotor::PupilMaskMotor(const PupilMaskMotor & rhs)
	: d_ptr(make_unique<PupilMaskMotorImpl>(*rhs.d_ptr))
{
}


PupilMaskMotor & OctDevice::PupilMaskMotor::operator=(const PupilMaskMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::PupilMaskMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfNormal(MOTOR_PUPIL_MASK_INIT_NORMAL);
		// setPositionOfSmall(MOTOR_PUPIL_MASK_INIT_SMALL);
		loadSystemParameters();
		updatePositionToNormal();
		return true;
	}
	return false;
}


bool OctDevice::PupilMaskMotor::updatePositionToNormal(void)
{
	int pos = getPositionOfNormal();
	return StepMotor::updatePosition(pos);
}


bool OctDevice::PupilMaskMotor::updatePositionToSmall(void)
{
	int pos = getPositionOfSmall();
	return StepMotor::updatePosition(pos);
}


int OctDevice::PupilMaskMotor::getPositionOfNormal(void) const
{
	return getImpl().posNormal;
	// return SystemConfig::pupilMaskPos(true);
}


int OctDevice::PupilMaskMotor::getPositionOfSmall(void) const
{
	return getImpl().posSmall ;
	// return SystemConfig::pupilMaskPos(false);
}


void OctDevice::PupilMaskMotor::setPositionOfNormal(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	getImpl().posNormal = pos;
	// SystemConfig::pupilMaskPos(true, true, pos);
	return;
}


void OctDevice::PupilMaskMotor::setPositionOfSmall(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	getImpl().posSmall = pos;
	// SystemConfig::pupilMaskPos(false, true, pos);
	return;
}


void OctDevice::PupilMaskMotor::setCurrentPositionAsNormal(void)
{
	int pos = getPosition();
	setPositionOfNormal(pos);
	return;
}


void OctDevice::PupilMaskMotor::setCurrentPositionAsSmall(void)
{
	int pos = getPosition();
	setPositionOfSmall(pos);
	return;
}


void OctDevice::PupilMaskMotor::loadSystemParameters(void)
{
	getImpl().posNormal = SystemConfig::pupilMaskPos(false);
	getImpl().posSmall = SystemConfig::pupilMaskPos(true);
	return;
}


void OctDevice::PupilMaskMotor::saveSystemParameters(void)
{
	SystemConfig::pupilMaskPos(false, true, getImpl().posNormal);
	SystemConfig::pupilMaskPos(true, true, getImpl().posSmall);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::PupilMaskMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfNormal(item->getSetupPosition(0));
			setPositionOfSmall(item->getSetupPosition(1));
			return true;
		}
	}
	return false;
}


bool OctDevice::PupilMaskMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfNormal());
			item->setSetupPosition(1, getPositionOfSmall());
			return true;
		}
	}
	return false;
}


PupilMaskMotor::PupilMaskMotorImpl & OctDevice::PupilMaskMotor::getImpl(void) const
{
	return *d_ptr;
}
