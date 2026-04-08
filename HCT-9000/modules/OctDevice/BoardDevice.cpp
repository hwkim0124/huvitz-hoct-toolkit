#include "stdafx.h"
#include "BoardDevice.h"
#include "MainBoard.h"

#include <string>

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct BoardDevice::BoardDeviceImpl
{
	MainBoard *board;

	BoardDeviceImpl() : board(nullptr) {
	}
};


BoardDevice::BoardDevice() :
	d_ptr(make_unique<BoardDeviceImpl>())
{
}


OctDevice::BoardDevice::BoardDevice(MainBoard * board) :
	d_ptr(make_unique<BoardDeviceImpl>())
{
	getImpl().board = board;
}


BoardDevice::~BoardDevice()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional definition of copy constructor and assignment for the struct to resolve 
// the situation. 
BoardDevice::BoardDevice::BoardDevice(BoardDevice && rhs) = default;
BoardDevice & BoardDevice::BoardDevice::operator=(BoardDevice && rhs) = default;


OctDevice::BoardDevice::BoardDevice(const BoardDevice & rhs)
	: d_ptr(make_unique<BoardDeviceImpl>(*rhs.d_ptr))
{
}


BoardDevice & OctDevice::BoardDevice::operator=(const BoardDevice & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctDevice::BoardDevice::loadSystemParameters(void)
{
}


void OctDevice::BoardDevice::saveSystemParameters(void)
{
}


BoardDevice::BoardDeviceImpl & OctDevice::BoardDevice::getImpl(void) const
{
	return *d_ptr;
}
