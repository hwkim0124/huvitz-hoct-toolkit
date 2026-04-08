#include "stdafx.h"
#include "OctCompensationMotor.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctConfig;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct OctCompensationMotor::OctCompensationMotorImpl
{
	OctCompensationMotorImpl()
	{

	}
};


OctCompensationMotor::OctCompensationMotor() :
	d_ptr(make_unique<OctCompensationMotorImpl>()), ReferenceMotor()
{
}


OctDevice::OctCompensationMotor::OctCompensationMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<OctCompensationMotorImpl>()), ReferenceMotor(board, type)
{
}


OctCompensationMotor::~OctCompensationMotor()
{
}


OctCompensationMotor::OctCompensationMotor::OctCompensationMotor(OctCompensationMotor && rhs) = default;
OctCompensationMotor & OctCompensationMotor::OctCompensationMotor::operator=(OctCompensationMotor && rhs) = default;


OctDevice::OctCompensationMotor::OctCompensationMotor(const OctCompensationMotor & rhs)
	: d_ptr(make_unique<OctCompensationMotorImpl>(*rhs.d_ptr))
{
}


OctCompensationMotor & OctDevice::OctCompensationMotor::operator=(const OctCompensationMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::OctCompensationMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfOrigin(MOTOR_OCT_COMPENSATION_INIT_ORIGIN);
		return true;
	}
	return false;
}


OctCompensationMotor::OctCompensationMotorImpl & OctDevice::OctCompensationMotor::getImpl(void) const
{
	return *d_ptr;
}
