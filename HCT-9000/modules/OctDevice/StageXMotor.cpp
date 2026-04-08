#include "stdafx.h"
#include "StageXMotor.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct StageXMotor::StageXMotorImpl
{
	StageXMotorImpl() 
	{
	}
};


StageXMotor::StageXMotor() :
	d_ptr(make_unique<StageXMotorImpl>()), StageMotor()
{
}


OctDevice::StageXMotor::StageXMotor(MainBoard * board) :
	d_ptr(make_unique<StageXMotorImpl>()), StageMotor(board, StageMotorType::STAGE_X)
{
	setLimitRange(STAGE_X_LIMIT_RANGE_LOW, STAGE_X_LIMIT_RANGE_HIGH);
}


StageXMotor::~StageXMotor()
{
}


bool OctDevice::StageXMotor::initialize(void)
{
	if (!StageMotor::initialize()) {
		return false;
	}

	updatePositionToCenter();
	return true;
}


StageXMotor::StageXMotorImpl & OctDevice::StageXMotor::getImpl(void) const
{
	return *d_ptr;
}
