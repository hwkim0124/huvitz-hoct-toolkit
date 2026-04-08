#include "stdafx.h"
#include "ReferenceMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct ReferenceMotor::ReferenceMotorImpl
{
	int32_t posOfOrigin;
	int32_t posOfOrigin2;

	ReferenceMotorImpl() : posOfOrigin(0), posOfOrigin2(0)
	{
	}
};


ReferenceMotor::ReferenceMotor() :
	d_ptr(make_unique<ReferenceMotorImpl>()), StepMotor()
{
}


OctDevice::ReferenceMotor::ReferenceMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<ReferenceMotorImpl>()), StepMotor(board, type)
{
}


ReferenceMotor::~ReferenceMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
ReferenceMotor::ReferenceMotor::ReferenceMotor(ReferenceMotor && rhs) = default;
ReferenceMotor & ReferenceMotor::ReferenceMotor::operator=(ReferenceMotor && rhs) = default;


OctDevice::ReferenceMotor::ReferenceMotor(const ReferenceMotor & rhs)
	: d_ptr(make_unique<ReferenceMotorImpl>(*rhs.d_ptr))
{
}


ReferenceMotor & OctDevice::ReferenceMotor::operator=(const ReferenceMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::ReferenceMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfOrigin(MOTOR_REFERENCE_INIT_ORIGIN);
		loadSystemParameters();
		updatePositionToRetinaOrigin();

		/*
		auto pattern = SystemProfile::getPatternSettings();
		int lower = getPositionOfRetinaOrigin() - pattern->getReferenceRangeLowerSize();
		int upper = getPositionOfRetinaOrigin() + pattern->getReferenceRangeUpperSize();
		setRangeMax(upper);
		setRangeMin(lower);
		LogD() << "Reference range, min: " << getRangeMin() << ", max: " << getRangeMax();
		*/
		return true;
	}
	return false;
}


int OctDevice::ReferenceMotor::getPositionOfRetinaOrigin(void) const
{
	return getImpl().posOfOrigin;
	//return SystemConfig::referencePosToRetina();
}


int OctDevice::ReferenceMotor::getPositionOfCorneaOrigin(void) const
{
	return getImpl().posOfOrigin2;
	//return SystemConfig::referencePosToCornea();
}


void OctDevice::ReferenceMotor::setPositionOfRetinaOrigin(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posOfOrigin = pos;
	// SystemConfig::referencePosToRetina(true, pos);
	return;
}


void OctDevice::ReferenceMotor::setPositionOfCorneaOrigin(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posOfOrigin2 = pos;
	// SystemConfig::referencePosToCornea(true, pos);
	return;
}


void OctDevice::ReferenceMotor::setCurrentPositionAsRetinaOrigin(void)
{
	int pos = getPosition();
	setPositionOfRetinaOrigin(pos);
	return;
}


void OctDevice::ReferenceMotor::setCurrentPositionAsCorneaOrigin(void)
{
	int pos = getPosition();
	setPositionOfCorneaOrigin(pos);
	return;
}


bool OctDevice::ReferenceMotor::updatePositionToRetinaOrigin(void)
{
	int pos = getPositionOfRetinaOrigin();
	bool result = updatePosition(pos);
	return result;
}


bool OctDevice::ReferenceMotor::updatePositionToCorneaOrigin(void)
{
	int pos = getPositionOfCorneaOrigin();
	bool result = updatePosition(pos);
	return result;
}


bool OctDevice::ReferenceMotor::updatePositionToUpperEnd(void)
{
	int pos = getRangeMax();
	bool result = updatePosition(pos);
	return result;
}


bool OctDevice::ReferenceMotor::updatePositionToLowerEnd(void)
{
	int pos = getRangeMin();
	bool result = updatePosition(pos);
	return result;
}


bool OctDevice::ReferenceMotor::isEndOfLowerPosition(bool inRange) const
{
	auto pattern = SystemProfile::getPatternSettings();
	int lowerEnd = getPositionOfRetinaOrigin() - pattern->getReferenceRangeLowerSize();

	if (!inRange) {
		lowerEnd = getRangeMin();
	}
	else {
		lowerEnd = max(lowerEnd, getRangeMin());
	}

	return (getPosition() <= (lowerEnd + 5) ? true : false);
	// return (fabs(getPosition() - getRangeMin()) <= 5 ? true : false) ;
}


bool OctDevice::ReferenceMotor::isEndOfUpperPosition(bool inRange) const
{
	auto pattern = SystemProfile::getPatternSettings();
	int upperEnd = getPositionOfRetinaOrigin() + pattern->getReferenceRangeUpperSize();

	if (!inRange) {
		upperEnd = getRangeMax();
	}
	else {
		upperEnd = min(upperEnd, getRangeMax());
	}

	return (getPosition() >= (upperEnd - 5) ? true : false);
	// return (fabs(getPosition() - getRangeMax()) <= 5 ? true : false);
}


bool OctDevice::ReferenceMotor::isAtLowerSide(bool isCornea) const
{
	/*
	if (getPosition() < ((getRangeMax() - getRangeMin()) / 2)) {
		return true;
	}
	*/
	int origin = (isCornea ? getPositionOfCorneaOrigin() : getPositionOfRetinaOrigin());
	if (getPosition() < origin) {
		return true;
	}
	return false;
}


bool OctDevice::ReferenceMotor::isAtUpperSide(bool isCornea) const
{
	/*
	if (getPosition() >= ((getRangeMax() - getRangeMin()) / 2)) {
		return true;
	}
	*/
	int origin = (isCornea ? getPositionOfCorneaOrigin() : getPositionOfRetinaOrigin());
	if (getPosition() > origin) {
		return true;
	}
	return false;
}


void OctDevice::ReferenceMotor::loadSystemParameters(void)
{
	getImpl().posOfOrigin = SystemConfig::referencePosToRetina();
	getImpl().posOfOrigin2 = SystemConfig::referencePosToCornea();
	return;
}


void OctDevice::ReferenceMotor::saveSystemParameters(void)
{
	SystemConfig::referencePosToRetina(true, getImpl().posOfOrigin);
	SystemConfig::referencePosToCornea(true, getImpl().posOfOrigin2);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::ReferenceMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfRetinaOrigin(item->getSetupPosition(0));
			return true;
		}
	}
	return false;
}


bool OctDevice::ReferenceMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfRetinaOrigin());
			return true;
		}
	}
	return false;
}


ReferenceMotor::ReferenceMotorImpl & OctDevice::ReferenceMotor::getImpl(void) const
{
	return *d_ptr;
}
