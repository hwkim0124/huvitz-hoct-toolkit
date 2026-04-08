#include "stdafx.h"
#include "OctSldLed.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct OctSldLed::OctSldLedImpl
{
	SldParameters params;

	OctSldLedImpl() : params{0}
	{
	}
};


OctSldLed::OctSldLed() :
	d_ptr(make_unique<OctSldLedImpl>()), LightLed()
{
}


OctDevice::OctSldLed::OctSldLed(MainBoard * board, LightLedType type) :
	d_ptr(make_unique<OctSldLedImpl>()), LightLed(board, type)
{
}


OctSldLed::~OctSldLed()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctSldLed::OctSldLed::OctSldLed(OctSldLed && rhs) = default;
OctSldLed & OctSldLed::OctSldLed::operator=(OctSldLed && rhs) = default;


OctDevice::OctSldLed::OctSldLed(const OctSldLed & rhs)
	: d_ptr(make_unique<OctSldLedImpl>(*rhs.d_ptr))
{
}


OctSldLed & OctDevice::OctSldLed::operator=(const OctSldLed & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::OctSldLed::initialize(void)
{
	if (LightLed::initialize()) {
		loadSystemParameters();
		lightOff();
		return true;
	}
	return false;
}


void OctDevice::OctSldLed::lightOn(void)
{
	updateIntensity(1);
	return;
}


void OctDevice::OctSldLed::lightOff(void)
{
	updateIntensity(0);
	return;
}


bool OctDevice::OctSldLed::updateParametersToSldMemory(void)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (!usbComm.OctSldUpdateParameters(0)) {
		DebugOut2() << "OctSldLed::updateParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldUpdateParameters(1)) {
		DebugOut2() << "OctSldLed::updateParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldUpdateParameters(2)) {
		DebugOut2() << "OctSldLed::updateParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldUpdateParameters(3)) {
		DebugOut2() << "OctSldLed::updateParameters() failed!";
		return false;
	}
	return true;
}


bool OctDevice::OctSldLed::fetchParametersToMainMemory(void)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (!usbComm.OctSldGetParameters(0)) {
		DebugOut2() << "OctSldLed::fetchParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldGetParameters(1)) {
		DebugOut2() << "OctSldLed::fetchParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldGetParameters(2)) {
		DebugOut2() << "OctSldLed::fetchParameters() failed!";
		return false;
	}
	if (!usbComm.OctSldGetParameters(3)) {
		DebugOut2() << "OctSldLed::fetchParameters() failed!";
		return false;
	}
	return true;
}


std::uint16_t OctDevice::OctSldLed::highCode(void)
{
	return getImpl().params.rmonHighCode;
}


std::uint16_t OctDevice::OctSldLed::lowCode1(void)
{
	return getImpl().params.rmonLowCode1;
}


std::uint16_t OctDevice::OctSldLed::lowCode2(void)
{
	return getImpl().params.rmonLowCode2;
}


std::uint16_t OctDevice::OctSldLed::rsiCode(void)
{
	return getImpl().params.rmonRsiCode;
}


bool OctDevice::OctSldLed::setHighCode(std::uint16_t code)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.OctSldPotentiometer(RMON_HIGH_CODE, code)) {
		getImpl().params.rmonHighCode = code;
		return true;
	}
	DebugOut2() << "OctSldLed::setHighCode() failed!";
	return false;
}


bool OctDevice::OctSldLed::setLowCode1(std::uint16_t code)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.OctSldPotentiometer(RMON_LOW_CODE1, code)) {
		getImpl().params.rmonLowCode1 = code;
		return true;
	}
	DebugOut2() << "OctSldLed::setLowCode1() failed!";
	return false;
}


bool OctDevice::OctSldLed::setLowCode2(std::uint16_t code)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.OctSldPotentiometer(RMON_LOW_CODE2, code)) {
		getImpl().params.rmonLowCode2 = code;
		return true;
	}
	DebugOut2() << "OctSldLed::setLowCode2() failed!";
	return false;
}


bool OctDevice::OctSldLed::setRsiCode(std::uint16_t code)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.OctSldPotentiometer(RMON_RSI_CODE, code)) {
		getImpl().params.rmonRsiCode = code;
		return true;
	}
	DebugOut2() << "OctSldLed::setRsiCode() failed!";
	return false;
}


void OctDevice::OctSldLed::loadSystemParameters(void)
{
	getImpl().params.rmonHighCode = SystemConfig::sldParameterHighCode();
	getImpl().params.rmonLowCode1 = SystemConfig::sldParameterLowCode1();
	getImpl().params.rmonLowCode2 = SystemConfig::sldParameterLowCode2();
	getImpl().params.rmonRsiCode = SystemConfig::sldParameterRsiCode();
	return;
}


void OctDevice::OctSldLed::saveSystemParameters(void)
{
	SystemConfig::sldParameterHighCode(true, getImpl().params.rmonHighCode);
	SystemConfig::sldParameterLowCode1(true, getImpl().params.rmonLowCode1);
	SystemConfig::sldParameterLowCode2(true, getImpl().params.rmonLowCode2);
	SystemConfig::sldParameterRsiCode(true, getImpl().params.rmonRsiCode);
	SystemConfig::setModified(true);
	return;
}


OctSldLed::OctSldLedImpl & OctDevice::OctSldLed::getImpl(void) const
{
	// TODO: insert return statement here
	return *d_ptr;
}
