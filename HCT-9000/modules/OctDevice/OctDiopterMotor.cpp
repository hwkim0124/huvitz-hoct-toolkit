#include "stdafx.h"
#include "OctDiopterMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct OctDiopterMotor::OctDiopterMotorImpl
{
	int32_t posMinus;
	int32_t posPlus;
	int32_t posZero;

	CompensationLensMode lensMode;

	OctDiopterMotorImpl() : posMinus(0), posPlus(0), posZero(0), 
							lensMode(CompensationLensMode::Zero)
	{

	}
};


OctDiopterMotor::OctDiopterMotor() :
	d_ptr(make_unique<OctDiopterMotorImpl>()), StepMotor()
{
}


OctDevice::OctDiopterMotor::OctDiopterMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<OctDiopterMotorImpl>()), StepMotor(board, type)
{
}


OctDiopterMotor::~OctDiopterMotor()
{
}


OctDiopterMotor::OctDiopterMotor::OctDiopterMotor(OctDiopterMotor && rhs) = default;
OctDiopterMotor & OctDiopterMotor::OctDiopterMotor::operator=(OctDiopterMotor && rhs) = default;


OctDevice::OctDiopterMotor::OctDiopterMotor(const OctDiopterMotor & rhs)
	: d_ptr(make_unique<OctDiopterMotorImpl>(*rhs.d_ptr))
{
}


OctDiopterMotor & OctDevice::OctDiopterMotor::operator=(const OctDiopterMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::OctDiopterMotor::initialize(void)
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


bool OctDevice::OctDiopterMotor::updatePositionToMinusLens(void)
{
	int pos = getPositionOfMinusLens();
	bool result = StepMotor::updatePosition(pos);
	if (result) {
		d_ptr->lensMode = CompensationLensMode::Minus;
	}
	return result;
}


bool OctDevice::OctDiopterMotor::updatePositionToPlusLens(void)
{
	int pos = getPositionOfPlusLens();
	bool result = StepMotor::updatePosition(pos);
	if (result) {
		d_ptr->lensMode = CompensationLensMode::Plus;
	}
	return result;
}


bool OctDevice::OctDiopterMotor::updatePositionToZeroLens(void)
{
	int pos = getPositionOfZeroLens();
	bool result = StepMotor::updatePosition(pos);
	if (result) {
		d_ptr->lensMode = CompensationLensMode::Zero;
	}
	return result;
}


bool OctDevice::OctDiopterMotor::isPlusLensMode(void) const
{
	return (d_ptr->lensMode == CompensationLensMode::Plus);
}


bool OctDevice::OctDiopterMotor::isZeroLensMode(void) const
{
	return (d_ptr->lensMode == CompensationLensMode::Zero);
}


bool OctDevice::OctDiopterMotor::isMinusLensMode(void) const
{
	return (d_ptr->lensMode == CompensationLensMode::Minus);
}


int OctDevice::OctDiopterMotor::getPositionOfMinusLens(void) const
{
	return getImpl().posMinus;
	// return (isScan ? SystemConfig::minusLensPosToScan() : SystemConfig::minusLensPosToFundus());
}


int OctDevice::OctDiopterMotor::getPositionOfPlusLens(void) const
{
	return getImpl().posPlus;
	// return (isScan ? SystemConfig::plusLensPosToScan() : SystemConfig::plusLensPosToFundus());
}


int OctDevice::OctDiopterMotor::getPositionOfZeroLens(void) const
{
	return getImpl().posZero;
	// return (isScan ? SystemConfig::zeroLensPosToScan() : SystemConfig::zeroLensPosToFundus());
}


void OctDevice::OctDiopterMotor::setPositionOfMinusLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posMinus = pos;
	// SystemConfig::minusLensPosToScan(true, pos);
	return;
}


void OctDevice::OctDiopterMotor::setPositionOfPlusLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posPlus = pos;
	// SystemConfig::plusLensPosToScan(true, pos);
	return;
}


void OctDevice::OctDiopterMotor::setPositionOfZeroLens(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posZero = pos;
	// SystemConfig::zeroLensPosToScan(true, pos);
	return;
}


void OctDevice::OctDiopterMotor::setCurrentPositionAsMinusLens(void)
{
	int pos = getPosition();
	setPositionOfMinusLens(pos);
	return;
}


void OctDevice::OctDiopterMotor::setCurrentPositionAsPlusLens(void)
{
	int pos = getPosition();
	setPositionOfPlusLens(pos);
	return;
}


void OctDevice::OctDiopterMotor::setCurrentPositionAsZeroLens(void)
{
	int pos = getPosition();
	setPositionOfZeroLens(pos);
	return;
}


void OctDevice::OctDiopterMotor::loadSystemParameters(void)
{
	getImpl().posMinus = SystemConfig::minusLensPosToScan();
	getImpl().posPlus = SystemConfig::plusLensPosToScan();
	getImpl().posZero = SystemConfig::zeroLensPosToScan();
	return;
}


void OctDevice::OctDiopterMotor::saveSystemParameters(void)
{
	SystemConfig::minusLensPosToScan(true, getImpl().posMinus);
	SystemConfig::plusLensPosToScan(true, getImpl().posPlus);
	SystemConfig::zeroLensPosToScan(true, getImpl().posZero);
	SystemConfig::setModified(true);
	return;
}


OctDiopterMotor::OctDiopterMotorImpl & OctDevice::OctDiopterMotor::getImpl(void) const
{
	return *d_ptr;
}
