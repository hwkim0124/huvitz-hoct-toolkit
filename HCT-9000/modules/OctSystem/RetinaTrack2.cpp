#include "stdafx.h"
#include "RetinaTrack2.h"

#include <mutex>
#include <atomic>

#include "OctDevice2.h"
#include "RetFocus2.h"
#include "CppUtil2.h"
#include "GlobalSettings.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace RetFocus;
using namespace CppUtil;
using namespace std;


struct RetinaTrack2::RetinaTrack2Impl
{
	TrackImage2 registImage;
	TrackImage2 cameraImage;

	EyeSide eyeSide;
	bool isDiscScan;

	float scanOffsetX;
	float scanOffsetY;
	float scanRangeX;
	float scanRangeY;
	float scanCenterPosX;
	float scanCenterPosY;
	float scanMovedX;
	float scanMovedY;

	float registCx;
	float registCy;
	int registCount;

	atomic<bool> prepared;
	atomic<bool> registered;
	atomic<bool> tracking;
	atomic<bool> pending;
	atomic<bool> canceled;

	bool targetDiplaced;
	bool targetLocated;
	int targetCount;
	float targetRange;

	float targetLastX;
	float targetLastY;

	int grabbedImageIdx;

	RetinaTrack2Impl() : registered(false), tracking(false), pending(false), prepared(false), canceled(false),
						targetDiplaced(false), targetLocated(false), targetCount(0), targetRange(1.5f)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<RetinaTrack2::RetinaTrack2Impl> RetinaTrack2::d_ptr(new RetinaTrack2Impl());


RetinaTrack2::RetinaTrack2()
{
}


RetinaTrack2::~RetinaTrack2()
{
}


void OctSystem::RetinaTrack2::prepareTracking(OctScanMeasure & meas)
{
	initiateTracking();

	float offsetX = (meas.isOD() ? FUNDUS_SCAN_CENTER_OFFSET_X_OD : FUNDUS_SCAN_CENTER_OFFSET_X_OS);
	float offsetY = FUNDUS_SCAN_CENTER_OFFSET_Y;

	bool isDisc = false;
	if (meas.getPattern().isDiscScan()) {
		offsetX = 0.0f;
		offsetY = 0.0f;
		isDisc = true;
	}
	else if (meas.getPattern().getScanRangeX() > 9.0f) {
		offsetX = 0.0f;
		offsetY = 0.0f;
	}

	getImpl().eyeSide = meas.getEyeSide();
	getImpl().isDiscScan = isDisc;
	getImpl().scanRangeX = meas.getPattern().getScanRangeX();
	getImpl().scanRangeY = meas.getPattern().getScanRangeY();

	resetScanCenterPosition(offsetX, offsetY);
	getImpl().prepared = true;

	LogD() << "Prepare retina track, scan offset x: " << getImpl().scanOffsetX << ", y: " << getImpl().scanOffsetY << ", isDisc: " << isDisc;
	return;
}


void OctSystem::RetinaTrack2::prepareTracking(EyeSide side, bool disc, float rangeX, float rangeY)
{
	initiateTracking();

	float offsetX = (side == EyeSide::OD ? FUNDUS_SCAN_CENTER_OFFSET_X_OD : FUNDUS_SCAN_CENTER_OFFSET_X_OS);
	float offsetY = FUNDUS_SCAN_CENTER_OFFSET_Y;

	if (disc) {
		offsetX = 0.0f;
		offsetY = 0.0f;
	}
	else if (rangeX > 9.0f) {
		offsetX = 0.0f;
		offsetY = 0.0f;
	}

	getImpl().eyeSide = side;
	getImpl().isDiscScan = disc;
	getImpl().scanRangeX = rangeX;
	getImpl().scanRangeY = rangeY;

	resetScanCenterPosition(offsetX, offsetY);
	getImpl().prepared = true;

	LogD() << "Prepare retina track, scan offset x: " << getImpl().scanOffsetX << ", y: " << getImpl().scanOffsetY;
	return;
}


void OctSystem::RetinaTrack2::startTracking(void)
{
	getImpl().tracking = true;

	getImpl().registered = false;
	getImpl().registCx = 0.0f;
	getImpl().registCy = 0.0f;
	getImpl().registCount = 0;

	// getImpl().targetLocated = false;
	// getImpl().targetDiplaced = false;
	getImpl().targetCount = 0;
	return;
}


void OctSystem::RetinaTrack2::cancelTracking(void)
{
	if (isTrackingRunning()) {
		getImpl().tracking = false;
		getImpl().canceled = true;
		getImpl().prepared = false;
		getImpl().registered = false;
		LogD() << "Retina tracking canceled";
	}
	return;
}


void OctSystem::RetinaTrack2::updateScanCenterPosition(float dx, float dy)
{
	float fx = dx * RETT_CAMERA_PIXEL_RESOL_IN_MM;
	float fy = dy * RETT_CAMERA_PIXEL_RESOL_IN_MM;
	float ox = getImpl().scanOffsetX;
	float oy = getImpl().scanOffsetY;

	getImpl().scanMovedX = fx;
	getImpl().scanMovedY = fy;

	getImpl().scanCenterPosX = RETT_CAMERA_FRAME_CENT_X + (fx + ox) / RETT_CAMERA_PIXEL_RESOL_IN_MM;
	getImpl().scanCenterPosY = RETT_CAMERA_FRAME_CENT_Y + (fy + oy) / RETT_CAMERA_PIXEL_RESOL_IN_MM;
	return;
}


bool OctSystem::RetinaTrack2::getTrackingMatchedInfo(int & cx, int & cy, int & w, int & h, float & val, bool cropped)
{
	float tx, ty, tw, th, tv;
	bool result = false;

	if (isTargetRegistered()) {
		if (getCameraImage().getMatchedInfo(tx, ty, tw, th, tv)) {
			cx = (int)tx;
			cy = (int)ty;
			w = (int)tw;
			h = (int)th;
			val = tv;
			result = true;
		}
	}

	if (result) {
		if (cropped) {
			auto center = GlobalSettings::centerOfRetinaROI();
			auto radius = GlobalSettings::radiusOfRetinaROI();
			cx = cx - (center.first - radius);
			cy = cy - (center.second - radius);
		}
	}

	return result;
}


bool OctSystem::RetinaTrack2::getTrackingTargetInfo(int & cx, int & cy, int & w, int & h, float & val, bool cropped)
{
	float tx, ty, tw, th, tv;
	bool result = false;

	if (isTrackingPrepared()) {
		if (getRegistImage().getTargetInfo(tx, ty, tw, th, tv)) {
			cx = (int)tx;
			cy = (int)ty;
			w = (int)tw;
			h = (int)th;
			val = tv;
			result = true;
		}
	}

	if (result) {
		if (cropped) {
			auto center = GlobalSettings::centerOfRetinaROI();
			auto radius = GlobalSettings::radiusOfRetinaROI();
			cx = cx - (center.first - radius);
			cy = cy - (center.second - radius);
		}
	}

	return result;
}


bool OctSystem::RetinaTrack2::getTrackingRegionInfo(int & cx, int & cy, int & w, int & h, float& val, bool cropped)
{
	float tx, ty, tw, th, tv;
	bool result = false;

	if (isTargetRegistered()) {
		if (getCameraImage().getMatchedInfo(tx, ty, tw, th, tv)) {
			cx = (int)tx;
			cy = (int)ty;
			w = (int)tw;
			h = (int)th;
			val = tv;
			result = true;
		}
	}
	else if (isTrackingPrepared()) {
		if (getRegistImage().getTargetInfo(tx, ty, tw, th, tv)) {
			cx = (int)tx;
			cy = (int)ty;
			w = (int)tw;
			h = (int)th;
			val = tv;
			result = true;
		}
	}

	if (result) {
		if (cropped) {
			auto center = GlobalSettings::centerOfRetinaROI();
			auto radius = GlobalSettings::radiusOfRetinaROI();
			cx = cx - (center.first - radius);
			cy = cy - (center.second - radius);
		}
	}

	return result;
}


bool OctSystem::RetinaTrack2::getScanningRegionInfo(int & cx, int & cy, int & w, int & h, bool & pending, bool cropped)
{
	if (!isTrackingPrepared()) {
		return false;
	}

	getScanCenterPosition(cx, cy);
	getScanRangeSize(w, h);
	pending = isPendingScanning();

	if (cropped) {
		auto center = GlobalSettings::centerOfRetinaROI();
		auto radius = GlobalSettings::radiusOfRetinaROI();
		cx = cx - (center.first - radius);
		cy = cy - (center.second - radius);
	}

	return true;
}


void OctSystem::RetinaTrack2::getScanCenterPosition(int & cx, int & cy)
{
	cx = (int)getImpl().scanCenterPosX;
	cy = (int)getImpl().scanCenterPosY;
	return;
}


void OctSystem::RetinaTrack2::getScanCenterOffset(float & cx, float & cy)
{
	cx = (getImpl().scanOffsetX + getImpl().scanMovedX);
	cy = (getImpl().scanOffsetY + getImpl().scanMovedY);
	return;
}


void OctSystem::RetinaTrack2::setScanCenterOffset(float cx, float cy)
{
	getImpl().scanOffsetX = cx;
	getImpl().scanOffsetY = cy;
	return;
}


void OctSystem::RetinaTrack2::getScanRangeSize(int & w, int & h)
{
	w = (int)(getImpl().scanRangeX / RETT_CAMERA_PIXEL_RESOL_IN_MM);
	h = (int)(getImpl().scanRangeY / RETT_CAMERA_PIXEL_RESOL_IN_MM);
	return;
}


bool OctSystem::RetinaTrack2::registRetinaFrame(const RetFocus::RetinaFrame & frame)
{
	auto& image = frame.getImage();
	if (image.isEmpty()) {
		return false;
	}

	TrackImage2 track;
	track.setFrame(image);
	
	bool found = false;
	if (track.appointTargetOnFrame(getImpl().eyeSide, getImpl().isDiscScan)) {
		if (track.targetVal() >= 0.15f) {
			found = true;
		}
	}

	const float TARGET_DIFF_MAX = 3.0f;
	const int TARGET_NEED_COUNT = 2;

	if (found) {
		float tx = track.targetCx();
		float ty = track.targetCy();
		float ref_x = getImpl().registCx;
		float ref_y = getImpl().registCy;
		float dist = sqrt(pow(tx - ref_x, 2.0f) + pow(ty - ref_y, 2.0f));

		getImpl().registImage = std::move(track);
		getImpl().registCx = tx;
		getImpl().registCy = ty;

		// LogD() << "Retina track target found, cx: " << tx << ", cy: " << ty << ", val: " << track.targetVal() << ", dist: " << dist << ", count: " << getImpl().registCount;

		if (dist <= TARGET_DIFF_MAX) {
			getImpl().registCount += 1;
			if (getImpl().registCount > TARGET_NEED_COUNT) {
				return true;
			}
		}
		else {
			getImpl().registCount = 0;
		}
	}
	else {
		float tx = track.targetCx();
		float ty = track.targetCy();
		getImpl().registCx = 0.0f;
		getImpl().registCy = 0.0f;
		getImpl().registCount = 0;
	}
	return false;
}


bool OctSystem::RetinaTrack2::searchTargetOnFrame(const RetFocus::RetinaFrame & frame)
{
	auto& image = frame.getImage();
	if (image.isEmpty()) {
		return false;
	}

	TrackImage2 track;
	track.setFrame(image);
	track.setTarget(getRegistImage().getTarget());

	if (track.measureSimilarity()) {
		getImpl().cameraImage = std::move(track);
	}
	return true;
}


bool OctSystem::RetinaTrack2::adjustScanningRegion(void)
{
	if (!getCameraImage().isMatched()) {
		setTargetDisplaced(true);
		// LogD() << "Target not matched";
		return false;
	}


	float mx = getCameraImage().matchedCx();
	float my = getCameraImage().matchedCy();
	float lastX = getImpl().targetLastX;
	float lastY = getImpl().targetLastY;

	float dist = sqrt(pow(mx - lastX, 2.0f) + pow(my - lastY, 2.0f));

	if (!(lastX * lastY) || dist > getImpl().targetRange) {
		setTargetDisplaced(true);
	}

	// To prevent the target from gradually moving its position within the allowed distance 
	// at the subsequent frames.  
	if (getImpl().targetCount == 0) {
		getImpl().targetLastX = mx;
		getImpl().targetLastY = my;
	}

	getImpl().targetCount += 1;

	// LogD() << "Target distance: " << dist << ", count: " << getImpl().targetCount;

	float dx = (mx - getRegistImage().targetCx());
	float dy = (my - getRegistImage().targetCy());

	updateScanCenterPosition(dx, dy);
	return true;
}


bool OctSystem::RetinaTrack2::isTrackingPrepared(void)
{
	return getImpl().prepared;
}

bool OctSystem::RetinaTrack2::isTrackingCanceled(void)
{
	return getImpl().canceled;
}


bool OctSystem::RetinaTrack2::isTrackingRunning(void)
{
	return getImpl().tracking;
}


bool OctSystem::RetinaTrack2::isTargetRegistered(void)
{
	return getImpl().registered;
}


bool OctSystem::RetinaTrack2::isTargetDisplaced(void)
{
	return getImpl().targetDiplaced;
}


bool OctSystem::RetinaTrack2::isTargetLocated(void)
{
	return (!isTargetDisplaced() && getImpl().targetCount > RETINA_TARCK_TARGET_NEED_COUNT);
}


bool OctSystem::RetinaTrack2::isPendingScanning(void)
{
	return getImpl().pending;
}


void OctSystem::RetinaTrack2::setTargetRegistered(bool flag)
{
	getImpl().registered = flag;
	// LogD() << "Retina track, target registered";
	return;
}

void OctSystem::RetinaTrack2::setTargetDisplaced(bool flag)
{
	getImpl().targetDiplaced = flag;
	getImpl().targetCount = (flag ? 0 : getImpl().targetCount);
	return;
}


void OctSystem::RetinaTrack2::setPendingScanning(bool flag)
{
	getImpl().pending = flag;
	return;
}


void OctSystem::RetinaTrack2::setTargetValidRange(float range)
{
	getImpl().targetRange = range;
	LogD() << "Retina track, target valid range: " << range;
	return;
}


int OctSystem::RetinaTrack2::getLastImageIndexGrabbed(void)
{
	return getImpl().grabbedImageIdx;
}


void OctSystem::RetinaTrack2::setLastImageIndexGrabbed(int index)
{
	getImpl().grabbedImageIdx = max(index, 0);
	return;
}


RetFocus::TrackImage2 & OctSystem::RetinaTrack2::getCameraImage(void)
{
	return getImpl().cameraImage;
}


RetFocus::TrackImage2 & OctSystem::RetinaTrack2::getRegistImage(void)
{
	return getImpl().registImage;
}


void OctSystem::RetinaTrack2::initiateTracking(void)
{
	getImpl().prepared = false;
	getImpl().registered = false;
	getImpl().tracking = false;
	getImpl().pending = false;
	getImpl().canceled = false;

	getImpl().scanOffsetX = 0.0f;
	getImpl().scanOffsetY = 0.0f;
	getImpl().scanRangeX = 3.0f;
	getImpl().scanRangeY = 3.0f;
	getImpl().scanMovedX = 0.0f;
	getImpl().scanMovedY = 0.0f;
	getImpl().scanCenterPosX = RETT_CAMERA_FRAME_CENT_X;
	getImpl().scanCenterPosY = RETT_CAMERA_FRAME_CENT_Y;

	getImpl().grabbedImageIdx = 0;

	getImpl().registCx = 0.0f;
	getImpl().registCy = 0.0f;
	getImpl().registCount = 0;

	getImpl().targetLocated = false;
	getImpl().targetDiplaced = false;
	getImpl().targetCount = 0;

	getImpl().targetLastX = 0.0f;
	getImpl().targetLastY = 0.0f;

	getCameraImage().initializeTrackImage();
	getRegistImage().initializeTrackImage();
	return;
}


void OctSystem::RetinaTrack2::resetScanCenterPosition(float offsetX, float offsetY)
{
	getImpl().scanOffsetX = offsetX;
	getImpl().scanOffsetY = offsetY;
	getImpl().scanMovedX = 0.0f;
	getImpl().scanMovedY = 0.0f;

	float dx = offsetX / RETT_CAMERA_PIXEL_RESOL_IN_MM;
	float dy = offsetY / RETT_CAMERA_PIXEL_RESOL_IN_MM;

	getImpl().scanCenterPosX = dx + RETT_CAMERA_FRAME_CENT_X;
	getImpl().scanCenterPosY = dy + RETT_CAMERA_FRAME_CENT_Y;

	// LogD() << "Scan center: " << getImpl().scanCenterX << ", " << getImpl().scanCenterY << ", " << offsetX << ", " << offsetY;
	return;
}



RetinaTrack2::RetinaTrack2Impl & OctSystem::RetinaTrack2::getImpl(void)
{
	return *d_ptr;
}
