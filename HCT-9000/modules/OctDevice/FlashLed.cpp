#include "stdafx.h"
#include "FlashLed.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct FlashLed::FlashLedImpl
{
	bool contMode;

	FlashLedImpl() : contMode(false)
	{
	}
};


FlashLed::FlashLed() :
	d_ptr(make_unique<FlashLedImpl>()), LightLed()
{
}


OctDevice::FlashLed::FlashLed(MainBoard * board, LightLedType type) :
	d_ptr(make_unique<FlashLedImpl>()), LightLed(board, type)
{
}


FlashLed::~FlashLed()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
FlashLed::FlashLed::FlashLed(FlashLed && rhs) = default;
FlashLed & FlashLed::FlashLed::operator=(FlashLed && rhs) = default;


OctDevice::FlashLed::FlashLed(const FlashLed & rhs)
	: d_ptr(make_unique<FlashLedImpl>(*rhs.d_ptr))
{
}


FlashLed & OctDevice::FlashLed::operator=(const FlashLed & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctDevice::FlashLed::setContinuousMode(bool flag)
{
	if (!isInitiated()) {
		return false;
	}

	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.FlashLedContinous(getMainBoard()->getBaseAddressOfSysConfiguration(), flag)) {
		getImpl().contMode = flag;
		setIntensity(getIntensity());
		return true;
	}
	return false;
}


bool OctDevice::FlashLed::getContinuousMode(void)
{
	return getImpl().contMode;
}


FlashLed::FlashLedImpl & OctDevice::FlashLed::getImpl(void) const
{
	return *d_ptr;
}
