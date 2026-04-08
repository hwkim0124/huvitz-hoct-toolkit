#include "stdafx.h"
#include "UsbLoopBack.h"

#include <mutex>
#include <atomic>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>

#include "OctDevice2.h"
#include "CppUtil2.h"

using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct UsbLoopBack::UsbLoopBackImpl
{
	MainBoard* board;
	bool initiated;

	int dataBytes;
	int downBytes;
	int totalBlocks;
	int downBlock;
	int repeats;
	int present;

	unsigned int downAddr;

	std::uint8_t bufferWrite[256];
	std::uint8_t bufferRead[256];
	const int bufferSize = 256;
	int readSize = 0;
	int writeSize = 0;

	atomic<bool> running;
	atomic<bool> stopping;
	thread threadWork;

	UsbLoopBackDownloadCallback* clbDownload;
	UsbLoopBackCompleteCallback* clbComplete;

	UsbLoopBackImpl() : initiated(false), board(nullptr), clbDownload(nullptr), clbComplete(nullptr)
	{
	}
};

// Direct initialization of static smart pointer.
std::unique_ptr<UsbLoopBack::UsbLoopBackImpl> UsbLoopBack::d_ptr(new UsbLoopBackImpl());



UsbLoopBack::UsbLoopBack()
{
}


UsbLoopBack::~UsbLoopBack()
{
}


bool UsbLoopBack::initialize(OctDevice::MainBoard * board)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	resetStatus();
	return true;
}


bool UsbLoopBack::isInitialized(void)
{
	return d_ptr->initiated;
}


bool UsbLoopBack::start(int kbSize, int repeats, UsbLoopBackDownloadCallback* clbDownload, UsbLoopBackCompleteCallback* clbComplete)
{
	if (!isInitialized()) {
		return false;
	}

	cancel(true);
	resetStatus();

	d_ptr->dataBytes = kbSize * 1024;
	d_ptr->totalBlocks = d_ptr->dataBytes / d_ptr->bufferSize;
	d_ptr->downBlock = 0;
	d_ptr->downBytes = 0;

	d_ptr->repeats = repeats;
	d_ptr->present = 1;
	d_ptr->readSize = 0;
	d_ptr->writeSize = 0;

	d_ptr->clbDownload = clbDownload;
	d_ptr->clbComplete = clbComplete;

	// Thread object should be joined before being reused. 
	if (getImpl().threadWork.joinable()) {
		getImpl().threadWork.join();
	}
	getImpl().threadWork = thread{ &UsbLoopBack::threadFunction };
	return true;
}


void UsbLoopBack::cancel(bool wait)
{
	if (isRunning() && !isCancelling()) {
		d_ptr->stopping = true;

		if (wait) {
			// Thread object should be joined before being reused. 
			if (getImpl().threadWork.joinable()) {
				getImpl().threadWork.join();
			}
		}
	}
	return;
}


bool UsbLoopBack::isRunning(void)
{
	return d_ptr->running;
}


bool UsbLoopBack::isCancelling(void)
{
	return d_ptr->stopping;
}


int UsbLoopBack::getBufferSize(void)
{
	return d_ptr->bufferSize;
}


std::uint8_t * UsbLoopBack::getBufferWrite(void)
{
	return d_ptr->bufferWrite;
}


std::uint8_t * UsbLoopBack::getBufferRead(void)
{
	return d_ptr->bufferRead;
}


std::wstring UsbLoopBack::getBufferContext(void)
{
	std::wstringstream wss;
	std::ios_base::fmtflags flags = wss.flags();

	wss << L"Written Bytes : " << d_ptr->writeSize << L" ===> \r\n";
	wss << std::setfill(L'0') << std::uppercase;
	for (int i = 0; i < d_ptr->writeSize; i++) {
		wss << std::hex << std::setw(2) << d_ptr->bufferWrite[i];
		wss << (((i + 1) % 16) == 0 ? L"\r\n" : L" ");
	}

	wss.flags(flags);
	wss << L"Read Bytes : " << d_ptr->readSize << L" ===> \r\n";
	wss << std::setfill(L'0') << std::uppercase;
	for (int i = 0; i < d_ptr->readSize; i++) {
		wss << std::hex << std::setw(2) << d_ptr->bufferRead[i];
		wss << (((i + 1) % 16) == 0 ? L"\r\n" : L" ");
	}
	return wss.str();
}


void UsbLoopBack::resetStatus(void)
{
	d_ptr->running = false;
	d_ptr->stopping = false;
	return;
}


void UsbLoopBack::threadFunction(void)
{
	d_ptr->running = true;

	while (true)
	{
		if (!countDownloadBlock()) {
			break;
		}
		
		if (!downloadBufferData(d_ptr->downBlock)) {
			break;
		}

		if (isCancelling()) {
			callbackUsbLoopBackComplete(false);
			break;
		}
	}

	d_ptr->running = false;
	return;
}


void UsbLoopBack::generateRandomData(int size)
{
	random_device device;
	mt19937 engine(device());
	uniform_int_distribution<int> dist(0, 255);
	auto gen = std::bind(dist, engine);

	for (int i = 0; i < size; i++) {
		d_ptr->bufferWrite[i] = gen();
		d_ptr->bufferRead[i] = 0;
	}
	return;
}


bool UsbLoopBack::downloadBufferData(int block)
{
	UsbComm& usbComm = getMainboard()->getUsbComm();
	std::uint32_t addr = getMainboard()->getBaseAddressOfDownload();
	addr += (block - 1) * d_ptr->bufferSize;

	d_ptr->downAddr = addr;
	d_ptr->writeSize = d_ptr->bufferSize;
	d_ptr->readSize = 0;
	
	generateRandomData(d_ptr->bufferSize);
	if (usbComm.writeAddress(addr, d_ptr->bufferWrite, d_ptr->bufferSize)) {
		if (usbComm.readAddress(addr, d_ptr->bufferRead, d_ptr->bufferSize)) {
			d_ptr->readSize = d_ptr->bufferSize;
			if (!checkBufferMismatch()) {
				callbackUsbLoopBackDownload(false);
				return true;
			}
		}
	}
	callbackUsbLoopBackDownload(true);
	return false;
}


bool UsbLoopBack::checkBufferMismatch(void)
{
	for (int i = 0; i < d_ptr->bufferSize; i++) {
		if (d_ptr->bufferWrite[i] != d_ptr->bufferRead[i]) {
			return true;
		}
	}
	return false;
}


bool UsbLoopBack::countDownloadBlock(void)
{
	if (d_ptr->downBlock >= d_ptr->totalBlocks) {
		d_ptr->present += 1;
		d_ptr->downBlock = 0;
		if (d_ptr->present >= d_ptr->repeats) {
			callbackUsbLoopBackComplete(true);
			return false;
		}
	}
	else {
		d_ptr->downBlock += 1;
	}
	return true;
}


void UsbLoopBack::callbackUsbLoopBackDownload(bool isError)
{
	if (d_ptr->clbDownload) {
		(*d_ptr->clbDownload)(d_ptr->downAddr, d_ptr->present, d_ptr->downBlock, d_ptr->totalBlocks, isError);
	}
	return;
}


void UsbLoopBack::callbackUsbLoopBackComplete(bool success)
{
	if (d_ptr->clbComplete) {
		(*d_ptr->clbComplete)(success);
	}
	return;
}


UsbLoopBack::UsbLoopBackImpl & UsbLoopBack::getImpl(void)
{
	return *d_ptr;
}


OctDevice::MainBoard * UsbLoopBack::getMainboard(void)
{
	return d_ptr->board;
}
