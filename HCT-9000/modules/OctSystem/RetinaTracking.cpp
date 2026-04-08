#include "stdafx.h"
#include "RetinaTracking.h"

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


struct RetinaTracking::RetinaTrackingImpl
{
    std::mutex singleMutex_;

    MainBoard* mainboard;
    OctScanMeasure protocol;
    RetinaTrackingStatus status;

    TrackingCamera trackCamera;
    TrackingRecords trackRecords;
    TrackingObject objRefer;
    TrackingObject objFrame;

    float scanCenterInitXpx;
    float scanCenterInitYpx;
    float scanCenterInitXmm;
    float scanCenterInitYmm;

    float trackingDeltaXpx;
    float trackingDeltaYpx;
    float trackingDeltaXmm;
    float trackingDeltaYmm;

    float scanCenterLastXpx;
    float scanCenterLastYpx;
    float scanCenterLastXmm;
    float scanCenterLastYmm;

    float retinaMean;
    float retinaStdev;
    float targetMean;
    float targetStdev;
    float targetScore;
    float targetPosX;
    float targetPosY;

    float registPosX;
    float registPosY;
	float trackingPosX;
	float trackingPosY;
	float trackingScore;

    int trackingCount;
    int registerCount;
    int grabImageIndex;
    
	bool isTestMode;
    bool adjustWithOct;
    bool isSplitLedsInitOn;
    bool isWdotsLedsInitOn;
    bool isSplitMirrorInitIn;

    RetinaTrackingImpl()
    {
        initializeRetinaTrackingImpl();
    }

    void initializeRetinaTrackingImpl(void)
    {
        protocol.init();
        status = RetinaTrackingStatus::RTS_UNKNOWN;
        mainboard = nullptr;

        trackingCount = 0;
        registerCount = 0;
        grabImageIndex = 0;

        isTestMode = false;

        adjustWithOct = true;
        isSplitLedsInitOn = true;
        isWdotsLedsInitOn = true;
        isSplitMirrorInitIn = true;

        retinaMean = 0.0f;
        retinaStdev = 0.0f;
        targetMean = 0.0f;
        targetStdev = 0.0f;
        targetScore = 0.0f;
        targetPosX = 0.0f;
        targetPosY = 0.0f;

        registPosX = 0.0f;
        registPosY = 0.0f;
		trackingPosX = 0.0f;
		trackingPosY = 0.0f;
		trackingScore = 0.0f;

        scanCenterInitXpx = 0.0f;
        scanCenterInitYpx = 0.0f;
        scanCenterInitXmm = 0.0f;
        scanCenterInitYmm = 0.0f;

        trackingDeltaXpx = 0.0f;
        trackingDeltaYpx = 0.0f;
        trackingDeltaXmm = 0.0f;
        trackingDeltaYmm = 0.0f;

        scanCenterLastXpx = 0.0f;
        scanCenterLastYpx = 0.0f;
        scanCenterLastXmm = 0.0f;
        scanCenterLastYmm = 0.0f;
    }
};

RetinaTracking::RetinaTracking() :
    d_ptr(new RetinaTrackingImpl())
{
}

RetinaTracking::~RetinaTracking()
{

}

RetinaTracking * OctSystem::RetinaTracking::getInstance(void)
{
    static RetinaTracking instance;
    return &instance;
}

bool OctSystem::RetinaTracking::initializeRetinaTracking(OctDevice::MainBoard * board)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    impl().initializeRetinaTrackingImpl();
    impl().mainboard = board;

    impl().trackCamera.initializeTrackingCamera(board);
    return true;
}


bool OctSystem::RetinaTracking::prepareRetinaTracking(const OctScanMeasure& meas)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    clearTrackingStatus();

    impl().protocol = meas;
    impl().status = RetinaTrackingStatus::RTS_PREPARED;

    LogD() << "Retina tracking prepared, side: " << (meas.isOD() ? "OD" : "OS") << ", disc: " << meas.preview().isDiscScan();

    if (impl().protocol.getPattern().isDiscScan()) {
        setInitialScanningCenter(0.0f, 0.0f);
    }
    else {
        float start_x = impl().protocol.isOD() ? RETINA_TRACK_SCAN_CENTER_OFFSET_X_OD : RETINA_TRACK_SCAN_CENTER_OFFSET_X_OS;
        float start_y = RETINA_TRACK_SCAN_CENTER_OFFSET_Y;
        setInitialScanningCenter(start_x, start_y);
    }
    return true;
}

bool OctSystem::RetinaTracking::prepareRetinaTracking(EyeSide side, bool disc, float rangeX, float rangeY)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    clearTrackingStatus();

    auto pattern = disc ? PatternName::Disc3D : PatternName::Macular3D;
    impl().protocol.getPattern().setup(pattern, 512, 96, rangeX, rangeY);
    impl().protocol.setEyeSide(side);
    impl().status = RetinaTrackingStatus::RTS_PREPARED;

    LogD() << "Retina tracking prepared, side: " << (side == EyeSide::OD ? "OD" : "OS") << ", disc: " << disc;

    if (disc) {
        setInitialScanningCenter(0.0f, 0.0f);
    }
    else {
        float start_x = side == EyeSide::OD ? RETINA_TRACK_SCAN_CENTER_OFFSET_X_OD : RETINA_TRACK_SCAN_CENTER_OFFSET_X_OS;
        float start_y = RETINA_TRACK_SCAN_CENTER_OFFSET_Y;
        setInitialScanningCenter(start_x, start_y);
    }
    return true;
}


bool OctSystem::RetinaTracking::startRetinaTracking(bool testMode)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_REGISTERED) {
        relocateScanningCenter(0.0f, 0.0f);
        impl().status = RetinaTrackingStatus::RTS_TRACKING;
		impl().isTestMode = testMode;
        impl().trackingCount = 0;
        impl().grabImageIndex = 0;
        LogD() << "Retina tracking started, test mode: " << testMode;
        /*
        if (testMode) {
            beginRecordingTarget();
        }
        */
       return true;
    }
    return false;
}

void OctSystem::RetinaTracking::pauseRetinaTracking(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().status = RetinaTrackingStatus::RTS_PAUSED;
        LogD() << "Retina tracking paused";
    }
    return;
}

void OctSystem::RetinaTracking::cancelRetinaTracking(bool release)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().status = RetinaTrackingStatus::RTS_CANCELED;
        if (release) {
            impl().trackCamera.releaseAdjustment();
        }
        LogD() << "Retina tracking canceled";
    }
    return;
}

void OctSystem::RetinaTracking::releaseRetinaTracking(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().status = RetinaTrackingStatus::RTS_UNKNOWN;
        impl().trackCamera.releaseAdjustment();
        LogD() << "Retina tracking with camera control released";
    }
    return;
}

bool OctSystem::RetinaTracking::requestFrameAdjustment(bool withOct)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().trackCamera.initiateAdjustment();
        impl().adjustWithOct = withOct;
        impl().status = RetinaTrackingStatus::RTS_ADJUSTING;
        LogD() << "Retina tracking, camera frame adjustment requested";
        return true;
    }
    return false;
}

bool OctSystem::RetinaTracking::completeFrameAdjustment(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status == RetinaTrackingStatus::RTS_ADJUSTING) {
        impl().trackCamera.completeAdjustment();
        impl().status = RetinaTrackingStatus::RTS_ADJUSTED;
        LogD() << "Retina tracking, camera frame adjustment completed.";
        return true;
    }
    return false;
}

bool OctSystem::RetinaTracking::requestTargetRegistration(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().registerCount = 0;
        impl().registPosX = 0.0f;
        impl().registPosY = 0.0f;
        impl().status = RetinaTrackingStatus::RTS_REGISTERING;
        impl().trackRecords.resetSession();
        LogD() << "Retina tracking, target registration requested";
        return true;
    }
    return false;
}

bool OctSystem::RetinaTracking::waitForTargetRegistration(void)
{
	const int REGIST_DELAY = 35;
	const int REGIST_RETRY_MAX = 150;

    auto status = impl().status;
    // if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        LogD() << "Retina tracking, waiting for target registration...";
        for (int i = 0; i < REGIST_RETRY_MAX; i++) {
            this_thread::sleep_for(chrono::milliseconds(REGIST_DELAY));
            if (impl().trackRecords.isReferenceTarget()) {
				impl().status = RetinaTrackingStatus::RTS_REGISTERED;
                LogD() << "Retina tracking, target registration completed";
                return true;
            }
        }
    // }
    LogD() << "Retina tracking, target registration timed out";
    return false;
}


bool OctSystem::RetinaTracking::waitForTargetAlignment(void)
{
    const int ALIGN_DELAY = 35;
    const int ALIGN_RETRY_MAX = 250;
    
    LogD() << "Retina tracking, waiting for target alignment...";
    impl().trackRecords.resumeSession();
    for (int i = 0; i < ALIGN_RETRY_MAX; i++) {
        this_thread::sleep_for(chrono::milliseconds(ALIGN_DELAY));
        if (impl().trackRecords.checkTargetStabilized(2)) {
            impl().trackRecords.closeSession();
            LogD() << "Retina tracking, target alignment completed";
            return true;
        }
    }
    impl().trackRecords.closeSession();
    LogD() << "Retina tracking, target alignment failed (timeout)";
    return false;
}

bool OctSystem::RetinaTracking::waitForTargetNextFrame(int count)
{
    const int NEXT_DELAY = 5;
    const int NEXT_RETRY_MAX = 100;

    int trackCount = impl().trackRecords.getTrackingCount();
    for (int i = 0; i < NEXT_RETRY_MAX; i++) {
        this_thread::sleep_for(chrono::milliseconds(NEXT_DELAY));
        int currCount = impl().trackRecords.getTrackingCount();
        if (currCount >= (trackCount + count)) {
            return true;
        }
    }
    LogD() << "Retina tracking, target next frame timed out";
    return false;
}


void OctSystem::RetinaTracking::processTrackingFrame(const CppUtil::CvImage& frame)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        if (updateTrackingFrame(frame)) {
            if (status == RetinaTrackingStatus::RTS_ADJUSTING) {
                performCameraAdjustment();
            }
            if (status == RetinaTrackingStatus::RTS_REGISTERING) {
                performTargetRegistration();
            }
            if (status == RetinaTrackingStatus::RTS_TRACKING) {
                performTargetDetection();
            }
        }
    }
    return;
}

void OctSystem::RetinaTracking::beginRecordingTarget(bool last_track)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        if (status == RetinaTrackingStatus::RTS_TRACKING) {
            float refer_x = impl().registPosX;
            float refer_y = impl().registPosY;
            impl().trackRecords.beginSession(true, refer_x, refer_y);
        }
        else {
            impl().trackRecords.beginSession(false);
        }

		if (last_track) {
			float targ_x = impl().trackingPosX;
			float targ_y = impl().trackingPosY;
			float targ_s = impl().trackingScore;
			impl().trackRecords.insertRecord(targ_x, targ_y, targ_s);
		}
    }
}

void OctSystem::RetinaTracking::closeRecordingTarget(void)
{
	/*
    if (isTrackingInProgress()) {
        waitForTargetNextFrame(1);
    }
	*/

    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    // if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        impl().trackRecords.closeSession();
    // }
    return;
}

bool OctSystem::RetinaTracking::isRecordingMotionFree(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = impl().trackRecords.hasTargetMotionFreed();
    return result;
}

bool OctSystem::RetinaTracking::isRecordingEyelidCovered(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = impl().trackRecords.hasTargetOccluded();
    return result;
}

void OctSystem::RetinaTracking::setupScanningRegion(float centXmm, float centYmm)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    setInitialScanningCenter(centXmm, centYmm);
    return;
}

bool OctSystem::RetinaTracking::updateScanningRegionByMotion(void)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    float dx = 0.0f;
    float dy = 0.0f;
    float dist = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
    float score = 0.0f;
    float rx = impl().registPosX;
    float ry = impl().registPosY;

    bool result = impl().trackRecords.getLastDistance(rx, ry, x, y, dx, dy, dist, score);
	LogD() << "Last target pos, x: " << x << ", y: " << y << ", dx: " << dx << ", dy: " << dy << ", dist: " << dist << ", score: " << score;
	
	if (result && score >= RETINA_TRACK_TARGET_SCORE_MIN /*&& dist <= RETINA_TRACK_TARGET_DIST_MAX*/) {
		relocateScanningCenter(dx, dy);
		impl().trackingPosX = x;
		impl().trackingPosY = y;
		impl().trackingScore = score;
		LogD() << "Scanning center relocated by tracking target position.";
		return true;
	}
	else {
		// If target location is not validated, retain the current scanning position. 
	}
    return false;
}

void OctSystem::RetinaTracking::getScanningCenterPosition(float& xmm, float& ymm)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    float sx = impl().scanCenterInitXmm;
    float sy = impl().scanCenterInitYmm;
    float dx = impl().trackingDeltaXmm;
    float dy = impl().trackingDeltaYmm;
    xmm = sx + dx;
    ymm = sy + dy;
    return;
}


bool OctSystem::RetinaTracking::getScanningRegionInfo(int& x, int& y, int& w, int& h, bool& pending, bool cropped) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    getTrackingScanRegion(x, y, w, h, true);

    pending = isTargetPending();

    if (cropped) {
        auto center = GlobalSettings::centerOfRetinaROI();
        auto radius = GlobalSettings::radiusOfRetinaROI();
        x = x - (center.first - radius);
        y = y - (center.second - radius);
    }
    return true;
}

bool OctSystem::RetinaTracking::getTrackingRegionInfo(int& x, int& y, int& w, int& h, float& value, bool cropped) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = getTrackingTargetRegion(x, y, w, h, true);

    if (result && isTargetTracking()) {
        value = impl().targetScore;
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

bool OctSystem::RetinaTracking::getScanningRegionInfo2(int& x, int& y, int& w, int& h, bool& pending, bool center) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    getTrackingScanRegion(x, y, w, h, center);
    pending = isTargetPending();
    return true;
}

bool OctSystem::RetinaTracking::getTrackingRegionInfo2(int& x, int& y, int& w, int& h, float& value, bool center) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = getTrackingTargetRegion(x, y, w, h, center);

    if (result) {
        value = impl().targetScore;
    }
    return result;
}

void OctSystem::RetinaTracking::getFrameStatistics(float& roi_mean, float& roi_stdev, float& targ_mean, float& targ_stdev)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    roi_mean = impl().retinaMean;
    roi_stdev = impl().retinaStdev;
    targ_mean = impl().targetMean;
    targ_stdev = impl().targetStdev;
    return;
}

bool OctSystem::RetinaTracking::getTrackingTargetResult(float& score, float& targ_cx, float& targ_cy, float& delta_x, float& delta_y)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    if (isTargetAvailable()) {
        score = impl().targetScore;
        targ_cx = impl().targetPosX;
        targ_cy = impl().targetPosY;
        delta_x = impl().trackingDeltaXpx;
        delta_y = impl().trackingDeltaYpx;
        return true;
    }
    return false;
}

bool OctSystem::RetinaTracking::isTrackingTargetOn(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool track = isTargetTracking();
    bool avail = isTargetAvailable();
    bool result = track && avail;
    return result;
}


bool OctSystem::RetinaTracking::isTrackingReference(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    bool result = isTargetRegistered();
    return result;
}

bool OctSystem::RetinaTracking::isTrackingReady(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    bool result = (status >= RetinaTrackingStatus::RTS_PREPARED);
    return result;
}

bool OctSystem::RetinaTracking::isTrackingInProgress(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    auto status = impl().status;
    bool result = (status >= RetinaTrackingStatus::RTS_TRACKING);
    return result;
}


void OctSystem::RetinaTracking::setGrabbingImageIndex(int index)
{
    lock_guard<mutex> lock(impl().singleMutex_);
    impl().grabImageIndex = index;
    return;
}

int OctSystem::RetinaTracking::getGrabbingImageIndex(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    return impl().grabImageIndex;
}

float OctSystem::RetinaTracking::getGrabbingImageRatio(void) const
{
    lock_guard<mutex> lock(impl().singleMutex_);
    int index = getGrabbingImageIndex();
    int lines = impl().protocol.getPattern().getNumberOfScanLines();
    int overlaps = impl().protocol.getPattern().getScanOverlaps();
    int size = lines * overlaps;
    float ratio = (float)index / (float)size;
    return ratio;
}

void OctSystem::RetinaTracking::setGuideLedsInitStatus(bool splitOn, bool wdotsOn)
{
    impl().isSplitLedsInitOn = splitOn;
    impl().isWdotsLedsInitOn = wdotsOn;
    return;
}

void OctSystem::RetinaTracking::setMirrorMotorInitStatus(bool splitIn)
{
    impl().isSplitMirrorInitIn = splitIn;
    return;
}


bool OctSystem::RetinaTracking::updateTrackingFrame(const CppUtil::CvImage& frame)
{
    TrackingObject obj;
    const auto& image = frame;
    auto isOd = impl().protocol.isOD();
    auto isDisc = impl().protocol.getPattern().isDiscScan();
    
    if (obj.assignTrackingFrame(isOd, isDisc, image)) {
        obj.assessTrackingFrame();
        
        auto ret_mean = obj.retinaPatchMean();
        auto ret_stdev = obj.retinaPatchStdev();
        auto obj_mean = obj.objectPatchMean();
        auto obj_stdev = obj.objectPatchStdev();

        impl().retinaMean = ret_mean;
        impl().retinaStdev = ret_stdev;
        impl().targetMean = obj_mean;
        impl().targetStdev = obj_stdev;
        impl().objFrame = std::move(obj);
        return true;
    }
    return false;
}


void OctSystem::RetinaTracking::performCameraAdjustment(void)
{
    const float QUALITY_OBJECT = 2.5f;
    float qidx = 0.0f;
    bool found = false;
    bool withOct = impl().adjustWithOct;

    if (withOct) {
        if (obtainOctPreviewQualityIndex(qidx)) {
            if (qidx >= QUALITY_OBJECT) {
                found = true;
            }
        }
    }
    else {
        found = true;
    }

    if (found) {
        impl().trackCamera.adjustFrameIntensity(&impl().objFrame);
    }
    return;
}


void OctSystem::RetinaTracking::performTargetRegistration(void)
{
    int count = impl().registerCount;
    if (count == 0) {
        if (impl().objFrame.verifyObjectWithinBox()) {
            float cx = impl().objFrame.objectCenterX();
            float cy = impl().objFrame.objectCenterY();

            impl().objRefer = std::move(impl().objFrame);
            impl().registerCount = 1;
            impl().registPosX = cx;
            impl().registPosY = cy;
            LogD() << "Retina tracking, registering target in position: " << cx << ", " << cy;
        }
        else {
            LogD() << "Retina tracking, registering target not verified";
        }
    }
    else {
        auto patch = impl().objRefer.getObjectPatch();
        impl().objFrame.detectTargetInFrame(patch);

        float tx = impl().objFrame.targetMatchedX();
        float ty = impl().objFrame.targetMatchedY();
        auto score = impl().objFrame.targetMatchScore();

        float rx = impl().registPosX;
        float ry = impl().registPosY;
        float dx = fabs(tx - rx);
        float dy = fabs(ty - ry);

        const float TARGET_SCORE_MIN = 0.85f;
        const float TARGET_X_SPAN = 3.0f;
        const float TARGET_Y_SPAN = 3.0f;
        const int TARGET_NEED_COUNT = 3;

        LogD() << "Retina tracking, registering target detected, dx: " << dx << ", dy: " << dy << ", score: " << score;

        if (score >= TARGET_SCORE_MIN && dx <= TARGET_X_SPAN && dy <= TARGET_Y_SPAN) {
            count += 1;
            if (count >= TARGET_NEED_COUNT) {
                impl().objRefer = std::move(impl().objFrame);
                impl().registPosX = tx;
                impl().registPosY = ty;
				impl().trackingPosX = tx;
				impl().trackingPosY = ty;
				impl().trackingScore = score;
                impl().trackRecords.registerReferenceTarget(tx, ty);
                LogD() << "Retina tracking, target reference registered, count: " << count;
            }
            else {
                LogD() << "Retina tracking, registering target matched, count: " << count;
            }
        }
        else {
            count = 0;
            LogD() << "Retina tracking, registering target not matched, count: " << count;
        }
        impl().registerCount = count;
    }
    return;
}

void OctSystem::RetinaTracking::performTargetDetection(void)
{
    auto patch = impl().objRefer.getTargetPatch();
    if (!patch.isEmpty()) {
        if (impl().objFrame.detectTargetInFrame(patch)) {
            auto score = impl().objFrame.targetMatchScore();
            auto cx = impl().objFrame.targetMatchedX();
            auto cy = impl().objFrame.targetMatchedY();
            impl().targetScore = score;
            impl().targetPosX = cx;
            impl().targetPosY = cy;
            impl().trackingCount += 1;
            impl().trackRecords.insertRecord(impl().objFrame);
        }
    }
    return;
}

void OctSystem::RetinaTracking::setInitialScanningCenter(float startXmm, float startYmm)
{
    impl().scanCenterInitXmm = startXmm;
    impl().scanCenterInitYmm = startYmm;
    impl().trackingDeltaXmm = 0.0f;
    impl().trackingDeltaYmm = 0.0f;
    impl().trackingDeltaXpx = 0.0f;
    impl().trackingDeltaYpx = 0.0f;

    float dx = startXmm / RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM;
    float dy = startYmm / RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM;

    float centerX = RETINA_TRACK_FRAME_CENT_X + dx;
    float centerY = RETINA_TRACK_FRAME_CENT_Y + dy;

    impl().scanCenterInitXpx = centerX;
    impl().scanCenterInitYpx = centerY;
    impl().scanCenterLastXpx = centerX;
    impl().scanCenterLastYpx = centerY;
    impl().scanCenterLastXmm = startXmm;
    impl().scanCenterLastYmm = startYmm;

    LogD() << "Retina tracking, scanning center disposed: " << startXmm << ", " << startYmm << " => center pos: " << centerX << ", " << centerY << ", center mm: " << startXmm << ", " << startYmm;
    return;
}

void OctSystem::RetinaTracking::relocateScanningCenter(float deltaX, float deltaY)
{
    float xmm = deltaX * RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM;
    float ymm = deltaY * RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM;

    impl().trackingDeltaXmm = xmm;
    impl().trackingDeltaYmm = ymm;
    impl().trackingDeltaXpx = deltaX;
    impl().trackingDeltaYpx = deltaY;

    float cx_pos = impl().scanCenterInitXpx;
    float cy_pos = impl().scanCenterInitYpx;
    float cx_mm = impl().scanCenterInitXmm;
    float cy_mm = impl().scanCenterInitYmm;
    float nx_pos = cx_pos + deltaX;
    float ny_pos = cy_pos + deltaY;
    float nx_mm = cx_mm + xmm;
    float ny_mm = cy_mm + ymm;

    impl().scanCenterLastXpx = nx_pos;
    impl().scanCenterLastYpx = ny_pos;
    impl().scanCenterLastXmm = nx_mm;
    impl().scanCenterLastYmm = ny_mm;

    LogD() << "Retina tracking, scanning center realigned: " << deltaX << ", " << deltaY << " => center pos: " << nx_pos << ", " << ny_pos << ", center mm: " << nx_mm << ", " << ny_mm;
    return;
}

void OctSystem::RetinaTracking::getTrackingScanRegion(int& x, int& y, int& w, int& h, bool center) const
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    w = (int)(impl().protocol.getPattern().getScanRangeX() / RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM);
    h = (int)(impl().protocol.getPattern().getScanRangeY() / RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM);
    x = (int)(impl().scanCenterInitXpx + impl().trackingDeltaXpx);
    y = (int)(impl().scanCenterInitYpx + impl().trackingDeltaYpx);

    if (!center) {
        x = x - w / 2;
        y = y - h / 2;
    }
    return;
}

bool OctSystem::RetinaTracking::getTrackingTargetRegion(int& x, int& y, int& w, int& h, bool center) const
{
    // lock_guard<mutex> lock(impl().singleMutex_);
    if (impl().trackingCount > 0) {
        w = RETINA_TRACK_BBOX_WIDTH;
        h = RETINA_TRACK_BBOX_HEIGHT;
        x = (int)impl().targetPosX;
        y = (int)impl().targetPosY;

        if (!center) {
            x = x - w / 2;
            y = y - h / 2;
        }
        return true;
    }
    return false;
}


bool OctSystem::RetinaTracking::isTargetAvailable(void) const
{
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        bool result = (impl().trackingCount > 0);
        return result;
    }
    return false;
}


bool OctSystem::RetinaTracking::isTargetRegistered(void) const
{
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        bool result = (status == RetinaTrackingStatus::RTS_REGISTERED);
        return result;
    }
    return false;
}


bool OctSystem::RetinaTracking::isTargetPending(void) const
{
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        bool result = (status != RetinaTrackingStatus::RTS_TRACKING);
        return result;
    }
    return false;
}

bool OctSystem::RetinaTracking::isTargetTracking(void) const
{
    auto status = impl().status;
    if (status >= RetinaTrackingStatus::RTS_PREPARED) {
        bool result = (status == RetinaTrackingStatus::RTS_TRACKING);
        return result;
    }
    return false;
}

void OctSystem::RetinaTracking::clearTrackingStatus(void)
{
    impl().adjustWithOct = true;
    impl().status = RetinaTrackingStatus::RTS_UNKNOWN;

    impl().trackingCount = 0;
    impl().registerCount = 0;
    impl().grabImageIndex = 0;
    impl().isTestMode = false;

    impl().retinaMean = 0.0f;
    impl().retinaStdev = 0.0f;
    impl().targetMean = 0.0f;
    impl().targetStdev = 0.0f;
    impl().targetScore = 0.0f;
    impl().targetPosX = 0.0f;
    impl().targetPosY = 0.0f;

    impl().registPosX = 0.0f;
    impl().registPosY = 0.0f;

    impl().scanCenterInitXpx = 0.0f;
    impl().scanCenterInitYpx = 0.0f;
    impl().scanCenterInitXmm = 0.0f;
    impl().scanCenterInitYmm = 0.0f;

    impl().trackingDeltaXpx = 0.0f;
    impl().trackingDeltaYpx = 0.0f;
    impl().trackingDeltaXmm = 0.0f;
    impl().trackingDeltaYmm = 0.0f;

    impl().scanCenterLastXpx = 0.0f;
    impl().scanCenterLastYpx = 0.0f;
    impl().scanCenterLastXmm = 0.0f;
    impl().scanCenterLastYmm = 0.0f;
    return;
}


bool OctSystem::RetinaTracking::obtainOctPreviewQualityIndex(float& qidx)
{
	float index, ratio;
	unsigned long count;

	if (ChainOutput::getPreviewImageResult(&index, &ratio, nullptr, &count)) {
		qidx = index;
		return true;
	}
	return false;
}

RetinaTracking::RetinaTrackingImpl& OctSystem::RetinaTracking::impl(void) const
{
	return *d_ptr;
}


