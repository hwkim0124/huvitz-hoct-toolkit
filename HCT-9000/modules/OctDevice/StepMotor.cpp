#include "stdafx.h"
#include "StepMotor.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include <string>
#include <sstream>

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct StepMotor::StepMotorImpl
{
	MainBoard *board;
	bool initiated;
	bool asyncMode;
	StepMotorType type;
	
	int curPos;
	int maxSpeed;
	int minSpeed;
	int accStep;
	int smPosMin;
	int smPosMax;

	int targetPos;

	CSliderCtrl* pSlider;
	CEdit* pEdit;

	StepMotorImpl() 
		: board(nullptr), initiated(false), asyncMode(false),
			pSlider(NULL), pEdit(NULL), 
			smPosMin(0), smPosMax(0), curPos(0), maxSpeed(0), minSpeed(0), accStep(0)
	{
	}
};


StepMotor::StepMotor() :
	d_ptr(make_unique<StepMotorImpl>())
{
}


OctDevice::StepMotor::StepMotor(MainBoard * board, StepMotorType type) :
	d_ptr(make_unique<StepMotorImpl>())
{
	getImpl().board = board;
	getImpl().type = type;
}


StepMotor::~StepMotor()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional definition of copy constructor and assignment for the struct to resolve 
// the situation. 
StepMotor::StepMotor::StepMotor(StepMotor && rhs) = default;
StepMotor & StepMotor::StepMotor::operator=(StepMotor && rhs) = default;


OctDevice::StepMotor::StepMotor(const StepMotor & rhs)
	: d_ptr(make_unique<StepMotorImpl>(*rhs.d_ptr))
{
}


StepMotor & OctDevice::StepMotor::operator=(const StepMotor & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::StepMotor::initialize(void)
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


bool OctDevice::StepMotor::isInitiated(void) const
{
	return getImpl().initiated;
}


void OctDevice::StepMotor::setControls(CSliderCtrl * pSlider, CEdit * pEdit)
{
	getImpl().pSlider = pSlider;
	if (pSlider != NULL) {
		pSlider->SetRange(getRangeMin(), getRangeMax());
		pSlider->SetLineSize(getSliderStepSize());
		pSlider->SetPageSize(getSliderStepSize());
		pSlider->SetPos(getPosition());
	}

	getImpl().pEdit = pEdit;
	if (pEdit != NULL) {
		pEdit->SetWindowTextW(to_wstring(getPosition()).c_str());
	}
	return;
}


bool OctDevice::StepMotor::updatePositionByEdit(void)
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


bool OctDevice::StepMotor::updatePosition(int pos)
{
	if (!isInitiated()) {
		return false;
	}

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


bool OctDevice::StepMotor::updatePositionByOffset(int offset)
{
	int pos = getPosition() + offset;
	return updatePosition(pos);
}


bool OctDevice::StepMotor::controlMove(int pos, bool async)
{
	pos = (pos < getRangeMin() ? getRangeMin() : pos);
	pos = (pos > getRangeMax() ? getRangeMax() : pos);
	getImpl().targetPos = pos;

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorMove(getMotorId(), pos)) 
	{
		if (!isAsyncMode() && !async) {
			return waitForUpdate();
		}
		return true;
	}
	LogE() << "Step motor control move failed!, name: " << getName() << ", pos: " << pos;
	return false;
}


bool OctDevice::StepMotor::controlHome(void)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.MotorHome(getMotorId())) 
	{
		if (!isAsyncMode()) {
			return waitForUpdate();
		}
		return true;
	}
	LogE() << "Step motor control home failed!, name: " << getName();
	return false;
}


bool OctDevice::StepMotor::updateStatus(void)
{
	// No need to be initialized for update from mainboard.
	/*
	if (!isInitiated()) {
		return false;
	}
	*/

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	StepMotorInfo info;
	if (usbComm.MotorStatus(getMotorId(), getMainBoard()->getBaseAddressOfMotorInfo(), &info)) {
		getImpl().curPos = info.cur_pos;
		getImpl().maxSpeed = info.max_speed;
		getImpl().minSpeed = info.min_speed;
		getImpl().smPosMax = info.sm_pos_max;
		getImpl().smPosMin = info.sm_pos_min;
		getImpl().accStep = info.acc_step;
		return true;
	}
	LogE() << "Step motor update status failed!, name: " << getName();
	return false;
}


void OctDevice::StepMotor::reportStatus(void)
{
	ostringstream ss;
	ss << "Step motor name: " << getName();
	ss << ", cur_pos: " << getImpl().curPos;
	ss << ", max_speed: " << getImpl().maxSpeed;
	ss << ", min_speed: " << getImpl().minSpeed;
	ss << ", smPosMax: " << getImpl().smPosMax;
	ss << ", smPosMin: " << getImpl().smPosMin;
	ss << ", accStep: " << getImpl().accStep;
	LogD() << ss.str();
	return;
}


void OctDevice::StepMotor::setAsyncMode(bool flag)
{
	getImpl().asyncMode = flag;
	return;
}


bool OctDevice::StepMotor::isAsyncMode(void) const
{
	return getImpl().asyncMode;
}


bool OctDevice::StepMotor::waitForUpdate(int posOffset, int timeDelay, int countMax)
{
	bool check = false;
	switch (getImpl().type) {
	case StepMotorType::OCT_FOCUS:
	case StepMotorType::REFERENCE:
	case StepMotorType::POLARIZATION:
	case StepMotorType::FUNDUS_FOCUS:
	case StepMotorType::SPLIT_FOCUS:
		break;
	//case StepMotorType::OCT_COMPEN:
		// DebugOut2() << "StepMotor::waitForUpdate() not implemented!, name: " << getName() << ", pos: " << getImpl().targetPos;
		// return true;
	default:
		break;
	}

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
		}
	}

	if (check) {
		return true;
	}
	DebugOut2() << "StepMotor::waitForUpdate() timeout!, name: " << getName() << ", curPos: " << getPosition() << ", target: " << getImpl().targetPos << ", count: " << count;
	return false;
}


void OctDevice::StepMotor::setPosition(int pos)
{
	getImpl().curPos = pos;
	return;
}


int OctDevice::StepMotor::getPosition(void) const
{
	return getImpl().curPos;
}


int OctDevice::StepMotor::getTargetPosition(void) const
{
	return getImpl().targetPos;
}


int OctDevice::StepMotor::getCenterPosition(void) const
{
	return getRangeMin() + getRangeSize() / 2;
}


int OctDevice::StepMotor::getRangeMax(void) const
{
	return getImpl().smPosMax;
}


int OctDevice::StepMotor::getRangeMin(void) const
{
	return getImpl().smPosMin;
}


int OctDevice::StepMotor::getRangeSize(void) const
{
	return abs(getRangeMax() - getRangeMin());
}

void OctDevice::StepMotor::setRangeMax(int value)
{
	getImpl().smPosMax = value;
	return;
}

void OctDevice::StepMotor::setRangeMin(int value)
{
	getImpl().smPosMin = value;
	return;
}


int OctDevice::StepMotor::getSliderStepSize(void) const
{
	int size;
	int range = getRangeSize();

	switch (getImpl().type) {
	case StepMotorType::OCT_FOCUS:
	case StepMotorType::REFERENCE:
	case StepMotorType::POLARIZATION:
		size = range / 100;
		break;
	default:
		if (range < 100) {
			size = range;
		}
		else {
			size = range / 10;
		}
		break;
	}

	return size;
}


bool OctDevice::StepMotor::isEndOfLowerPosition(void) const
{
	if (getPosition() <= (getRangeMin() + MOTOR_END_OF_RANGE_OFFSET)) {
		return true;
	}
	return false;
}


bool OctDevice::StepMotor::isEndOfUpperPosition(void) const
{
	if (getPosition() >= (getRangeMax() - MOTOR_END_OF_RANGE_OFFSET)) {
		return true;
	}
	return false;
}


bool OctDevice::StepMotor::isAtLowerSideOfPosition(void) const
{
	return (getPosition() < getCenterPosition() ? true : false);
}


bool OctDevice::StepMotor::isAtUpperSideOfPosition(void) const
{
	return (getPosition() >= getCenterPosition() ? true : false);
}


bool OctDevice::StepMotor::isAtCenterOfPosition(void) const
{
	return (abs(getPosition() - getCenterPosition()) <= MOTOR_CENTER_OF_POSITION_OFFSET ? true : false);
}


bool OctDevice::StepMotor::loadConfig(DeviceSettings * dset)
{
	return true;

	StepMotorItem* item = dset->getStepMotorItem(getType());
	if (item != nullptr) {
		updatePosition(item->getValue());
		return true;
	}
	return false;
}


bool OctDevice::StepMotor::saveConfig(DeviceSettings * dset)
{
	StepMotorItem* item = dset->getStepMotorItem(getType());
	if (item != nullptr) {
		item->setValue(getPosition());
		return true;
	}
	return false;
}


const char * OctDevice::StepMotor::getName(void) const
{
	switch (getImpl().type) {
		case StepMotorType::OCT_FOCUS:
			return MOTOR_OCT_FOCUS_NAME;
		case StepMotorType::REFERENCE:
			return MOTOR_REFERENCE_NAME;
		case StepMotorType::POLARIZATION:
			return MOTOR_POLARIZATION_NAME;
		case StepMotorType::FUNDUS_FOCUS:
			return MOTOR_FUNDUS_FOCUS_NAME;
		case StepMotorType::OCT_SAMPLE:
			return MOTOR_OCT_SAMPLE_NAME;
		case StepMotorType::SPLIT_FOCUS:
			return MOTOR_SPLIT_FOCUS_NAME;
		case StepMotorType::OCT_DIOPTER:
			return MOTOR_OCT_DIOPTER_NAME;
		case StepMotorType::PUPIL_MASK:
			return MOTOR_PUPIL_MASK_NAME;
		case StepMotorType::FUNDUS_DIOPTER:
			return MOTOR_FUNDUS_DIOPTER_NAME;
		case StepMotorType::STAGE_X:
			return MOTOR_STAGE_X_NAME;
		case StepMotorType::STAGE_Y:
			return MOTOR_STAGE_Y_NAME;
		case StepMotorType::STAGE_Z:
			return MOTOR_STAGE_Z_NAME;
	}
	return MOTOR_UNKNOWN_NAME;
}


StepMotorType OctDevice::StepMotor::getType(void) const
{
	return getImpl().type;
}


StepMotor::StepMotorImpl & OctDevice::StepMotor::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::StepMotor::getMainBoard(void) const
{
	return getImpl().board;
}


std::uint8_t OctDevice::StepMotor::getMotorId(void) const
{
	return static_cast<std::uint8_t>(getImpl().type);
}


std::int32_t OctDevice::StepMotor::getInitPosition(void) const
{
	switch (getImpl().type) {
	case StepMotorType::OCT_FOCUS:
		return MOTOR_OCT_FOCUS_INIT_POS;
	case StepMotorType::REFERENCE:
		return MOTOR_REFERENCE_INIT_POS;
	case StepMotorType::POLARIZATION:
		return MOTOR_POLARIZATION_INIT_POS;
	case StepMotorType::FUNDUS_FOCUS:
		return MOTOR_FUNDUS_FOCUS_INIT_POS;
	case StepMotorType::OCT_SAMPLE:
		return MOTOR_OCT_SAMPLE_INIT_POS;
	case StepMotorType::SPLIT_FOCUS:
		return MOTOR_SPLIT_FOCUS_INIT_POS;
	case StepMotorType::OCT_DIOPTER:
		return MOTOR_DIOPTER_SEL_INIT_POS;
	case StepMotorType::PUPIL_MASK:
		return MOTOR_PUPIL_MASK_INIT_POS;
	case StepMotorType::FUNDUS_DIOPTER:
		return MOTOR_DIOPTER_SEL_INIT_POS;
	case StepMotorType::STAGE_X:
		return MOTOR_STAGE_X_INIT_POS;
	case StepMotorType::STAGE_Y:
		return MOTOR_STAGE_Y_INIT_POS;
	case StepMotorType::STAGE_Z:
		return MOTOR_STAGE_Z_INIT_POS;
	}
	return 0;
}
