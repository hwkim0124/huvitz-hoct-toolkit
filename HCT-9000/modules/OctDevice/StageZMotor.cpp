#include "stdafx.h"
#include "StageZMotor.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct StageZMotor::StageZMotorImpl
{
	StageZMotorImpl()
	{
	}
};


StageZMotor::StageZMotor() :
	d_ptr(make_unique<StageZMotorImpl>()), StageMotor()
{
}


OctDevice::StageZMotor::StageZMotor(MainBoard * board) :
	d_ptr(make_unique<StageZMotorImpl>()), StageMotor(board, StageMotorType::STAGE_Z)
{
	setLimitRange(STAGE_Z_LIMIT_RANGE_LOW, STAGE_Z_LIMIT_RANGE_HIGH);
}


StageZMotor::~StageZMotor()
{
}


bool OctDevice::StageZMotor::initialize(void)
{
	if (!StageMotor::initialize()) {
		return false;
	}

	updatePositionToCenter();
	return true;
}


StageZMotor::StageZMotorImpl & OctDevice::StageZMotor::getImpl(void) const
{
	return *d_ptr;
}
