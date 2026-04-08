#include "stdafx.h"
#include "ScanPost.h"
#include "Measure.h"

#include <mutex>
#include <atomic>
#include <functional>

#include "OctPattern2.h"
#include "OctDevice2.h"
#include "OctGrab2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

using namespace OctSystem;
using namespace OctPattern;
using namespace OctDevice;
using namespace OctGrab;
using namespace SigChain;
using namespace std;
using namespace CppUtil;


struct ScanPost::ScanPostImpl
{
	MainBoard* board;
	OctProcess* process;

	bool initiated;
	bool measureDone;

	mutex mutexPost;
	atomic<bool> posterRun;
	atomic<bool> postComplete;
	thread threadPost;

	ScanPostImpl() : initiated(false), posterRun(false), postComplete(false), process(nullptr) {
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanPost::ScanPostImpl> ScanPost::d_ptr(new ScanPostImpl());


ScanPost::ScanPost()
{
}


ScanPost::~ScanPost()
{
}


void OctSystem::ScanPost::initiateScanPost(OctDevice::MainBoard* board, SigChain::OctProcess* process)
{
	getImpl().board = board;
	getImpl().process = process;
	getImpl().initiated = true;
	getImpl().measureDone = false;
	return;
}


bool OctSystem::ScanPost::postMeasure(bool result)
{
	getImpl().measureDone = result;
	return startScanPost();
}


bool OctSystem::ScanPost::isInitiated(void)
{
	return getImpl().initiated;
}


bool OctSystem::ScanPost::startScanPost(void)
{
	if (!isInitiated()) {
		return false;
	}

	closeScanPost();

	if (getImpl().threadPost.joinable()) {
		getImpl().threadPost.join();
	}

	getImpl().threadPost = thread{ &ScanPost::threadPostFunction };
	return true;
}


bool OctSystem::ScanPost::closeScanPost(void)
{
	if (isPosting()) {
		getImpl().posterRun = false;

		if (getImpl().threadPost.joinable()) {
			getImpl().threadPost.join();
		}
	}
	return true;
}


bool OctSystem::ScanPost::isPosting(void)
{
	return getImpl().posterRun;
}


bool OctSystem::ScanPost::isProcessingCompleted(void)
{
	return getImpl().postComplete;
}


void OctSystem::ScanPost::threadPostFunction(void)
{
	getImpl().posterRun = true;
	getImpl().postComplete = false;
	bool init = true;

	CppUtil::ClockTimer::start();

	while (true)
	{
		if (isPosting()) {
			if (!waitForMeasureProcessed(init)) {
				break;
			}
			else {
				init = false;
				if (checkIfMeasureProcessed()) {
					getImpl().postComplete = true;
					break;
				}
			}
		}
		else {
			break;
		}
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Measure pattern processed, completed: " << getImpl().postComplete << ", time: " << msec;

	getImpl().posterRun = false;
	bool result = completeMeasureResult();

	GlobalRegister::runScanPatternCompletedCallback(result);
	return;
}


bool OctSystem::ScanPost::waitForMeasureProcessed(bool init)
{
	static int waitCnt = 0;

	if (init) {
		waitCnt = 0;
	}

	if (waitCnt < SCANNER_WAIT_MEASURE_PROCESSED_COUNT_MAX) {
		this_thread::sleep_for(chrono::milliseconds(SCANNER_WAIT_MEASURE_PROCESSED_DELAY));
		return true;
	}
	LogD() << "Measure pattern processing timeout";
	return false;
}


bool OctSystem::ScanPost::checkIfMeasureProcessed(void)
{
	if (!getProcess()->isBusying()) {
		this_thread::sleep_for(chrono::milliseconds(SCANNER_WAIT_MEASURE_PROCESSED_DELAY*3));
		return true;
	}
	return false;
}


bool OctSystem::ScanPost::completeMeasureResult(void)
{
	bool result = getImpl().measureDone && isProcessingCompleted();

	if (result) {
		Measure::completeScannerResult();
	}
	
	LogD() << "Measure pattern completed, result: " << result;
	return result;
}


SigChain::OctProcess * OctSystem::ScanPost::getProcess(void)
{
	return getImpl().process;
}


ScanPost::ScanPostImpl & OctSystem::ScanPost::getImpl(void)
{
	return *d_ptr;
}
