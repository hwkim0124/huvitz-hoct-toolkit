#include "stdafx.h"
#include "DiopterSelMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct DiopterSelMotor::DiopterSelMotorImpl
{
	uint32_t posMinus;
	uint32_t posPlus;
	uint32_t posZero;
	uint32_t posMinus2;
	uint32_t posPlus2;
	uint32_t posZero2;

	DiopterSelMotorImpl() : posMinus(0), posPlus(0), posZero(0), 
							posMinus2(0), posPlus2(0), posZero2(0)
	{

	}
};


DiopterSelMotor::DiopterSelMotor() :
	d_ptr(make_unique<DiopterSelMotorImpl>()), StepMotor()
{
}


OctDevice::DiopterSelMotor::DiopterSelMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<DiopterSelMotorImpl>()), StepMotor(board, type)
{
}


DiopterSelMotor::~DiopterSelMotor()
{
}


DiopterSelMotor::DiopterSelMotor::DiopterSelMotor(DiopterSelMotor && rhs) = default;
DiopterSelMotor & DiopterSelMotor::DiopterSelMotor::operator=(DiopterSelMotor && rhs) = default;


OctDevice::DiopterSelMotor::DiopterSelMotor(const DiopterSelMotor & rhs)
	: d_ptr(make_unique<DiopterSelMotorImpl>(*rhs.d_ptr))
{
}


DiopterSelMotor & OctDevice::DiopterSelMotor::operator=(const DiopterSelMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::DiopterSelMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		// setPositionOfZeroLens(MOTOR_DIOPTER_SEL_INIT_ZERO);
		// setPositionOfMinusLens(MOTOR_DIOPTER_SEL_INIT_MINUS);
		// setPositionOfPlusLens(MOTOR_DIOPTER_SEL_INIT_PLUS);
		loadSystemParameters();
		return true;
	}
	return false;
}


bool OctDevice::DiopterSelMotor::updatePositionToMinusLens(bool isScan)
{
	int pos = getPositionOfMinusLens(isScan);
	return StepMotor::updatePosition(pos);
}


bool OctDevice::DiopterSelMotor::updatePositionToPlusLens(bool isScan)
{
	int pos = getPositionOfPlusLens(isScan);
	return StepMotor::updatePosition(pos);
}


bool OctDevice::DiopterSelMotor::updatePositionToZeroLens(bool isScan)
{
	int pos = getPositionOfZeroLens(isScan);
	return StepMotor::updatePosition(pos);
}


int OctDevice::DiopterSelMotor::getPositionOfMinusLens(bool isScan) const
{
	return (isScan ? getImpl().posMinus : getImpl().posMinus2);
	// return (isScan ? SystemConfig::minusLensPosToScan() : SystemConfig::minusLensPosToFundus());
}


int OctDevice::DiopterSelMotor::getPositionOfPlusLens(bool isScan) const
{
	return (isScan ? getImpl().posPlus : getImpl().posPlus2);
	// return (isScan ? SystemConfig::plusLensPosToScan() : SystemConfig::plusLensPosToFundus());
}


int OctDevice::DiopterSelMotor::getPositionOfZeroLens(bool isScan) const
{
	return (isScan ? getImpl().posZero : getImpl().posZero2) ;
	// return (isScan ? SystemConfig::zeroLensPosToScan() : SystemConfig::zeroLensPosToFundus());
}


void OctDevice::DiopterSelMotor::setPositionOfMinusLens(bool isScan, int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	 
	if (isScan) {
		getImpl().posMinus = pos;
		// SystemConfig::minusLensPosToScan(true, pos);
	}
	else {
		getImpl().posMinus2 = pos;
		// SystemConfig::minusLensPosToFundus(true, pos);
	}
	return;
}


void OctDevice::DiopterSelMotor::setPositionOfPlusLens(bool isScan, int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	
	if (isScan) {
		getImpl().posPlus = pos;
		// SystemConfig::plusLensPosToScan(true, pos);
	}
	else {
		getImpl().posPlus2 = pos;
		// SystemConfig::plusLensPosToFundus(true, pos);
	}
	return;
}


void OctDevice::DiopterSelMotor::setPositionOfZeroLens(bool isScan, int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	if (isScan) {
		getImpl().posZero = pos;
		// SystemConfig::zeroLensPosToScan(true, pos);
	}
	else {
		getImpl().posZero2 = pos;
		// SystemConfig::zeroLensPosToFundus(true, pos);
	}
	return;
}


void OctDevice::DiopterSelMotor::setCurrentPositionAsMinusLens(bool isScan)
{
	int pos = getPosition();
	setPositionOfMinusLens(isScan, pos);
	return;
}


void OctDevice::DiopterSelMotor::setCurrentPositionAsPlusLens(bool isScan)
{
	int pos = getPosition();
	setPositionOfPlusLens(isScan, pos);
	return;
}


void OctDevice::DiopterSelMotor::setCurrentPositionAsZeroLens(bool isScan)
{
	int pos = getPosition();
	setPositionOfZeroLens(isScan, pos);
	return;
}


void OctDevice::DiopterSelMotor::loadSystemParameters(void)
{
	getImpl().posMinus = SystemConfig::minusLensPosToScan();
	getImpl().posMinus2 = SystemConfig::minusLensPosToFundus();
	getImpl().posPlus = SystemConfig::plusLensPosToScan();
	getImpl().posPlus2 = SystemConfig::plusLensPosToFundus();
	getImpl().posZero = SystemConfig::zeroLensPosToScan();
	getImpl().posZero2 = SystemConfig::zeroLensPosToFundus();
	return;
}


void OctDevice::DiopterSelMotor::saveSystemParameters(void)
{
	SystemConfig::minusLensPosToScan(true, getImpl().posMinus);
	SystemConfig::minusLensPosToFundus(true, getImpl().posMinus2);
	SystemConfig::plusLensPosToScan(true, getImpl().posPlus);
	SystemConfig::plusLensPosToFundus(true, getImpl().posPlus2);
	SystemConfig::zeroLensPosToScan(true, getImpl().posZero);
	SystemConfig::zeroLensPosToFundus(true, getImpl().posZero2);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::DiopterSelMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfMinusLens(true, item->getSetupPosition(0));
			setPositionOfZeroLens(true, item->getSetupPosition(1));
			setPositionOfPlusLens(true, item->getSetupPosition(2));
			return true;
		}
	}
	return false;
}


bool OctDevice::DiopterSelMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfMinusLens(true));
			item->setSetupPosition(1, getPositionOfZeroLens(true));
			item->setSetupPosition(2, getPositionOfPlusLens(true));
			return true;
		}
	}
	return false;
}


DiopterSelMotor::DiopterSelMotorImpl & OctDevice::DiopterSelMotor::getImpl(void) const
{
	return *d_ptr;
}
