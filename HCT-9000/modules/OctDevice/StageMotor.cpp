#include "stdafx.h"
#include "StageMotor.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include <string>

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct StageMotor::StageMotorImpl
{
	MainBoard *board;
	StageMotorType type;
	bool initiated;
	bool asyncMode;
	bool stopped;

	int curPos;
	int centerPos;
	int targetPos;
	int limitRange[2];
	int limitStatus[2];

	CSliderCtrl* pSlider;
	CEdit* pEdit;

	StageMotorImpl()
		: board(nullptr), initiated(false), asyncMode(true), stopped(false),
		curPos(0), centerPos(0), targetPos(0), limitRange{0}, limitStatus{false}, 
		pSlider(NULL), pEdit(NULL)
	{
	}
};


StageMotor::StageMotor() :
	d_ptr(make_unique<StageMotorImpl>())
{
}


OctDevice::StageMotor::StageMotor(MainBoard * board, StageMotorType type) :
	d_ptr(make_unique<StageMotorImpl>())
{
	getImpl().board = board;
	getImpl().type = type;
}


StageMotor::~StageMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional definition of copy constructor and assignment for the struct to resolve 
// the situation. 
StageMotor::StageMotor::StageMotor(StageMotor && rhs) = default;
StageMotor & StageMotor::StageMotor::operator=(StageMotor && rhs) = default;


OctDevice::StageMotor::StageMotor(const StageMotor & rhs)
	: d_ptr(make_unique<StageMotorImpl>(*rhs.d_ptr))
{
}


StageMotor & OctDevice::StageMotor::operator=(const StageMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::StageMotor::initialize(void)
{
	bool ret = updateStatus();
	if (!ret) {
		return false;
	}
	else {
		reportStatus();
		/*
		int value = getInitPosition();
		if (value != getPosition()) {
			controlMove(value);
		}
		*/
	}

	if (getMainBoard()->isNoMotors()) {
		return true;
	}
	getImpl().initiated = true;
	return ret;
}


bool OctDevice::StageMotor::isInitiated(void) const
{
	return getImpl().initiated;
}


bool OctDevice::StageMotor::isStepMotor(void) const
{
	return (getType() == StageMotorType::STAGE_Y);
}


void OctDevice::StageMotor::setControls(CSliderCtrl * pSlider, CEdit * pEdit)
{
	getImpl().pSlider = pSlider;

	if (pSlider != nullptr) {
		pSlider->SetRange(getRangeMin(), getRangeMax());
		pSlider->SetLineSize(getSliderStepSize());
		pSlider->SetPageSize(getSliderStepSize());
		pSlider->SetPos(getPosition());
	}

	getImpl().pEdit = pEdit;
	if (pEdit != nullptr) {
		pEdit->SetWindowTextW(to_wstring(getPosition()).c_str());
	}
	return;
}


int OctDevice::StageMotor::getSliderStepSize(void) const
{
	int size = 0;
	if (isStepMotor()) {
		size = (getRangeSize() / 100);
	}
	size = (size <= 0 ? 1 : size);
	return size;
}


bool OctDevice::StageMotor::updatePositionByDelta(int delta)
{
	return controlDelta(delta);
}


bool OctDevice::StageMotor::updatePositionByEdit(void)
{
	if (getImpl().pEdit != NULL) {
		CString value;
		getImpl().pEdit->GetWindowTextW(value);
		if (value.GetLength() > 0) {
			int pos = _ttoi(value);
			return updatePosition(pos);
		}
	}
	return false;
}


bool OctDevice::StageMotor::updatePosition(int pos)
{
	if (controlMove(pos)) {
		int res = getPosition();
		if (getImpl().pSlider != NULL) {
			getImpl().pSlider->SetPos(res);
		}
		if (getImpl().pEdit != NULL) {
			CString text;
			text.Format(_T("%d"), res);
			getImpl().pEdit->SetWindowText(text);
			getImpl().pEdit->SetSel(0, -1, FALSE);
		}
		return true;
	}
	return false;
}


bool OctDevice::StageMotor::updatePositionByOffset(int offset)
{
	// int pos = getPosition() + offset;
	// return updatePosition(pos);
	return controlJogg(offset);
}


bool OctDevice::StageMotor::updatePositionToCenter(void)
{
	int offset = getCenterPosition() - getPosition();
	return updatePositionByOffset(offset);
}


bool OctDevice::StageMotor::updateStatus(void)
{
	/*
	if (!isInitiated()) {
		return false;
	}
	*/

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (isStepMotor()) {
		StageStepMotorInfo info;
		if (usbComm.StageStatus(getType(), getMainBoard()->getBaseAddressOfStageInfo(), &info)) {
			getImpl().curPos = info.cur_pos;
			getImpl().limitStatus[0] = info.limit_sensor_state[0];
			getImpl().limitStatus[1] = info.limit_sensor_state[1];
			getImpl().limitRange[0] = info.sm_pos_min;
			getImpl().limitRange[1] = info.sm_pos_max;
			return true;
		}
	}
	else {
		StageDcMotorInfo info;
		if (usbComm.StageStatus(getType(), getMainBoard()->getBaseAddressOfStageInfo(), &info)) {
			getImpl().curPos = info.enc_pos;
			getImpl().centerPos = info.center_pos;
			return true;
		}
	}
	LogE() << "Stage motor update status failed!, name: " << getName();
	return false;
}



bool OctDevice::StageMotor::controlMove(int pos)
{
	/*
	if (!isInitiated()) {
		return false;
	}
	*/

	if (isStepMotor() == false) {
		pos = (pos < getRangeMin() ? getRangeMin() : pos);
		pos = (pos > getRangeMax() ? getRangeMax() : pos);
	}
	getImpl().targetPos = pos;

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorMove(getMotorId(), pos))
	{
		getImpl().stopped = false;
		if (!isAsyncMode()) {
			return waitForUpdate();
		}
		return true;
	}
	DebugOut2() << "StageMotor::controlMove() failed!, name: " << getName() << ", pos: " << pos;
	return false;
}


bool OctDevice::StageMotor::controlJogg(int delta)
{
	/*
	if (!isInitiated()) {
		return false;
	}
	*/

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorJog(getMotorId(), delta)) {
		getImpl().stopped = false;
		return true;
	}

	DebugOut2() << "StageMotor::controlJogg() failed!, name: " << getName() ;
	return false;
}


bool OctDevice::StageMotor::controlDelta(int delta)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.StageJog(getMotorId(), delta)) {
		getImpl().stopped = false;
		return true;
	}

	DebugOut2() << "StageMotor::controlDelta() failed!, name: " << getName();
	return false;
}


bool OctDevice::StageMotor::controlHome(void)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorHome(getMotorId()))
	{
		getImpl().stopped = false;
		if (!isAsyncMode()) {
			return waitForUpdate();
		}
		return true;
	}
	DebugOut2() << "StageMotor::controlHome() failed!, name =" << getName();
	return false;
}


bool OctDevice::StageMotor::controlStop(void)
{
	if (!isInitiated()) {
		return false;
	}
	if (getImpl().stopped) {
		return true;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorStop(getMotorId())) {
		getImpl().stopped = true;
		return true;
	}
	DebugOut2() << "StageMotor::controlHome() failed!, name =" << getName();
	return false;
}


void OctDevice::StageMotor::reportStatus(void)
{
	ostringstream ss;

	if (isStepMotor()) {
		ss << "Stage motor name: " << getName();
		ss << ", cur_pos: " << getImpl().curPos;
		ss << ", limit[0]: " << getImpl().limitStatus[0];
		ss << ", limit[1]: " << getImpl().limitStatus[1];
		ss << ", range[0]: " << getImpl().limitRange[0];
		ss << ", range[1]: " << getImpl().limitRange[1];
	}
	else {
		ss << "Stage motor name: " << getName();
		ss << ", cur_pos: " << getImpl().curPos;
		ss << ", center_pos: " << getImpl().centerPos;
	}
	LogD() << ss.str();
	return;
}


void OctDevice::StageMotor::setAsyncMode(bool flag)
{
	getImpl().asyncMode = flag;
	return;
}


bool OctDevice::StageMotor::isAsyncMode(void) const
{
	return getImpl().asyncMode;
}


bool OctDevice::StageMotor::waitForUpdate(int posOffset, int timeDelay, int countMax)
{
	bool check = false;

	// Polling the current position of moving motor.
	int count = 0;
	while (!check) {
		std::this_thread::sleep_for(std::chrono::milliseconds(timeDelay));
		if (!updateStatus() || ++count >= countMax) {
			break;
		}
		else {
			if (abs(getPosition() - getImpl().targetPos) <= posOffset) {
				check = true;
				break;
			}
			if (isEndOfLowerPosition() || isEndOfUpperPosition()) {
				check = true;
				break;
			}
		}
	}

	if (check) {
		return true;
	}
	DebugOut2() << "StageMotor::waitForUpdate() timeout!, name: " << getName() << ", curPos: " << getPosition() << ", target: " << getImpl().targetPos << ", count: " << count;
	return false;
}


const char * OctDevice::StageMotor::getName(void) const
{
	switch (getType()) {
	case StageMotorType::STAGE_X:
		return MOTOR_STAGE_X_NAME;
	case StageMotorType::STAGE_Y:
		return MOTOR_STAGE_Y_NAME;
	case StageMotorType::STAGE_Z:
		return MOTOR_STAGE_Z_NAME;
	}

	return MOTOR_UNKNOWN_NAME;
}


StageMotorType OctDevice::StageMotor::getType(void) const
{
	return getImpl().type;
}


bool OctDevice::StageMotor::isEndOfLowerPosition(void) const
{
	if (isStepMotor()) {
		if (getImpl().limitStatus[0]) {
			return true;
		}
	}
	else {
		if (getPosition() <= (getRangeMin() + STAGE_END_OF_LIMIT_OFFSET)) {
			return true;
		}
	}
	return false;
}


bool OctDevice::StageMotor::isEndOfUpperPosition(void) const
{
	if (isStepMotor()) {
		if (getImpl().limitStatus[1]) {
			return true;
		}
	}
	else {
		if (getPosition() >= (getRangeMax() - STAGE_END_OF_LIMIT_OFFSET)) {
			return true;
		}
	}
	return false;
}


bool OctDevice::StageMotor::isAtLowerSideOfPosition(void) const
{
	return (getPosition() < getCenterPosition() ? true : false);
}


bool OctDevice::StageMotor::isAtUpperSideOfPosition(void) const
{
	return (getPosition() >= getCenterPosition() ? true : false);
}


bool OctDevice::StageMotor::isAtCenterOfPosition(void) const
{
	return (abs(getPosition() - getCenterPosition()) <= STAGE_END_OF_LIMIT_OFFSET ? true : false);
}


bool OctDevice::StageMotor::loadConfig(OctConfig::DeviceSettings * dset)
{
	return true;
}


bool OctDevice::StageMotor::saveConfig(OctConfig::DeviceSettings * dset)
{
	return true;
}


StageMotor::StageMotorImpl & OctDevice::StageMotor::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::StageMotor::getMainBoard(void) const
{
	return getImpl().board;
}


std::uint8_t OctDevice::StageMotor::getMotorId(void) const
{
	return static_cast<std::uint8_t>(getImpl().type);
}


std::int32_t OctDevice::StageMotor::getInitPosition(void) const
{
	switch (getImpl().type) {
	case StageMotorType::STAGE_X:
		return MOTOR_STAGE_X_INIT_POS;
	case StageMotorType::STAGE_Y:
		return MOTOR_STAGE_Y_INIT_POS;
	case StageMotorType::STAGE_Z:
		return MOTOR_STAGE_Z_INIT_POS;
	}
	return 0;
}


void OctDevice::StageMotor::setPosition(int pos)
{
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	getImpl().curPos = pos;
	return;
}


int OctDevice::StageMotor::getPosition(void) const
{
	return getImpl().curPos;
}


int OctDevice::StageMotor::getTargetPosition(void) const
{
	return getImpl().targetPos;
}


int OctDevice::StageMotor::getCenterPosition(void) const
{
	// return getRangeMin() + getRangeSize() / 2;
	return getImpl().centerPos;
}


void OctDevice::StageMotor::setLimitRange(int low, int high)
{
	getImpl().limitRange[0] = low;
	getImpl().limitRange[1] = high;
	return;
}


int OctDevice::StageMotor::getRangeMax(void) const
{
	if (isStepMotor()) {
		return getImpl().limitRange[1];
	}
	else {
		return getImpl().centerPos + getImpl().limitRange[1];
	}
}


int OctDevice::StageMotor::getRangeMin(void) const
{
	if (isStepMotor()) {
		return getImpl().limitRange[0];
	}
	else {
		return getImpl().centerPos + getImpl().limitRange[0];
	}
}


int OctDevice::StageMotor::getRangeSize(void) const
{
	return abs(getRangeMax() - getRangeMin());
}
