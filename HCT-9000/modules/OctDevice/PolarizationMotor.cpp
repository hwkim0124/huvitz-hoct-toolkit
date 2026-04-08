#include "stdafx.h"
#include "PolarizationMotor.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct PolarizationMotor::PolarizationMotorImpl
{
	int32_t posOfOrigin;
	int32_t posPerDegree;

	CEdit* pEditDegree;

	PolarizationMotorImpl() : posOfOrigin(0), posPerDegree(0), pEditDegree(NULL)
	{

	}
};


PolarizationMotor::PolarizationMotor() :
	d_ptr(make_unique<PolarizationMotorImpl>()), StepMotor()
{
}


OctDevice::PolarizationMotor::PolarizationMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<PolarizationMotorImpl>()), StepMotor(board, type)
{
}


PolarizationMotor::~PolarizationMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
PolarizationMotor::PolarizationMotor::PolarizationMotor(PolarizationMotor && rhs) = default;
PolarizationMotor & PolarizationMotor::PolarizationMotor::operator=(PolarizationMotor && rhs) = default;


OctDevice::PolarizationMotor::PolarizationMotor(const PolarizationMotor & rhs)
	: d_ptr(make_unique<PolarizationMotorImpl>(*rhs.d_ptr))
{
}


PolarizationMotor & OctDevice::PolarizationMotor::operator=(const PolarizationMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



bool OctDevice::PolarizationMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		setPositionsPerDegree(MOTOR_POLARIZATION_STEPS_PER_DEGREE);
		// setPositionOfZeroDegree(MOTOR_POLARIZATION_INIT_ORIGIN);
		loadSystemParameters();
		updatePositionToZeroDegree();
		return true;
	}
	return false;
}


void OctDevice::PolarizationMotor::setControls(CSliderCtrl * pSlider, CEdit * pEdit, CEdit * pEditDegree)
{
	StepMotor::setControls(pSlider, pEdit);
	getImpl().pEditDegree = pEditDegree;
	if (pEditDegree != nullptr) {
		CString str;
		str.Format(_T("%.0f"), getCurrentDegree());
		getImpl().pEditDegree->SetWindowTextW(str);
	}
	return;
}


bool OctDevice::PolarizationMotor::updatePositionByDegree(void)
{
	if (getImpl().pEditDegree != NULL) {
		CString value;
		getImpl().pEditDegree->GetWindowTextW(value);
		if (value.GetLength() > 0) {
			float diopt = (float)_ttof(value);
			return updateDegree(diopt);
		}
	}
	return false;
}


bool OctDevice::PolarizationMotor::updatePositionByEdit(void)
{
	if (StepMotor::updatePositionByEdit()) {
		if (getImpl().pEditDegree != NULL) {
			CString str;
			str.Format(_T("%.0f"), getCurrentDegree());
			getImpl().pEditDegree->SetWindowTextW(str);
		}
		return true;
	}
	return false;
}


bool OctDevice::PolarizationMotor::updatePosition(int pos)
{
	if (StepMotor::updatePosition(pos)) {
		if (getImpl().pEditDegree != NULL) {
			CString str;
			str.Format(_T("%.0f"), getCurrentDegree());
			getImpl().pEditDegree->SetWindowTextW(str);
		}
		return true;
	}
	return false;
}


int OctDevice::PolarizationMotor::getPositionOfZeroDegree(void) const
{
	return getImpl().posOfOrigin;
}


void OctDevice::PolarizationMotor::setPositionOfZeroDegree(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	getImpl().posOfOrigin = pos;
	return;
}


void OctDevice::PolarizationMotor::setCurrentPositionAsZeroDegree(void)
{
	int pos = getPosition();
	setPositionOfZeroDegree(pos);
	return;
}


float OctDevice::PolarizationMotor::getCurrentDegree(void) const
{
	float diopt = (float)(getPosition() - getPositionOfZeroDegree()) / (float)getPositionsPerDegree();
	diopt *= getDegreeDirection();
	return diopt;
}


int OctDevice::PolarizationMotor::getPositionsPerDegree(void) const
{
	return getImpl().posPerDegree;
}


void OctDevice::PolarizationMotor::setPositionsPerDegree(int pos)
{
	getImpl().posPerDegree = pos;
	return;
}


bool OctDevice::PolarizationMotor::updateDegree(float degree)
{
	int pos = convertDegreeToPosition(degree);
	return updatePosition(pos);
}


bool OctDevice::PolarizationMotor::updateDegreeByOffset(float offset)
{
	float diopt = getCurrentDegree() + offset;
	return updateDegree(diopt);
}


bool OctDevice::PolarizationMotor::updatePositionToZeroDegree(void)
{
	return updateDegree(0.0f);
}


int OctDevice::PolarizationMotor::convertDegreeToPosition(float diopt)
{
	int pos = (int)(diopt * getPositionsPerDegree() * getDegreeDirection()) + getPositionOfZeroDegree();
	return pos;
}


float OctDevice::PolarizationMotor::convertPositionToDegree(int pos)
{
	float diopt = (float)(pos - getPositionOfZeroDegree()) / (float)getPositionsPerDegree();
	diopt *= getDegreeDirection();
	return diopt;
}


int OctDevice::PolarizationMotor::getDegreeDirection(void) const
{
	return 1;
}


void OctDevice::PolarizationMotor::loadSystemParameters(void)
{
	getImpl().posOfOrigin = SystemConfig::polarizationPos();
	return;
}


void OctDevice::PolarizationMotor::saveSystemParameters(void)
{
	SystemConfig::polarizationPos(true, getImpl().posOfOrigin);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::PolarizationMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfZeroDegree(item->getSetupPosition(0));
			return true;
		}
	}
	return false;
}


bool OctDevice::PolarizationMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfZeroDegree());
			return true;
		}
	}
	return false;
}


PolarizationMotor::PolarizationMotorImpl & OctDevice::PolarizationMotor::getImpl(void) const
{
	return *d_ptr;
}
