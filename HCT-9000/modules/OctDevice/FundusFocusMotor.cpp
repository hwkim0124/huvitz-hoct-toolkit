#include "stdafx.h"
#include "FundusFocusMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;



struct FundusFocusMotor::FundusFocusMotorImpl
{
	int32_t posOfOrigin;

	FundusFocusMotorImpl() : posOfOrigin(0)
	{
		
	}
};


FundusFocusMotor::FundusFocusMotor() :
	d_ptr(make_unique<FundusFocusMotorImpl>()), OctFocusMotor()
{
}


OctDevice::FundusFocusMotor::FundusFocusMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<FundusFocusMotorImpl>()), OctFocusMotor(board, type)
{
}


FundusFocusMotor::~FundusFocusMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
FundusFocusMotor::FundusFocusMotor::FundusFocusMotor(FundusFocusMotor && rhs) = default;
FundusFocusMotor & FundusFocusMotor::FundusFocusMotor::operator=(FundusFocusMotor && rhs) = default;


OctDevice::FundusFocusMotor::FundusFocusMotor(const FundusFocusMotor & rhs)
	: d_ptr(make_unique<FundusFocusMotorImpl>(*rhs.d_ptr))
{
}


FundusFocusMotor & OctDevice::FundusFocusMotor::operator=(const FundusFocusMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::FundusFocusMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		setPositionsPerDiopter(MOTOR_FUNDUS_FOCUS_STEPS_PER_DIOPTER);
		// setPositionOfZeroDiopter(MOTOR_FUNDUS_FOCUS_INIT_ORIGIN);
		loadSystemParameters();
		updatePositionToZeroDiopter();
		return true;
	}
	return false;
}


int OctDevice::FundusFocusMotor::getPositionOfZeroDiopter(void) const
{
	return getImpl().posOfOrigin;
	//return SystemConfig::zeroDioptPosToIrFocus();
}


void OctDevice::FundusFocusMotor::setPositionOfZeroDiopter(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posOfOrigin = pos;
	//SystemConfig::zeroDioptPosToIrFocus(true, pos);
	return;
}


void OctDevice::FundusFocusMotor::loadSystemParameters(void)
{
	getImpl().posOfOrigin = SystemConfig::zeroDioptPosToIrFocus();
	return;
}


void OctDevice::FundusFocusMotor::saveSystemParameters(void)
{
	int pos = getImpl().posOfOrigin;
	SystemConfig::zeroDioptPosToIrFocus(true, pos);
	SystemConfig::setModified(true);
	return;
}


FundusFocusMotor::FundusFocusMotorImpl & OctDevice::FundusFocusMotor::getImpl(void) const
{
	return *d_ptr;
}


