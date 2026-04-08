#include "stdafx.h"
#include "SysCalib.h"
#include "MainBoard.h"
#include "UsbComm.h"

#include "CppUtil2.h"

using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct SysCalib::SysCalibImpl
{
	SysCal_st sysCal;
	MainBoard *board;
	bool initiated;

	SysCalibImpl() : board(nullptr), initiated(false), sysCal{ 0 } {

	}
};


SysCalib::SysCalib() :
	d_ptr(make_unique<SysCalibImpl>())
{
}


OctDevice::SysCalib::SysCalib(MainBoard * board) :
	d_ptr(make_unique<SysCalibImpl>())
{
	getImpl().board = board;
}


SysCalib::~SysCalib()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::SysCalib::SysCalib(SysCalib && rhs) = default;
SysCalib & OctDevice::SysCalib::operator=(SysCalib && rhs) = default;


SysCal_st & OctDevice::SysCalib::getData(void) const
{
	return d_ptr->sysCal;
}


bool OctDevice::SysCalib::loadData(void)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.SysCalRead(getMainBoard()->getBaseAddressOfSysCalibration(), &getImpl().sysCal)) {
		return true;
	}
	return false;
}


bool OctDevice::SysCalib::saveData(void)
{
	UsbComm& usbComm = getMainBoard()->getUsbComm();
	if (usbComm.SysCalWrite(getMainBoard()->getBaseAddressOfSysCalibration(), &getImpl().sysCal)) {
		return true;
	}
	return false;
}


MainBoard * OctDevice::SysCalib::getMainBoard(void) const
{
	return getImpl().board;
}


void OctDevice::SysCalib::updateCheckSum(void)
{
	size_t size = sizeof(getImpl().sysCal) - 8;		// except checksum bytes
	uint8_t* ptr = (uint8_t*)&(getImpl().sysCal);

	uint64_t chksum = 0;
	for (int i = 0; i < size; i++) {
		chksum += ptr[i];
	}

	getImpl().sysCal.chksum = chksum;
	return;
}


SysCalib::SysCalibImpl & OctDevice::SysCalib::getImpl(void) const
{
	return *d_ptr;
}
