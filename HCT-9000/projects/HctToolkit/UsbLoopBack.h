#pragma once

#include <functional>
#include <string>

#include "OctSystem2.h"

using namespace OctSystem;

typedef std::function<void(unsigned int, int, int, int, bool)> UsbLoopBackDownloadCallback;
typedef std::function<void(bool)> UsbLoopBackCompleteCallback;


class UsbLoopBack
{
public:
	UsbLoopBack();
	virtual ~UsbLoopBack();

public:
	static bool initialize(OctDevice::MainBoard * board);
	static bool isInitialized(void);

	static bool start(int kbSize, int repeats, UsbLoopBackDownloadCallback* clbDownload, UsbLoopBackCompleteCallback* clbComplete);
	static void cancel(bool wait);
	static bool isRunning(void);
	static bool isCancelling(void);

	static int getBufferSize(void);
	static std::uint8_t* getBufferWrite(void);
	static std::uint8_t* getBufferRead(void);
	static std::wstring getBufferContext(void);

protected:
	static void resetStatus(void);
	static void threadFunction(void);

	static void generateRandomData(int size);
	static bool downloadBufferData(int block);
	static bool checkBufferMismatch(void);
	static bool countDownloadBlock(void);

	static void callbackUsbLoopBackDownload(bool isError);
	static void callbackUsbLoopBackComplete(bool success);

private:
	struct UsbLoopBackImpl;
	static std::unique_ptr<UsbLoopBackImpl> d_ptr;
	static UsbLoopBackImpl& getImpl(void);

	static OctDevice::MainBoard* getMainboard(void);

};

