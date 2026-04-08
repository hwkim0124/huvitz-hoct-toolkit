#include "stdafx.h"
#include "RetinaTracker.h"

#include <mutex>
#include <atomic>

#include "OctDevice2.h"
#include "RetFocus2.h"
#include "CppUtil2.h"
#include "SigChain2.h"

#include "GlobalSettings.h"


using namespace OctSystem;
using namespace OctDevice;
using namespace SigChain;
using namespace RetFocus;
using namespace CppUtil;
using namespace std;


std::mutex RetinaTracker::singleMutex_;


struct RetinaTracker::RetinaTrackerImpl
{
	OctScanMeasure protocol;
	RetinaTrackState state;

	TrackCamera trackCamera;
	TrackRecords records;
	TrackFeature featRefer;
	TrackFeature featFrame;
	TrackFeature featAccum;

	atomic<int> matchedCount;
	atomic<int> invalidCount;
	atomic<int> alteredCount;
	atomic<int> trackedCount;

	int octObjectCount;
	int octNonObjCount;

	atomic<float> match_val;
	atomic<float> moved_x;
	atomic<float> moved_y;
	atomic<float> feat_mean;
	atomic<float> feat_stdev;
	atomic<float> feat_covar;
	atomic<bool> feat_valid;
	atomic<bool> feat_covered;

	atomic<float> trans_x;
	atomic<float> trans_y;

	atomic<int> grabImageIndex;

	float scanCenterPosX;
	float scanCenterPosY;
	float scanOffsetXmm;
	float scanOffsetYmm;

	float scanMovedXmm;
	float scanMovedYmm;
	float scanMovedPosX;
	float scanMovedPosY;

	float scanRangeX;
	float scanRangeY;

	bool adjustWithOct;
	bool isFocusLedsOn;
	bool isFirstRegist;

	bool isSplitInitOn;
	bool isWdotsInitOn;

	MainBoard* mainboard;

	RetinaTrackerImpl()
	{
		initializeRetinaTrackerImpl();
	}

	void initializeRetinaTrackerImpl(void)
	{
		protocol.init();
		state = RetinaTrackState::UNKNOWN;
		matchedCount = 0;
		alteredCount = 0;
		invalidCount = 0;
		trackedCount = 0;

		octObjectCount = 0;
		octNonObjCount = 0;

		scanCenterPosX = RET_TRACK_CAMERA_FRAME_CENT_X;
		scanCenterPosY = RET_TRACK_CAMERA_FRAME_CENT_Y;
		scanOffsetXmm = 0.0f;
		scanOffsetYmm = 0.0f;
		scanMovedXmm = 0.0f;
		scanMovedYmm = 0.0f;
		scanMovedPosX = 0.0f;
		scanMovedPosY = 0.0f;

		match_val = 0.0f;
		moved_x = 0.0f;
		moved_y = 0.0f;
		trans_x = 0.0f;
		trans_y = 0.0f;
		
		feat_mean = 0.0f;
		feat_stdev = 0.0f;
		feat_covar = 0.0f;
		feat_valid = false;
		feat_covered = false;

		grabImageIndex = 0;

		adjustWithOct = true;
		isFocusLedsOn = true;
		isFirstRegist = true;
		mainboard = nullptr;

		isSplitInitOn = true;
		isWdotsInitOn = true;
	}
};


RetinaTracker::RetinaTracker() :
	d_ptr(new RetinaTrackerImpl())
{
}


RetinaTracker::~RetinaTracker()
{
}


RetinaTracker * OctSystem::RetinaTracker::getInstance(void)
{
	// Instance is constructed by public static method.
	// Static local variable initialization is thread-safe 
	// and will be initailized only once. 
	static RetinaTracker instance;
	return &instance;
}

bool OctSystem::RetinaTracker::initializeRetinaTracker(OctDevice::MainBoard * board)
{
	impl().initializeRetinaTrackerImpl();
	impl().mainboard = board;

	impl().trackCamera.setupTrackCamera(board);
	return true;
}

void OctSystem::RetinaTracker::resetTrackingStatus(void)
{
	impl().matchedCount = 0;
	impl().alteredCount = 0;
	impl().invalidCount = 0;
	impl().trackedCount = 0;

	impl().octObjectCount = 0;
	impl().octNonObjCount = 0;

	impl().match_val = 0.0f;
	impl().moved_x = 0.0f;
	impl().moved_y = 0.0f;
	impl().trans_x = 0.0f;
	impl().trans_y = 0.0f;

	impl().feat_mean = 0.0f;
	impl().feat_stdev = 0.0f;
	impl().feat_covar = 0.0f;
	impl().feat_valid = false;
	impl().feat_covered = false;

	impl().scanMovedXmm = 0.0f;
	impl().scanMovedYmm = 0.0f;
	impl().scanMovedPosX = 0.0f;
	impl().scanMovedPosY = 0.0f;

	impl().grabImageIndex = 0;
	impl().state = RetinaTrackState::UNKNOWN;
}

bool OctSystem::RetinaTracker::setupRetinaTracking(const OctScanMeasure& meas)
{
	resetTrackingStatus();

	impl().protocol = meas;
	impl().state = RetinaTrackState::PREPARED;

	LogD() << "Retina tracking prepared, side: " << (meas.isOD() ? "OD" : "OS") << ", disc: " << meas.preview().isDiscScan();

	if (impl().protocol.getPattern().isDiscScan() /*|| impl().protocol.getPattern().getScanRangeX() > 9.0f */) {
		relocateScanningRegion(0.0f, 0.0f);
	}
	else {
		float offs_x = impl().protocol.isOD() ? FUNDUS_SCAN_CENTER_OFFSET_X_OD : FUNDUS_SCAN_CENTER_OFFSET_X_OS;
		float offs_y = FUNDUS_SCAN_CENTER_OFFSET_Y;
		relocateScanningRegion(offs_x, offs_y);
	}
	return true;
}

bool OctSystem::RetinaTracker::setupRetinaTracking(EyeSide side, bool disc, float rangeX, float rangeY)
{
	resetTrackingStatus();

	auto pattern = disc ? PatternName::Disc3D : PatternName::Macular3D;
	impl().protocol.getPattern().setup(pattern, 512, 96, rangeX, rangeY);
	impl().protocol.setEyeSide(side);
	impl().state = RetinaTrackState::PREPARED;

	LogD() << "Retina tracking prepared, side: " << (side == EyeSide::OD ? "OD" : "OS") << ", disc: " << disc;

	if (disc /*|| rangeX > 9.0f*/) {
		relocateScanningRegion(0.0f, 0.0f);
	}
	else {
		float offs_x = side == EyeSide::OD ? FUNDUS_SCAN_CENTER_OFFSET_X_OD : FUNDUS_SCAN_CENTER_OFFSET_X_OS;
		float offs_y = FUNDUS_SCAN_CENTER_OFFSET_Y;
		relocateScanningRegion(offs_x, offs_y);
	}
	return true;
}

bool OctSystem::RetinaTracker::startRetinaTracking(void)
{
	if (impl().state == RetinaTrackState::FEATURE_REGISTERED) {
		impl().state = RetinaTrackState::STARTED;
		impl().grabImageIndex = 0;
		LogD() << "Retina tracking started";
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::resumeRetinaTracking(void)
{
	if (isTrackingPrepared()) {
		impl().state = RetinaTrackState::STARTED;
		LogD() << "Retina tracking resumed";
		return true;
	}
	return false;
}

void OctSystem::RetinaTracker::pauseRetinaTracking(void)
{
	if (impl().state == RetinaTrackState::STARTED) {
		impl().state = RetinaTrackState::PAUSED;
		LogD() << "Retina tracking paused";
	}
	return;
}

void OctSystem::RetinaTracker::cancelRetinaTracking(void)
{
	resetTrackingStatus();
	LogD() << "Retina tracking canceled";

	if (impl().mainboard) {
		impl().trackCamera.releaseCameraControl();
	}
	return;
}

void OctSystem::RetinaTracker::resetRetinaTracking(void)
{
	resetTrackingStatus();
	LogD() << "Retina tracking cleared";
	return;
}

void OctSystem::RetinaTracker::setLightLedsInitStatus(bool splitOn, bool wdotsOn)
{
	impl().isSplitInitOn = splitOn;
	impl().isWdotsInitOn = wdotsOn;
	return;
}

bool OctSystem::RetinaTracker::processTrackingFrame(const RetFocus::RetinaFrame& frame)
{
	if (isTrackingPrepared()) {
		if (updateRetinaFrame(frame)) {
			if (impl().state == RetinaTrackState::AUTO_ADJUSTMENT) {
				optimizeRetinaFrame(frame);
			}
			else if (impl().state == RetinaTrackState::FEATURE_REQUESTED) {
				registerFeatureFrame(frame);
			}
			else if (impl().state == RetinaTrackState::STARTED || impl().state == RetinaTrackState::PENDING) {
				compareCurrentFrame(frame);
			}
			else if (impl().state == RetinaTrackState::CAMERA_ADJUSTED || impl().state == RetinaTrackState::PAUSED) {
				recordCurrentFrame(frame);
			}
		}
	}
	return true;
}

bool OctSystem::RetinaTracker::requestAutoAdjustment(bool withOct)
{
	lock_guard<mutex> lock(singleMutex_);
	if (isTrackingPrepared()) {
		impl().state = RetinaTrackState::AUTO_ADJUSTMENT;
		impl().adjustWithOct = withOct;
		impl().isFocusLedsOn = true;
		impl().octNonObjCount = 0;
		impl().octObjectCount = 0;

		impl().trackCamera.setupTrackCamera(impl().mainboard);
		impl().trackCamera.initiateCameraControl();
		LogD() << "Retina tracker, camera frame opimization requested.";
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::requestRegistration(bool first)
{
	lock_guard<mutex> lock(singleMutex_);
	if (isTrackingPrepared()) {
		impl().state = RetinaTrackState::FEATURE_REQUESTED;
		impl().matchedCount = 0;
		impl().invalidCount = 0;
		impl().alteredCount = 0;
		impl().trackedCount = 0;
		impl().isFirstRegist = first;
		LogD() << "Retina tracker, feature registration requested.";
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::completeAutoAdjustment(void)
{
	lock_guard<mutex> lock(singleMutex_);
	if (impl().state == RetinaTrackState::AUTO_ADJUSTMENT) {
		/*
		if (impl().isFocusLedsOn) {
			impl().state = RetinaTrackState::FRAME_INVALID;
			LogD() << "Retina tracker, frame adjustment failed.";
		}
		else {
			impl().state = RetinaTrackState::FRAME_ADJUSTED;
			LogD() << "Retina tracker, frame adjustment success.";
			return true;
		}
		*/
		impl().state = RetinaTrackState::CAMERA_ADJUSTED;
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::waitForRegistration(int retryMax)
{
	const int REGIST_DELAY = 35;
	const int REGIST_RETRY_MAX = 100;
	const int INVALID_COUNT1 = 5;  // 33 x 5 = 165 ms
	const int INVALID_COUNT2 = 3;  // 66 x 3 = 198 ms
	const int LIMIT_COUNT1 = 15;  // 33 x 15 = 500 ms
	const int LIMIT_COUNT2 = 8;  // 66 x 8 = 530 ms

	int invalid = impl().trackCamera.isHighSpeedCamera() ? INVALID_COUNT1 : INVALID_COUNT2;
	int limit = retryMax > 0 ? retryMax : (impl().trackCamera.isHighSpeedCamera() ? LIMIT_COUNT1 : LIMIT_COUNT2);

	if (impl().isFirstRegist) {
		if (impl().state == RetinaTrackState::FEATURE_REQUESTED) {
			for (int i = 0; i < limit; i++) {
				this_thread::sleep_for(chrono::milliseconds(REGIST_DELAY));
				if (isFeatureRegistered()) {
					updateTargetMovement(0.0f, 0.0f);
					LogD() << "Retina tracker, initial feature registered";
					return true;
				}
				/*
				// If the invalid frames are continuously detected, then the registration is failed.
				if (impl().invalidCount >= invalid) {
					break;
				}
				*/
			}
		}
	}
	else {
		limit = 1;
		if (impl().state == RetinaTrackState::FEATURE_REQUESTED) {
			for (int i = 0; i < limit; i++) {
				this_thread::sleep_for(chrono::milliseconds(REGIST_DELAY));
				if (isFeatureRegistered()) {
					updateTargetMovement(0.0f, 0.0f);
					LogD() << "Retina tracker, new feature registered";
					return true;
				}
			}
		}
	}
	impl().state = RetinaTrackState::CAMERA_ADJUSTED;
	LogD() << "Retina tracker, feature registration failed";
	return false;
}

bool OctSystem::RetinaTracker::waitForTargetAlignment(void)
{
	// Time limit is about 0.5 second.
	const int WAIT_DELAY = 35; 
	const int WAIT_RETRY_MAX = 10; 
	const int CONFIRM_COUNT = 1; 

	if (impl().state == RetinaTrackState::STARTED) {
		impl().state = RetinaTrackState::PENDING;
		LogD() << "Retina tracker, wating re-alignment...";

		// Wait for the registered target is detected, which is still valid yet. 
		// The new positional offset updates the scanning position to restart. 
		// If failed to target, then target registration is required. 
		int count = 0;
		beginTrackRecording(false);
		for (int i = 0; i < WAIT_RETRY_MAX; i++) {
			this_thread::sleep_for(chrono::milliseconds(WAIT_DELAY));
			if (impl().records.isAvailable(2)) {
				if (isTargetConsistent(false)) {
					count += 1;
					if (count >= CONFIRM_COUNT) {
						float x, y;
						impl().records.getLastPosition(x, y);
						updateTargetMovement(x, y);

						LogD() << "Retina tracker, target aligned at location: " << x << ", " << y << ", count: " << count;
						impl().state = RetinaTrackState::STARTED;
						return true;
					}
				}
				else {
					beginTrackRecording(false);
					count = 0;
				}
			}
		}
	}
	LogD() << "Retina tracker, target not aligned";
	return false;
}

bool OctSystem::RetinaTracker::waitForTargetIdentified(void)
{
	// Time limit is about 0.5 second.
	const int WAIT_DELAY = 35;
	const int WAIT_RETRY_MAX = 10;
	const int CONFIRM_COUNT = 1;

	if (impl().state == RetinaTrackState::STARTED) {
		impl().state = RetinaTrackState::PENDING;
		LogD() << "Retina tracker, wating re-target...";

		// Wait for the registered target is detected, which is still valid yet. 
		// The new positional offset updates the scanning position to restart. 
		// If failed to target, then target registration is required. 
		int count = 0;
		beginTrackRecording(false);
		for (int i = 0; i < WAIT_RETRY_MAX; i++) {
			this_thread::sleep_for(chrono::milliseconds(WAIT_DELAY));
			if (impl().records.isAvailable(1)) {
				if (isTargetIdentified()) {
					count += 1;
					if (count >= CONFIRM_COUNT) {
						float x, y;
						impl().records.getLastPosition(x, y);
						updateTargetMovement(x, y);

						LogD() << "Retina tracker, target aligned at location: " << x << ", " << y << ", count: " << count;
						impl().state = RetinaTrackState::STARTED;
						return true;
					}
				}
				else {
					beginTrackRecording(false);
					count = 0;
				}
			}
		}
	}
	LogD() << "Retina tracker, target not found";
	return false;
}


bool OctSystem::RetinaTracker::updateRetinaFrame(const RetFocus::RetinaFrame& frame)
{
	TrackFeature feat;
	const auto& image = frame.getImage();
	auto isOd = impl().protocol.isOD();
	auto isDisc = impl().protocol.getPattern().isDiscScan();

	if (feat.setupFrameImage(isOd, isDisc, image)) {
		feat.evaluateValidity();

		auto mean = feat.getFeatureMean();
		auto stdev = feat.getFeatureStdev();
		auto covar = feat.getCoefficientOfVariation();
		auto valid = feat.isValidFeature();
		auto covered = feat.isFeatureCovered();

		// LogD() << "Retina frame updated, mean: " << mean << ", covar: " << covar << ", valid: " << valid << ", covered: " << covered;

		impl().feat_mean = mean;
		impl().feat_stdev = stdev;
		impl().feat_covar = covar;
		impl().feat_valid = valid;
		impl().feat_covered = covered;
		impl().featFrame = std::move(feat);
		return true;
	}

	LogD() << "Retina frame update failed";
	return false;
}

bool OctSystem::RetinaTracker::optimizeRetinaFrame(const RetFocus::RetinaFrame & frame)
{
	lock_guard<mutex> lock(singleMutex_);

	const float QUALITY_OBJECT = 2.5f;
	const int MISSED_COUNT1 = 50;
	const int MISSED_COUNT2 = 25;
	const int COUNT_MAX = 9999;

	float qidx = 0.0f;
	bool found = false;

	// Ignore frames during oct scan image doesn't obtain any valid object.
	if (impl().adjustWithOct) {
		if (obtainOctPreviewQualityIndex(qidx)) {
			if (qidx >= QUALITY_OBJECT) {
				impl().octObjectCount = min(impl().octObjectCount + 1, COUNT_MAX);
				impl().octNonObjCount = 0;
				found = true;
			}
		}
		if (!found) {
			impl().octObjectCount = 0;
			impl().octNonObjCount = min(impl().octNonObjCount + 1, COUNT_MAX);
		}
	}
	else {
		found = true;
	}

	if (found)
	{
		if (impl().isFocusLedsOn) {
			auto board = impl().mainboard;
			if (board) {
				// impl().mainboard->getSplitFocusLed()->control(false);
				impl().mainboard->getWorkingDotsLed()->control(false);
				impl().mainboard->getWorkingDot2Led()->control(false);
				impl().isFocusLedsOn = false;
			}
		}
		else {
			impl().trackCamera.adjustCameraIntensity(&impl().featFrame);
			return true;
		}
	}
	else {
		int missed = impl().trackCamera.isHighSpeedCamera() ? MISSED_COUNT1 : MISSED_COUNT2;
		if (impl().octNonObjCount > missed) {
			if (impl().isFocusLedsOn == false) {
				auto board = impl().mainboard;
				if (board) {
					bool splitOn = impl().isSplitInitOn;
					bool wdotsOn = impl().isWdotsInitOn;
					// impl().mainboard->getSplitFocusLed()->control(splitOn);
					impl().mainboard->getWorkingDotsLed()->control(wdotsOn);
					impl().mainboard->getWorkingDot2Led()->control(wdotsOn);
					impl().isFocusLedsOn = true;
				}
			}
		}
	}
	return false;
}

bool OctSystem::RetinaTracker::registerFeatureFrame(const RetFocus::RetinaFrame& frame)
{
	lock_guard<mutex> lock(singleMutex_);

	static int N_MATCHED_FRAMES1 = 3; // 4;
	static int N_MATCHED_FRAMES2 = 2;
	static int N_INVALID_FRAMES = 0; // 3;
	static int N_ALTERED_FRAMES = 0; // 3;
	static float SIMILARITY_THRESHOLD = RETT_FEAT_THRESH_TO_REGIST;
	static float TRANSITION_THRESHOLD = RETT_FEAT_THRESH_TO_REGIST;
	static float TARGET_MOVE_X_MAX = 1.0f; // 0.5f;
	static float TARGET_MOVE_Y_MAX = 1.0f; // 0.5f;

	int matched = impl().matchedCount;
	int invalid = impl().invalidCount;
	int altered = impl().alteredCount;

	bool valid = impl().feat_valid;
	bool covered = impl().feat_covered;
	float mean = impl().feat_mean;
	float stdev = impl().feat_stdev;
	float covar = impl().feat_covar;
	LogD() << "Retina feature, valid: " << valid << ", mean: " << mean << ", stdev: " << stdev << ", covar: " << covar << " => matched: " << matched << ", invalid: " << invalid;
	
	if (valid) {
		// Retina feature is valid to register.
		invalid = 0;

		// Initial feature to compare. 
		if (matched == 0) {
			matched = 1;
			altered = 0;

			if (impl().isFirstRegist) {
				impl().featAccum = std::move(impl().featFrame);
				LogD() << "Initial target feature registered.";
			}
			else {
				impl().featRefer = std::move(impl().featFrame);
				impl().state = RetinaTrackState::FEATURE_REGISTERED;
				impl().trackedCount = 0;
				LogD() << "Tracking target feature registered.";
			}
		}
		else {
			const auto& image = frame.getImage();
			float dx = 0.0f, dy = 0.0f, value = 0.0f;

			bool found = impl().featAccum.estimateMovement(image);
			if (found) {
				value = impl().featAccum.getSimilarity();
				dx = impl().featAccum.getMovedX();
				dy = impl().featAccum.getMovedY();
				LogD() << "Matching feature, sim-val: " << value << ", dx: " << dx << ", dy: " << dy;

				if (value > SIMILARITY_THRESHOLD) {
					if (fabs(dx) < TARGET_MOVE_X_MAX && fabs(dy) < TARGET_MOVE_Y_MAX) {
						matched += 1;
						altered = 0;

						int required = impl().trackCamera.isHighSpeedCamera() ? N_MATCHED_FRAMES1 : N_MATCHED_FRAMES2;
						if (matched >= required) {
							impl().featRefer = std::move(impl().featFrame);
							impl().state = RetinaTrackState::FEATURE_REGISTERED;
							impl().trackedCount = 0;
							LogD() << "Tracking target feature registered.";
						}
						else {
							LogD() << "Registering target matched, matched: " << matched;
						}
					}
					else {
						altered += 1;
						LogD() << "Registering target drifted, altered: " << altered;
					}
				}
				else {
					altered += 1;
					LogD() << "Registering target not matched, altered: " << altered;
				}
			}
			else {
				invalid += 1;
				LogD() << "Registering target not found, invalid: " << invalid;
			}
		}
	}
	else {
		invalid += 1;
		LogD() << "Registering target not valid, invalid: " << invalid;
	}

	// If retina feature is not valid, then reset the tracking status.
	if (invalid > N_INVALID_FRAMES) {
		matched = 0;
		altered = 0;
	}
	else {
		// If the feature is altered, then register the feature again.
		if (altered > N_ALTERED_FRAMES) {
			matched = 1;
			altered = 0;
			impl().featAccum = std::move(impl().featFrame);
			LogD() << "Initial target feature registered.";
		}
	}

	impl().matchedCount = matched;
	impl().invalidCount = invalid;
	impl().alteredCount = altered;
	return true;
}

bool OctSystem::RetinaTracker::compareCurrentFrame(const RetFocus ::RetinaFrame& frame)
{
	lock_guard<mutex> lock(singleMutex_);
	static float COMPARE_THRESHOLD = RETT_FEAT_THRESH_TO_COMPARE; 

	float mean = impl().feat_mean;
	float stdev = impl().feat_stdev;
	float covar = impl().feat_covar;
	bool covered = impl().feat_covered;
	bool valid = impl().feat_valid;
	
	float simval = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;

	LogD() << "Retina feature, valid: " << valid << ", mean: " << mean << ", stdev: " << stdev << ", covar: " << covar << ", covered: " << covered;

	if (valid) {
		const auto& image = frame.getImage();
		impl().featRefer.estimateMovement(image);

		simval = impl().featRefer.getSimilarity();
		dx = impl().featRefer.getMovedX();
		dy = impl().featRefer.getMovedY();
		bool found = (simval > COMPARE_THRESHOLD);

		impl().match_val = (found ? simval : 0.0f);
		impl().moved_x = dx;
		impl().moved_y = dy;
		LogD() << "Tracking feature, found: " << found << ", simval: " << simval << ", moved_x: " << dx << ", moved_y: " << dy;
	}
	else {
		impl().match_val = 0.0f;
		impl().moved_x = 0.0f;
		impl().moved_y = 0.0f;
	}

	impl().trackedCount += 1;
	impl().records.insertMovement(valid, covered, mean, stdev, simval, dx, dy);
	return true;
}

bool OctSystem::RetinaTracker::recordCurrentFrame(const RetFocus::RetinaFrame & frame)
{
	lock_guard<mutex> lock(singleMutex_);

	float mean = impl().feat_mean;
	float stdev = impl().feat_stdev;
	float covar = impl().feat_covar;
	bool covered = impl().feat_covered;
	bool valid = impl().feat_valid;

	float simval = 0.0f;
	float dx = 0.0f;
	float dy = 0.0f;

	impl().records.insertMovement(valid, covered, mean, stdev, simval, dx, dy);
	return true;
}

int OctSystem::RetinaTracker::getTrackingState(void) const
{
	return (int)impl().state;
}

bool OctSystem::RetinaTracker::isTrackingPrepared(void) const
{
	return impl().state >= RetinaTrackState::PREPARED;
}

bool OctSystem::RetinaTracker::isFeatureRequested(void) const
{
	return impl().state >= RetinaTrackState::FEATURE_REQUESTED;
}

bool OctSystem::RetinaTracker::isFeatureRegistered(void) const
{
	return impl().state >= RetinaTrackState::FEATURE_REGISTERED;
}

bool OctSystem::RetinaTracker::isFeatureValid(void) const
{
	return impl().feat_valid;
}

bool OctSystem::RetinaTracker::isTrackingStarted(void) const
{
	auto result = impl().state == RetinaTrackState::STARTED;
	return result;
}

bool OctSystem::RetinaTracker::isTrackingPending(void) const
{
	return impl().state == RetinaTrackState::PENDING;
}

bool OctSystem::RetinaTracker::isTrackingRunning(void) const
{
	return impl().state >= RetinaTrackState::STARTED;
}

bool OctSystem::RetinaTracker::isTrackingPaused(void) const
{
	return impl().state == RetinaTrackState::PAUSED;
}

void OctSystem::RetinaTracker::beginTrackRecording(bool init)
{
	impl().records.clearRecords();
	impl().records.setIsRecording(true);

	if (init) {
		auto x = impl().scanMovedPosX;
		auto y = impl().scanMovedPosY;
		impl().records.setInitialPosition(x, y);
	}
}

void OctSystem::RetinaTracker::pauseTrackRecording(void)
{
	impl().records.setIsRecording(false);
}

bool OctSystem::RetinaTracker::isTrackRecordAvailable(int size)
{
	return impl().records.isAvailable(size);
}


bool OctSystem::RetinaTracker::isTargetConsistent(bool align) const
{
	bool result = impl().records.isConsistent(align);
	return result;
}

bool OctSystem::RetinaTracker::isTargetDisplaced(void) const
{
	bool result = impl().records.isDisplaced();
	return result;
}

bool OctSystem::RetinaTracker::isTargetIdentified(void) const
{
	bool result = impl().records.isIdentified();
	return result;
}

bool OctSystem::RetinaTracker::isTargetCenterAligned(void) const
{
	bool result = impl().records.isCenterAligned();
	return result;
}

bool OctSystem::RetinaTracker::isEyelidCovered(void) const
{
	bool result = impl().records.isCovered();
	return result;
}

void OctSystem::RetinaTracker::relocateScanningRegion(float offsetX, float offsetY)
{
	impl().scanOffsetXmm = offsetX;
	impl().scanOffsetYmm = offsetY;
	impl().scanMovedXmm = 0.0f;
	impl().scanMovedYmm = 0.0f;

	impl().scanMovedPosX = 0.0f;
	impl().scanMovedPosY = 0.0f;

	float dx = offsetX / RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM;
	float dy = offsetY / RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM;

	impl().scanCenterPosX = RET_TRACK_CAMERA_FRAME_CENT_X + ceil(dx);
	impl().scanCenterPosY = RET_TRACK_CAMERA_FRAME_CENT_Y + ceil(dy);
	LogD() << "Retina tracker, scanning center relocated: " << offsetX << ", " << offsetY << " => pos: " << impl().scanCenterPosX << ", " << impl().scanCenterPosY;
	return;
}

void OctSystem::RetinaTracker::updateTargetMovement(float movedX, float movedY)
{
	float dx = movedX * RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM;
	float dy = movedY * RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM;

	impl().scanMovedPosX = movedX;
	impl().scanMovedPosY = movedY;
	impl().scanMovedXmm = dx;
	impl().scanMovedYmm = dy;
	return;
}

void OctSystem::RetinaTracker::getTargetPosition(float & posX, float & posY)
{
	impl().records.getLastPosition(posX, posY);
	return;
}

void OctSystem::RetinaTracker::getTargetDrifts(float & sizeX, float & sizeY)
{
	impl().records.getLastDrifts(sizeX, sizeY);
	return;
}

void OctSystem::RetinaTracker::getTargetOffsetInMM(float & mmX, float & mmY)
{
	mmX = impl().scanMovedXmm;
	mmY = impl().scanMovedYmm;
	return;
}

void OctSystem::RetinaTracker::getScanningOffset(float & offsetX, float & offsetY)
{
	offsetX = (impl().scanOffsetXmm + impl().scanMovedXmm);
	offsetY = (impl().scanOffsetYmm + impl().scanMovedYmm);
}


void OctSystem::RetinaTracker::getFeatureRegion(int& x, int& y, int& w, int& h)  const
{
	impl().featRefer.getFeatureRect(x, y, w, h);
	// impl().featFrame.getFeatureRect(x, y, w, h);
	return;
}

void OctSystem::RetinaTracker::getScanningRegion(int& x, int& y, int& w, int& h) const
{
	getTrackingRegion(x, y, w, h, true);
	return;
}

bool OctSystem::RetinaTracker::getScanningRegionInfo(int & x, int & y, int & w, int & h, bool & pending, bool cropped) const
{
	getTrackingRegion(x, y, w, h, false);

	pending = false;
	if (isTrackingPrepared()) {
		if (isFeatureRegistered()) {
			pending = (isTrackingStarted() == false);
		}
		else {
			pending = (isFeatureValid() == false);
		}
	}

	if (cropped) {
		auto center = GlobalSettings::centerOfRetinaROI();
		auto radius = GlobalSettings::radiusOfRetinaROI();
		x = x - (center.first - radius);
		y = y - (center.second - radius);
	}
	return true;
}

void OctSystem::RetinaTracker::getTrackingRegion(int& x, int& y, int& w, int& h, bool start) const
{
	w = (int)(impl().protocol.getPattern().getScanRangeX() / RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM);
	h = (int)(impl().protocol.getPattern().getScanRangeY() / RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM);
	x = (int)(impl().scanCenterPosX + impl().moved_x);
	y = (int)(impl().scanCenterPosY + impl().moved_y);

	if (start) {
		x = x - w / 2;
		y = y - h / 2;
	}
	return;
}

bool OctSystem::RetinaTracker::getTrackingRegionInfo(int & x, int & y, int & w, int & h, float & value, bool cropped) const
{
	getTrackingRegion(x, y, w, h, false);

	if (true || isTrackingStarted()) {
		getFrameSimilarity(value);

		if (cropped) {
			auto center = GlobalSettings::centerOfRetinaROI();
			auto radius = GlobalSettings::radiusOfRetinaROI();
			x = x - (center.first - radius);
			y = y - (center.second - radius);
		}
		return true;
	}
	return false;
}

void OctSystem::RetinaTracker::setGrabbingImageIndex(int index)
{
	impl().grabImageIndex = index;
	return;
}

int OctSystem::RetinaTracker::getGrabbingImageIndex(void) const
{
	int index = impl().grabImageIndex;
	return index;
}

float OctSystem::RetinaTracker::getGrabbingImageRatio(void) const
{
	int index = getGrabbingImageIndex();
	int size = impl().protocol.getPattern().getNumberOfScanLines() * impl().protocol.getPattern().getScanOverlaps();
	float ratio = (float)index / (float)size;
	return ratio;
}


bool OctSystem::RetinaTracker::getFrameMovements(float& dx, float& dy) const
{
	if (true || isTrackingPrepared()) {
		dx = impl().moved_x;
		dy = impl().moved_y;
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::getFrameStatistics(float& mean, float& stdev) const
{
	if (true || isTrackingPrepared()) {
		mean = impl().feat_mean;
		stdev = impl().feat_stdev;
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::getFrameVariance(float& covar) const
{
	if (true || isTrackingPrepared()) {
		covar = impl().feat_covar;
		return true;
	}
	return false;
}

bool OctSystem::RetinaTracker::getFrameSimilarity(float& match) const
{
	if (true || isTrackingPrepared()) {
		match = impl().match_val;
		return true;
	}
	return false;
}


bool OctSystem::RetinaTracker::obtainOctPreviewQualityIndex(float & qidx)
{
	float index, ratio;
	unsigned long count;

	if (ChainOutput::getPreviewImageResult(&index, &ratio, nullptr, &count)) {
		qidx = index;
		return true;
	}
	return false;
}

RetinaTracker::RetinaTrackerImpl& OctSystem::RetinaTracker::impl(void) const
{
	return *d_ptr;
}

