#include "stdafx.h"
#include "UsbPort.h"
#include "CppUtil2.h"

#include <string>
#include <mutex>

#include "FTD3XX.h"


using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct UsbPort::UsbPortImpl
{
	FT_HANDLE handle;
	string strDesc;
	unsigned char pidReadCmd;
	unsigned char pidWriteCmd;
	unsigned char pidReadIr1;
	unsigned char pidReadIr2;
	unsigned long timeout;

	bool isErrorPipes[4];
	mutex mutexPort;

	UsbPortImpl() : handle(nullptr), timeout(15000), isErrorPipes{ false } {
	}
};


UsbPort::UsbPort() :
	d_ptr(make_unique<UsbPortImpl>())
{
}


UsbPort::~UsbPort()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::UsbPort::UsbPort(UsbPort && rhs) = default;
UsbPort & OctDevice::UsbPort::operator=(UsbPort && rhs) = default;


UsbPort::UsbPortImpl & OctDevice::UsbPort::getImpl(void) const
{
	return *d_ptr;
}


int OctDevice::UsbPort::getIndexOfPipe(unsigned char pid) const
{
	int idx = 0;
	if (pid == getImpl().pidReadCmd) {
		idx = 0;
	}
	else if (pid == getImpl().pidWriteCmd) {
		idx = 1;
	}
	else if (pid == getImpl().pidReadIr1) {
		idx = 2;
	}
	else if (pid == getImpl().pidReadIr2) {
		idx = 3;
	}
	return idx;
}


bool OctDevice::UsbPort::open(const std::string & desc, unsigned long timeout, unsigned char readPid, 
							unsigned char writePid, unsigned char readIr1, unsigned char readIr2)
{
	close();

	lock_guard<mutex> lock(getImpl().mutexPort);

	getImpl().strDesc = desc;
	getImpl().timeout = timeout;
	getImpl().pidReadCmd = readPid;
	getImpl().pidWriteCmd = writePid;
	getImpl().pidReadIr1 = readIr1;
	getImpl().pidReadIr2 = readIr2;

	char argv[64] = { 0 };
	memcpy(argv, desc.c_str(), desc.size());

	try {
		FT_STATUS ret = FT_Create(argv, FT_OPEN_BY_DESCRIPTION, (FT_HANDLE*)&getImpl().handle);
		if (FT_FAILED(ret)) {
			clear();
		}
		else {
			FT_SetPipeTimeout(getImpl().handle, getImpl().pidReadCmd, getImpl().timeout);
			FT_SetPipeTimeout(getImpl().handle, getImpl().pidWriteCmd, getImpl().timeout);
			FT_SetPipeTimeout(getImpl().handle, getImpl().pidReadIr1, getImpl().timeout);
			FT_SetPipeTimeout(getImpl().handle, getImpl().pidReadIr2, getImpl().timeout);

			/*
			unsigned long streamSize = IR_CAMERA_FRAME_DATA_SIZE;
			FT_SetStreamPipe(getImpl().handle, FALSE, TRUE, getImpl().pidReadIr1, streamSize);
			FT_SetStreamPipe(getImpl().handle, FALSE, TRUE, getImpl().pidReadIr2, streamSize);
			*/

			FT_FlushPipe(getImpl().handle, getImpl().pidReadCmd);
			FT_FlushPipe(getImpl().handle, getImpl().pidWriteCmd);
			FT_FlushPipe(getImpl().handle, getImpl().pidReadIr1);
			FT_FlushPipe(getImpl().handle, getImpl().pidReadIr2);

			// FT_EnableGPIO(getImpl().handle, 0, 0);
			return true;
		}
	}
	catch (...) {
		// throw runtime_error("FT_Cread() error!");
	}
	DebugOut2() << "UsbPort::open() failed!";
	return false;
}


void OctDevice::UsbPort::close(void)
{
	lock_guard<mutex> lock(getImpl().mutexPort);

	if (isOpened()) {
		try {
			FT_FlushPipe(getImpl().handle, getImpl().pidReadCmd);
			FT_FlushPipe(getImpl().handle, getImpl().pidWriteCmd);
			FT_FlushPipe(getImpl().handle, getImpl().pidReadIr1);
			FT_FlushPipe(getImpl().handle, getImpl().pidReadIr2);

			/*
			FT_ClearStreamPipe(getImpl().handle, FALSE, TRUE, getImpl().pidReadIr1);
			FT_ClearStreamPipe(getImpl().handle, FALSE, TRUE, getImpl().pidReadIr2);
			*/

			//FT_AbortPipe(getImpl().handle, getImpl().pidReadCmd);
			//FT_AbortPipe(getImpl().handle, getImpl().pidWriteCmd);
			//FT_AbortPipe(getImpl().handle, getImpl().pidReadIr1);
			//FT_AbortPipe(getImpl().handle, getImpl().pidReadIr2);

			FT_Close(getImpl().handle);
			getImpl().handle = nullptr;
			
			setPipeError(getImpl().pidReadCmd, false);
			setPipeError(getImpl().pidWriteCmd, false);
			setPipeError(getImpl().pidReadIr1, false);
			setPipeError(getImpl().pidReadIr2, false);
		}
		catch (...) {
			throw runtime_error("FT_Close() error!");
		}
	}
	return;
}


void OctDevice::UsbPort::clear(void)
{
	close();
	return;
}


void OctDevice::UsbPort::setPipeError(unsigned char pid, bool flag)
{
	int idx = getIndexOfPipe(pid);
	getImpl().isErrorPipes[idx] = flag;
	return;
}


bool OctDevice::UsbPort::isPipeError(unsigned char pid) const
{
	int idx = getIndexOfPipe(pid);
	return getImpl().isErrorPipes[idx];
}


bool OctDevice::UsbPort::isAvailable(unsigned char pid) const
{
	return (isOpened() && !isPipeError(pid));
}


bool OctDevice::UsbPort::isOpened(void) const
{
	return (getImpl().handle != nullptr);
}


PVOID OctDevice::UsbPort::getHandle(void) const
{
	return getImpl().handle;
}


void OctDevice::UsbPort::setTimeout(unsigned long timeout)
{
	getImpl().timeout = timeout;
	return;
}


void OctDevice::UsbPort::setReadPipeID(unsigned char pid)
{
	getImpl().pidReadCmd = pid;
	return;
}


void OctDevice::UsbPort::setWritePipeID(unsigned char pid)
{
	getImpl().pidWriteCmd = pid;
	return;
}


void OctDevice::UsbPort::abortPipe(unsigned char pid)
{
	if (isOpened()) {
		FT_AbortPipe(getImpl().handle, pid);
	}
	return;
}


bool OctDevice::UsbPort::receive(unsigned char * buff, unsigned long size, bool check)
{
	uint8_t epid = getImpl().pidReadCmd;
	bool ret = receive2(epid, buff, size, check);
	if (!ret) {
		setPipeError(getImpl().pidWriteCmd, true);
		setPipeError(getImpl().pidReadCmd, true);
	}
	return ret;
}


bool OctDevice::UsbPort::receiveFromIr1(unsigned char * buff, unsigned long size, bool check)
{
	uint8_t epid = getImpl().pidReadIr1;
	bool ret = receive(epid, buff, size, check);
	return ret;
}


bool OctDevice::UsbPort::receiveFromIr2(unsigned char * buff, unsigned long size, bool check)
{
	uint8_t epid = getImpl().pidReadIr2;
	bool ret = receive(epid, buff, size, check);
	return ret;
}


bool OctDevice::UsbPort::receive(unsigned char epid, unsigned char * buff, unsigned long size, bool check)
{
	if (!isAvailable(epid)) {
		return false;
	}

	// lock_guard<mutex> lock(getImpl().mutexPort);

	unsigned char* p = buff;
	unsigned long n_read = 0;
	unsigned long n_data = size;

	try
	{
		while (n_data > 0) {
			FT_STATUS ret = FT_ReadPipe(getImpl().handle, epid, p, n_data, &n_read, NULL);
			if (FT_SUCCESS(ret)) {
				if (check == false) {
					return true;
				}
				else {
					if (n_read <= 0) {
						// Timeout??
						// return false ;
						DebugOut2() << "FT_ReadPipe() transferred data with zero length";
						break;
					}
					else {
						n_data -= n_read;
						if (n_data <= 0) {
							return true;
						}
						else {
							p += n_read;
						}
					}
				}
			}
			else {
				// If the timeout (default 5 seconds) occurred, FT_ReadPipe returns with
				// an error code FT_TIMEOUT.
				DebugOut2() << "FT_ReadPipe() returned with error: " << ret;
				break;
			}
		}
	}
	catch (...) {
		throw runtime_error("FT_ReadPipe() error!");
	}
	DebugOut2() << "UsbPort::receive() failed!, epid=%x" << epid;

	FT_AbortPipe(getImpl().handle, epid);
	setPipeError(epid, true);
	return false;
}


bool OctDevice::UsbPort::receive2(unsigned char epid, unsigned char * buff, unsigned long size, bool check)
{
	if (!isAvailable(epid)) {
		return false;
	}

	// lock_guard<mutex> lock(getImpl().mutexPort);

	unsigned char* p = buff;
	unsigned long n_read = 0;
	unsigned long n_data = size;

	OVERLAPPED overlapped = { 0 };
	FT_STATUS ftStatus = FT_InitializeOverlapped(getImpl().handle, &overlapped);

	try
	{
		while (n_data > 0) {
			ftStatus = FT_ReadPipe(getImpl().handle, epid, p, n_data, &n_read, &overlapped);

			if (FT_IO_PENDING == ftStatus) {
				ftStatus = FT_GetOverlappedResult(getImpl().handle, &overlapped, &n_read, TRUE);
				
				if (FT_SUCCESS(ftStatus)) {
					if (check == false) {
						return true;
					}
					else {
						if (n_read <= 0) {
							// Timeout??
							// return false ;
							DebugOut2() << "FT_GetOverlappedResult() transferred data with zero length";
							break;
						}
						else {
							n_data -= n_read;
							if (n_data <= 0) {
								return true;
							}
							else {
								p += n_read;
							}
						}
					}
				}
				else {
					DebugOut2() << "FT_GetOverlappedResult() returned with error: " << ftStatus;
					break;
				}
			}
			else {
				// If the timeout (default 5 seconds) occurred, FT_ReadPipe returns with
				// an error code FT_TIMEOUT.
				DebugOut2() << "FT_ReadPipe() returned with error: " << ftStatus;
				break;
			}
		}
	}
	catch (...) {
		throw runtime_error("FT_ReadPipe() error!");
	}

	FT_ReleaseOverlapped(getImpl().handle, &overlapped);

	DebugOut2() << "UsbPort::receive2() failed!, epid=%x" << epid;

	FT_AbortPipe(getImpl().handle, epid);
	setPipeError(epid, true);
	return false;
}


bool OctDevice::UsbPort::send(unsigned char * buff, unsigned long size, bool check)
{
	unsigned char epid = getImpl().pidWriteCmd;
	if (!isAvailable(epid)) {
		return false;
	}

	// lock_guard<mutex> lock(getImpl().mutexPort);

	unsigned char* p = buff;
	unsigned long n_sent = 0;
	unsigned long n_data = size;

	/*
	std::string str;
	for (int i = 0; i < n_data; i++) {
		str += to_string(p[i]);
		str += ":";
	}
	DebugOut2() << "Send: '" << str << "', size: " << n_data;
	*/

	try
	{
		while (n_data > 0) {
			FT_STATUS ret = FT_WritePipe(getImpl().handle, epid, p, n_data, &n_sent, NULL);
			if (FT_SUCCESS(ret)) {
				if (check == false) {
					return true;
				}
				else {
					if (n_sent <= 0) {
						// Timeout??
						// return false;
						DebugOut2() << "FT_WritePipe() transferred data with zero length";
						break;
					}
					else {
						n_data -= n_sent;
						if (n_data <= 0) {
							return true;
						}
						else {
							p += n_sent;
						}
					}
				}
			}
			else {
				DebugOut2() << "FT_WritePipe() returned with error: " << ret;
				break;
			}
		}
	}
	catch (...) {
		throw runtime_error("FT_WritePipe() error!");
	}
	DebugOut2() << "UsbPort::send() failed!, epid=%x" << epid;

	FT_AbortPipe(getImpl().handle, epid);
	setPipeError(epid, true);
	return false;
}
