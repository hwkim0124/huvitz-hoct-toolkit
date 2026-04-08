#include "stdafx.h"
#include "SplitFocusMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct SplitFocusMotor::SplitFocusMotorImpl
{
	SplitFocusMotorImpl()
	{

	}
};


SplitFocusMotor::SplitFocusMotor() :
	d_ptr(make_unique<SplitFocusMotorImpl>()), OctSampleMotor()
{
}


OctDevice::SplitFocusMotor::SplitFocusMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<SplitFocusMotorImpl>()), OctSampleMotor(board, type)
{
}


SplitFocusMotor::~SplitFocusMotor()
{
}


SplitFocusMotor::SplitFocusMotor::SplitFocusMotor(SplitFocusMotor && rhs) = default;
SplitFocusMotor & SplitFocusMotor::SplitFocusMotor::operator=(SplitFocusMotor && rhs) = default;


OctDevice::SplitFocusMotor::SplitFocusMotor(const SplitFocusMotor & rhs)
	: d_ptr(make_unique<SplitFocusMotorImpl>(*rhs.d_ptr))
{
}


SplitFocusMotor & OctDevice::SplitFocusMotor::operator=(const SplitFocusMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::SplitFocusMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionsPerDiopter(MOTOR_SPLIT_FOCUS_STEPS_PER_DIOPTER);
		// setPositionOfZeroDiopter(MOTOR_SPLIT_FOCUS_INIT_ORIGIN);
		loadSystemParameters();
		updatePositionToMirrorIn();
		return true;
	}
	return false;
}


void OctDevice::SplitFocusMotor::loadSystemParameters(void)
{
	int pos = SystemConfig::splitFocusMirrorPos(false);
	setPositionOfMirrorIn(pos);
	pos = SystemConfig::splitFocusMirrorPos(true);
	setPositionOfMirrorOut(pos);
	return;
}


void OctDevice::SplitFocusMotor::saveSystemParameters(void)
{
	int pos = getPositionOfMirrorIn();
	SystemConfig::splitFocusMirrorPos(false, true, pos);
	pos = getPositionOfMirrorOut();
	SystemConfig::splitFocusMirrorPos(true, true, pos);
	SystemConfig::setModified(true);
	return;
}


SplitFocusMotor::SplitFocusMotorImpl & OctDevice::SplitFocusMotor::getImpl(void) const
{
	return *d_ptr;
}
