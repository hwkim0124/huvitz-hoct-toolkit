#include "stdafx.h"
#include "OctFocusMotor.h"
#include "MainBoard.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct OctFocusMotor::OctFocusMotorImpl
{
	int32_t posOfOrigin;
	int32_t posOfOrigin2;
	int32_t posPerDiopt;

	CEdit* pEditDiopt;

	OctFocusMotorImpl() : posOfOrigin(0), posOfOrigin2(0), posPerDiopt(0), pEditDiopt(NULL)
	{

	}
};


OctFocusMotor::OctFocusMotor() :
	d_ptr(make_unique<OctFocusMotorImpl>()), StepMotor()
{
}


OctDevice::OctFocusMotor::OctFocusMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<OctFocusMotorImpl>()), StepMotor(board, type)
{
}


OctFocusMotor::~OctFocusMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctFocusMotor::OctFocusMotor::OctFocusMotor(OctFocusMotor && rhs) = default;
OctFocusMotor & OctFocusMotor::OctFocusMotor::operator=(OctFocusMotor && rhs) = default;


OctDevice::OctFocusMotor::OctFocusMotor(const OctFocusMotor & rhs)
	: d_ptr(make_unique<OctFocusMotorImpl>(*rhs.d_ptr))
{
}


OctFocusMotor & OctDevice::OctFocusMotor::operator=(const OctFocusMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::OctFocusMotor::initialize(void)
{
	if (StepMotor::initialize()) {
		setPositionsPerDiopter(MOTOR_OCT_FOCUS_STEPS_PER_DIOPTER);
		// setPositionOfZeroDiopter(MOTOR_OCT_FOCUS_INIT_ORIGIN);
		loadSystemParameters();
		updatePositionToZeroDiopter();
		return true;
	}
	return false;
}


void OctDevice::OctFocusMotor::setControls(CSliderCtrl * pSlider, CEdit * pEdit, CEdit * pEditDiopt)
{
	StepMotor::setControls(pSlider, pEdit);
	getImpl().pEditDiopt = pEditDiopt;
	if (pEditDiopt != nullptr) {
		CString str;
		str.Format(_T("%.1f"), getCurrentDiopter());
		pEditDiopt->SetWindowTextW(str);
	}
	return;
}


bool OctDevice::OctFocusMotor::updatePositionByDiopt(void)
{
	if (getImpl().pEditDiopt != NULL) {
		CString value;
		getImpl().pEditDiopt->GetWindowTextW(value);
		if (value.GetLength() > 0) {
			float diopt = (float) _ttof(value);
			return updateDiopter(diopt);
		}
	}
	return false;
}


bool OctDevice::OctFocusMotor::updatePositionByEdit(void)
{
	if (StepMotor::updatePositionByEdit()) {
		if (getImpl().pEditDiopt != NULL) {
			CString str;
			str.Format(_T("%.1f"), getCurrentDiopter());
			getImpl().pEditDiopt->SetWindowTextW(str);
		}
		return true;
	}
	return false;
}


bool OctDevice::OctFocusMotor::updatePosition(int pos)
{
	if (StepMotor::updatePosition(pos)) {
		if (getImpl().pEditDiopt != NULL) {
			CString str;
			str.Format(_T("%.1f"), getCurrentDiopter());
			getImpl().pEditDiopt->SetWindowTextW(str);
		}
		return true;
	}
	return false;
}


int OctDevice::OctFocusMotor::getPositionOfZeroDiopter(void) const
{
	return getImpl().posOfOrigin;
}


int OctDevice::OctFocusMotor::getPositionOfTopography(void) const
{
	return getImpl().posOfOrigin2;
}


int OctDevice::OctFocusMotor::getPositionOfTopographyDiopter(void)
{
	float diopt = convertPositionToDiopter(getImpl().posOfOrigin2);

	return diopt;
}


void OctDevice::OctFocusMotor::setPositionOfZeroDiopter(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	
	getImpl().posOfOrigin = pos;
	return;
}


void OctDevice::OctFocusMotor::setPositionOfTopogrphy(int pos)
{
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	pos = (pos < getRangeMin() ? getRangeMin() : pos);

	getImpl().posOfOrigin2 = pos;
	return;
}


void OctDevice::OctFocusMotor::setCurrentPositionAsZeroDiopter(void)
{
	int pos = getPosition();
	setPositionOfZeroDiopter(pos);
	return;
}


void OctDevice::OctFocusMotor::setCurrentPositionAsTopogrphyDiopter(void)
{
	int pos = getPosition();
	setPositionOfTopogrphy(pos);
	return;
}


float OctDevice::OctFocusMotor::getCurrentDiopter(void) const
{
	float diopt = (float)(getPosition() - getPositionOfZeroDiopter()) / (float) getPositionsPerDiopter();
	diopt *= getDiopterDirection();
	return diopt ;
}


int OctDevice::OctFocusMotor::getPositionsPerDiopter(void) const
{
	return getImpl().posPerDiopt;
}


void OctDevice::OctFocusMotor::setPositionsPerDiopter(int pos)
{
	getImpl().posPerDiopt = pos;
	return;
}


bool OctDevice::OctFocusMotor::updateDiopter(float diopt)
{
	int pos = convertDiopterToPosition(diopt);
	return updatePosition(pos);
}


bool OctDevice::OctFocusMotor::updateTopographyDiopter(void)
{
	int pos = getPositionOfTopography();
	return updatePosition(pos);
}


bool OctDevice::OctFocusMotor::updateDiopterByOffset(float offset)
{
	float diopt = getCurrentDiopter() + offset;
	return updateDiopter(diopt);
}


bool OctDevice::OctFocusMotor::updatePositionToZeroDiopter(void)
{
	return updateDiopter(0.0f);
}


int OctDevice::OctFocusMotor::convertDiopterToPosition(float diopt)
{
	int pos = (int) (diopt * getPositionsPerDiopter() * getDiopterDirection()) + getPositionOfZeroDiopter();
	return pos;
}


float OctDevice::OctFocusMotor::convertPositionToDiopter(int pos)
{
	float diopt = (float)(pos - getPositionOfZeroDiopter()) / (float) getPositionsPerDiopter();
	diopt *= getDiopterDirection();
	return diopt;
}


int OctDevice::OctFocusMotor::getDiopterDirection(void) const
{
	return -1;
}


bool OctDevice::OctFocusMotor::isEndOfPlusDiopter(void) const
{
	// return (getCurrentDiopter() >= MOTOR_OCT_FOCUS_DIOPTER_MAX ? true : false) ;
	// return isEndOfUpperPosition();
	return isEndOfLowerPosition();
}


bool OctDevice::OctFocusMotor::isEndOfMinusDiopter(void) const
{
	// return (getCurrentDiopter() <= MOTOR_OCT_FOCUS_DIOPTER_MIN ? true : false) ;
	// return isEndOfLowerPosition();
	return isEndOfUpperPosition();
}


void OctDevice::OctFocusMotor::loadSystemParameters(void)
{
	getImpl().posOfOrigin = SystemConfig::zeroDioptPosToScanFocus();
	getImpl().posOfOrigin2 = SystemConfig::zeroDioptPosToTopographyScanFocus();

	return;
}


void OctDevice::OctFocusMotor::saveSystemParameters(void)
{
	SystemConfig::zeroDioptPosToScanFocus(true, getImpl().posOfOrigin);
	SystemConfig::zeroDioptPosToTopographyScanFocus(true, getImpl().posOfOrigin2);
	SystemConfig::setModified(true);
	return;
}


bool OctDevice::OctFocusMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::loadConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			setPositionOfZeroDiopter(item->getSetupPosition(0));
			setPositionOfTopogrphy(SystemConfig::zeroDioptPosToTopographyScanFocus(false));
			return true;
		}
	}
	return false;
}


bool OctDevice::OctFocusMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	if (StepMotor::saveConfig(dset)) {
		StepMotorItem* item = dset->getStepMotorItem(getType());
		if (item != nullptr) {
			item->setSetupPosition(0, getPositionOfZeroDiopter());
			return true;
		}
	}
	return false;
}


OctFocusMotor::OctFocusMotorImpl & OctDevice::OctFocusMotor::getImpl(void) const
{
	return *d_ptr;
}
