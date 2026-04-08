#include "stdafx.h"
#include "StageYMotor.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct StageYMotor::StageYMotorImpl
{
	StageYMotorImpl()
	{
	}
};


StageYMotor::StageYMotor() :
	d_ptr(make_unique<StageYMotorImpl>()), StageMotor()
{
}


OctDevice::StageYMotor::StageYMotor(MainBoard * board) :
	d_ptr(make_unique<StageYMotorImpl>()), StageMotor(board, StageMotorType::STAGE_Y)
{
	setLimitRange(STAGE_Y_LIMIT_RANGE_LOW, STAGE_Y_LIMIT_RANGE_HIGH);
}


StageYMotor::~StageYMotor()
{
}


StageYMotor::StageYMotorImpl & OctDevice::StageYMotor::getImpl(void) const
{
	return *d_ptr;
}
