#include "stdafx.h"
#include "ScanGrab.h"
#include "ScanFunc.h"
#include "Measure.h"
#include "Optimizer.h"
#include "RetinaTrack.h"
#include "RetinaTrack2.h"
#include "RetinaTracker.h"
#include "RetinaTracking.h"

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


struct ScanGrab::ScanGrabImpl
{
	MainBoard* board;
	FrameGrabber* grabber;
	Usb3Grabber* usb3Grab;
	Galvanometer* galvano;
	PatternPlan* pattern;
	OctProcess* process;

	bool initiated;
	bool noImageGrab;

	OctScanMeasure* scanDesc;
	ScanSpeed scanSpeed;
	float triggerTimeStep;
	uint32_t triggerTimeDelay;

	ULONGLONG tickMsrAcqStart;
	ULONGLONG tickMsrAcqEnd;

	bool repeatOfPreviews;
	int countOfPreviews;
	int countOfMeasures;
	int countOfEnfaces;
	int countOfGrabErrors;

	mutex mutexGrab;
	atomic<bool> previewRun;
	atomic<bool> measureRun;
	atomic<bool> grabberRun;
	atomic<bool> measureCanceled;
	atomic<bool> measureCompleted;
	thread threadGrab;



	ScanGrabImpl() : galvano(nullptr), grabber(nullptr), usb3Grab(nullptr), process(nullptr), pattern(nullptr), initiated(false),
		scanSpeed(ScanSpeed::Unknown), triggerTimeStep(TRAJECT_TIME_STEP_IN_US), triggerTimeDelay(TRAJECT_TRIGGER_DELAY),
		previewRun(false), measureRun(false), grabberRun(false), noImageGrab(false), 
		countOfPreviews(0), countOfMeasures(0), countOfEnfaces(0), countOfGrabErrors(0), 
		tickMsrAcqStart(0), tickMsrAcqEnd(0), repeatOfPreviews(false), measureCompleted(false), measureCanceled(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ScanGrab::ScanGrabImpl> ScanGrab::d_ptr(new ScanGrabImpl());


ScanGrab::ScanGrab()
{
}


ScanGrab::~ScanGrab()
{
}


void OctSystem::ScanGrab::initiateScanGrab(OctDevice::MainBoard* board, OctGrab::FrameGrabber* grabber)
{
	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().grabber = grabber;
	getImpl().initiated = true;
	return;
}


void OctSystem::ScanGrab::initiateScanGrab(OctDevice::MainBoard * board, OctGrab::Usb3Grabber * grabber)
{
	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().usb3Grab = grabber;
	getImpl().initiated = true;
	return;
}


bool OctSystem::ScanGrab::grabPreview(OctPattern::PatternPlan* pattern, SigChain::OctProcess* process, bool noImageGrab)
{
	getImpl().pattern = pattern;
	getImpl().process = process;
	getImpl().noImageGrab = noImageGrab;

	startScanGrab();
	return true;
}


bool OctSystem::ScanGrab::grabMeasure(bool start, OctScanMeasure* desc)
{
	getImpl().scanDesc = desc;
	closeScanGrab(start);

	if (start) {
		bool result = isMeasureCompleted();
		GlobalRegister::runScanPatternAcquiredCallback(result);
	}
	return true;
}


float OctSystem::ScanGrab::getMeasureAcquisitionTime(void)
{
	return (float)(getImpl().tickMsrAcqEnd - getImpl().tickMsrAcqStart) / 1000.0f;
}


bool OctSystem::ScanGrab::isInitiated(void)
{
	return getImpl().initiated;
}


bool OctSystem::ScanGrab::startScanGrab(void)
{
	resetCountOfEnfaceImages();
	resetCountOfMeasureImages();
	resetCountOfPreviewImages(false);
	clearGrabErrorCount();

	getImpl().previewRun = true;
	getImpl().measureRun = false;
	getImpl().measureCanceled = false;
	getImpl().measureCompleted = false;

	getImpl().threadGrab = thread{ &ScanGrab::threadGrabFunction };
	return true;
}


bool OctSystem::ScanGrab::closeScanGrab(bool measure)
{
	if (isGrabbing()) {
		getImpl().measureRun = measure; 
		getImpl().previewRun = false;

		if (getImpl().threadGrab.joinable()) {
			getImpl().threadGrab.join();
		}
	}
	return true;
}


void OctSystem::ScanGrab::cancelMeasureGrab(void)
{
	getImpl().measureCanceled = true;
	return;
}



bool OctSystem::ScanGrab::isGrabbing(void)
{
	return getImpl().grabberRun;
}


bool OctSystem::ScanGrab::isPreviewing(void)
{
	return getImpl().previewRun;
}


bool OctSystem::ScanGrab::isMeasuring(void)
{
	// return (getImpl().previewRun == false && getImpl().measureRun == true);
	return getImpl().measureRun;
}


bool OctSystem::ScanGrab::isMeasureCanceled(void)
{
	return getImpl().measureCanceled;
}


bool OctSystem::ScanGrab::isMeasureCompleted(void)
{
	return getImpl().measureCompleted;
}


void OctSystem::ScanGrab::threadGrabFunction(void)
{
	getImpl().grabberRun = true;

	bool hidden_init = true;
	bool hidden_done = false;

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getImpl().usb3Grab->startCameraAcquisition(true);
	}

	if (getImpl().board) {
		getImpl().board->prepareDevicesForScan();
	}

	while (true)
	{
		if (isGrabErrorOverDelay()) {
			this_thread::sleep_for(chrono::milliseconds(SCANNER_GRAB_ERROR_DELAY_TIME));
			if (isGrabErrorOverLimit()) {
				break;
			}
		}
		else {
			if (!processGrabHidden(hidden_init, hidden_done)) {
				increaseGrabErrorCount();
			}
			else {
				clearGrabErrorCount();
			}
		}

		if (hidden_done) {
			break;
		}
		if (!isPreviewing()) {
			break;
		}
	}


	if (isPreviewing() && !isGrabErrorOverLimit())
	{
		bool preview_init = true;
		bool enface_init = true;

		while (true)
		{
			if (isGrabErrorOverLimit()) {
				this_thread::sleep_for(chrono::milliseconds(SCANNER_GRAB_ERROR_DELAY_TIME));
				if (isGrabErrorOverLimit()) {
					break;
				}
			}
			else {
				if (!processGrabPreview(preview_init)) {
					increaseGrabErrorCount();
				}
				else {
					clearGrabErrorCount();
				}
			}

			if (isGrabErrorOverLimit()) {
				this_thread::sleep_for(chrono::milliseconds(SCANNER_GRAB_ERROR_DELAY_TIME));
				if (isGrabErrorOverLimit()) {
					break;
				}
			}
			else {
				if (!processGrabEnface(enface_init)) {
					increaseGrabErrorCount();
				}
				else {
					clearGrabErrorCount();
				}
			}

			if (!isPreviewing()) {
				break;
			}
		}
	}

	if (waitForPreviewAndEnfaceProcessed() && !isGrabErrorOverLimit())
	{
		if (isMeasuring()) {
			resetCountOfPreviewImages(true);
			if (processGrabPreviewHD()) {
				waitForPreviewAndEnfaceProcessed();
			}

			getImpl().measureCompleted = proceedToMeasure();
		}
	}

	if (getImpl().board) {
		getImpl().board->releaseDevicesForScan();
	}

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getImpl().usb3Grab->cancelCameraAcquisition();
	}

	getImpl().grabberRun = false;
	return;
}


bool OctSystem::ScanGrab::waitForPreviewAndEnfaceProcessed(void)
{
	//CppUtil::ClockTimer::start();

	bool res = false;
	for (int i = 0; i < SCANNER_WAIT_PREVIEW_PROCESSED_COUNT_MAX; i++) {
		this_thread::sleep_for(chrono::milliseconds(SCANNER_WAIT_PREVIEW_PROCESSED_DELAY));
		if (!getProcess()->isBusying()) {
			res = true;
			break;
		}
	}

	//double msec = CppUtil::ClockTimer::elapsedMsec();
	//LogD() << "Preview and enface processed elapsed: " << msec;

	if (!res) {
		LogD() << "Preview and enface processed timeout!";
	}
	return res;
}


void OctSystem::ScanGrab::notifyMeasureFrameReceived(void)
{
	getProcess()->notifyMeasureDataReceived();
	return;
}


bool OctSystem::ScanGrab::checkIfScanOptimizing(void)
{
	if (Optimizer::isOptimizing()) {
		return true;
	}
	else {
		return false;
	}
}


bool OctSystem::ScanGrab::processGrabHidden(bool& initiate, bool& completed)
{
	PreviewScan& preview = getPattern()->getPreviewScan();
	int numFrames = preview.getNumberOfPatternFrames();

	/*
	static int count = 0;
	if (count == 0) {
	CppUtil::ClockTimer::start();
	}
	*/

	const int HIDDEN_LINES_FOR_BACKGROUND = 5;
	static int frameIdx = 0;
	static int hiddens = 0;

	if (frameIdx >= numFrames || initiate) {
		frameIdx = 0;
		resetCountOfPreviewImages(false);

		if (initiate) {
			hiddens = 0;
			initiate = false;
		}
		else {
			if (++hiddens > HIDDEN_LINES_FOR_BACKGROUND) {
				// LogD() << "Preview hidden lines completed";
				completed = true;
				return true;
			}
		}
	}

	if (frameIdx >= 0 && frameIdx < numFrames)
	{
		auto line = preview.getPatternFrame(frameIdx)->getLineTraceFirst();

		if (line->isHidden()) {
			if (getImpl().noImageGrab) {
				if (!triggerLineTracesToPreview(frameIdx, false)) {
					return false;
				}
			}
			else {
				if (!assignGrabBufferToPreview(frameIdx, 0, false)) {
					return false;
				}

				if (!triggerLineTracesToPreview(frameIdx, false)) {
					return false;
				}

				int numLines = processGrabBufferToPreview(frameIdx, 0, false);
				if (numLines <= 0) {
					return false;
				}
			}
			if (!isPreviewing()) {
				return false;
			}
			// LogD() << "Preview hidden line found";
		}
		frameIdx++;
	}

	/*
	count++;
	if (count > 100) {
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Preview Grab 100 elapsed: " << msec;
	count = 0;
	}
	*/
	return true;
}


bool OctSystem::ScanGrab::processGrabPreview(bool& initiate)
{
	PreviewScan& preview = getPattern()->getPreviewScan();
	int numFrames = preview.getNumberOfPatternFrames();

	/*
	static int count = 0;
	if (count == 0) {
	CppUtil::ClockTimer::start();
	}
	*/

	static int frameIdx = 0;
	static int buffStart = 0;
	if (frameIdx >= numFrames || initiate) {
		frameIdx = 0;
		buffStart = 0;
		resetCountOfPreviewImages(false);
		initiate = false;
	}

next_frame:

	if (frameIdx >= 0 && frameIdx < numFrames)
	{
		auto line = preview.getPatternFrame(frameIdx)->getLineTraceFirst();
		if (line->isHidden() || line->isLineHD()) {
			frameIdx++;
			goto next_frame;
		}

		if (getImpl().noImageGrab) {
			if (!triggerLineTracesToPreview(frameIdx, false)) {
				return false;
			}
		}
		else {
			if (!assignGrabBufferToPreview(frameIdx, buffStart, false)) {
				return false;
			}

			if (!triggerLineTracesToPreview(frameIdx, false)) {
				return false;
			}

			int numLines = processGrabBufferToPreview(frameIdx, buffStart, false);
			if (numLines <= 0) {
				return false;
			}
			buffStart += numLines;
		}
		if (!isPreviewing()) {
			return false;
		}
		frameIdx++;
	}

	/*
	count++;
	if (count > 100) {
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Preview Grab 100 elapsed: " << msec;
	count = 0;
	}
	*/
	return true;
}


bool OctSystem::ScanGrab::processGrabEnface(bool& initiate)
{
	EnfaceScan& enface = getPattern()->getEnfaceScan();
	int numFrames = enface.getNumberOfPatternFrames();

	if (checkIfScanOptimizing()) {
		return true;
	}

	static int frameIdx = 0;
	/*
	if (frameIdx == 0) {
		CppUtil::ClockTimer::start();
	}
	*/
	if (frameIdx >= numFrames || initiate) {
		frameIdx = 0;
		resetCountOfEnfaceImages();
		initiate = false;
		// double msec = CppUtil::ClockTimer::elapsedMsec();
		// LogD() << "Grab enface elapsed: " << msec;
	}


	if (frameIdx >= 0 && frameIdx < numFrames)
	{
		if (getImpl().noImageGrab) {
			if (!triggerLineTracesToEnface(frameIdx)) {
				return false;
			}
		}
		else {
			if (!assignGrabBufferToEnface(frameIdx, 0)) {
				return false;
			}

			if (!triggerLineTracesToEnface(frameIdx)) {
				return false;
			}

			int numLines = processGrabBufferToEnface(frameIdx, 0);
			if (numLines <= 0) {
				return false;
			}
		}

		if (!isPreviewing()) {
			return false;
		}

		frameIdx++;
	}

	/*
	count++;
	if (count > 100) {
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Enface Grab 100 elapsed: " << msec;
	count = 0;
	}
	*/
	return true;
}


bool OctSystem::ScanGrab::processGrabPreviewHD(void)
{
	CppUtil::ClockTimer::start();
	getImpl().tickMsrAcqStart = GetTickCount64();

	/*
	if (getImpl().scanDesc->getPattern().isAngioScan()) {
		return true;
	}
	*/

	PreviewScan& preview = getPattern()->getPreviewScan();
	int numFrames = preview.getNumberOfPatternFrames();
	int buffStart = 0;
	int numLines = 0;

	for (int frameIdx = 0; frameIdx < numFrames; frameIdx++)
	{
		auto line = preview.getPatternFrame(frameIdx)->getLineTraceFirst();
		LogD() << "preview frame: " << frameIdx << ", lineHD: " << line->isLineHD();

		if (!line->isLineHD()) {
			continue;
		}

		if (getImpl().noImageGrab) {
			return false;
		}

		if (!assignGrabBufferToPreview(frameIdx, buffStart, true)) {
			return false;
		}

		if (!triggerLineTracesToPreview(frameIdx, true)) {
			return false;
		}

		/*
		if (frameIdx == (numFrames - 1)) {
			if (!ScanFunc::uploadPatternProfiles(*getPattern(), true)) {
				return false;
			}
		}
		*/

		int numLines = processGrabBufferToPreview(frameIdx, buffStart, true);
		if (numLines <= 0) {
			return false;
		}

		buffStart += numLines;
	}

	getImpl().tickMsrAcqEnd = GetTickCount64();
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Grab preview HD elapsed: " << msec;
	return true;
}


bool OctSystem::ScanGrab::proceedToMeasure(void)
{
	LogD() << "Proceed to measure scan";

	if (!isMeasuring() || isGrabErrorOverLimit()) {
		return false;
	}

	// The line camera's speed cannot be set while it is grabbing. 
	// the measurement scan is performed at the same speed as the preview scan.
	/*
	if (!updateScanSpeedToMeasure()) {
		LogD() << "Failed to update scan speed to measure";
		return false;
	}
	*/

	if (getImpl().scanDesc->useNoImageGrab()) {
		return false;
	}

	if (getImpl().scanDesc->usePhaseEnface()) {
		if (!ScanFunc::updatePatternBuffers(*getPattern(), true)) {
			return false;
		}
	}

	ScanFunc::prepareChainSetup(getImpl().scanDesc->getPattern().getPatternDomain(), 
								getImpl().scanDesc->getPattern().getPatternName(),
								getImpl().scanDesc->getPattern().getScanSpeed(), 
								true);

	bool result = false;
	if (processGrabMeasure3()) {
		result = true;
	}

	LogD() << "Process grab measure result: " << result;
	getImpl().measureRun = false;
	return result;
}


bool OctSystem::ScanGrab::processGrabMeasure(void)
{
	int numFrames = getPattern()->getMeasureScan().getNumberOfPatternFrames();
	int buffStart = 0;
	int buffCount = 0;
	int backLines = 0;
	int waitCount = 0;
	int trackResetCount = 0;
	bool trackExpired = false;
	bool targetRemaped = false;
	bool grabCanceled = false;

	std::int16_t offsetX, offsetY;
	std::int16_t startX, startY;
	if (!getMainboard()->getGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	CppUtil::ClockTimer::start();
	getImpl().tickMsrAcqStart = GetTickCount64();
	RetinaTrack2::setTargetDisplaced(false);
	RetinaTrack2::setPendingScanning(false);

	for (int frameIdx = 0; frameIdx < numFrames; frameIdx++)
	{
		auto frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
		auto trace = frame->getLineTraceFirst();
		LogD() << "Grab Measure, frameIdx: " << frameIdx << ", traceId: " << trace->getTraceId();
		if (!ScanFunc::uploadTraceProfile(*trace)) {
			return false;
		}

	retrace:
		if (RetinaTrack2::isTargetRegistered()) {
			if (targetRemaped) {
				float centerX, centerY;
				RetinaTrack2::getScanCenterOffset(centerX, centerY);
				LogD() << "Grab measure center shift: " << centerX << ", " << centerY;
				if (!getMainboard()->setGalvanoDynamicOffset(centerX, centerY, false)) {
					return false;
				}
			}
		}

		int index = getIndexOfMeasureImageNext(false);
		RetinaTrack2::setLastImageIndexGrabbed(index);

		if (GlobalSettings::useUsbCmosCameraEnable()) {
			getUsb3Grab()->setStartMeasureBuffer(buffStart);
		}
		else {
			getGrabber()->setStartMeasureBuffer(buffStart);
		}

		int lines = assignGrabBufferToMeasure(frameIdx, buffCount);
		if (lines <= 0) {
			return false;
		}
		buffCount += lines;

		if (!triggerLineTracesToMeasure(frameIdx, false, offsetX, offsetY, startX, startY)) {
			return false;
		}

		int numLines = processGrabBufferToMeasure(frameIdx, buffStart);
		if (numLines <= 0) {
			return false;
		}
		buffStart += numLines;

		notifyMeasureFrameReceived();

		if (frameIdx < (numFrames - 1)) {
			if (isMeasureCanceled()) {
				grabCanceled = true;
				LogD() << "Measure scan grabbing is being canceled";
				// Exit grabbing loop.
				break;
			}

			if (!trackExpired) {
				if (RetinaTrack2::isTargetDisplaced()) {
					getGalvanometer()->slewMoveXY(startX, startY);
					LogD() << "Holding grab frames, return to " << startX << ", " << startY;

					RetinaTrack2::setPendingScanning(true);
					do {
						RetinaTrack2::setTargetDisplaced(false);
						this_thread::sleep_for(chrono::milliseconds(RETINA_TRACK_FRAME_DELAY_TIME));
						waitCount++;

						if (waitCount > RETINA_TRACK_FRAME_COUNT_TO_CANCEL) {
							trackExpired = true;
							LogD() << "Holding grabber timed out, tracking expired!";
							break;
						}
						if (waitCount == RETINA_TRACK_FRAME_COUNT_TO_RESET) {
							LogD() << "Registering new target...";
							RetinaTrack2::startTracking();
							trackResetCount++;
						}
						if (waitCount % 10 == 0) {
							LogD() << "Holding grabber...";
						}

						if (isMeasureCanceled()) {
							grabCanceled = true;
							LogD() << "Measure scan grabbing is being canceled";
							break;
						}
					} while (!RetinaTrack2::isTargetLocated());
					RetinaTrack2::setPendingScanning(false);

					if (grabCanceled) {
						// Exit grabbing loop.
						break;
					}
					else {
						if (!trackExpired) {
							backLines = numLines;
							buffCount = max((buffCount - backLines), 0);
							buffStart = max((buffStart - backLines), 0);
							retreatIndexOfMeasureImage(backLines);
							targetRemaped = true;
							waitCount = 0;
							LogD() << "Retreat grab frames, frameIdx: " << frameIdx << ", buffCount: " << buffCount << ", buffStart: " << buffStart << ", waitCount: " << waitCount;
							goto retrace;
						}
					}
				}
			}
		}
	}

	getImpl().tickMsrAcqEnd = GetTickCount64();
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Acquisition of measure frames elapsed: " << msec;

	if (!getMainboard()->setGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Grab measure elapsed: " << msec;

	notifyMeasureFrameReceived();

	bool result = grabCanceled ? false : true;
	return result;
}


bool OctSystem::ScanGrab::processGrabMeasure2(void)
{
	int buffStart = 0;
	int buffCount = 0;
	int backLines = 0;
	int trackFaultCount = 0;
	int trackRetryCount = 0;
	int targetMissCount = 0;

	bool grabCanceled = false;
	bool trackRescan = false;

	const int TRACK_FAULT_MAX = 5; 
	const int TRACK_RETRY_MAX = 5;

	std::int16_t offsetX = 0, offsetY = 0;
	std::int16_t startX = 0, startY = 0;
	if (!getMainboard()->getGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	LogD() << "Process grab measure, initial galvano offsetX: " << offsetX << ", " << offsetY;

	auto* rett = RetinaTracker::getInstance();
	int numFrames = getPattern()->getMeasureScan().getNumberOfPatternFrames();

	CppUtil::ClockTimer::start();
	getImpl().tickMsrAcqStart = GetTickCount64();

	for (int frameIdx = 0; frameIdx < numFrames; frameIdx++)
	{
		auto frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
		auto trace = frame->getLineTraceFirst();

		LogD() << "----------------------------------------------------------";
		LogD() << "Grab Measure, frameIdx: " << frameIdx << ", traceId: " << trace->getTraceId();
		if (!ScanFunc::uploadTraceProfile(*trace)) {
			LogD() << "Failed to upload trace profile";
			return false;
		}

	retrace:
		int index = getIndexOfMeasureImageNext(false);
		rett->setGrabbingImageIndex(index);

		CppUtil::ClockTimer::start();

		if (GlobalSettings::useUsbCmosCameraEnable()) {
			getUsb3Grab()->setStartMeasureBuffer(buffStart);
		}
		else {
			getGrabber()->setStartMeasureBuffer(buffStart);
		}

		int lines = assignGrabBufferToMeasure(frameIdx, buffCount);
		if (lines <= 0) {
			return false;
		}
		buffCount += lines;

		double msec = CppUtil::ClockTimer::elapsedMsec();
		LogD() << "Frame buffer elapsed: " << msec;


		// Monitor target movements during scanning data acquistion.
		if (rett->isTrackingPrepared()) {
			bool init = rett->isTrackingStarted();
			rett->beginTrackRecording(init);
		}

		if (!triggerLineTracesToMeasure(frameIdx, false, offsetX, offsetY, startX, startY)) {
			return false;
		}

		int numLines = processGrabBufferToMeasure(frameIdx, buffStart);
		if (numLines <= 0) {
			return false;
		}
		buffStart += numLines;

		if (rett->isTrackingPrepared()) {
			rett->pauseTrackRecording();
		}

		msec = CppUtil::ClockTimer::elapsedMsec();
		LogD() << "Frame trigger elapsed: " << msec;

		if (frameIdx < (numFrames - 1)) {
			if (isMeasureCanceled()) {
				grabCanceled = true;
				LogD() << "Measure scan grabbing is being canceled";
				break;
			}

			LogD() << "Tracking state: " << rett->getTrackingState() << ", retry: " << trackRetryCount << ", fault: " << trackFaultCount;

			trackRescan = false;
			if (rett->isTrackingStarted()) {
				// Tracking target.
				if (!rett->isTrackRecordAvailable(1)) {
					LogD() << "Tracking record is not avaliable";
					// trackRetryCount += 1;
					trackRescan = true;
				}
				else if (rett->isTargetIdentified()) {
					if (!rett->isTargetDisplaced()) {
						// Continue to next frame without galvano offset adjustment.
						trackRetryCount = 0;

						float pos_x, pos_y;
						rett->getTargetPosition(pos_x, pos_y);
						rett->updateTargetMovement(pos_x, pos_y);

						float offs_x, offs_y;
						rett->getScanningOffset(offs_x, offs_y);
						LogD() << "Tracking target position: " << pos_x << ", " << pos_y << " => galvano offset: " << offs_x << ", " << offs_y;
						if (!getMainboard()->setGalvanoDynamicOffset(offs_x, offs_y, false)) {
							return false;
						}
					}
					else {
						// Target was drifted or displaced during the frame scanning.
						LogD() << "Tracking traget displaced, retry: " << trackRetryCount;
						if (trackRetryCount < TRACK_RETRY_MAX) {
							trackRetryCount += 1;
							trackRescan = true;
						}
						else {
							// Target is not stable, retry limit reached, then continue to next frame without rescanning.
						}

						float pos_x, pos_y;
						rett->getTargetPosition(pos_x, pos_y);
						rett->updateTargetMovement(pos_x, pos_y);

						float offs_x, offs_y;
						rett->getScanningOffset(offs_x, offs_y);
						LogD() << "Tracking target position: " << pos_x << ", " << pos_y << " => galvano offset: " << offs_x << ", " << offs_y;
						if (!getMainboard()->setGalvanoDynamicOffset(offs_x, offs_y, false)) {
							return false;
						}
					}
					trackFaultCount = 0;
				}
				else {
					// Target not valid or mismatched.
					if (trackFaultCount < TRACK_FAULT_MAX) {
						trackFaultCount += 1;
						if (rett->isEyelidCovered()) {
							trackRescan = true;
							LogD() << "Eyelid covered on tracking, fault: " << trackFaultCount;
						}
						else {
							rett->requestRegistration(false);
							if (rett->waitForRegistration()) {
								float offs_x, offs_y;
								rett->getScanningOffset(offs_x, offs_y);
								LogD() << "Tracking new target => galvano offset: " << offs_x << ", " << offs_y;
								if (!getMainboard()->setGalvanoDynamicOffset(offs_x, offs_y, false)) {
									return false;
								}
								rett->resumeRetinaTracking();
								trackRescan = true;
							}
							else {
								LogD() << "Tracking target missed, fault: " << trackFaultCount;
								rett->resumeRetinaTracking();
								trackRescan = true;
							}
						}
					}
					else {
						// Fault limit reached, then continue to next frame without rescanning.
						rett->pauseRetinaTracking();
						trackFaultCount = 0;
						trackRescan = true;
						LogD() << "Tracking target paused, fault: " << trackFaultCount;

						rett->updateTargetMovement(0.0f, 0.0f);
						float offs_x, offs_y;
						rett->getScanningOffset(offs_x, offs_y);
						LogD() << "Tracking target reset => galvano offset: " << offs_x << ", " << offs_y;
						if (!getMainboard()->setGalvanoDynamicOffset(offs_x, offs_y, false)) {
							return false;
						}
					}
				}
			}
			else if (rett->isTrackingPrepared() || rett->isTrackingPaused()) {
				// Tracking started without target registration or paused. 
				if (rett->isEyelidCovered()) {
					if (trackFaultCount < TRACK_FAULT_MAX) {
						trackFaultCount += 1;
						trackRescan = true;
						LogD() << "Eyelid covered on tracking, fault: " << trackFaultCount;
					}
				}
				else {
					trackFaultCount = 0;
				}
			}

			if (trackRescan) {
				backLines = numLines;
				buffCount = max((buffCount - backLines), 0);
				buffStart = max((buffStart - backLines), 0);
				retreatIndexOfMeasureImage(backLines);
				LogD() << "Retreat grab frames, frameIdx: " << frameIdx << ", buffCount: " << buffCount << ", buffStart: " << buffStart;
				goto retrace;
			}
			else {
				// Notify measure frame received as a result of scanning.
				notifyMeasureFrameReceived();
			}
		}
	}

	getImpl().tickMsrAcqEnd = GetTickCount64();
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Acquisition of measure frames elapsed: " << msec;

	if (!getMainboard()->setGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	notifyMeasureFrameReceived();
	bool result = grabCanceled ? false : true;

	msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Grab measure elapsed: " << msec;
	return result;
}


bool OctSystem::ScanGrab::processGrabMeasure3(void)
{
	int buffStart = 0;
	int buffCount = 0;
	int backLines = 0;

	bool grabCanceled = false;
	bool isRescanning = false;

	int trackRetryCount = 0;
	int trackAlignCount = 0;
	int trackResetCount = 0;

	std::int16_t offsetX = 0, offsetY = 0;
	std::int16_t startX = 0, startY = 0;
	if (!getMainboard()->getGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	LogD() << "Process grab measure, initial galvano offsetX: " << offsetX << ", " << offsetY;

	CppUtil::ClockTimer::start();
	getImpl().tickMsrAcqStart = GetTickCount64();

	auto* rett = RetinaTracking::getInstance();
	int numFrames = getPattern()->getMeasureScan().getNumberOfPatternFrames();

	for (int frameIdx = 0; frameIdx < numFrames; frameIdx++)
	{
		auto frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
		auto trace = frame->getLineTraceFirst();

		LogD() << "----------------------------------------------------------";
		LogD() << "Grab Measure, frameIdx: " << frameIdx << ", traceId: " << trace->getTraceId();
		if (!ScanFunc::uploadTraceProfile(*trace)) {
			LogD() << "Failed to upload trace profile";
			return false;
		}

	retrace:
		isRescanning = false;
	    int index = getIndexOfMeasureImageNext(false);
		rett->setGrabbingImageIndex(index);

		if (GlobalSettings::useUsbCmosCameraEnable()) {
			getUsb3Grab()->setStartMeasureBuffer(buffStart);
		}
		else {
			getGrabber()->setStartMeasureBuffer(buffStart);
		}

		int lines = assignGrabBufferToMeasure(frameIdx, buffCount);
		if (lines <= 0) {
			return false;
		}
		buffCount += lines;

		rett->beginRecordingTarget(true);
		if (!triggerLineTracesToMeasure(frameIdx, false, offsetX, offsetY, startX, startY)) {
			return false;
		}

		int numLines = processGrabBufferToMeasure(frameIdx, buffStart);
		if (numLines <= 0) {
			return false;
		}
		buffStart += numLines;
		rett->closeRecordingTarget();

		if (frameIdx < (numFrames - 1)) {
			if (isMeasureCanceled()) {
				grabCanceled = true;
				LogD() << "Measure scan grabbing is being canceled";
				break;
			}
			
			if (rett->isTrackingInProgress())
			{
				float offsX, offsY;
				rett->updateScanningRegionByMotion();
				rett->getScanningCenterPosition(offsX, offsY);
				if (!getMainboard()->setGalvanoDynamicOffset(offsX, offsY, false)) {
					return false;
				}

				if (rett->isRecordingMotionFree()) {
					trackRetryCount = 0;
					LogD() << "Tracking target, motion free during acqusition";
				}
				else {
					trackRetryCount += 1;
					isRescanning = true;
					LogD() << "Tracking targe aligned, try rescanning, retry: " << trackRetryCount;
				}
				/*
				if (rett->isRecordingMotionFree()) {
					trackRetryCount = 0;
					LogD() << "Tracking target, motion free during acqusition";
				}
				else {
					if (rett->waitForTargetAlignment()) {
						float offsX, offsY;
						rett->updateScanningRegionByMotion();
						rett->getScanningCenterPosition(offsX, offsY);
						if (!getMainboard()->setGalvanoDynamicOffset(offsX, offsY, false)) {
							return false;
						}
						trackRetryCount += 1;
						isRescanning = true;
						LogD() << "Tracking targe aligned, try rescanning, retry: " << trackRetryCount;
					}
					else {
						rett->pauseRetinaTracking();
					}
				}
				*/
			}
			/*
			else if (rett->isTrackingReady())
			{
				if (rett->isRecordingEyelidCovered()) {
				}
			}
			*/

			if (trackRetryCount > 999) {
				rett->cancelRetinaTracking(false);
				isRescanning = false;
				LogD() << "Tracking retry count expired";
			}

			if (isRescanning) {
				backLines = numLines;
				buffCount = max((buffCount - backLines), 0);
				buffStart = max((buffStart - backLines), 0);
				retreatIndexOfMeasureImage(backLines);
				LogD() << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>";
				LogD() << "Retreat grab frames, frameIdx: " << frameIdx << ", buffCount: " << buffCount << ", buffStart: " << buffStart;
				goto retrace;
			}
			else {
				// Notify measure frame received as a result of scanning.
				notifyMeasureFrameReceived();
			}
		}
	}

	getImpl().tickMsrAcqEnd = GetTickCount64();
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Acquisition of measure frames elapsed: " << msec;

	if (!getMainboard()->setGalvanoDynamicOffset(offsetX, offsetY)) {
		return false;
	}

	notifyMeasureFrameReceived();
	bool result = grabCanceled ? false : true;

	msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Grab measure elapsed: " << msec;
	return result;
}


bool OctSystem::ScanGrab::assignGrabBufferToPreview(int frameIdx, int buffIdx, bool repeat)
{
	PatternFrame* frame = getPattern()->getPreviewScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to preview buffer, frameIdx: " << frameIdx;
		return false;
	}

	int sizeY = frame->getLateralSizeFirst();
	// int numLines = frame->getLineTraceListSize();
	int numLines = frame->getNumberOfLineTraces(repeat);
	
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getUsb3Grab()->setSourceImageSizeY(sizeY);
		getUsb3Grab()->clearGrabbedPreviewBuffer();

		if (repeat) {
			LogD() << "Assign usb3 buffers to preview-hd, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx << ", numLines: " << numLines;
		}

		bool res = getUsb3Grab()->grabPreviewProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Process grab preview usb3 buffers failed!";
		}
		return res;
	}
	else {
		getGrabber()->setSourceImageSizeY(sizeY);
		getGrabber()->clearGrabbedPreviewBuffer();

		if (repeat) {
			LogD() << "Assign grab buffers to preview-hd, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx << ", numLines: " << numLines;
		}

		bool res = getGrabber()->grabPreviewProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Process grab preview buffers failed!";
		}
		return res;
	}
}


int OctSystem::ScanGrab::assignGrabBufferToMeasure(int frameIdx, int buffIdx)
{
	PatternFrame* frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to measure buffer, frameIdx: " << frameIdx;
		return false;
	}

	int sizeY = frame->getLateralSizeFirst();
	int numLines = frame->getNumberOfLineTraces(true);

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getUsb3Grab()->setSourceImageSizeY(sizeY);
		getUsb3Grab()->clearGrabbedMeasureBuffer();

		LogD() << "Assign grab measure usb3 buffers, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx << ", numLines: " << numLines;

		bool res = getUsb3Grab()->grabMeasureProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Process grab measure usb3 buffers failed!";
		}
	}
	else {
		// if (frameIdx == 0) {
		getGrabber()->setSourceImageSizeY(sizeY);
		getGrabber()->clearGrabbedMeasureBuffer();
		// }
		LogD() << "Assign grab measure buffers, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx << ", numLines: " << numLines;

		bool res = getGrabber()->grabMeasureProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Process grab measure buffers failed!";
		}
	}
	return numLines;
}


int OctSystem::ScanGrab::assignGrabBufferOfMeasureFrames(int buffIdx)
{
	int numFrames = getPattern()->getMeasureScan().getNumberOfPatternFrames();
	int numLines = 0;

	for (int frameIdx = 0; frameIdx < numFrames; frameIdx++)
	{
		PatternFrame* frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
		if (frame == nullptr) {
			LogD() << "Invalid frame to measure buffer, frameIdx: " << frameIdx;
			return false;
		}

		if (frameIdx == 0) {
			int sizeY = frame->getLateralSizeFirst();
			getGrabber()->setSourceImageSizeY(sizeY);
			getGrabber()->clearGrabbedMeasureBuffer();
		}

		numLines += frame->getNumberOfLineTraces(true);
	}

	LogD() << "Process grab measure buffers, numLines: " << numLines;

	bool res = getGrabber()->grabMeasureProcess(buffIdx, numLines);
	if (!res) {
		LogD() << "Process grab measure buffers failed!";
	}
	return numLines;
}


bool OctSystem::ScanGrab::assignGrabBufferToEnface(int frameIdx, int buffIdx)
{
	PatternFrame* frame = getPattern()->getEnfaceScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to enface buffer, frameIdx: " << frameIdx;
		return false;
	}

	int sizeY = frame->getLateralSizeFirst();
	int numLines = frame->getLineTraceListSize();

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getUsb3Grab()->setSourceImageSizeY(sizeY);
		getUsb3Grab()->clearGrabbedEnfaceBuffer();

		bool res = getUsb3Grab()->grabEnfaceProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Grab enface usb3 buffers failed!";
		}
		return res;
	}
	else {
		getGrabber()->setSourceImageSizeY(sizeY);
		getGrabber()->clearGrabbedEnfaceBuffer();
		// LogD() << "Assign grab enface buffers, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx << ", numLines: " << numLines;

		bool res = getGrabber()->grabEnfaceProcess(buffIdx, numLines);
		if (!res) {
			LogD() << "Grab enface buffers failed!";
		}
		return res;
	}
}


bool OctSystem::ScanGrab::triggerLineTracesToPreview(int frameIdx, bool repeat)
{
	PatternFrame* frame = getPattern()->getPreviewScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to trigger preview, frameIdx: " << frameIdx;
		return false;
	}

	int offsetX = frame->getPositionOffsetX();
	int offsetY = frame->getPositionOffsetY();
	// int numLines = frame->getLineTraceListSize();
	int numLines = frame->getNumberOfLineTraces(repeat);

	LineTrace* line = frame->getLineTraceFirst();
	if (line == nullptr) {
		LogD() << "Empty frame to trigger preview, frameIdx: " << frameIdx;
		return false;
	}

	short firstX = line->getPositionX(0);
	short firstY = line->getPositionY(0);
	bool res = true;

	if (repeat) {
		LogD() << "Trigger preview-hd lines, frameIdx: " << frameIdx << ", numLines: " << numLines;
		LogD() << "First x, y: " << firstX << ", " << firstY << ", Offset x, y: " << offsetX << ", " << offsetY;
	}

	getImpl().galvano->slewMoveXY(firstX, firstY);
	res = triggerTraceProfile(*line, numLines, offsetX, offsetY);
	if (!res) {
		LogD() << "Trigger trace profile to preview failed!";
	}
	return res;
}


bool OctSystem::ScanGrab::triggerLineTracesToMeasure(int frameIdx, bool inferOffset, std::int16_t dynOffsetX, std::int16_t dynOffsetY, std::int16_t& startX, std::int16_t& startY)
{
	PatternFrame* frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to trigger measure, frameIdx: " << frameIdx;
		return false;
	}

	LineTrace* line = frame->getLineTraceFirst();
	if (line == nullptr) {
		LogD() << "Empty frame to trigger measure, frameIdx: " << frameIdx;
		return false;
	}

	bool repeat = !(line->isRasterX() || line->isRasterY());
	int offsetX = frame->getPositionOffsetX();
	int offsetY = frame->getPositionOffsetY();
	int numLines = frame->getNumberOfLineTraces(repeat);


	short firstX = line->getPositionX(0);
	short firstY = line->getPositionY(0);
	bool res = true;

	if (inferOffset) {
		if (frameIdx >= TRACE_ID_MAX_LINES) {
			int startIdx = frameIdx % TRACE_ID_MAX_LINES;
			PatternFrame* sframe = getPattern()->getMeasureScan().getPatternFrame(startIdx);
			LineTrace* sline = sframe->getLineTraceFirst();
			auto movePosX = firstX - sline->getPositionX(0) + dynOffsetX;
			auto movePosY = firstY - sline->getPositionY(0) + dynOffsetY;

			if (startIdx == 0) {
				if (!getMainboard()->setGalvanoDynamicOffset(movePosX, movePosY)) {
					return false;
				}
			}
		}
	}

	LogD() << "Trigger measure lines, frameIdx: " << frameIdx << ", numLines: " << numLines << ", type: " << line->getTraceTypeStr();
	LogD() << "First x, y: " << firstX << ", " << firstY << ", Offset x, y: " << offsetX << ", " << offsetY;

	getImpl().galvano->slewMoveXY(firstX, firstY);
	LogD() << "Slew moved x, y: " << firstX << ", " << firstY;

	//double msec1 = CppUtil::ClockTimer::elapsedMsec();
	//LogD() << "SlewMoveXY elapsed: " << msec1;

	res = triggerTraceProfile(*line, numLines, offsetX, offsetY);
	if (!res) {
		LogD() << "Trigger trace profile to measure failed!";
	}
	LogD() << "Trace profile triggered, offset x, y:" << offsetX << ", " << offsetY;

	//double msec2 = CppUtil::ClockTimer::elapsedMsec();
	//LogD() << "TraceProfile elapsed: " << msec2;

	startX = firstX;
	startY = firstY;
	return res;
}


bool OctSystem::ScanGrab::triggerLineTracesToEnface(int frameIdx)
{
	PatternFrame* frame = getPattern()->getEnfaceScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to trigger enface, frameIdx: " << frameIdx;
		return false;
	}

	int offsetX = frame->getPositionOffsetX();
	int offsetY = frame->getPositionOffsetY();
	int numLines = frame->getLineTraceListSize();

	LineTrace* line = frame->getLineTraceFirst();
	if (line == nullptr) {
		LogD() << "Empty frame to trigger enface, frameIdx: " << frameIdx;
		return false;
	}

	short firstX = line->getPositionX(0);
	short firstY = line->getPositionY(0);
	bool res = true;

	// LogD() << "Trigger preview lines, frameIdx: " << frameIdx << ", numLines: " << numLines;
	// LogD() << "First x, y: " << firstX << ", " << firstY << ", Offset x, y: " << offsetX << ", " << offsetY;

	getImpl().galvano->slewMoveXY(firstX, firstY);
	res = triggerTraceProfile(*line, numLines, offsetX, offsetY);
	if (!res) {
		LogD() << "Trigger trace profile to enface failed!";
	}
	return res;
}


bool OctSystem::ScanGrab::triggerTraceProfile(OctPattern::LineTrace & line, short numLines, short offsetX, short offsetY)
{
	if (!isInitiated()) {
		return false;
	}

	Galvanometer* galvano = getImpl().galvano;

	int traceId = line.getTraceId();
	bool res = false;

	if (line.isLine() || line.isCircle()) {
		res = galvano->scanMoveXY(traceId);
	}
	else if (line.isRasterX()) {
		//res = galvano->scanRepeatX(traceId, numLines, offsetX, offsetY);
		return galvano->scanRepeatY(traceId, numLines, offsetX, offsetY);
	}
	else if (line.isRasterY()) {
		//res = galvano->scanRepeatY(traceId, numLines, offsetX, offsetY);
		return galvano->scanRepeatX(traceId, numLines, offsetX, offsetY);
	}
	else if (line.isRasterFastX()) {
		//return galvano->scanRepeatFastX();
		//return galvano->scanMoveXY(traceId);
		res = galvano->scanRepeatFastY(traceId, numLines, offsetX, offsetY);
	}
	else if (line.isRasterFastY()) {
		//	return galvano->scanRepeatFastY();
		res = galvano->scanRepeatFastX(traceId, numLines, offsetX, offsetY);
	}
	return res;
}


int OctSystem::ScanGrab::processGrabBufferToPreview(int frameIdx, int buffIdx, bool repeat)
{
	int numLines = 0;

	PatternFrame* frame = getPattern()->getPreviewScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to grab preview, frameIdx: " << frameIdx;
		return 0;
	}

	bool res = false;

	// numLines = frame->getLineTraceListSize();
	numLines = frame->getNumberOfLineTraces(repeat);

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		res = getUsb3Grab()->grabPreviewProcessWait(buffIdx, numLines);
		if (res) {
			if (repeat) {
				LogD() << "Preview-hd buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
			}
		}
		else {
			LogD() << "Grab preview buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
			return 0;
		}
		return numLines;
	}
	else {
		if (getGrabber()->waitForFirstGrabOfPreviewBuffer(buffIdx)) {
			res = getGrabber()->grabPreviewProcessWait(buffIdx, numLines);
			if (res) {
				if (repeat) {
					LogD() << "Preview-hd buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
				}
			}
			else {
				LogD() << "Grab preview buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
				return 0;
			}
			return numLines;
		}
		else {
			LogD() << "Grab the first preview buffer failed!, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx;
			return 0;
		}
	}
}


int OctSystem::ScanGrab::processGrabBufferToMeasure(int frameIdx, int buffIdx)
{
	int numLines = 0;

	PatternFrame* frame = getPattern()->getMeasureScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to grab measure, frameIdx: " << frameIdx;
		return 0;
	}

	bool res = false;
	numLines = frame->getNumberOfLineTraces(true);

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		res = getUsb3Grab()->grabMeasureProcessWait(buffIdx, numLines);
		if (res) {
			LogD() << "Measure usb3 buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
		}
		else {
			LogD() << "Grab measure usb3 buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
			return 0;
		}
		return numLines;
	}
	else {
		if (getGrabber()->waitForFirstGrabOfMeasureBuffer(buffIdx)) {
			res = getGrabber()->grabMeasureProcessWait(buffIdx, numLines);
			if (res) {
				LogD() << "Measure buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
			}
			else {
				LogD() << "Grab measure buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
				return 0;
			}
			return numLines;
		}
		else {
			LogD() << "Grab the first measure buffer failed!, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx;
			return 0;
		}
	}
}


int OctSystem::ScanGrab::processGrabBuffers(int buffIdx, int numLines)
{
	bool res = getGrabber()->grabMeasureProcessWait(buffIdx, numLines);
	if (res) {
		LogD() << "Measure buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
		return numLines;
	}
	else {
		LogD() << "Grab measure buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
		return 0;
	}
}


int OctSystem::ScanGrab::processGrabBufferToEnface(int frameIdx, int buffIdx)
{
	int numLines = 0;

	PatternFrame* frame = getPattern()->getEnfaceScan().getPatternFrame(frameIdx);
	if (frame == nullptr) {
		LogD() << "Invalid frame to grab enface, frameIdx: " << frameIdx;
		return 0;
	}

	bool res = false;
	numLines = frame->getLineTraceListSize();

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		res = getUsb3Grab()->grabEnfaceProcessWait(buffIdx, numLines);
		if (res) {
			// LogD() << "Preview buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
		}
		else {
			LogD() << "Grab enface usb3 buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
			return 0;
		}
		return numLines;
	}
	else {
		if (getGrabber()->waitForFirstGrabOfEnfaceBuffer(buffIdx)) {
			res = getGrabber()->grabEnfaceProcessWait(buffIdx, numLines);
			if (res) {
				// LogD() << "Preview buffer grabbed, buffIdx: " << buffIdx << ", numLines: " << numLines;
			}
			else {
				LogD() << "Grab enface buffer failed!, buffIdx: " << buffIdx << ", numLines: " << numLines;
				return 0;
			}
			return numLines;
		}
		else {
			LogD() << "Grab the first enface buffer failed!, frameIdx: " << frameIdx << ", buffIdx: " << buffIdx;
			return 0;
		}
	}
}


bool OctSystem::ScanGrab::updateScanSpeedToMeasure(void)
{
	if (getImpl().scanDesc->getPattern().isNormalSpeed()) {
		LogD() << "Measure scan, normal speed";
		if (!changeScanSpeedToNormal()) {
			return false;
		}
	}
	else if (getImpl().scanDesc->getPattern().isFasterSpeed()) {
		LogD() << "Measure scan, faster speed";
		if (!changeScanSpeedToFaster()) {
			return false;
		}
	}
	else if (getImpl().scanDesc->getPattern().isCustomSpeed()) {
		LogD() << "Measure scan, custom speed";
		if (!changeScanSpeedToCustom()) {
			return false;
		}
	}
	else {
		LogD() << "Measure scan, fastest speed";
		if (!changeScanSpeedToFastest()) {
			return false;
		}
	}

	return true;
}


bool OctSystem::ScanGrab::changeScanSpeedToFastest(bool noImageGrab)
{
	/*
	if (isScanSpeedFastest()) {
		return true;
	}
	*/

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->setCameraSpeedToFastest()) {
			return false;
		}
		auto trig_step = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Fastest);
		getGalvanometer()->setTriggerTimeStep(trig_step);
	} else {
		if (!noImageGrab) {
			if (!changeLineCameraScanSpeed(ScanSpeed::Fastest)) {
				return false;
			}
		}

		wstring path = DCF_FILE_NAME_TO_SPEED_FASTEST; // L"oct_hw_trigger_160929_68k.dcf";
		if (!getGrabber()->startFrameGrabber(path)) {
			return false;
		}
		getGalvanometer()->setTriggerTimeStep(TRIGGER_TIME_STEP_AT_FASTEST_SPEED);
	}

	d_ptr->scanSpeed = ScanSpeed::Fastest;
	return true;
}

bool OctSystem::ScanGrab::changeScanSpeedToFaster(bool noImageGrab)
{
	/*
	if (isScanSpeedFaster()) {
		return true;
	}
	*/

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->setCameraSpeedToFaster()) {
			return false;
		}
		auto trig_step = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Faster);
		getGalvanometer()->setTriggerTimeStep(trig_step);
	} else {
		if (!noImageGrab) {
			if (!changeLineCameraScanSpeed(ScanSpeed::Faster)) {
				return false;
			}
		}

		wstring path = DCF_FILE_NAME_TO_SPEED_FASTER; // L"oct_hw_trigger_160929_68k.dcf";
		if (!getGrabber()->startFrameGrabber(path)) {
			return false;
		}
		getGalvanometer()->setTriggerTimeStep(TRIGGER_TIME_STEP_AT_FASTER_SPEED);
	}
	
	d_ptr->scanSpeed = ScanSpeed::Faster;
	return true;
}


bool OctSystem::ScanGrab::changeScanSpeedToNormal(bool noImageGrab)
{
	/*
	if (isScanSpeedNormal()) {
		return true;
	}
	*/

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->setCameraSpeedToNormal()) {
			return false;
		}

		auto trig_step = GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed::Normal);
		getGalvanometer()->setTriggerTimeStep(trig_step);
	}
	else {
		if (!noImageGrab) {
			if (!changeLineCameraScanSpeed(ScanSpeed::Normal)) {
				return false;
			}
		}

		wstring path = DCF_FILE_NAME_TO_SPEED_NORMAL; //  L"oct_hw_trigger_160929.dcf";
		if (!getGrabber()->startFrameGrabber(path)) {
			return false;
		}
		getGalvanometer()->setTriggerTimeStep(TRIGGER_TIME_STEP_AT_NORMAL_SPEED);
	}

	d_ptr->scanSpeed = ScanSpeed::Normal;
	return true;
}


bool OctSystem::ScanGrab::changeScanSpeedToCustom(bool noImageGrab)
{
	/*
	if (isScanSpeedCustom()) {
		return true;
	}
	*/

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		if (!getUsb3Grab()->setCameraSpeedToFastest()) {
			return false;
		}
	}
	else {
		if (!noImageGrab) {
			if (!changeLineCameraScanSpeed(ScanSpeed::Custom)) {
				return false;
			}
		}


		wstring path = DCF_FILE_NAME_TO_SPEED_CUSTOM; //  L"oct_hw_trigger_160929.dcf";
		if (!getGrabber()->startFrameGrabber(path)) {
			return false;
		}
	}

	getGalvanometer()->setTriggerTimeStep(TRIGGER_TIME_STEP_AT_CUSTOM_SPEED);
	d_ptr->scanSpeed = ScanSpeed::Custom;
	return true;
}


bool OctSystem::ScanGrab::isScanSpeedFastest(void)
{
	return (d_ptr->scanSpeed == ScanSpeed::Fastest);
}


bool OctSystem::ScanGrab::isScanSpeedFaster(void)
{
	return (d_ptr->scanSpeed == ScanSpeed::Faster);
}


bool OctSystem::ScanGrab::isScanSpeedNormal(void)
{
	return (d_ptr->scanSpeed == ScanSpeed::Normal);
}


bool OctSystem::ScanGrab::isScanSpeedCustom(void)
{
	return (d_ptr->scanSpeed == ScanSpeed::Custom);
}


bool OctSystem::ScanGrab::changeLineCameraScanSpeed(ScanSpeed speed)
{
	/*
	// Hamamatsu CCD USB camera.
	string cmd = "037:0\r\n";
	string res;

	SerialPort sport;
	if (sport.open(4, 9600) || sport.open(3, 9600)) {
		if (sport.write(cmd)) {
			if (sport.read(res, 5)) {
			}
		}
	}
	return true;
	*/

	string cmd, res;
	if (speed == ScanSpeed::Normal || speed == ScanSpeed::Custom) {
		cmd = "res 2\r";
	}
	else if (speed == ScanSpeed::Faster) {
		cmd = "res 0\r";
	}
	else {
		cmd = "res 1\r";
	}

	SerialPort sport;
	bool ret = false;
	if (sport.open(4) || sport.open(3)) {
		if (sport.write(cmd)) {
			if (sport.read(res, 5)) {
				if (res.find("OK")) {
					ret = true;
				}
			}
		}
	}

	if (!ret) {
		LogD() << "Failed to command rate to line camera, recv: " + res;
	}

	// getMainboard()->setLineCameraGain(0x40);
	return ret;
}

bool OctSystem::ScanGrab::changeLineCameraFrameHeight(int height)
{
	// Hamamatsu CCD USB camera.
	// Stop acquisition.
	{
		string cmd = "037:1\r\n";
		string res;

		SerialPort sport;
		if (sport.open(4, 9600) || sport.open(3, 9600)) {
			if (sport.write(cmd)) {
				if (sport.read(res, 7)) {
				}
			}
			sport.close();
		}
	}

	// Change resolution. 
	char buff[64];
	snprintf(buff, sizeof(buff), "025:%d\r\n", height);
	string cmd = buff;
	string res;

	SerialPort sport;
	if (sport.open(4, 9600) || sport.open(3, 9600)) {
		if (sport.write(cmd)) {
			if (sport.read(res, 7)) {
			}
			sport.close();
		}
	}

	// Restart acquisition.
	{
		string cmd = "037:0\r\n";
		string res;

		SerialPort sport;
		if (sport.open(4, 9600) || sport.open(3, 9600)) {
			if (sport.write(cmd)) {
				if (sport.read(res, 7)) {
				}
			}
			sport.close();
		}
	}

	return true;
}

bool OctSystem::ScanGrab::setUsbCameraExposureTime(float expTime)
{
	bool result = getUsb3Grab()->setLineCameraExposureTime(expTime);
	return result;
}

float OctSystem::ScanGrab::getUsbCameraExposureTime(void)
{
	auto result = getUsb3Grab()->getLineCameraExposureTime();
	return result;
}


void OctSystem::ScanGrab::increaseGrabErrorCount(void)
{
	getImpl().countOfGrabErrors++;
	return;
}


void OctSystem::ScanGrab::clearGrabErrorCount(void)
{
	getImpl().countOfGrabErrors = 0;
	return;
}


bool OctSystem::ScanGrab::isGrabErrorOverLimit(void)
{
	if (getImpl().countOfGrabErrors > SCANNER_GRAB_ERROR_COUNT_LIMIT) {
		return true;
	}
	return false;
}


bool OctSystem::ScanGrab::isGrabErrorOverDelay(void)
{
	if (getImpl().countOfGrabErrors > SCANNER_GRAB_ERROR_COUNT_DELAY) {
		return true;
	}
	return false;
}


bool OctSystem::ScanGrab::isPreviewImagesRepeated(void)
{
	return d_ptr->repeatOfPreviews;
}


void OctSystem::ScanGrab::resetCountOfPreviewImages(bool repeat)
{
	d_ptr->countOfPreviews = 0;
	d_ptr->repeatOfPreviews = repeat;
	return;
}


void OctSystem::ScanGrab::resetCountOfMeasureImages(void)
{
	d_ptr->countOfMeasures = 0;
	return;
}


void OctSystem::ScanGrab::resetCountOfEnfaceImages(void)
{
	d_ptr->countOfEnfaces = 0;
	return;
}


int OctSystem::ScanGrab::getIndexOfPreviewImageNext(void)
{
	int index = d_ptr->countOfPreviews;
	d_ptr->countOfPreviews++;
	return index;
}


int OctSystem::ScanGrab::getIndexOfMeasureImageNext(bool autoinc)
{
	int index = d_ptr->countOfMeasures;
	if (autoinc) {
		d_ptr->countOfMeasures++;
	}
	return index;
}


int OctSystem::ScanGrab::getIndexOfEnfaceImageNext(void)
{
	int index = d_ptr->countOfEnfaces;
	d_ptr->countOfEnfaces++;
	return index;
}


void OctSystem::ScanGrab::retreatIndexOfMeasureImage(int size)
{
	int index = d_ptr->countOfMeasures;
	index = max(index - size, 0);
	d_ptr->countOfMeasures = index;
	return;
}


OctPattern::PatternPlan * OctSystem::ScanGrab::getPattern(void)
{
	return getImpl().pattern;
}


SigChain::OctProcess * OctSystem::ScanGrab::getProcess(void)
{
	return getImpl().process;
}


OctGrab::FrameGrabber * OctSystem::ScanGrab::getGrabber(void)
{
	return getImpl().grabber;
}


OctGrab::Usb3Grabber * OctSystem::ScanGrab::getUsb3Grab(void)
{
	return getImpl().usb3Grab;
}


OctDevice::MainBoard * OctSystem::ScanGrab::getMainboard(void)
{
	return getImpl().board;
}


OctDevice::Galvanometer * OctSystem::ScanGrab::getGalvanometer(void)
{
	return getImpl().galvano;
}


ScanGrab::ScanGrabImpl & OctSystem::ScanGrab::getImpl(void)
{
	return *d_ptr;
}

