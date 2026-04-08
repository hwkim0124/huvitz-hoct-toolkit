#include "stdafx.h"
#include "SplitAutoFocus.h"

#include <mutex>
#include <atomic>

#include "OctDevice2.h"
#include "CppUtil2.h"

using namespace RetFocus;
using namespace OctDevice;
using namespace CppUtil;
using namespace std;


struct SplitAutoFocus::SplitAutoFocusImpl
{
	MainBoard* board;
	bool initiated;
	bool aligning;

	float offset;
	float offset_last;
	float diopter;
	float dioptStep;
	float dioptInit;

	int emptyErrorCnt;
	int closeErrorCnt;
	int focusCnt;
	int alignDelayCnt;

	atomic<bool> running;
	atomic<bool> stopping;
	atomic<bool> completed;
	thread threadWork;

	SplitAutoFocusImpl() : initiated(false), board(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<SplitAutoFocus::SplitAutoFocusImpl> SplitAutoFocus::d_ptr(new SplitAutoFocusImpl());


SplitAutoFocus::SplitAutoFocus()
{
}


SplitAutoFocus::~SplitAutoFocus()
{
}


bool RetFocus::SplitAutoFocus::initialize(OctDevice::MainBoard * board)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	return true;
}


bool RetFocus::SplitAutoFocus::isInitialized(void)
{
	return d_ptr->initiated;
}


bool RetFocus::SplitAutoFocus::start(void)
{
	if (!isInitialized()) {
		return false;
	}

	cancel(true);
	resetSplitStatus();

	// Thread object should be joined before being reused. 
	if (getImpl().threadWork.joinable()) {
		getImpl().threadWork.join();
	}
	getImpl().threadWork = thread{ &SplitAutoFocus::threadFunction };
	return true;
}


void RetFocus::SplitAutoFocus::cancel(bool wait)
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


bool RetFocus::SplitAutoFocus::isRunning(void)
{
	return d_ptr->running;
}


bool RetFocus::SplitAutoFocus::isCancelling(void)
{
	return d_ptr->stopping;
}


bool RetFocus::SplitAutoFocus::isCompleted(void)
{
	return d_ptr->completed;
}


void RetFocus::SplitAutoFocus::resetSplitStatus(void)
{
	d_ptr->running = false;
	d_ptr->stopping = false;
	d_ptr->completed = false;

	d_ptr->offset = AUTO_SPLIT_OFFSET_INVALID;
	d_ptr->offset_last = AUTO_SPLIT_OFFSET_INVALID;

	d_ptr->emptyErrorCnt = 0;
	d_ptr->closeErrorCnt = 0;
	d_ptr->focusCnt = 0;

	d_ptr->aligning = false;
	d_ptr->alignDelayCnt = 0;

	FundusFocusMotor* motor = getMainBoard()->getFundusFocusMotor();
	d_ptr->dioptInit = motor->getCurrentDiopter();
	return;
}


void RetFocus::SplitAutoFocus::threadFunction(void)
{
	d_ptr->running = true;

	while (true)
	{
		if (!processAlignment()) {
			break;
		}
		if (isCompleted()) {
			LogD() << "Auto Split: completed";
			break;
		}
		if (isCancelling()) {
			LogD() << "Auto Split: cancelled";
			break;
		}
	}

	d_ptr->running = false;
	return;
}


bool RetFocus::SplitAutoFocus::processAlignment(void)
{
	bool resetToInit = false;

	if (++d_ptr->focusCnt > AUTO_SPLIT_FOCUS_TIMES_MAX) {
		LogD() << "Auto Split: focus timeout";
		goto exit_proc;
	}

	if (!renewSplitOffset()) {
		d_ptr->emptyErrorCnt++;
		if (d_ptr->emptyErrorCnt > AUTO_SPLIT_FOCUS_EMPTY_ERROR_MAX) {
			LogD() << "Auto Split: offset not obtained";
			goto exit_proc;
		}
		return true;
	}
	else {
		d_ptr->emptyErrorCnt = 0;
	}

	updateDiopter();
	updateFocusStep();
	LogD() << "Auto Split: offset: " << d_ptr->offset << ", diopt: " << d_ptr->diopter << ", dstep: " << d_ptr->dioptStep;

	if (!isSplitFocusOnAlignment())
	{
		if (!isSplitFocusBeingClose()) {
			d_ptr->closeErrorCnt++;
			if (d_ptr->closeErrorCnt > AUTO_SPLIT_FOCUS_CLOSE_ERROR_MAX) {
				LogD() << "Auto Split: distance error, split is not closing";
				resetToInit = true;
				goto exit_proc;
			}
		}
		else {
			d_ptr->closeErrorCnt = 0;
			d_ptr->offset_last = AUTO_SPLIT_OFFSET_INVALID;
		}

		if (isSplitFocusOnTarget()) {
			LogD() << "Auto Split: focus on target";
			d_ptr->aligning = true;
		}
		else {
			if (!moveFocusMotor()) {
				LogD() << "Auto Split: motor error";
				resetToInit = true;
				goto exit_proc;
			}
		}
	}
	else {
		++d_ptr->alignDelayCnt;
		if (d_ptr->alignDelayCnt < AUTO_SPLIT_FOCUS_DELAY_TO_ALIGN) {
			LogD() << "Auto Split: align count: " << d_ptr->alignDelayCnt;
		}
		else if (d_ptr->alignDelayCnt == AUTO_SPLIT_FOCUS_DELAY_TO_ALIGN) {
			if (!moveFocusMotor()) {
				LogD() << "Auto Split: motor error";
				resetToInit = true;
				goto exit_proc;
			}
		}
		else {
			d_ptr->completed = true;
		}
	}
	return true;

exit_proc:
	if (resetToInit) {
		if (fabs(d_ptr->diopter) > AUTO_SPLIT_DIOPT_DISTANT) {
			moveFocusMotorToInit();
		}
	}
	return false;
}


bool RetFocus::SplitAutoFocus::renewSplitOffset(void)
{
	float offset;
	if (!obtainSplitFocusFromRetina(offset)) {
		return false;
	}
	if (d_ptr->offset_last == AUTO_SPLIT_OFFSET_INVALID) {
		d_ptr->offset_last = d_ptr->offset;
	}
	d_ptr->offset = offset;
	return true;
}


bool RetFocus::SplitAutoFocus::isSplitFocusBeingClose(void)
{
	if ((fabs(d_ptr->offset_last) - fabs(d_ptr->offset)) > AUTO_SPLIT_OFFSET_CLOSED) {
		return true;
	}
	return false;
}


bool RetFocus::SplitAutoFocus::isSplitFocusOnTarget(void)
{
	if (fabs(d_ptr->diopter) <= AUTO_SPLIT_DIOPT_RANGE_TO_TARGET) {
		return true;
	}
	return false;
}


bool RetFocus::SplitAutoFocus::isSplitFocusOnAlignment(void)
{
	return d_ptr->aligning;
}


void RetFocus::SplitAutoFocus::updateDiopter(void)
{
	float diopt = d_ptr->offset / AUTO_SPLIT_OFFSET_PER_DIOPT;
	d_ptr->diopter = diopt;
	return;
}


void RetFocus::SplitAutoFocus::updateFocusStep(void)
{
	float dstep = AUTO_SPLIT_MOVE_STEP_IN_DIOPT;
	float dsize = fabs(d_ptr->diopter);

	if (dsize < 0.5f) {
		dstep = 0.25f;
	}
	else if (dsize < 1.5f) {
		dstep = 0.50f;
	}
	else if (dsize < 3.0f) {
		dstep = 0.75f;
	}
	else if (dsize < 5.0f) {
		dstep = 1.25f;
	}
	else {
		dstep = 2.5f;
	}

	// d_ptr->dioptStep = dstep * (d_ptr->diopter < 0.0f ? -1.0f : +1.0f);
	d_ptr->dioptStep = d_ptr->diopter;
	return;
}


bool RetFocus::SplitAutoFocus::moveFocusMotor(void)
{
	FundusFocusMotor* motor = getMainBoard()->getFundusFocusMotor();
	float dstep = d_ptr->dioptStep;

	if (dstep < 0.0f && motor->isEndOfMinusDiopter()) {
		LogD() << "Auto Split: end of minus diopt";
		return false;
	}
	else if (dstep > 0.0f && motor->isEndOfPlusDiopter()) {
		LogD() << "Auto Split: end of plus diopt";
		return false;
	}
	else {
		LogD() << "Auto Split: move focus motor, dstep: " << dstep;
		if (!motor->updateDiopterByOffset(dstep)) {
			return false;
		}
	}
	return true;
}


bool RetFocus::SplitAutoFocus::moveFocusMotorToInit(void)
{
	FundusFocusMotor* motor = getMainBoard()->getFundusFocusMotor();
	float diopt = d_ptr->dioptInit;

	if (fabs(diopt) > 10.0f) {
		diopt = 0.0f;
	}

	if (!motor->updateDiopter(diopt)) {
		return false;
	}
	return true;
}


SplitAutoFocus::SplitAutoFocusImpl & RetFocus::SplitAutoFocus::getImpl(void)
{
	return *d_ptr;
}


OctDevice::MainBoard * RetFocus::SplitAutoFocus::getMainBoard(void)
{
	return d_ptr->board;
}
