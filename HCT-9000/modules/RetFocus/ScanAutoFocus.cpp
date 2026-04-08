#include "stdafx.h"
#include "ScanAutoFocus.h"

#include <mutex>
#include <atomic>

#include "OctDevice2.h"
#include "CppUtil2.h"
#include "SigChain2.h"

using namespace RetFocus;
using namespace OctDevice;
using namespace CppUtil;
using namespace SigChain;
using namespace std;


struct ScanAutoFocus::ScanAutoFocusImpl
{
	MainBoard* board;
	bool initiated;

	AutoFocusPhase phase;
	float quality;
	float qualityMax;
	float signalRatio;
	float signalRatioMax;

	int signalMaxPosition;
	int targetMaxPosition;
	float targetMaxDiopter;
	float signalMaxDiopter;

	int startPosition;
	int motorPosition;
	float startDiopter;
	float motorDiopter;
	int motorDirection;
	float moveDioptStep;

	int retryCount;
	int resistCount;
	int forwardCount;
	int retraceCount;
	
	AutoFocusCompleteCallback* cbAutoFocus;
	OctFocusMotor* focusMotor;

	atomic<bool> running;
	atomic<bool> stopping;
	atomic<bool> completed;
	thread threadWork;

	ScanAutoFocusImpl() : initiated(false), board(nullptr), cbAutoFocus(nullptr), running(false),
		stopping(false), completed(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanAutoFocus::ScanAutoFocusImpl> ScanAutoFocus::d_ptr(new ScanAutoFocusImpl());


ScanAutoFocus::ScanAutoFocus()
{
}


ScanAutoFocus::~ScanAutoFocus()
{
}


bool RetFocus::ScanAutoFocus::initialize(OctDevice::MainBoard * board, AutoFocusCompleteCallback* callback)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	d_ptr->cbAutoFocus = callback;
	d_ptr->focusMotor = board->getOctFocusMotor();
	return true;
}

bool RetFocus::ScanAutoFocus::isInitialized(void)
{
	return impl().initiated;
}


bool RetFocus::ScanAutoFocus::startOptimizing(void)
{
	if (!isInitialized()) {
		return false;
	}

	cancelOptimizing(true);
	initializeFocusStatus();

	// Thread object should be joined before being reused. 
	if (impl().threadWork.joinable()) {
		impl().threadWork.join();
	}
	impl().threadWork = thread{ &ScanAutoFocus::threadFunction };
	return true;
}


void RetFocus::ScanAutoFocus::cancelOptimizing(bool wait)
{
	if (isRunning() && !isCancelling()) {
		d_ptr->stopping = true;

		if (wait) {
			// Thread object should be joined before being reused. 
			if (impl().threadWork.joinable()) {
				impl().threadWork.join();
			}
		}
	}
	d_ptr->stopping = false;
	return;
}


bool RetFocus::ScanAutoFocus::isRunning(void)
{
	return d_ptr->running;
}


bool RetFocus::ScanAutoFocus::isCancelling(void)
{
	return d_ptr->stopping;
}


bool RetFocus::ScanAutoFocus::isCompleted(void)
{
	return (d_ptr->phase == AutoFocusPhase::COMPLETE);
}


bool RetFocus::ScanAutoFocus::isCancelled(void)
{
	return (d_ptr->phase == AutoFocusPhase::CANCELED);
}


void RetFocus::ScanAutoFocus::initializeFocusStatus(void)
{
	impl().phase = AutoFocusPhase::INIT;

	impl().running = false;
	impl().stopping = false;
	impl().completed = false;

	impl().quality = 0.0f;
	impl().qualityMax = 0.0f;
	impl().signalRatio = 0.0f;
	impl().signalRatioMax = 0.0f;

	resetTargetTrackCount();
	updateMotorPosition();
	updateStartPosition();
	return;
}


void RetFocus::ScanAutoFocus::threadFunction(void)
{
	impl().running = true;
	onScanOptimizingStarted();

	while (true)
	{
		if (!processAutoFocusPhase()) {
			break;
		}
		if (isCompleted()) {
			LogD() << "Auto Focus: completed";
			break;
		}
		if (isCancelled()) {
			LogD() << "Auto Focus: cancelled";
			break;
		}
		if (isCancelling()) {
			LogD() << "Auto Focus: stopping";
			break;
		}
	}

	impl().running = false;
	onScanOptimizingClosed();
	return;
}


bool RetFocus::ScanAutoFocus::processAutoFocusPhase(void)
{
	bool res = false;

	switch (impl().phase) {
	case AutoFocusPhase::INIT:
		res = phaseAutoFocus_Initiate();
		break;
	case AutoFocusPhase::NO_SIGNAL:
	case AutoFocusPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoFocus_NoSignal();
		break;
	case AutoFocusPhase::FORWARD:
		res = phaseAutoFocus_Forward();
		break;
	case AutoFocusPhase::RETRACE:
		res = phaseAutoFocus_Retrace();
		break;
	case AutoFocusPhase::FORWARD2:
		res = phaseAutoFocus_Forward2();
		break;
	case AutoFocusPhase::RETRACE2:
		res = phaseAutoFocus_Retrace2();
		break;
	case AutoFocusPhase::FIRST_GUESS:
		res = phaseAutoFocus_FirstGuess();
		break;
	}
	
	if (res) {
		if (impl().phase == AutoFocusPhase::COMPLETE) {
			phaseAutoFocus_Complete();
		}
		if (impl().phase == AutoFocusPhase::CANCELED) {
			phaseAutoFocus_Canceled();
		}
		return true;
	}
	else {
		phaseAutoFocus_Canceled();
		return false;
	}
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_Initiate(void)
{
	if (!renewQualityIndex()) {
		return false;
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	LogD() << "Auto Focus: initiated, qidx: " << quality << ", mpos: " << position;

	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: target complete, qidx: " << quality;
	}
	else if (isTargetFound()) {
		changeFocusPhase(AutoFocusPhase::FORWARD);
		LogD() << "Auto Focus: target found => forward, qidx: " << quality;
	}
	else {
		changeFocusPhase(AutoFocusPhase::NO_SIGNAL);
		LogD() << "Auto Focus: target not found => no_signal, qidx: " << quality;
		/*
		changeFocusPhase(AutoFocusPhase::CANCELED);
		DebugOut2() << "Auto Focus: target not found => canceled";
		*/
	}
	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_NoSignal(void)
{
	// Move a step in forward or reversed direction, then update motor status.
	if (!moveMotorPosition()) {
		return false;
	}

	if (!renewQualityIndex()) {
		return false;
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	// Record the target found signal and the current position.
	if (sigrate > impl().signalRatioMax) {
		impl().qualityMax = impl().quality;
		impl().signalRatioMax = impl().signalRatio;
		impl().signalMaxPosition = impl().motorPosition;
	}

	if (isTargetFound())
	{
		impl().qualityMax = 0.0f;
		impl().signalRatioMax = 0.0f;
		impl().targetMaxDiopter = impl().motorDiopter;
		impl().targetMaxPosition = impl().motorPosition;
		impl().signalMaxPosition = impl().motorPosition;
		impl().signalMaxDiopter = impl().motorDiopter;

		// Change the phase to locate the optimal signal. 
		if (d_ptr->phase == AutoFocusPhase::NO_SIGNAL) {
			changeFocusPhase(AutoFocusPhase::FORWARD);
			LogD() << "Auto Focus: target found => forward, qidx: " << impl().quality << ", mpos: " << getFocusMotorPosition();
		}
		else {
			changeFocusPhase(AutoFocusPhase::RETRACE);
			LogD() << "Auto Focus: target found => retrace, qidx: " << impl().quality << ", mpos: " << getFocusMotorPosition();
		}
	}
	else
	{
		if (d_ptr->phase == AutoFocusPhase::NO_SIGNAL) {
			if (isAtMinusDioptEnd()) {
				returnToStartPosition();
				changeFocusPhase(AutoFocusPhase::NO_SIGNAL_REVERSE);
				LogD() << "Auto Focus: no signal, minus diopt end => reverse";
			}
		}
		else {
			// Return to the starting position, (<= highest signal location), then stop.
			if (isAtPlusDioptEnd()) {
				// returnToSignalPosition();
				returnToStartPosition();
				changeFocusPhase(AutoFocusPhase::CANCELED);
				LogD() << "Auto Focus: signal reverse, plus diopt end => canceled, mpos: " << getFocusMotorPosition();
			}
		}
	}
	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_FirstGuess(void)
{
	if (!isRetryCountNotZero()) {
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtMinusDioptEnd()) {
				returnToStartPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE);
				LogD() << "Auto Focus: minus diopt end => retrace";
				return true;
			}
		}
	}

	float prevIdx = d_ptr->quality;
	if (!renewQualityIndex()) {
		return false;
	}
	float currIdx = d_ptr->quality;

	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		d_ptr->retryCount = 0;
		if (currIdx < prevIdx)
		{
			changeFocusPhase(AutoFocusPhase::RETRACE);
			LogD() << "Auto Focus: poor first guess => retrace";
		}
		else
		{
			changeFocusPhase(AutoFocusPhase::FORWARD);
			LogD() << "Auto Focus: good first guess => forward";
		}
	}
	else
	{
		d_ptr->retryCount++;
		if (checkRetryOverToFirstGuess()) {
			returnToStartPosition();
			changeFocusPhase(AutoFocusPhase::RETRACE);
			LogD() << "Auto Focus: retry over first guess => retrace";
		}
	}
	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_Forward(void)
{
	if (isRetryCountNotZero()) {
		// Make a delay consuming next frame to recover target status. 
		if (!renewQualityIndex(true)) {
			return false;
		}
	}
	else {
		// Move motor by step size, then update motor status. 
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtMinusDioptEnd()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track forward, minus diopt end => retrace, mpos: " << getFocusMotorPosition();
				return true;
			}
		}
		if (!renewQualityIndex(false)) {
			return false;
		}
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;
	
	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: track forward, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Focus: track forward, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDiopter = impl().motorDiopter;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;
			impl().forwardCount += 1;
		}
		else {
			// Go to the other side from the searched position. 
			d_ptr->resistCount += 1;
			if (checkResistOverToForward()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track forward, resists over => retrace, mpos: " << getFocusMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the searched position.
		d_ptr->retryCount += 1;
		if (checkRetryOverToForward()) {
			returnToSignalPosition();
			changeFocusPhase(AutoFocusPhase::RETRACE);

			// Renew the signal ratio at the returned position.
			if (renewQualityIndex(false)) {
				impl().qualityMax = impl().quality;
				impl().signalRatioMax = impl().signalRatio;
				impl().signalMaxDiopter = impl().motorDiopter;
				impl().signalMaxPosition = impl().motorPosition;
			}
			LogD() << "Auto Focus: track forward, retry over => retrace, mpos: " << getFocusMotorPosition();
		}
	}
	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_Retrace(void)
{
	if (isRetryCountNotZero()) {
		// Make a delay consuming next frame to recover target status. 
		if (!renewQualityIndex(true)) {
			return false;
		}
	}
	else {
		// Move motor by step size, then update motor status. 
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtPlusDioptEnd()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::FORWARD2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track retrace, plus dpt end => forward2, mpos: " << getFocusMotorPosition();
				return true;
			}
			if (!renewQualityIndex(false)) {
				return false;
			}
		}
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: track retrace, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Focus: track retrace, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDiopter = impl().motorDiopter;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;
			impl().retraceCount += 1;
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToRetrace()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::FORWARD2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track retrace, resists over => forward2, mpos: " << getFocusMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the last optimal position. 
		d_ptr->retryCount += 1;
		if (checkRetryOverToRetrace()) {
			returnToSignalPosition();
			changeFocusPhase(AutoFocusPhase::CANCELED);
			LogD() << "Auto Focus: track retrace, retry over => cancelled, mpos: " << getFocusMotorPosition();
		}
	}
	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_Forward2(void)
{
	if (isRetryCountNotZero()) {
		// Make a delay consuming next frame to recover target status. 
		if (!renewQualityIndex(true)) {
			return false;
		}
	}
	else {
		// Move motor by step size, then update motor status. 
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtMinusDioptEnd()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE2);
				
				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track forward2, minus diopt end => retrace2, mpos: " << getFocusMotorPosition();
				return true;
			}
		}
		if (!renewQualityIndex(false)) {
			return false;
		}
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: track forward2, target complete, qidx: " << quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Focus: track forward2, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDiopter = impl().motorDiopter;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;

			impl().forwardCount += 1;
			if (checkLimitsOverToForward2()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track forward2, limits over => retrace2, mpos: " << getFocusMotorPosition();
			}
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToForward2()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::RETRACE2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDiopter = impl().motorDiopter;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Focus: track forward2, resists over => retrace2, mpos: " << getFocusMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the searched position.
		d_ptr->retryCount += 1;
		if (checkRetryOverToForward()) {
			returnToSignalPosition();
			changeFocusPhase(AutoFocusPhase::RETRACE2);

			// Renew the signal ratio at the returned position.
			if (renewQualityIndex(false)) {
				impl().qualityMax = impl().quality;
				impl().signalRatioMax = impl().signalRatio;
				impl().signalMaxDiopter = impl().motorDiopter;
				impl().signalMaxPosition = impl().motorPosition;
			}
			LogD() << "Auto Focus: track forward2, retry over => retrace2, mpos: " << getFocusMotorPosition();
		}
	}
	return true;
}

bool RetFocus::ScanAutoFocus::phaseAutoFocus_Retrace2(void)
{
	if (isRetryCountNotZero()) {
		// Make a delay consuming next frame to recover target status. 
		if (!renewQualityIndex(true)) {
			return false;
		}
	}
	else {
		// Move motor by step size, then update motor status. 
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtPlusDioptEnd()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::COMPLETE);
				LogD() << "Auto Focus: track retrace2 => complete, mpos: " << getFocusMotorPosition();
				return true;
			}
		}
		if (!renewQualityIndex(false)) {
			return false;
		}
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	if (isTargetComplete()) {
		changeFocusPhase(AutoFocusPhase::COMPLETE);
		LogD() << "Auto Focus: target complete, qidx: " << quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Focus: track retrace2, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDiopter = impl().motorDiopter;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;

			impl().retraceCount += 1;
			if (checkLimitsOverToRetrace2()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::COMPLETE);
				LogD() << "Auto Focus: track retrace2, limits over => complete, mpos: " << getFocusMotorPosition();
			}
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToRetrace2()) {
				returnToSignalPosition();
				changeFocusPhase(AutoFocusPhase::COMPLETE);
				LogD() << "Auto Focus: track retrace2, resists over => complete, mpos: " << getFocusMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the last optimal position. 
		d_ptr->retryCount += 1;
		if (checkRetryOverToRetrace()) {
			returnToSignalPosition();
			changeFocusPhase(AutoFocusPhase::CANCELED);
			LogD() << "Auto Focus: track retrace2, retry over => cancelled, mpos: " << getFocusMotorPosition();
		}
	}
	return true;
}

bool RetFocus::ScanAutoFocus::phaseAutoFocus_Complete(void)
{
	impl().completed = true;

	if (impl().cbAutoFocus != nullptr) {
		(*impl().cbAutoFocus)(impl().completed, impl().quality, impl().motorDiopter);
	}

	return true;
}


bool RetFocus::ScanAutoFocus::phaseAutoFocus_Canceled(void)
{
	impl().completed = false;

	if (impl().cbAutoFocus != nullptr) {
		(*impl().cbAutoFocus)(impl().completed, impl().quality, impl().motorDiopter);
	}
	return true;
}


ScanAutoFocus::ScanAutoFocusImpl & RetFocus::ScanAutoFocus::impl(void)
{
	return *d_ptr;
}

OctDevice::MainBoard * RetFocus::ScanAutoFocus::getMainBoard(void)
{
	return d_ptr->board;
}


bool RetFocus::ScanAutoFocus::changeFocusPhase(AutoFocusPhase phase, bool resetPos)
{
	d_ptr->phase = phase;
	resetTargetTrackCount();

	if (resetPos) {
		if (!returnToStartPosition()) {
			LogD() << "Auto Focus: return to init failed";
			return false;
		}
	}
	else {
		updateMotorPosition();
	}
	return true;
}


void RetFocus::ScanAutoFocus::resetTargetTrackCount(void)
{
	impl().retryCount = 0;
	impl().resistCount = 0;
	impl().forwardCount = 0;
	impl().retraceCount = 0;
	return;
}


bool RetFocus::ScanAutoFocus::isRetryCountNotZero(void)
{
	return (impl().retryCount > 0);
}


bool RetFocus::ScanAutoFocus::checkResistOverToForward(void)
{
	if (impl().resistCount > AUTO_FOCUS_RESISTS_FORWARD) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoFocus::checkResistOverToRetrace(void)
{
	if (impl().resistCount > AUTO_FOCUS_RESISTS_RETRACE) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoFocus::checkResistOverToForward2(void)
{
	if (impl().resistCount > AUTO_FOCUS_RESISTS_FORWARD2) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoFocus::checkResistOverToRetrace2(void)
{
	if (impl().resistCount > AUTO_FOCUS_RESISTS_RETRACE2) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoFocus::checkLimitsOverToForward2(void)
{
	if (impl().forwardCount > AUTO_FOCUS_FORWARDS_LIMIT2) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoFocus::checkLimitsOverToRetrace2(void)
{
	if (impl().retraceCount > AUTO_FOCUS_RETRACES_LIMIT2) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoFocus::checkRetryOverToForward(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_FOCUS_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoFocus::checkRetryOverToRetrace(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_FOCUS_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoFocus::checkRetryOverToFirstGuess(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_FOCUS_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_FOCUS_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoFocus::renewQualityIndex(bool next)
{
	float qidx, ratio;
	if (!obtainQualityIndexFromPreview(qidx, ratio, next)) {
		return false;
	}
	impl().quality = qidx;
	impl().signalRatio = ratio;
	return true;
}


bool RetFocus::ScanAutoFocus::isTargetFound(void)
{
	return isQualityToSignal(d_ptr->quality);
}


bool RetFocus::ScanAutoFocus::isTargetComplete(void)
{
	// return isQualityToComplete(d_ptr->quality);
	auto ratio = impl().signalRatio;
	auto snrMax = 0.0f;
	if (ChainSetup::isSpeedFaster()) {
		snrMax = IMAGE_QUALITY_SNR_MAX_FASTER;
	}
	else if (ChainSetup::isSpeedFastest()) {
		snrMax = IMAGE_QUALITY_SNR_MAX_FASTEST;
	}
	else {
		snrMax = IMAGE_QUALITY_SNR_MAX_NORMAL;
	}
	if (ratio >= snrMax) {
		return true;
	}
	return false;
}


float RetFocus::ScanAutoFocus::updateMotorMoveStep(void)
{
	float mstep;

	switch (d_ptr->phase) {
	case AutoFocusPhase::NO_SIGNAL:
	case AutoFocusPhase::NO_SIGNAL_REVERSE:
	case AutoFocusPhase::SEARCH:
	case AutoFocusPhase::SEARCH_REVERSE:
		mstep = AUTO_FOCUS_MOVE_STEP_SEARCH;
		break;
	case AutoFocusPhase::FORWARD:
	case AutoFocusPhase::RETRACE:
		mstep = AUTO_FOCUS_MOVE_STEP_ALIGN;
		break;
	case AutoFocusPhase::FORWARD2:
	case AutoFocusPhase::RETRACE2:
		mstep = AUTO_FOCUS_MOVE_STEP_CONFIRM;
		break;
	case AutoFocusPhase::FIRST_GUESS:
	default:
		mstep = AUTO_FOCUS_MOVE_STEP_GUESS;
		break;
	}

	d_ptr->moveDioptStep = mstep;
	return mstep;
}


int RetFocus::ScanAutoFocus::updateMotorPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	int mpos = motor->getPosition();
	float diopt = motor->getCurrentDiopter();
	impl().motorPosition = mpos;
	impl().motorDiopter = diopt;
	return mpos;
}


int RetFocus::ScanAutoFocus::updateMotorDirection(void)
{
	int mdir;

	switch (d_ptr->phase) {
	case AutoFocusPhase::NO_SIGNAL:
	case AutoFocusPhase::FORWARD:
	case AutoFocusPhase::FORWARD2:
	case AutoFocusPhase::SEARCH:
		mdir = AUTO_FOCUS_TO_MINUS_DIOPT;
		break;
	case AutoFocusPhase::NO_SIGNAL_REVERSE:
	case AutoFocusPhase::SEARCH_REVERSE:
	case AutoFocusPhase::RETRACE:
	case AutoFocusPhase::RETRACE2:
		mdir = AUTO_FOCUS_TO_PLUS_DIOPT;
		break;
	case AutoFocusPhase::FIRST_GUESS:
		mdir = AUTO_FOCUS_TO_MINUS_DIOPT;
		break;
	default:
		mdir = d_ptr->motorDirection;
		break;
	}

	d_ptr->motorDirection = mdir;
	return mdir;
}


float RetFocus::ScanAutoFocus::getMotorOffsetToMove(void)
{
	updateMotorDirection();
	updateMotorMoveStep();

	float dsetp = impl().moveDioptStep;
	int direct = impl().motorDirection;
	float offset = dsetp * direct;
	return offset;
}

void RetFocus::ScanAutoFocus::updateStartPosition(void)
{
	auto mpos = impl().motorPosition;
	auto mval = impl().motorDiopter;
	impl().startPosition = mpos;
	impl().startDiopter = mval;
	impl().targetMaxDiopter = mval;
	impl().targetMaxPosition = mpos;
	impl().signalMaxDiopter = mval;
	impl().signalMaxPosition = mpos;
	return;
}


bool RetFocus::ScanAutoFocus::moveMotorPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	float offset = getMotorOffsetToMove();

	/*
	float diopt = motor->getCurrentDiopter() + offset;
	if (diopt > getDiopterRangeMax()) {
		if (isMovingToPlusDiopt()) {
			diopt = getDiopterRangeMax();
			if (!motor->updateDiopter(diopt)) {
				return false;
			}
		}
	}
	else if (diopt < getDiopterRangeMin()) {
		if (isMovingToMinusDiopt()) {
			diopt = getDiopterRangeMin();
			if (!motor->updateDiopter(diopt)) {
				return false;
			}
		}
	}
	else */ {
		// Move forward/reverse to find a signal. 
		if (!motor->updateDiopterByOffset(offset)) {
			return false;
		}
	}
	
	updateMotorPosition();
	return true;
}


int RetFocus::ScanAutoFocus::getFocusMotorPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	return motor->getPosition();
}


bool RetFocus::ScanAutoFocus::returnToStartPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	int mpos = d_ptr->startPosition;
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}


bool RetFocus::ScanAutoFocus::returnToTargetPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	int mpos = d_ptr->targetMaxPosition;
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}

bool RetFocus::ScanAutoFocus::returnToSignalPosition(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();
	int mpos = d_ptr->signalMaxPosition;
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}


bool RetFocus::ScanAutoFocus::isMovingToPlusDiopt(void)
{
	auto direct = impl().motorDirection;
	return (direct == AUTO_FOCUS_TO_PLUS_DIOPT);
}


bool RetFocus::ScanAutoFocus::isMovingToMinusDiopt(void)
{
	auto direct = impl().motorDirection;
	return (direct == AUTO_FOCUS_TO_MINUS_DIOPT);
}


bool RetFocus::ScanAutoFocus::isAtPlusDioptEnd(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();

	auto diopt = impl().motorDiopter;
	auto limit = getDiopterRangeMax();
	auto dover = false;
	if (diopt >= limit) {
		// dover = true;
	}

	auto pos_end = motor->isEndOfLowerPosition();
	auto dir_dpt = isMovingToPlusDiopt();
	return ((pos_end || dover) && dir_dpt);
}


bool RetFocus::ScanAutoFocus::isAtMinusDioptEnd(void)
{
	OctFocusMotor* motor = getMainBoard()->getOctFocusMotor();

	auto diopt = impl().motorDiopter;
	auto limit = getDiopterRangeMin();
	auto dover = false;
	if (diopt <= limit) {
		// dover = true;
	}

	auto pos_end = motor->isEndOfUpperPosition();
	auto dir_dpt = isMovingToMinusDiopt();
	return ((pos_end || dover) && dir_dpt);
}

bool RetFocus::ScanAutoFocus::isAnteriorMode(void)
{
	if (ChainSetup::isCorneaScan()) {
		return true;
	}
	return false;
}

float RetFocus::ScanAutoFocus::getDiopterRangeMax(void)
{
	if (isAnteriorMode()) {
		return AUTO_FOCUS_PLUS_DIOPT_END_ANTERIOR;
	}
	return AUTO_FOCUS_PLUS_DIOPT_END;
}

float RetFocus::ScanAutoFocus::getDiopterRangeMin(void)
{
	if (isAnteriorMode()) {
		return AUTO_FOCUS_MINUS_DIOPT_END_ANTERIOR;
	}
	return AUTO_FOCUS_MINUS_DIOPT_END;
}
