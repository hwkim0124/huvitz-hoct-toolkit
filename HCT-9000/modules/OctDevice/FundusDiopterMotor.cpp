#include "stdafx.h"
#include "FundusDiopterMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct FundusDiopterMotor::FundusDiopterMotorImpl
{
	int32_t posMinus;
	int32_t posPlus;
	int32_t posZero;

	FundusDiopterMotorImpl() : posMinus(0), posPlus(0), posZero(0)
	{

	}
};


FundusDiopterMotor::FundusDiopterMotor() :
	d_ptr(make_unique<FundusDiopterMotorImpl>()), StepMotor()
{
}


OctDevice::FundusDiopterMotor::FundusDiopterMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<FundusDiopterMotorImpl>()), StepMotor(board, type)
{
}


FundusDiopterMotor::~FundusDiopterMotor()
{
}


FundusDiopterMotor::FundusDiopterMotor::FundusDiopterMotor(FundusDiopterMotor && rhs) = default;
FundusDiopterMotor & FundusDiopterMotor::FundusDiopterMotor::operator=(FundusDiopterMotor && rhs) = default;


OctDevice::FundusDiopterMotor::FundusDiopterMotor(const FundusDiopterMotor & rhs)
	: d_ptr(make_unique<FundusDiopterMotorImpl>(*rhs.d_ptr))
{
}


FundusDiopterMotor & OctDevice::FundusDiopterMotor::operator=(const FundusDiopterMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::FundusDiopterMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfZeroLens(MOTOR_DIOPTER_SEL_INIT_ZERO);
		// setPositionOfMinusLens(MOTOR_DIOPTER_SEL_INIT_MINUS);
		// setPositionOfPlusLens(MOTOR_DIOPTER_SEL_INIT_PLUS);
		loadSystemParameters();
		updatePositionToZeroLens();
		return true;
	}
	return false;
}


bool OctDevice::FundusDiopterMotor::updatePositionToMinusLens(void)
{
	int pos = getPositionOfMinusLens();
	return StepMotor::updatePosition(pos);
}


bool OctDevice::FundusDiopterMotor::updatePositionToPlusLens(void)
{
	int pos = getPositionOfPlusLens();
	return StepMotor::updatePosition(pos);
}


bool OctDevice::FundusDiopterMotor::updatePositionToZeroLens(void)
{
	int pos = getPositionOfZeroLens();
	return StepMotor::updatePosition(pos);
}


int OctDevice::FundusDiopterMotor::getPositionOfMinusLens(void) const
{
	return getImpl().posMinus;
	// return (isScan ? SystemConfig::minusLensPosToScan() : SystemConfig::minusLensPosToFundus());
}


int OctDevice::FundusDiopterMotor::getPositionOfPlusLens(void) const
{
	return getImpl().posPlus;
	// return (isScan ? SystemConfig::plusLensPosToScan() : SystemConfig::plusLensPosToFundus());
}


int OctDevice::FundusDiopterMotor::getPositionOfZeroLens(void) const
{
	return getImpl().posZero;
	// return (isScan ? SystemConfig::zeroLensPosToScan() : SystemConfig::zeroLensPosToFundus());
}


void OctDevice::FundusDiopterMotor::setPositionOfMinusLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posMinus = pos;
	// SystemConfig::minusLensPosToScan(true, pos);
	return;
}


void OctDevice::FundusDiopterMotor::setPositionOfPlusLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posPlus = pos;
	// SystemConfig::plusLensPosToScan(true, pos);
	return;
}


void OctDevice::FundusDiopterMotor::setPositionOfZeroLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posZero = pos;
	// SystemConfig::zeroLensPosToScan(true, pos);
	return;
}


void OctDevice::FundusDiopterMotor::setCurrentPositionAsMinusLens(void)
{
	int pos = getPosition();
	setPositionOfMinusLens(pos);
	return;
}


void OctDevice::FundusDiopterMotor::setCurrentPositionAsPlusLens(void)
{
	int pos = getPosition();
	setPositionOfPlusLens(pos);
	return;
}


void OctDevice::FundusDiopterMotor::setCurrentPositionAsZeroLens(void)
{
	int pos = getPosition();
	setPositionOfZeroLens(pos);
	return;
}


void OctDevice::FundusDiopterMotor::loadSystemParameters(void)
{
	getImpl().posMinus = SystemConfig::minusLensPosToFundus();
	getImpl().posPlus = SystemConfig::plusLensPosToFundus();
	getImpl().posZero = SystemConfig::zeroLensPosToFundus();
	return;
}


void OctDevice::FundusDiopterMotor::saveSystemParameters(void)
{
	SystemConfig::minusLensPosToFundus(true, getImpl().posMinus);
	SystemConfig::plusLensPosToFundus(true, getImpl().posPlus);
	SystemConfig::zeroLensPosToFundus(true, getImpl().posZero);
	SystemConfig::setModified(true);
	return;
}


FundusDiopterMotor::FundusDiopterMotorImpl & OctDevice::FundusDiopterMotor::getImpl(void) const
{
	return *d_ptr;
}
