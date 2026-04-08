#include "stdafx.h"
#include "ScanAutoPolar.h"

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


struct ScanAutoPolar::ScanAutoPolarImpl
{
	MainBoard* board;
	bool initiated;

	AutoPolarPhase phase;
	float quality;
	float qualityMax;
	float signalRatio;
	float signalRatioMax;

	int signalMaxPosition;
	int targetMaxPosition;
	float targetMaxDegree;
	float signalMaxDegree;

	int startPosition;
	int motorPosition;
	float startDegree;
	float motorDegree;
	int motorDirection;
	float moveDegStep;

	int retryCount;
	int resistCount;
	int forwardCount;
	int retraceCount;

	AutoPolarizationCompleteCallback* cbAutoPolar;

	atomic<bool> running;
	atomic<bool> stopping;
	atomic<bool> completed;
	thread threadWork;

	ScanAutoPolarImpl() : initiated(false), board(nullptr), cbAutoPolar(nullptr), running(false),
		stopping(false), completed(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanAutoPolar::ScanAutoPolarImpl> ScanAutoPolar::d_ptr(new ScanAutoPolarImpl());


ScanAutoPolar::ScanAutoPolar()
{
}


ScanAutoPolar::~ScanAutoPolar()
{
}


bool RetFocus::ScanAutoPolar::initialize(OctDevice::MainBoard * board, AutoPolarizationCompleteCallback * callback)
{
	d_ptr->board = board;
	d_ptr->initiated = true;
	d_ptr->cbAutoPolar = callback;
	return true;
}


bool RetFocus::ScanAutoPolar::isInitialized(void)
{
	return d_ptr->initiated;
}


bool RetFocus::ScanAutoPolar::startOptimizing(void)
{
	if (!isInitialized()) {
		return false;
	}

	cancelOptimizing(true);
	initializePolarStatus();

	// Thread object should be joined before being reused. 
	if (impl().threadWork.joinable()) {
		impl().threadWork.join();
	}
	impl().threadWork = thread{ &ScanAutoPolar::threadFunction };
	return true;
}


void RetFocus::ScanAutoPolar::cancelOptimizing(bool wait)
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


bool RetFocus::ScanAutoPolar::isRunning(void)
{
	return d_ptr->running;
}


bool RetFocus::ScanAutoPolar::isCancelling(void)
{
	return d_ptr->stopping;
}


bool RetFocus::ScanAutoPolar::isCompleted(void)
{
	return (d_ptr->phase == AutoPolarPhase::COMPLETE);
}


bool RetFocus::ScanAutoPolar::isCancelled(void)
{
	return (d_ptr->phase == AutoPolarPhase::CANCELED);
}


void RetFocus::ScanAutoPolar::initializePolarStatus(void)
{
	d_ptr->phase = AutoPolarPhase::INIT;

	d_ptr->running = false;
	d_ptr->stopping = false;
	d_ptr->completed = false;

	d_ptr->quality = 0.0f;
	d_ptr->qualityMax = 0.0f;
	impl().signalRatio = 0.0f;
	impl().signalRatioMax = 0.0f;

	resetTargetTrackCount();
	updateMotorPosition();
	updateStartPosition();
	return;
}


void RetFocus::ScanAutoPolar::threadFunction(void)
{
	d_ptr->running = true;
	onScanOptimizingStarted();

	while (true)
	{
		if (!processAutoPolarPhase()) {
			break;
		}
		if (isCompleted()) {
			LogD() << "Auto Polar: completed";
			break;
		}
		if (isCancelled()) {
			LogD() << "Auto Polar: cancelled";
			break;
		}
		if (isCancelling()) {
			LogD() << "Auto Polar: stopping";
			break;
		}
	}

	d_ptr->running = false;
	onScanOptimizingClosed();
	return;
}


bool RetFocus::ScanAutoPolar::processAutoPolarPhase(void)
{
	bool res = false;

	switch (impl().phase) {
	case AutoPolarPhase::INIT:
		res = phaseAutoPolar_Initiate();
		break;
	case AutoPolarPhase::NO_SIGNAL:
	case AutoPolarPhase::NO_SIGNAL_REVERSE:
		res = phaseAutoPolar_NoSignal();
		break;
	case AutoPolarPhase::FORWARD:
		res = phaseAutoPolar_Forward();
		break;
	case AutoPolarPhase::RETRACE:
		res = phaseAutoPolar_Retrace();
		break;
	case AutoPolarPhase::FORWARD2:
		res = phaseAutoPolar_Forward2();
		break;
	case AutoPolarPhase::RETRACE2:
		res = phaseAutoPolar_Retrace2();
		break;
	case AutoPolarPhase::FIRST_GUESS:
		res = phaseAutoPolar_FirstGuess();
		break;
	}

	if (res) {
		if (impl().phase == AutoPolarPhase::COMPLETE) {
			phaseAutoPolar_Complete();
		}
		if (impl().phase == AutoPolarPhase::CANCELED) {
			phaseAutoPolar_Canceled();
		}
		return true;
	}
	else {
		phaseAutoPolar_Canceled();
		return false;
	}
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_Initiate(void)
{
	if (!renewQualityIndex()) {
		return false;
	}

	float quality = impl().quality;
	float sigrate = impl().signalRatio;
	int position = impl().motorPosition;

	LogD() << "Auto Polar: initiated, qidx: " << quality << ", mpos: " << position;

	if (isTargetComplete()) {
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: target complete, qidx: " << quality;
	}
	else if (isTargetFound()) {
		changePolarPhase(AutoPolarPhase::FORWARD);
		LogD() << "Auto Polar: target found => forward, qidx: " << quality;
	}
	else {
		changePolarPhase(AutoPolarPhase::NO_SIGNAL);
		LogD() << "Auto Polar: target not found => no_signal, qidx: " << quality;
		/*
		changePolarPhase(AutoPolarPhase::CANCELED, true);
		LogD() << "Auto Polar: target not found => canceled";
		*/
	}
	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_NoSignal(void)
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
		impl().targetMaxDegree = impl().motorDegree;
		impl().targetMaxPosition = impl().motorPosition;
		impl().signalMaxPosition = impl().motorPosition;
		impl().signalMaxDegree = impl().motorDegree;

		if (d_ptr->phase == AutoPolarPhase::NO_SIGNAL) {
			changePolarPhase(AutoPolarPhase::FORWARD);
			LogD() << "Auto Polar: target found => forward, qidx: " << impl().quality << ", mpos: " << getPolarMotorPosition();
		}
		else {
			changePolarPhase(AutoPolarPhase::RETRACE);
			LogD() << "Auto Polar: target found => retrace, qidx: " << impl().quality << ", mpos: " << getPolarMotorPosition();
		}
	}
	else
	{
		if (d_ptr->phase == AutoPolarPhase::NO_SIGNAL) {
			if (isAtPlusDegreeEnd()) {
				returnToStartPosition();
				changePolarPhase(AutoPolarPhase::NO_SIGNAL_REVERSE);
				LogD() << "Auto Polar: no signal, plus degree end => reverse";
			}
		}
		else {
			// Return to the starting position, (<= highest signal location), then stop.
			if (isAtMinusDegreeEnd()) {
				returnToStartPosition();
				changePolarPhase(AutoPolarPhase::CANCELED);
				LogD() << "Auto Polar: signal reverse, minus degree end => canceled, mpos: " << getPolarMotorPosition();
			}
		}
	}
	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_FirstGuess(void)
{
	if (!isRetryCountNotZero()) {
		if (!moveMotorPosition()) {
			return false;
		}
		else {
			if (isAtPlusDegreeEnd()) {
				changePolarPhase(AutoPolarPhase::RETRACE, true);
				LogD() << "Auto Polar: plus degree end => retrace";
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
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		d_ptr->retryCount = 0;

		if (currIdx < prevIdx)
		{
			changePolarPhase(AutoPolarPhase::RETRACE);
			LogD() << "Auto Polar: poor first guess => retrace";
		}
		else
		{
			changePolarPhase(AutoPolarPhase::FORWARD);
			LogD() << "Auto Polar: good first guess => forward";
		}
	}
	else
	{
		d_ptr->retryCount++;

		if (checkRetryOverToFirstGuess()) {
			changePolarPhase(AutoPolarPhase::RETRACE, true);
			LogD() << "Auto Polar: retry over first guess => retrace";
		}
	}
	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_Forward(void)
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
			if (isAtPlusDegreeEnd()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::RETRACE);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track forward, plus degree end => retrace, mpos: " << getPolarMotorPosition();
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
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: track forward, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Polar: track forward, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDegree = impl().motorDegree;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;
			impl().forwardCount += 1;
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToForward()) {
				returnToTargetPosition();
				changePolarPhase(AutoPolarPhase::RETRACE);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track forward, resists over => retrace, mpos: " << getPolarMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the searched position.
		d_ptr->retryCount += 1;
		if (checkRetryOverToForward()) {
			returnToSignalPosition();
			changePolarPhase(AutoPolarPhase::RETRACE);

			// Renew the signal ratio at the returned position.
			if (renewQualityIndex(false)) {
				impl().qualityMax = impl().quality;
				impl().signalRatioMax = impl().signalRatio;
				impl().signalMaxDegree = impl().motorDegree;
				impl().signalMaxPosition = impl().motorPosition;
			}
			LogD() << "Auto Polar: retry over forward => retrace, mpos: " << getPolarMotorPosition();
		}
	}
	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_Retrace(void)
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
			if (isAtMinusDegreeEnd()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::FORWARD2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track retrace, minus deg end => forward2, mpos: " << getPolarMotorPosition();
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
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: track retrace, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Polar: track retrace, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDegree = impl().motorDegree;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;
			impl().retraceCount += 1;
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToRetrace()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::FORWARD2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track retrace, resists over => forward2, mpos: " << getPolarMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the last optimal position. 
		d_ptr->retryCount += 1;
		if (checkRetryOverToRetrace()) {
			returnToSignalPosition();
			changePolarPhase(AutoPolarPhase::CANCELED);
			LogD() << "Auto Focus: track retrace, retry over => cancelled, mpos: " << getPolarMotorPosition();
		}
	}
	return true;
}

bool RetFocus::ScanAutoPolar::phaseAutoPolar_Forward2(void)
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
			if (isAtPlusDegreeEnd()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::RETRACE2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track forward2, plus degree end => retrace2, mpos: " << getPolarMotorPosition();
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
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: track forward2, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Polar: track forward2, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDegree = impl().motorDegree;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;

			impl().forwardCount += 1;
			if (checkLimitsOverToForward2()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::RETRACE2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track forward2, limits over => retrace2, mpos: " << getPolarMotorPosition();
			}
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToForward2()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::RETRACE2);

				// Renew the signal ratio at the returned position.
				if (renewQualityIndex(false)) {
					impl().qualityMax = impl().quality;
					impl().signalRatioMax = impl().signalRatio;
					impl().signalMaxDegree = impl().motorDegree;
					impl().signalMaxPosition = impl().motorPosition;
				}
				LogD() << "Auto Polar: track forward2, resists over => retrace2, mpos: " << getPolarMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the searched position.
		d_ptr->retryCount += 1;
		if (checkRetryOverToForward()) {
			returnToSignalPosition();
			changePolarPhase(AutoPolarPhase::RETRACE2);

			// Renew the signal ratio at the returned position.
			if (renewQualityIndex(false)) {
				impl().qualityMax = impl().quality;
				impl().signalRatioMax = impl().signalRatio;
				impl().signalMaxDegree = impl().motorDegree;
				impl().signalMaxPosition = impl().motorPosition;
			}
			LogD() << "Auto Polar: track forward2, retry over => retrace2, mpos: " << getPolarMotorPosition();
		}
	}
	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_Retrace2(void)
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
			if (isAtMinusDegreeEnd()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::COMPLETE);
				LogD() << "Auto Polar: track retrace2, target complete, qidx: " << d_ptr->quality; 
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
		changePolarPhase(AutoPolarPhase::COMPLETE);
		LogD() << "Auto Polar: track retrace2, target complete, qidx: " << d_ptr->quality;
	}
	else if (isTargetFound())
	{
		LogD() << "Auto Polar: track retrace2, qidx: " << quality << ", ratio: " << sigrate << ", mpos: " << position << ", resist: " << impl().resistCount;
		d_ptr->retryCount = 0;
		if (sigrate > impl().signalRatioMax) {
			impl().qualityMax = quality;
			impl().signalRatioMax = sigrate;
			impl().signalMaxDegree = impl().motorDegree;
			impl().signalMaxPosition = impl().motorPosition;
			d_ptr->resistCount = 0;

			impl().retraceCount += 1;
			if (checkLimitsOverToRetrace2()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::COMPLETE);
				LogD() << "Auto Focus: track retrace2, limits over => complete, mpos: " << getPolarMotorPosition();
			}
		}
		else {
			d_ptr->resistCount += 1;
			if (checkResistOverToRetrace2()) {
				returnToSignalPosition();
				changePolarPhase(AutoPolarPhase::COMPLETE);
				LogD() << "Auto Focus: track retrace2, resists over => complete, mpos: " << getPolarMotorPosition();
			}
		}
	}
	else
	{
		// If target missed, return to the last optimal position. 
		d_ptr->retryCount += 1;
		if (checkRetryOverToRetrace()) {
			returnToSignalPosition();
			changePolarPhase(AutoPolarPhase::CANCELED);
			LogD() << "Auto Focus: track retrace2, retry over => cancelled, mpos: " << getPolarMotorPosition();
		}
	}
	return true;
}

bool RetFocus::ScanAutoPolar::phaseAutoPolar_Complete(void)
{
	d_ptr->completed = true;

	if (impl().cbAutoPolar != nullptr) {
		(*impl().cbAutoPolar)(impl().completed, impl().quality, impl().motorDegree);
	}

	return true;
}


bool RetFocus::ScanAutoPolar::phaseAutoPolar_Canceled(void)
{
	d_ptr->completed = false;

	if (impl().cbAutoPolar != nullptr) {
		(*impl().cbAutoPolar)(impl().completed, impl().quality, impl().motorDegree);
	}
	return true;
}


bool RetFocus::ScanAutoPolar::changePolarPhase(AutoPolarPhase phase, bool resetPos)
{
	d_ptr->phase = phase;
	resetTargetTrackCount();

	if (resetPos) {
		if (!returnToStartPosition()) {
			LogD() << "Auto Polar: return to init failed";
			return false;
		}
	}
	else {
		updateMotorPosition();
	}
	return true;
}


void RetFocus::ScanAutoPolar::resetTargetTrackCount(void)
{
	impl().retryCount = 0;
	impl().resistCount = 0;
	impl().forwardCount = 0;
	impl().retraceCount = 0;
	return;
}


bool RetFocus::ScanAutoPolar::isRetryCountNotZero(void)
{
	return (impl().retryCount > 0);
}


bool RetFocus::ScanAutoPolar::checkResistOverToForward(void)
{
	if (impl().resistCount > AUTO_POLAR_RESISTS_FORWARD) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoPolar::checkResistOverToRetrace(void)
{
	if (impl().resistCount > AUTO_POLAR_RESISTS_RETRACE) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoPolar::checkResistOverToForward2(void)
{
	if (impl().resistCount > AUTO_POLAR_RESISTS_FORWARD) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoPolar::checkResistOverToRetrace2(void)
{
	if (impl().resistCount > AUTO_POLAR_RESISTS_RETRACE) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoPolar::checkLimitsOverToForward2(void)
{
	if (impl().forwardCount > AUTO_POLAR_FORWARDS_LIMIT2) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoPolar::checkLimitsOverToRetrace2(void)
{
	if (impl().retraceCount > AUTO_POLAR_RETRACES_LIMIT2) {
		return true;
	}
	return false;
}

bool RetFocus::ScanAutoPolar::checkRetryOverToForward(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_POLAR_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_POLAR_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_POLAR_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoPolar::checkRetryOverToRetrace(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_POLAR_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_POLAR_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_POLAR_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoPolar::checkRetryOverToFirstGuess(void)
{
	int countMax = 0;

	if (ChainSetup::isSpeedFaster()) {
		countMax = AUTO_POLAR_WAIT_TARGET_NORMAL;
	}
	else if (ChainSetup::isSpeedFastest()) {
		countMax = AUTO_POLAR_WAIT_TARGET_FASTER;
	}
	else {
		countMax = AUTO_POLAR_WAIT_TARGET_SLOWER;
	}

	if (impl().retryCount > countMax) {
		return true;
	}
	return false;
}


bool RetFocus::ScanAutoPolar::renewQualityIndex(bool next)
{
	float qidx, ratio;
	if (!obtainQualityIndexFromPreview(qidx, ratio, next)) {
		return false;
	}
	impl().quality = qidx;
	impl().signalRatio = ratio;
	return true;
}


bool RetFocus::ScanAutoPolar::isTargetFound(void)
{
	return isQualityToSignal(d_ptr->quality);
}


bool RetFocus::ScanAutoPolar::isTargetComplete(void)
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


float RetFocus::ScanAutoPolar::updateMotorMoveStep(void)
{
	float mstep;

	switch (d_ptr->phase) {
	case AutoPolarPhase::NO_SIGNAL:
	case AutoPolarPhase::NO_SIGNAL_REVERSE:
		mstep = AUTO_POLAR_MOVE_STEP_SEARCH;
		break;
	case AutoPolarPhase::FORWARD:
	case AutoPolarPhase::RETRACE:
		mstep = AUTO_POLAR_MOVE_STEP_ALIGN;
		break;
	case AutoPolarPhase::FORWARD2:
	case AutoPolarPhase::RETRACE2:
		mstep = AUTO_POLAR_MOVE_STEP_CONFIRM;
		break;
	case AutoPolarPhase::FIRST_GUESS:
	default:
		mstep = AUTO_POLAR_MOVE_STEP_GUESS;
		break;
	}

	impl().moveDegStep = mstep;
	return mstep;
}


int RetFocus::ScanAutoPolar::updateMotorPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	int mpos = motor->getPosition();
	float degree = motor->getCurrentDegree();
	impl().motorPosition = mpos;
	impl().motorDegree = degree;
	return mpos;
}


int RetFocus::ScanAutoPolar::updateMotorDirection(void)
{
	int mdir;

	switch (d_ptr->phase) {
	case AutoPolarPhase::NO_SIGNAL:
	case AutoPolarPhase::FORWARD:
	case AutoPolarPhase::FORWARD2:
		mdir = AUTO_POLAR_TO_PLUS_DEGREE;
		break;
	case AutoPolarPhase::NO_SIGNAL_REVERSE:
	case AutoPolarPhase::RETRACE:
	case AutoPolarPhase::RETRACE2:
		mdir = AUTO_POLAR_TO_MINUS_DEGREE;
		break;
	case AutoPolarPhase::FIRST_GUESS:
		mdir = AUTO_POLAR_TO_PLUS_DEGREE;
		break;
	default:
		mdir = d_ptr->motorDirection;
		break;
	}

	d_ptr->motorDirection = mdir;
	return mdir;
}


float RetFocus::ScanAutoPolar::getMotorOffsetToMove(void)
{
	updateMotorDirection();
	updateMotorMoveStep();

	float dsetp = impl().moveDegStep;
	int direct = impl().motorDirection;
	float offset = dsetp * direct;
	return offset;
}

void RetFocus::ScanAutoPolar::updateStartPosition(void)
{
	auto mpos = impl().motorPosition;
	auto mval = impl().motorDegree;
	impl().startPosition = mpos;
	impl().startDegree = mval;
	impl().targetMaxDegree = mval;
	impl().targetMaxPosition = mpos;
	impl().signalMaxDegree = mval;
	impl().signalMaxPosition = mpos;
}


bool RetFocus::ScanAutoPolar::moveMotorPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	float offset = getMotorOffsetToMove();

	// Move forward/reverse to find a signal. 
	if (!motor->updateDegreeByOffset(offset)) {
		return false;
	}

	updateMotorPosition();
	return true;
}


int RetFocus::ScanAutoPolar::getPolarMotorPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	return motor->getPosition();
}


bool RetFocus::ScanAutoPolar::returnToStartPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	int mpos = d_ptr->startPosition; 
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}


bool RetFocus::ScanAutoPolar::returnToTargetPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	int mpos = d_ptr->targetMaxPosition;
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}

bool RetFocus::ScanAutoPolar::returnToSignalPosition(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	int mpos = d_ptr->signalMaxPosition;
	if (!motor->updatePosition(mpos)) {
		return false;
	}
	updateMotorPosition();
	return true;
}


bool RetFocus::ScanAutoPolar::isMovingToPlusDegree(void)
{
	auto direct = impl().motorDirection;
	return (direct == AUTO_POLAR_TO_PLUS_DEGREE);
}


bool RetFocus::ScanAutoPolar::isMovingToMinusDegree(void)
{
	auto direct = impl().motorDirection;
	return (direct == AUTO_POLAR_TO_MINUS_DEGREE);
}


bool RetFocus::ScanAutoPolar::isAtPlusDegreeEnd(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	auto pos_end = motor->isEndOfUpperPosition();
	auto dir_deg = isMovingToPlusDegree();
	return (pos_end && dir_deg);
}


bool RetFocus::ScanAutoPolar::isAtMinusDegreeEnd(void)
{
	PolarizationMotor* motor = getMainBoard()->getPolarizationMotor();
	auto pos_end = motor->isEndOfLowerPosition();
	auto dir_deg = isMovingToMinusDegree();
	return (pos_end && dir_deg);
}


ScanAutoPolar::ScanAutoPolarImpl & RetFocus::ScanAutoPolar::impl(void)
{
	return *d_ptr;
}


OctDevice::MainBoard * RetFocus::ScanAutoPolar::getMainBoard(void)
{
	return d_ptr->board;
}
