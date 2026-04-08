#include "stdafx.h"
#include "LightLed.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

#include <string>

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct LightLed::LightLedImpl
{
	MainBoard *board;
	bool initiated;
	LightLedType type;
	unsigned short value;
	unsigned short valueSet1;
	unsigned short valueSet2;
	unsigned short valueMin;
	unsigned short valueMax;
	unsigned short valueStep;
	unsigned short valueMem;

	CSliderCtrl* pSlider;
	CEdit* pEdit;

	LightLedImpl() : board(nullptr), initiated(false), pSlider(NULL), pEdit(NULL),
		value(0), valueSet1(0), valueSet2(0), valueMem(0), valueMin(0), valueMax(100), valueStep(5)
	{
	}
};


LightLed::LightLed() :
	d_ptr(make_unique<LightLedImpl>())
{
}



OctDevice::LightLed::LightLed(MainBoard * board, LightLedType type) :
	d_ptr(make_unique<LightLedImpl>())
{
	getImpl().board = board;
	getImpl().type = type;

	if (getImpl().type == LightLedType::OCT_SLD) {
		getImpl().valueMin = SLD_INTENSITY_MIN;
		getImpl().valueMax = SLD_INTENSITY_MAX;
		getImpl().value = SLD_INTENSITY_INIT;
		getImpl().valueStep = SLD_INTENSITY_STEP;
	}
	else {
		getImpl().valueMin = LED_INTENSITY_MIN;
		getImpl().valueMax = LED_INTENSITY_MAX;
		getImpl().value = LED_INTENSITY_INIT;
		getImpl().valueStep = LED_INTENSITY_STEP;
	}

	switch (getImpl().type) {
	case LightLedType::PANNEL:
		getImpl().valueMin = LED_PANNEL_VALUE_MIN;
		getImpl().valueMax = LED_PANNEL_VALUE_MAX;
		break;
	}
	return;
}



LightLed::~LightLed()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::LightLed::LightLed(LightLed && rhs) = default;
LightLed & OctDevice::LightLed::operator=(LightLed && rhs) = default;


OctDevice::LightLed::LightLed(const LightLed & rhs)
	: d_ptr(make_unique<LightLedImpl>(*rhs.d_ptr))
{
}


LightLed & OctDevice::LightLed::operator=(const LightLed & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::LightLed::initialize(void)
{
	if (getMainBoard()->isNoLeds()) {
		return true;
	}

	getImpl().initiated = true;

	unsigned short value = getInitValue();

	if (value > 0) {
		bool ret = setIntensity(value);
		if (!ret) {
			LogE() << "led init failed!, name: " << getName();
		}
	}
	return true;
}


bool OctDevice::LightLed::isInitiated(void) const
{
	return getImpl().initiated;
}


void OctDevice::LightLed::setControls(CSliderCtrl * pSlider, CEdit * pEdit)
{
	getImpl().pSlider = pSlider;

	if (pSlider != nullptr) {
		pSlider->SetRange(getImpl().valueMin, getImpl().valueMax);
		pSlider->SetTicFreq(10);
		pSlider->SetPageSize(getImpl().valueStep);
		pSlider->SetPos(getIntensity());
	}

	getImpl().pEdit = pEdit;

	if (pEdit != nullptr) {
		pEdit->SetWindowTextW(to_wstring(getIntensity()).c_str());
	}
	return;
}


bool OctDevice::LightLed::updateIntensityByEdit(void)
{
	if (getImpl().pEdit != NULL) {
		CString value;
		getImpl().pEdit->GetWindowTextW(value);
		if (value.GetLength() > 0) {
			int pos = _ttoi(value);
			return updateIntensity(pos);
		}
	}
	return false;
}


bool OctDevice::LightLed::updateIntensityByOffset(int offset)
{
	int value = getIntensity() + offset;
	return updateIntensity(value);
}


bool OctDevice::LightLed::loadConfig(DeviceSettings * dset)
{
	LightLedItem* item = dset->getLightLedItem(getType());
	if (item != nullptr) {
		setIntensity(item->getValue());
		getImpl().valueSet1 = item->getValue();
	}

	if (getImpl().type == LightLedType::RETINA_IR) {
		LightLedItem* item = dset->getLightLedItem(LightLedType::RETINA_IR_BRIGHT);
		if (item != nullptr) {
			getImpl().valueSet2 = item->getValue();
		}
	}
	return true ;
}


bool OctDevice::LightLed::saveConfig(DeviceSettings * dset)
{
	LightLedItem* item = dset->getLightLedItem(getType());
	if (item != nullptr) {
		item->setValue(getIntensity());
	}
	return true;
}


bool OctDevice::LightLed::updateIntensity(int value)
{
	if (setIntensity(value)) {
		int res = getIntensity();
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


bool OctDevice::LightLed::setIntensity(unsigned short value)
{
	if (!isInitiated()) {
		return false;
	}

	value = (value < getImpl().valueMin ? getImpl().valueMin : value);
	value = (value > getImpl().valueMax ? getImpl().valueMax : value);

	/*
	if (value == getImpl().value) {
		return true;
	}
	*/

	/*
	switch (getImpl().type) {
	//case LightLedType::SPLIT_FOCUS:
	case LightLedType::EXT_FIXATION:
	case LightLedType::WORKING_DOTS:
	//case LightLedType::PANNEL:
		DebugOut2() << "LightLed::setIntensity() not implemented!, name=" << getName() << ", value=" << value;
		getImpl().value = value;
		return true;
	}
	*/

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.LedSetIntensity(getImpl().type, (uint8_t)value)) {
		getImpl().value = value;
		return true;
	}
	LogD() << "LightLed::setIntensity() failed!, name=" << getName() << ", value=" << value;
	return false;
}


unsigned short OctDevice::LightLed::getIntensity(void) const
{
	return getImpl().value;
}


unsigned short OctDevice::LightLed::getValueSet1(void) const
{
	return getImpl().valueSet1;
}


unsigned short OctDevice::LightLed::getValueSet2(void) const
{
	return getImpl().valueSet2;
}


const char * OctDevice::LightLed::getName(void) const
{
	switch (getImpl().type) {
		case LightLedType::RETINA_IR:
			return LED_RETINA_IR_NAME;
		case LightLedType::CORNEA_IR:
			return LED_CORNEA_IR_NAME;
		case LightLedType::KER_RING:
			return LED_KER_RING_NAME;
		case LightLedType::KER_FOCUS:
			return LED_KER_FOCUS_NAME;
		case LightLedType::FUNDUS_FLASH:
			return LED_FUNDUS_FLASH_NAME;
		case LightLedType::SPLIT_FOCUS:
			return LED_SPLIT_FOCUS_NAME;
		case LightLedType::EXT_FIXATION:
			return LED_EXT_FIXATION_NAME;
		case LightLedType::WORKING_DOTS:
			return LED_WORKING_DOTS_NAME;
		case LightLedType::OCT_SLD:
			return LED_OCT_SLD_NAME;
		case LightLedType::PANNEL:
			return LED_PANNEL_NAME;
	}
	return LED_UNKNOWN_NAME;
}


LightLedType OctDevice::LightLed::getType(void) const
{
	return getImpl().type;
}


bool OctDevice::LightLed::lightOn(void)
{
	unsigned short value = getIntensity();

	if (value > 0) {
		getImpl().valueMem = value;
	}
	else if (getImpl().valueMem > 0) {
		value = getImpl().valueMem;
	}

	LogD() << "LightLed on, name: " << getName() << ", value: " << value;
	return setIntensity(value);
}


bool OctDevice::LightLed::lightOff(void)
{
	unsigned short value = getIntensity();

	if (value > 0) {
		getImpl().valueMem = value;
	}
	return setIntensity(0);
}


bool OctDevice::LightLed::control(bool flag)
{
	return (flag ? lightOn() : lightOff());
}

bool OctDevice::LightLed::isLightOn(void)
{
	return getIntensity() > 0;
}


LightLed::LightLedImpl & OctDevice::LightLed::getImpl(void) const
{
	return *d_ptr;
}


MainBoard * OctDevice::LightLed::getMainBoard(void) const
{
	return getImpl().board;
}


std::uint8_t OctDevice::LightLed::getLightLedId(void) const
{
	return std::uint8_t();
}


std::uint16_t OctDevice::LightLed::getInitValue(void) const
{
	switch (getImpl().type) {
	case LightLedType::RETINA_IR:
		return LED_RETINA_IR_INIT_VALUE;
	case LightLedType::CORNEA_IR:
		return LED_CORNEA_IR_INIT_VALUE;
	case LightLedType::KER_RING:
		return LED_KER_RING_INIT_VALUE;
	case LightLedType::KER_FOCUS:
		return LED_KER_FOCUS_INIT_VALUE;
	case LightLedType::FUNDUS_FLASH:
		return LED_FUNDUS_FLASH_INIT_VALUE;
	case LightLedType::SPLIT_FOCUS:
		return LED_SPLIT_FOCUS_INIT_VALUE;
	case LightLedType::EXT_FIXATION:
		return LED_EXT_FIXATION_INIT_VALUE;
	case LightLedType::WORKING_DOTS:
		return LED_WORKING_DOTS_INIT_VALUE;
	case LightLedType::WORKING_DOT2:
		return LED_WORKING_DOTS_INIT_VALUE;
	case LightLedType::OCT_SLD:
		return LED_OCT_SLD_INIT_VALUE;
	case LightLedType::PANNEL:
		return LED_PANNEL_INIT_VALUE;
	}
	return 0;
}
