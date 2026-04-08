#include "stdafx.h"
#include "ScanOptimizer.h"
#include "Scanner.h"

#include "OctDevice2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

#include <mutex>
#include <atomic>

using namespace OctSystem;
using namespace OctDevice;
using namespace SigChain;
using namespace std;


struct ScanOptimizer::ScanOptimizerImpl
{
	bool initiated;
	MainBoard* mainboard;
	Scanner* scanner;

	float afQuality;
	float afDiopter;
	float afDioptStep;
	int afDirection;

	int afRetryCnt;
	int afForwardCnt;
	int afRetraceCnt;

	float apQuality;
	float apDegree;
	float apMoveStep;
	int apDirection;

	int apRetryCnt;
	int apForwardCnt;
	int apRetraceCnt;

	float arQuality;
	int arRefPoint;
	int arPosition;
	int arMoveStep;
	int arDirection;

	int arRetryCnt;
	int arForwardCnt;
	int arRetraceCnt;


	AutoFocusCompleteCallback* cbAutoFocus;
	AutoPolarizationCompleteCallback* cbAutoPolar;
	AutoReferenceCompleteCallback* cbAutoRefer;

	AutoFocusPhase afPhase;
	AutoReferPhase arPhase;
	AutoPolarPhase apPhase;

	atomic<bool> stopping;
	atomic<bool> running;
	atomic<bool> completed;
	condition_variable condVarWork;
	thread threadWork;
	mutex mutexWork;

	ScanOptimizerImpl() : initiated(false), mainboard(nullptr), scanner(nullptr),
		running(false), stopping(false), completed(false), 
		cbAutoFocus(nullptr), cbAutoPolar(nullptr), cbAutoRefer(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanOptimizer::ScanOptimizerImpl> ScanOptimizer::d_ptr(new ScanOptimizerImpl());


ScanOptimizer::ScanOptimizer()
{
}


ScanOptimizer::~ScanOptimizer()
{
}


bool OctSystem::ScanOptimizer::initialize(OctDevice::MainBoard * board, Scanner* scanner)
{
	getImpl().mainboard = board;
	getImpl().scanner = scanner;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::ScanOptimizer::isInitiated(void)
{
	return getImpl().initiated;
}


bool OctSystem::ScanOptimizer::startAutoOptimize(void)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}
	cancel();

	getImpl().afPhase = AutoFocusPhase::INIT;
	getImpl().apPhase = AutoPolarPhase::INIT;
	getImpl().arPhase = AutoReferPhase::INIT;
	getImpl().completed = false;
	getImpl().threadWork = thread{ &ScanOptimizer::threadAutoOptimizeFunction };
	return true;
}


bool OctSystem::ScanOptimizer::startAutoPosition(void)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}
	cancel();

	getImpl().arPhase = AutoReferPhase::INIT;
	getImpl().completed = false;
	getImpl().threadWork = thread{ &ScanOptimizer::threadAutoPositionFunction };
	return true;
}


bool OctSystem::ScanOptimizer::startAutoFocus(AutoFocusCompleteCallback* callback)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}

	// if (isOptimizing()) {
		cancel();
	// }

	getImpl().afPhase = AutoFocusPhase::INIT;
	getImpl().cbAutoFocus = callback;
	getImpl().completed = false;
	getImpl().threadWork = thread{ &ScanOptimizer::threadAutoFocusFunction };
	return true;
}


bool OctSystem::ScanOptimizer::startAutoPolarization(AutoPolarizationCompleteCallback* callback)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}
	cancel();

	getImpl().apPhase = AutoPolarPhase::INIT;
	getImpl().cbAutoPolar = callback;
	getImpl().completed = false;
	getImpl().threadWork = thread{ &ScanOptimizer::threadAutoPolarFunction };
	return true;
}


bool OctSystem::ScanOptimizer::startAutoReference(AutoReferenceCompleteCallback* callback)
{
	if (!isInitiated()) {
		return false;
	}
	if (!getScanner()->isPreviewing()) {
		return false;
	}
	cancel();

	getImpl().arPhase = AutoReferPhase::INIT;
	getImpl().cbAutoRefer = callback;
	getImpl().completed = false;
	getImpl().threadWork = thread{ &ScanOptimizer::threadAutoReferFunction };
	return true;
}


bool OctSystem::ScanOptimizer::isOptimizing(void)
{
	return getImpl().running ;
}


bool OctSystem::ScanOptimizer::isCancelling(void)
{
	return getImpl().stopping;
}


bool OctSystem::ScanOptimizer::isCompleted(void)
{
	return getImpl().completed;
}


void OctSystem::ScanOptimizer::cancel(void)
{
	/*
	if (!isOptimizing()) {
		return;
	}
	*/

	getImpl().stopping = true;

	/*
	// Caution:
	// If this routine owning the lock falls into waiting status by calling join, 
	// the worker thread can't wake up until the lock released. 
	// However, the lock should be held to be the waiting status of the worker thread 
	// before notification, RAII fashion within a bracket scope could resolve this problem.
	{
		unique_lock<mutex> lock(getImpl().mutexWork);
		getImpl().condVarWork.notify_all();
	}
	*/

	// Thread object should be joined before being reused. 
	if (getImpl().threadWork.joinable()) {
		getImpl().threadWork.join();
	}

	getImpl().stopping = false;
	return;
}


ScanOptimizer::ScanOptimizerImpl & OctSystem::ScanOptimizer::getImpl(void)
{
	return *d_ptr;
}


OctSystem::Scanner * OctSystem::ScanOptimizer::getScanner(void)
{
	return getImpl().scanner;
}


OctDevice::MainBoard * OctSystem::ScanOptimizer::getMainBoard(void)
{
	return getImpl().mainboard;
}


void OctSystem::ScanOptimizer::threadAutoOptimizeFunction(void)
{
	/*
	threadAutoFocusFunction();
	if (!isCancelling()) {
		threadAutoPolarFunction();
		if (!isCancelling()) {
			threadAutoReferFunction();
		}
	}
	*/

	threadAutoReferFunction();
	bool result = isCompleted();

	if (!isCancelling()) {
		threadAutoFocusFunction();
		if (!isCancelling()) {
			threadAutoPolarFunction();
			if (!isCancelling()) {
				if (!result) {
					getImpl().arPhase = AutoReferPhase::CENTER;
					threadAutoReferFunction();
				}
			}
		}
	}

	result = isCompleted();
	GlobalRegister::runAutoOptimizeCompletedCallback(result);
	return;
}


void OctSystem::ScanOptimizer::threadAutoPositionFunction(void)
{
	threadAutoReferFunction();
	
	bool result = isCompleted();
	GlobalRegister::runAutoPositionCompletedCallback(result);
	return;
}


void OctSystem::ScanOptimizer::threadAutoFocusFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoFocusProcess()) {
			DebugOut2() << "Auto focus cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto focus completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto focus cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


void OctSystem::ScanOptimizer::threadAutoPolarFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoPolarProcess()) {
			DebugOut2() << "Auto polarizaton cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto polarizaton completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto polarizaton cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


void OctSystem::ScanOptimizer::threadAutoReferFunction(void)
{
	getImpl().running = true;
	getImpl().completed = false;

	while (true)
	{
		if (!phaseAutoReferProcess()) {
			DebugOut2() << "Auto reference cancelled with error!";
			break;
		}
		if (isCompleted()) {
			DebugOut2() << "Auto reference completed!";
			break;
		}
		if (isCancelling()) {
			DebugOut2() << "Auto reference cancelled!";
			break;
		}
	}

	getImpl().running = false;
	return;
}


bool OctSystem::ScanOptimizer::phaseAutoFocusProcess(void)
{
	bool res = false;

	switch (getImpl().afPhase) {
	case AutoFocusPhase::INIT:
		res = phaseAutoFocus_Initiate();
		break;
	case AutoFocusPhase::NO_SIGNAL:
	case AutoFocusPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoFocus_NoSignal();
		break;
	case AutoFocusPhase::FIRST_STEP:
		res = phaseAutoFocus_FirstStep();
		break;
	case AutoFocusPhase::FORWARD:
		res = phaseAutoFocus_Forward();
		break;
	case AutoFocusPhase::RETRACE:
		res = phaseAutoFocus_Retrace();
		break;
	}

	if (res) {
		if (getImpl().afPhase == AutoFocusPhase::COMPLETE) {
			phaseAutoFocus_Complete();
		}
		if (getImpl().afPhase == AutoFocusPhase::CANCELED) {
			phaseAutoFocus_Canceled();
		}
		return true;
	}
	else {
		phaseAutoFocus_Canceled();
		return false;
	}
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_Initiate(void)
{
	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetCompleteOfFocus()) {
		getImpl().afPhase = AutoFocusPhase::COMPLETE;
	}
	else if (isTargetFoundOfFocus()) {
		getImpl().afPhase = AutoFocusPhase::FIRST_STEP;
	}
	else {
		getImpl().afPhase = AutoFocusPhase::NO_SIGNAL;
	}

	getImpl().afDirection = AUTO_FOCUS_MINUS_DIRECTION; // (isAtPlusSideOfFocusDiopter() ? AUTO_FOCUS_MINUS_DIRECTION : AUTO_FOCUS_PLUS_DIRECTION);

	updateFocusMoveStep();
	updateFocusDiopter();
	resetRetryCountToFocus();
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_NoSignal(void)
{
	if (!moveMotorPositionOfFocus()) {
		return false;
	}

	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		updateFocusMoveStep();
		updateFocusDiopter();

		getImpl().afPhase = AutoFocusPhase::FORWARD;
		getImpl().afForwardCnt = 0;
	}
	else
	{
		// Reach an end of diopter range in the moving direction. 
		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter())
		{
			if (getImpl().afPhase == AutoFocusPhase::NO_SIGNAL_REVERSE) {
				getImpl().afPhase = AutoFocusPhase::CANCELED;
			}
			else {
				getImpl().afPhase = AutoFocusPhase::NO_SIGNAL_REVERSE;
				getImpl().afDirection *= -1;
			}

			// Return to the starting position. 
			if (returnToFocusDiopter()) {
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_FirstStep(void)
{
	// Move forward to strengthen the signal.
	if (!moveMotorPositionOfFocus()) {
		return false;
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}
	float curr = getImpl().afQuality;

	// If quality got worse, reverse the moving direction. 
	if (curr < prev || !isTargetFoundOfFocus())
	{
		getImpl().afDirection *= -1;
		// Return to the starting position. 
		if (!returnToFocusDiopter()) {
			return false;
		}
	}
	else if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter())
	{
		getImpl().afDirection *= -1;
		if (!returnToFocusDiopter()) {
			return false;
		}
	}

	getImpl().afPhase = AutoFocusPhase::FORWARD;
	getImpl().afForwardCnt = 0;

	updateFocusMoveStep();
	updateFocusDiopter();
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_Forward(void)
{
	if (!isRetryCountToFocus()) {
		if (!moveMotorPositionOfFocus()) {
			return false;
		}

		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter()) {
			getImpl().afPhase = AutoFocusPhase::COMPLETE;
		}
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().afForwardCnt++ > 0) 
		{
			float curr = getImpl().afQuality;
			if (curr >= prev)
			{
			}
			else {
				// Keep moving foward until quality getting worse enough to retrace.
				if (fabs(curr - prev) >= AUTO_FOCUS_QINDEX_DIFF_TO_RETRACE) {
					// Return to the currently best position.
					if (!returnToFocusDiopter()) {
						return false;
					}

					// Retrace with the minimum step.
					getImpl().afPhase = AutoFocusPhase::RETRACE;
					getImpl().afDirection *= -1;
					getImpl().afRetraceCnt = 0;
				}
			}
		}
		updateFocusMoveStep();
		updateFocusDiopter();
		resetRetryCountToFocus();
	}
	else
	{
		getImpl().arForwardCnt = 0;
		if (checkRetryOverToReference()) {
			getImpl().afPhase = AutoFocusPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_Retrace(void)
{
	if (!isRetryCountToFocus()) {
		if (!moveMotorPositionOfFocus()) {
			return false;
		}

		if (isAtPlusEndOfFocusDiopter() || isAtMinusEndOfFocusDiopter()) {
			getImpl().afPhase = AutoFocusPhase::COMPLETE;
		}
	}

	float prev = getImpl().afQuality;
	if (!renewFocusQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().afRetraceCnt++ > 0)
		{
			float curr = getImpl().afQuality;
			if (curr >= prev)
			{
				// Keep moving foward until quality getting worse enough to retrace.
			}
			else {
				// Return to the currently best position.
				if (!returnToFocusDiopter()) {
					return false;
				}

				getImpl().afPhase = AutoFocusPhase::COMPLETE;
			}
		}

		updateFocusMoveStep();
		updateFocusDiopter();
		resetRetryCountToFocus();
	}
	else
	{
		getImpl().arRetraceCnt = 0;
		if (checkRetryOverToReference()) {
			getImpl().afPhase = AutoFocusPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_Complete(void)
{
	if (getImpl().cbAutoFocus != nullptr) {
		(*getImpl().cbAutoFocus)(true, getImpl().afQuality, getImpl().afDiopter);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoFocus_Canceled(void)
{
	if (getImpl().cbAutoFocus != nullptr) {
		(*getImpl().cbAutoFocus)(false, getImpl().afQuality, getImpl().afDiopter);
	}

	// Completed by getting nowhere.
	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::renewFocusQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().afQuality = qidx;
	return true;
}


float OctSystem::ScanOptimizer::getMotorStepToFocus(void)
{
	float dstep = AUTO_FOCUS_DIOPT_STEP_MAX;
	if (getImpl().afPhase == AutoFocusPhase::FIRST_STEP) {
		dstep = AUTO_FOCUS_DIOPT_STEP_FIRST;
	}
	else if (getImpl().afPhase == AutoFocusPhase::RETRACE) {
		dstep = AUTO_FOCUS_DIOPT_STEP_MIN;
	}
	else {
		float qidx = getImpl().afQuality;
		for (int i = 0; i < AUTO_FOCUS_DIOPT_STEP_SIZE; i++) {
			if (qidx >= AutoFocusQualitySteps[i]) {
				dstep = AutoFocusDioptSteps[i];
				break;
			}
		}
	}
	return dstep;
}


float OctSystem::ScanOptimizer::getMotorOffsetToFocus(void)
{
	return getImpl().afDioptStep * getImpl().afDirection;
}


bool OctSystem::ScanOptimizer::moveMotorPositionOfFocus(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	float offset = getMotorOffsetToFocus();

	// Move forward/reverse to find a signal. 
	if (!motor->updateDiopterByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::updateFocusMoveStep(void)
{
	getImpl().afDioptStep = getMotorStepToFocus();
	return true;
}


bool OctSystem::ScanOptimizer::updateFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	getImpl().afDiopter = motor->getCurrentDiopter();
	return true;
}


bool OctSystem::ScanOptimizer::returnToFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	if (!motor->updateDiopter(getImpl().afDiopter)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::isHeadingForPlusEndOfFocusDiopter(void)
{
	return (getImpl().afDirection == AUTO_FOCUS_PLUS_DIRECTION);
}


bool OctSystem::ScanOptimizer::isHeadingForMinusEndOfFocusDiopter(void)
{
	return (getImpl().afDirection == AUTO_FOCUS_MINUS_DIRECTION);
}


bool OctSystem::ScanOptimizer::isAtPlusSideOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->getCurrentDiopter() >= 0);
}


bool OctSystem::ScanOptimizer::isAtMinusSideOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->getCurrentDiopter() <= 0);
}


bool OctSystem::ScanOptimizer::isAtPlusEndOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->isEndOfPlusDiopter() && isHeadingForPlusEndOfFocusDiopter());
}


bool OctSystem::ScanOptimizer::isAtMinusEndOfFocusDiopter(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return (motor->isEndOfMinusDiopter() && isHeadingForMinusEndOfFocusDiopter());
}


bool OctSystem::ScanOptimizer::isTargetFoundOfFocus(void)
{
	return isQualityToSignal(getImpl().afQuality);
}


bool OctSystem::ScanOptimizer::isTargetCompleteOfFocus(void)
{
	return isQualityToComplete(getImpl().afQuality);
}


void OctSystem::ScanOptimizer::resetRetryCountToFocus(void)
{
	getImpl().afRetryCnt = 0;
	return;
}


bool OctSystem::ScanOptimizer::checkRetryOverToFocus(void)
{
	getImpl().afRetryCnt++;
	if (getImpl().afRetryCnt > 3) {
		resetRetryCountToFocus();
		return true;
	}
	return false;
}


bool OctSystem::ScanOptimizer::isRetryCountToFocus(void)
{
	return (getImpl().afRetryCnt > 0);
}


bool OctSystem::ScanOptimizer::phaseAutoPolarProcess(void)
{
	bool res = false;

	switch (getImpl().apPhase) {
	case AutoPolarPhase::INIT:
		res = phaseAutoPolar_Initiate();
		break;
	case AutoPolarPhase::NO_SIGNAL:
	case AutoPolarPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoPolar_NoSignal();
		break;
	case AutoPolarPhase::FIRST_STEP:
		res = phaseAutoPolar_FirstStep();
		break;
	case AutoPolarPhase::FORWARD:
		res = phaseAutoPolar_Forward();
		break;
	case AutoPolarPhase::RETRACE:
		res = phaseAutoPolar_Retrace();
		break;
	}

	if (res) {
		if (getImpl().apPhase == AutoPolarPhase::COMPLETE) {
			phaseAutoPolar_Complete();
		}
		if (getImpl().apPhase == AutoPolarPhase::CANCELED) {
			phaseAutoPolar_Canceled();
		}
		return true;
	}
	else {
		phaseAutoPolar_Canceled();
		return false;
	}
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_Initiate(void)
{
	if (!renewPolarQualityFromPreview(false)) {
		return false;
	}

	if (isTargetCompleteOfPolar()) {
		getImpl().apPhase = AutoPolarPhase::COMPLETE;
	}
	else if (isTargetFoundOfPolar()) {
		getImpl().apPhase = AutoPolarPhase::FIRST_STEP;
	}
	else {
		getImpl().apPhase = AutoPolarPhase::NO_SIGNAL;
	}

	getImpl().apDirection = (isAtUpperSideOfPolarDegree() ? AUTO_POLAR_LOWER_DIRECTION : AUTO_POLAR_UPPER_DIRECTION);

	updatePolarMoveStep();
	updatePolarDegree();
	resetRetryCountToPolar();
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_NoSignal(void)
{
	if (!moveMotorPositionOfPolar()) {
		return false;
	}

	if (!renewFocusQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfPolar())
	{
		updatePolarMoveStep();
		updatePolarDegree();

		getImpl().apPhase = AutoPolarPhase::FORWARD;
		getImpl().apForwardCnt = 0;
	}
	else {
		// Reach an end of diopter range in the moving direction. 
		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree())
		{
			if (getImpl().apPhase == AutoPolarPhase::NO_SIGNAL_REVERSE) {
				getImpl().apPhase = AutoPolarPhase::CANCELED;
			}
			else {
				getImpl().apPhase = AutoPolarPhase::NO_SIGNAL_REVERSE;
				getImpl().apDirection *= -1;
			}

			// Return to the starting position. 
			if (returnToPolarDegree()) {
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_FirstStep(void)
{
	// Move forward to strengthen the signal.
	if (!moveMotorPositionOfPolar()) {
		return false;
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}
	float curr = getImpl().apQuality;

	// If quality got worse, reverse the moving direction. 
	if (curr < prev || !isTargetFoundOfPolar())
	{
		getImpl().apDirection *= -1;
		// Return to the starting position. 
		if (!returnToPolarDegree()) {
			return false;
		}
	}
	else if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree())
	{
		getImpl().apDirection *= -1;
		if (!returnToPolarDegree()) {
			return false;
		}
	}

	getImpl().apPhase = AutoPolarPhase::FORWARD;
	getImpl().apForwardCnt = 0;

	updatePolarMoveStep();
	updatePolarDegree();
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_Forward(void)
{
	if (!isRetryCountToPolar()) {
		if (!moveMotorPositionOfPolar()) {
			return false;
		}

		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree()) {
			getImpl().apPhase = AutoPolarPhase::COMPLETE;
		}
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}


	if (isTargetFoundOfFocus())
	{
		if (true) // getImpl().apForwardCnt++ > 0) 
		{
			float curr = getImpl().apQuality;
			if (curr >= prev)
			{
			}
			else {
				// Keep moving foward until quality getting worse enough to retrace.
				if (fabs(curr - prev) >= AUTO_POLAR_QINDEX_DIFF_TO_RETRACE) {
					// Return to the currently best position.
					if (!returnToPolarDegree()) {
						return false;
					}

					// Retrace with the minimum step.
					getImpl().apPhase = AutoPolarPhase::RETRACE;
					getImpl().apDirection *= -1;
					getImpl().apRetraceCnt = 0;
				}
			}
		}
		updatePolarMoveStep();
		updatePolarDegree();
		resetRetryCountToPolar();
	}
	else
	{
		getImpl().apForwardCnt = 0;
		if (checkRetryOverToPolar()) {
			getImpl().apPhase = AutoPolarPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_Retrace(void)
{
	if (!isRetryCountToPolar()) {
		if (!moveMotorPositionOfPolar()) {
			return false;
		}

		if (isAtUpperEndOfPolarDegree() || isAtLowerEndOfPolarDegree()) {
			getImpl().apPhase = AutoPolarPhase::COMPLETE;
		}
	}

	float prev = getImpl().apQuality;
	if (!renewPolarQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfPolar())
	{
		if (true) // getImpl().apRetraceCnt++ > 0)
		{
			float curr = getImpl().apQuality;
			if (curr >= prev)
			{
				// Keep moving foward until quality getting worse enough to retrace.
			}
			else {
				// Return to the currently best position.
				if (!returnToPolarDegree()) {
					return false;
				}

				getImpl().apPhase = AutoPolarPhase::COMPLETE;
			}
		}

		updatePolarMoveStep();
		updatePolarDegree();
		resetRetryCountToPolar();
	}
	else
	{
		getImpl().apRetraceCnt = 0;
		if (checkRetryOverToPolar()) {
			getImpl().apPhase = AutoPolarPhase::CANCELED;
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_Complete(void)
{
	if (getImpl().cbAutoPolar != nullptr) {
		(*getImpl().cbAutoPolar)(true, getImpl().apQuality, getImpl().apDegree);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoPolar_Canceled(void)
{
	if (getImpl().cbAutoPolar != nullptr) {
		(*getImpl().cbAutoPolar)(false, getImpl().apQuality, getImpl().apDegree);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::renewPolarQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().apQuality = qidx;
	return true;
}


float OctSystem::ScanOptimizer::getMotorStepToPolar(void)
{
	float qidx = getImpl().apQuality;

	if (isQualityToComplete(qidx)) {
		return AUTO_POLAR_MOVE_STEP_MIN;
	}
	else if (isQualityToSignal(qidx)) {
		return AUTO_POLAR_MOVE_STEP_FIRST;
	}
	else {
		return AUTO_POLAR_MOVE_STEP_MAX;
	}
}


float OctSystem::ScanOptimizer::getMotorOffsetToPolar(void)
{
	return getImpl().apMoveStep * getImpl().apDirection;
}


bool OctSystem::ScanOptimizer::moveMotorPositionOfPolar(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	float offset = getMotorOffsetToPolar();

	// Move forward/reverse to find a signal. 
	if (!motor->updateDegreeByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::updatePolarMoveStep(void)
{
	getImpl().apMoveStep = getMotorStepToPolar();
	return true;
}


bool OctSystem::ScanOptimizer::updatePolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	getImpl().apDegree = motor->getCurrentDegree();
	return true;
}


bool OctSystem::ScanOptimizer::returnToPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	if (!motor->updateDegree(getImpl().apDegree)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::isHeadingForUpperEndOfPolarDegree(void)
{
	return (getImpl().apDirection == AUTO_POLAR_UPPER_DIRECTION);
}


bool OctSystem::ScanOptimizer::isHeadingForLowerEndOfPolarDegree(void)
{
	return (getImpl().apDirection == AUTO_POLAR_LOWER_DIRECTION);
}


bool OctSystem::ScanOptimizer::isAtUpperSideOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isAtUpperSideOfPosition());
}


bool OctSystem::ScanOptimizer::isAtLowerSideOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isAtLowerSideOfPosition());
}


bool OctSystem::ScanOptimizer::isAtUpperEndOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isEndOfLowerPosition() && isHeadingForLowerEndOfPolarDegree());
}


bool OctSystem::ScanOptimizer::isAtLowerEndOfPolarDegree(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return (motor->isEndOfLowerPosition() && isHeadingForLowerEndOfPolarDegree());
}


bool OctSystem::ScanOptimizer::isTargetFoundOfPolar(void)
{
	return isQualityToSignal(getImpl().apQuality);
}


bool OctSystem::ScanOptimizer::isTargetCompleteOfPolar(void)
{
	return isQualityToComplete(getImpl().apQuality);
}


void OctSystem::ScanOptimizer::resetRetryCountToPolar(void)
{
	getImpl().apRetryCnt = 0;
	return;
}


bool OctSystem::ScanOptimizer::checkRetryOverToPolar(void)
{
	getImpl().apRetryCnt++;
	if (getImpl().apRetryCnt > 3) {
		resetRetryCountToPolar();
		return true;
	}
	return false;
}


bool OctSystem::ScanOptimizer::isRetryCountToPolar(void)
{
	return (getImpl().apRetryCnt > 0);
}


bool OctSystem::ScanOptimizer::phaseAutoReferProcess(void)
{
	bool res = false;

	switch (getImpl().arPhase) {
	case AutoReferPhase::INIT:
		res = phaseAutoRefer_Initiate();
		break;
	case AutoReferPhase::NO_SIGNAL:
	case AutoReferPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoRefer_NoSignal();
		break;
	case AutoReferPhase::FORWARD:
		res = phaseAutoRefer_Forward();
		break;
	case AutoReferPhase::RETRACE:
		res = phaseAutoRefer_Retrace();
		break;
	case AutoReferPhase::CENTER:
		res = phaseAutoRefer_Center();
		break;
	}

	if (res) {
		if (getImpl().arPhase == AutoReferPhase::COMPLETE) {
			phaseAutoRefer_Complete();
		}
		if (getImpl().arPhase == AutoReferPhase::CANCELED) {
			phaseAutoRefer_Canceled();
		}
		return true;
	}
	else {
		phaseAutoRefer_Canceled();
		return false;
	}
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Initiate(void)
{
	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	DebugOut2() << "Start auto reference, qidx: " << getImpl().arQuality;

	if (isTargetFoundOfReference()) {
		getImpl().arPhase = AutoReferPhase::FORWARD;
		getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;	
		getImpl().arForwardCnt = 0;
		DebugOut2() << "Target found to forward";
	}
	else {
		getImpl().arPhase = AutoReferPhase::NO_SIGNAL;
		// getImpl().arDirection = (isAtLowerSideOfReference() ? AUTO_REFER_UPPER_DIRECTION : AUTO_REFER_LOWER_DIRECTION);
		getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
		DebugOut2() << "Target not found to no signal";
	}

	updateReferenceMoveStep();
	updateReferencePosition();
	resetRetryCountToReference();
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_NoSignal(void)
{
	if (!moveMotorPositionOfReference()) {
		return false;
	}

	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfReference()) 
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		updateReferenceMoveStep();
		updateReferencePosition();

		if (isHeadingForLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::FORWARD;
			getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION; // (isTargetAboveReferenceCenter() ? AUTO_REFER_UPPER_DIRECTION : AUTO_REFER_LOWER_DIRECTION);
			getImpl().arRetraceCnt = 0;
			DebugOut2() << "Target found to retrace, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", mpos: " << getImpl().arPosition;
		}
		else {
			getImpl().arPhase = AutoReferPhase::FORWARD;
			getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			getImpl().arForwardCnt = 0;
			DebugOut2() << "Target found to forward, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", mpos: " << getImpl().arPosition;
		}
	}
	else {
		// Reach an end of diopter range in the moving direction. 
		if (isAtUpperEndOfReference() || isAtLowerEndOfReference())
		{
			if (getImpl().arPhase == AutoReferPhase::NO_SIGNAL_REVERSE) {
				getImpl().arPhase = AutoReferPhase::CANCELED;
				DebugOut2() << "Target not found to canceled, return to mpos: " << getImpl().arPosition;
			}
			else {
				getImpl().arPhase = AutoReferPhase::NO_SIGNAL_REVERSE;
				getImpl().arDirection *= -1;		// Reverse the direction.
				DebugOut2() << "Target not found to reverse, return to mpos: " << getImpl().arPosition;
			}

			// Return to the starting position. 
			if (!returnToReferencePosition()) {
				DebugOut2() << "Return to initial position failed";
				return false;
			}
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Forward(void)
{
	if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Retrace completed at end of reference";
		}
	}

	if (!renewReferenceQualityFromPreview(false)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		int prev = getImpl().arRefPoint;
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}
		int curr = getImpl().arRefPoint;

		if (getImpl().arForwardCnt++ > 0) {
			if (curr > prev) {
				getImpl().arPhase = AutoReferPhase::RETRACE;
				getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();
		DebugOut2() << "Forwarding target, qidx: " << getImpl().arQuality << ", refPoint: " << getImpl().arRefPoint << ", mpos: " << getImpl().arPosition;
	}
	else {
		getImpl().arForwardCnt = 0;

		if (checkRetryOverToReference()) {
			// getImpl().arPhase = AutoReferPhase::CANCELED;
			// DebugOut2() << "Target not found in forward, canceled";
			getImpl().arPhase = AutoReferPhase::RETRACE;
			getImpl().arDirection = AUTO_REFER_LOWER_DIRECTION;
			DebugOut2() << "Target not found in forward, retrace";
		}
		else {
			DebugOut2() << "Target not found in forward, retrying...";
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Retrace(void)
{
	// if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Retrace canceled at end of reference";
		}
	// }

	if (!renewReferenceQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		if (isHeadingForUpperEndOfReference())
		{
			if (isTargetBelowReferenceCenter()) {
				getImpl().arPhase = AutoReferPhase::CENTER;
				DebugOut2() << "Retrace completed at below, to center";
			}
		}
		else
		{
			if (isTargetAboveReferenceCenter()) {
				getImpl().arPhase = AutoReferPhase::CENTER;
				DebugOut2() << "Retrace completed at above, to center";
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();
	}
	else
	{
		if (checkRetryOverToReference()) {
			getImpl().arPhase = AutoReferPhase::CANCELED;
			DebugOut2() << "Target not found in retrace, canceled";
		}
		else {
			DebugOut2() << "Target not found in retrace, retrying";
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Center(void)
{
	if (!isRetryCountToReference()) {
		if (!moveMotorPositionOfReference()) {
			return false;
		}

		if (isAtUpperEndOfReference() || isAtLowerEndOfReference()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Center canceled at end of reference";
		}
	}

	if (!renewReferenceQualityFromPreview(true)) {
		return false;
	}

	if (isTargetFoundOfReference())
	{
		if (!renewReferencePointFromPreview(false)) {
			return false;
		}

		if (isTargetAtReferenceCenter()) {
			getImpl().arPhase = AutoReferPhase::COMPLETE;
			DebugOut2() << "Center completed";
		}
		else
		{
			if (isTargetBelowReferenceCenter())
			{
				getImpl().arDirection = AUTO_REFER_LOWER_DIRECTION;
			}
			else
			{
				getImpl().arDirection = AUTO_REFER_UPPER_DIRECTION;
			}
		}

		updateReferenceMoveStep();
		updateReferencePosition();
		resetRetryCountToReference();

		DebugOut2() << "Reference point: " << getImpl().arRefPoint << ", step: " << getImpl().arMoveStep << ", dir: " << getImpl().arDirection << ", dist: " << getTargetDistanceFromReferCenter();
	}
	else
	{
		if (checkRetryOverToReference()) {
			getImpl().arPhase = AutoReferPhase::CANCELED;
			DebugOut2() << "Target not found in center, canceled";
		}
		else {
			DebugOut2() << "Target not found in center, retrying";
		}
	}
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Complete(void)
{
	if (getImpl().cbAutoRefer != nullptr) {
		(*getImpl().cbAutoRefer)(true, getImpl().arRefPoint, getImpl().arPosition);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::phaseAutoRefer_Canceled(void)
{
	if (getImpl().cbAutoRefer != nullptr) {
		(*getImpl().cbAutoRefer)(false, getImpl().arRefPoint, getImpl().arPosition);
	}

	getImpl().completed = true;
	return true;
}


bool OctSystem::ScanOptimizer::obtainQualityIndexOfPreview(float & qidx, bool next)
{
	unsigned long prevCount, nextCount;
	float prevIndex, nextIndex;

	// Ignore the preview image right after the motor moving.
	ChainOutput::getPreviewImageResult(&prevIndex, nullptr, &prevCount);
	if (!next) {
		qidx = prevIndex;
		return true;
	}

	int waits = 0;
	const int waitsMax = 50;

	while (true) {
		// wait for 250ms to obtain the next preview image.
		this_thread::sleep_for(chrono::microseconds(5));
		if (++waits > waitsMax) {
			break;
		}

		ChainOutput::getPreviewImageResult(&nextIndex, nullptr, &nextCount);
		if (prevCount != nextCount) {
			qidx = nextIndex;
			return true;
		}
	}
	return false;
}


bool OctSystem::ScanOptimizer::obtainReferencePointOfPreview(int & refPoint, bool next)
{
	unsigned long prevCount, nextCount;
	int prevPoint, nextPoint;

	// Ignore the preview image right after the motor moving.
	ChainOutput::getPreviewImageResult(nullptr, &prevPoint, &prevCount);
	if (!next) {
		refPoint = prevPoint;
		return true;
	}

	int waits = 0;
	const int waitsMax = 50;

	while (true) {
		// wait for 250ms to obtain the next preview image.
		this_thread::sleep_for(chrono::microseconds(5));
		if (++waits > waitsMax) {
			break;
		}

		ChainOutput::getPreviewImageResult(nullptr, &nextPoint, &nextCount);
		if (prevCount != nextCount) {
			refPoint = nextPoint;
			return true;
		}
	}
	return false;
}


float OctSystem::ScanOptimizer::getFocusDiopterStep(float qidx, bool init)
{
	float dstep = AUTO_FOCUS_DIOPT_STEP_MAX;
	for (int i = 0; i < AUTO_FOCUS_DIOPT_STEP_SIZE; i++) {
		if (qidx >= AutoFocusQualitySteps[i]) {
			dstep = AutoFocusDioptSteps[i];
			break;
		}
	}
	return dstep;
}


float OctSystem::ScanOptimizer::getFocusDiopterOffset(void)
{
	float offset = getImpl().afDioptStep * getImpl().afDirection;
	return offset;
}


float OctSystem::ScanOptimizer::getPolarDegreeStep(float qidx)
{
	if (isQualityToComplete(qidx)) {
		return AUTO_POLAR_MOVE_STEP_MIN;
	}
	else if (isQualityToSignal(qidx)) {
		return AUTO_POLAR_MOVE_STEP_FIRST;
	}
	else {
		return AUTO_POLAR_MOVE_STEP_MAX;
	}
}


bool OctSystem::ScanOptimizer::renewReferenceQualityFromPreview(bool next)
{
	float qidx;
	if (!obtainQualityIndexOfPreview(qidx, next)) {
		return false;
	}
	getImpl().arQuality = qidx;
	return true;
}


bool OctSystem::ScanOptimizer::renewReferencePointFromPreview(bool next)
{
	int refPoint;
	if (!obtainReferencePointOfPreview(refPoint, next)) {
		return false;
	}

	getImpl().arRefPoint = refPoint;
	return true;
}


int OctSystem::ScanOptimizer::getMotorStepToReference(void)
{
	int mstep = AUTO_REFER_MOVE_STEP_MAX;
	if (getImpl().arPhase == AutoReferPhase::NO_SIGNAL ||
		getImpl().arPhase == AutoReferPhase::NO_SIGNAL_REVERSE) {
		mstep = AUTO_REFER_MOVE_STEP_FIND;
	}
	else if (getImpl().arPhase == AutoReferPhase::FORWARD) {
		mstep = AUTO_REFER_MOVE_STEP_MAX;
	}
	else {
		int dist = getTargetDistanceFromReferCenter();
		for (int i = 0; i < AUTO_REFER_MOVE_STEP_SIZE; i++) {
			if (dist <= AutoReferPointSteps[i]) {
				mstep = AutoReferMoveSteps[i];
				break;
			}
		}
	}
	return mstep;
}


int OctSystem::ScanOptimizer::getMotorOffsetToReference(void)
{
	return getImpl().arMoveStep * getImpl().arDirection;
}


bool OctSystem::ScanOptimizer::moveMotorPositionOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	int offset = getMotorOffsetToReference();

	// Move forward/reverse to find a signal. 
	if (!motor->updatePositionByOffset(offset)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::updateReferenceMoveStep(void)
{
	getImpl().arMoveStep = getMotorStepToReference();
	return true;
}


bool OctSystem::ScanOptimizer::updateReferencePosition(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	getImpl().arPosition = motor->getPosition();
	return true;
}


bool OctSystem::ScanOptimizer::returnToReferencePosition(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	if (!motor->updatePosition(getImpl().arPosition)) {
		return false;
	}
	return true;
}


bool OctSystem::ScanOptimizer::isTargetFoundOfReference(void)
{
	return isQualityToSignal(getImpl().arQuality);
}


bool OctSystem::ScanOptimizer::isHeadingForUpperEndOfReference(void)
{
	return (getImpl().arDirection == AUTO_REFER_UPPER_DIRECTION);
}


bool OctSystem::ScanOptimizer::isHeadingForLowerEndOfReference(void)
{
	return (getImpl().arDirection == AUTO_REFER_LOWER_DIRECTION);
}


bool OctSystem::ScanOptimizer::isAtUpperSideOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return motor->isAtUpperSide();
}


bool OctSystem::ScanOptimizer::isAtLowerSideOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return motor->isAtLowerSide();
}


bool OctSystem::ScanOptimizer::isAtUpperEndOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return (motor->isEndOfUpperPosition() && isHeadingForUpperEndOfReference());
}


bool OctSystem::ScanOptimizer::isAtLowerEndOfReference(void)
{
	ReferenceMotor* motor = getMainBoard()->getReferenceMotor();
	return (motor->isEndOfLowerPosition() && isHeadingForLowerEndOfReference());
}


int OctSystem::ScanOptimizer::getTargetDistanceFromReferCenter(void)
{
	return abs(getTargetOffsetFromReferCenter());
}


int OctSystem::ScanOptimizer::getTargetOffsetFromReferCenter(void)
{
	int offset = (getImpl().arRefPoint - AUTO_REFER_CENTER_POINT);
	return offset;
}


bool OctSystem::ScanOptimizer::isTargetBelowReferenceCenter(void)
{
	return (getTargetOffsetFromReferCenter() >= 0);
}


bool OctSystem::ScanOptimizer::isTargetAboveReferenceCenter(void)
{
	return (getTargetOffsetFromReferCenter() <= 0);
}


bool OctSystem::ScanOptimizer::isTargetAtReferenceCenter(void)
{
	/*
	if (getTargetDistanceFromReferCenter() <= AUTO_REFER_CENTER_OFFSET) {
		return true;
	}
	*/

	int offset = getTargetOffsetFromReferCenter();
	if (offset >= 0 && offset <= AUTO_REFER_CENTER_OFFSET) {
		return true;
	}
	return false;
}


void OctSystem::ScanOptimizer::resetRetryCountToReference(void)
{
	getImpl().arRetryCnt = 0;
	return;
}


bool OctSystem::ScanOptimizer::checkRetryOverToReference(void)
{
	getImpl().arRetryCnt++;
	if (getImpl().arRetryCnt > 3) {
		resetRetryCountToReference();
		return true;
	}
	return false;
}


bool OctSystem::ScanOptimizer::isRetryCountToReference(void)
{
	return (getImpl().arRetryCnt > 0);
}


bool OctSystem::ScanOptimizer::isQualityToComplete(float qidx)
{
	return (qidx >= AUTO_FOCUS_QINDEX_TO_COMPLETE ? true : false);
}


bool OctSystem::ScanOptimizer::isQualityToSignal(float qidx)
{
	return (qidx >= AUTO_FOCUS_QINDEX_TO_SIGNAL ? true : false);
}


