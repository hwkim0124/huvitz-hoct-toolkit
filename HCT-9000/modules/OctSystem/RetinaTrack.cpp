#include "stdafx.h"
#include "RetinaTrack.h"

#include <mutex>

#include "OctDevice2.h"
#include "RetFocus2.h"
#include "CppUtil2.h"
#include "GlobalSettings.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace RetFocus;
using namespace CppUtil;
using namespace std;



struct RetinaTrack::RetinaTrackImpl
{
	TrackImage target;
	TrackImage cframe;

	bool registered;
	bool tracking;
	bool blinked;
	bool motorMoving;
	bool motorEnabled;
	bool grabHolding;

	float detectCx;
	float detectCy;
	bool detectOn;

	float targetCx;
	float targetCy;
	int targetCount;
	int errorCount;

	int grabbedImageIdx;

	float focusDist = 3.0f;

	RetinaTrackImpl() : registered(false), tracking(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<RetinaTrack::RetinaTrackImpl> RetinaTrack::d_ptr(new RetinaTrackImpl());


RetinaTrack::RetinaTrack()
{
}


RetinaTrack::~RetinaTrack()
{
}


void OctSystem::RetinaTrack::initializeRetinaTrack(void)
{
	getImpl().registered = false;
	getImpl().tracking = false;
	getImpl().blinked = false;

	getImpl().motorMoving = false;
	getImpl().grabHolding = false;
	getImpl().motorEnabled = false;

	getImpl().detectCx = 0;
	getImpl().detectCy = 0;
	getImpl().detectOn = false;

	getImpl().targetCx = 0;
	getImpl().targetCy = 0;
	getImpl().targetCount = 0;
	getImpl().errorCount = 0;

	getImpl().grabbedImageIdx = 0;
	return;
}


bool OctSystem::RetinaTrack::registRetinaFrame(const RetFocus::RetinaFrame & frame)
{
	auto& image = frame.getImage();
	if (image.isEmpty()) {
		return false;
	}

	bool found = false;
	bool result = false;
	bool blinked = false;

	TrackImage track;
	track.setSource(image);

	const int TARGET_X_SPAN = 9;
	const int TARGET_Y_SPAN = 9;
	const int TARGET_NEED_COUNT = 3;

	if (track.checkIfEyelidsCovered()) {
		blinked = true;
	}
	else {
		if (track.decideTargetRegion()) {
			if (track.isValidTarget()) 
			{
				found = true;
				LogD() << "Retina target regist, pos: " << track.getTargetCx() << ", " << track.getTargetCy() << ", count: " << getImpl().targetCount;

				if (abs(track.getTargetCx() - getImpl().targetCx) <= TARGET_X_SPAN && 
					abs(track.getTargetCy() - getImpl().targetCy) <= TARGET_Y_SPAN) 
				{
					getImpl().targetCount += 1;
					if (getImpl().targetCount >= TARGET_NEED_COUNT) {
						getTrackCamera().setTarget(track.getTarget());
						d_ptr->target = std::move(track);
						result = true;
					}
				}
				else {
					getImpl().targetCx = (float)track.getTargetCx();
					getImpl().targetCy = (float)track.getTargetCy();
				}
			}
		}
	}
	
	if (!found) {
		getImpl().targetCount = 0;
	}
	getImpl().blinked = blinked;
	getImpl().registered = result;
	return result;
}


bool OctSystem::RetinaTrack::detectTargetOnFrame(const RetFocus::RetinaFrame & frame)
{
	auto& image = frame.getImage();
	if (image.isEmpty()) {
		return false;
	}

	bool blinked = false;

	getTrackCamera().setSource(image);

	bool result = false;
	if (getTrackCamera().checkIfEyelidsCovered()) {
		blinked = true;
	}
	else {
		if (getTrackCamera().measureSimilarity()) {
			result = true;
		}
	}

	if (result) {
		getImpl().detectCx = (float)getTrackCamera().getMatchedCx();
		getImpl().detectCy = (float)getTrackCamera().getMatchedCy();
		getImpl().detectOn = true;

		result = !checkIfTargetDisplaced();
	}
	else {
		getImpl().detectCx = 0;
		getImpl().detectCy = 0;
		getImpl().detectOn = false;
	}

	getImpl().blinked = blinked;
	getImpl().errorCount = (result ? 0 : getImpl().errorCount + 1);
	return result;
}


bool OctSystem::RetinaTrack::registCorneaTarget(float centerX, float centerY)
{
	const int TARGET_X_SPAN = 5;
	const int TARGET_Y_SPAN = 5;
	// const float TARGET_DIST_SPAN = 7.0f;
	float TARGET_DIST_SPAN = getTargetFocusDistance();
	const int TARGET_NEED_COUNT = 5;

	float dx = centerX - getImpl().targetCx;
	float dy = centerY - getImpl().targetCy;
	if (sqrt(dx*dx+dy*dy) <= TARGET_DIST_SPAN) {
		LogD() << "Retina target regist, pos: " << centerX << ", " << centerY << ", count: " << getImpl().targetCount;
		getImpl().targetCount += 1;

		if (getImpl().targetCount > TARGET_NEED_COUNT) {
			getImpl().registered = true;
			getImpl().targetCount = 0;
			return true;
		}
	}
	else {
		getImpl().targetCx = centerX;
		getImpl().targetCy = centerY;
		getImpl().targetCount = 0;
		getImpl().errorCount += 1;
	}
	return false;
}


bool OctSystem::RetinaTrack::detectCorneaCenter(bool found, float centerX, float centerY)
{
	float TARGET_DIST_SPAN = getTargetFocusDistance();
	float dx = centerX - getImpl().detectCx;
	float dy = centerY - getImpl().detectCy;

	if (found && fabs(dx) <= TARGET_DIST_SPAN && fabs(dy) <= TARGET_DIST_SPAN) {
		getImpl().targetCount += 1;
	}
	else {
		getImpl().targetCount = 0;
	}

	if (found) {
		getImpl().detectCx = centerX;
		getImpl().detectCy = centerY;
	}

	getImpl().detectOn = found;
	getImpl().errorCount += (found ? 0 : 1);
	return true;
}


void OctSystem::RetinaTrack::setTargetFocusDistance(float dist)
{
	getImpl().focusDist = dist;
	return;
}


float OctSystem::RetinaTrack::getTargetFocusDistance(void)
{
	return getImpl().focusDist;
}


bool OctSystem::RetinaTrack::getCorneaTargetOffset(float & distX, float & distY)
{
	float centerX = getImpl().detectCx;
	float centerY = getImpl().detectCy;
	distX = centerX - getImpl().targetCx;
	distY = centerY - getImpl().targetCy;
	return true;
}


bool OctSystem::RetinaTrack::checkIfCorneaTargetDisplaced(void)
{
	const int TARGET_X_SPAN = 5;
	const int TARGET_Y_SPAN = 5;
	// const float TARGET_DIST_SPAN = 7.0f;
	float TARGET_DIST_SPAN = getTargetFocusDistance();

	float centerX = getImpl().detectCx;
	float centerY = getImpl().detectCy;
	float dx = centerX - getImpl().targetCx;
	float dy = centerY - getImpl().targetCy;

	// if (sqrt(dx*dx + dy*dy) > TARGET_DIST_SPAN) {
	if (fabs(dx) > TARGET_DIST_SPAN || fabs(dy) > TARGET_DIST_SPAN) {
		if (getImpl().errorCount == 0) {
			LogD() << "Retina track target, pos: " << centerX << ", " << centerY << ", target: " << getImpl().targetCx << ", " << getImpl().targetCy << ", offset: " << dx << ", " << dy << ", error: " << getImpl().errorCount;
		}
		getImpl().errorCount += 1;
		return true;
	}
	else {
		if (getImpl().errorCount > 0) {
			LogD() << "Retina track target, pos: " << centerX << ", " << centerY << ", target: " << getImpl().targetCx << ", " << getImpl().targetCy << ", offset: " << dx << ", " << dy;
		}
		getImpl().errorCount = 0;
	}
	return false;
}


bool OctSystem::RetinaTrack::checkIfCorneaTargetMoving(void)
{
	if (getImpl().targetCount <= 5) {
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::checkIfTargetDisplaced(void)
{
	int tx, ty;
	getTargetCenter(tx, ty);

	int dx, dy;
	getDetectedCenter(dx, dy);

	const int TARGET_X_SPAN = 7; // 9;
	const int TARGET_Y_SPAN = 5; // 9;
	const float TARGET_MATCH_VALUE = 0.9f;

	int ex = abs(tx - dy);
	int ey = abs(ty - dy);

	LogD() << "Retina track target, offset: " << ex << ", " << ey << ", similarity: " << getTrackCamera().getMatchedValue();

	if (abs(tx - dx) > TARGET_X_SPAN || abs(ty - dy) > TARGET_Y_SPAN) {
		return true;
	}
	if (getTrackCamera().getMatchedValue() < TARGET_MATCH_VALUE) {
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::checkIfEyeBlinked(void)
{
	return getImpl().blinked;
}


bool OctSystem::RetinaTrack::checkIfDetectionSuspended(void)
{
	int ERROR_COUNT_MAX = (30 * 20);
	if (getImpl().errorCount > ERROR_COUNT_MAX) {
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::getTargetImage(CppUtil::CvImage & image)
{
	if (!getTrackTarget().isEmpty()) {
		image = getTrackTarget().getTarget();
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::getTargetCenter(int & x, int & y)
{
	if (isTargetRegistered()) {
		/*
		x = getTrackTarget().getTargetCx();
		y = getTrackTarget().getTargetCy();
		*/
		x = (int)getImpl().targetCx;
		y = (int)getImpl().targetCy;
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::getTargetSize(int & w, int & h)
{
	if (isTargetRegistered()) {
		w = getTrackTarget().getTargetWidth();
		h = getTrackTarget().getTargetHeight();
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::getDetectedCenter(int & x, int & y)
{
	if (isTargetDetected()) {
		/*
		x = getTrackCamera().getMatchedCx();
		y = getTrackCamera().getMatchedCy();
		*/
		x = (int)getImpl().detectCx;
		y = (int)getImpl().detectCy;
		return true;
	}
	return false;
}


bool OctSystem::RetinaTrack::getDetectedValue(float & value)
{
	if (isTargetDetected()) {
		value = getTrackCamera().getMatchedValue();
		return true;
	}
	return false;
}


void OctSystem::RetinaTrack::startTracking(void)
{
	initializeRetinaTrack();
	getImpl().tracking = true;
	return;
}


void OctSystem::RetinaTrack::cancelTracking(void)
{
	getImpl().tracking = false;
	return;
}


void OctSystem::RetinaTrack::setMotorControl(bool flag)
{
	getImpl().motorEnabled = flag;
}


bool OctSystem::RetinaTrack::isTrackingStarted(void)
{
	return getImpl().tracking;
}


bool OctSystem::RetinaTrack::isTargetRegistered(void)
{
	return getImpl().registered;
}


bool OctSystem::RetinaTrack::isTargetDetected(void)
{
	/*
	if (getTrackCamera().isMatched()) {
		return true;
	}
	*/
	if (getImpl().detectOn) {
		return true;
	}
	return false;
}

bool OctSystem::RetinaTrack::isMotorEnabled(void)
{
	return getImpl().motorEnabled;
}

bool OctSystem::RetinaTrack::isMotorMoving(void)
{
	return getImpl().motorMoving;
}

bool OctSystem::RetinaTrack::isGrabHolding(void)
{
	return getImpl().grabHolding;
}

void OctSystem::RetinaTrack::setMotorMoving(bool flag)
{
	getImpl().motorMoving = flag;
	return;
}

void OctSystem::RetinaTrack::setGrabHolding(bool flag)
{
	getImpl().grabHolding = flag;
	return;
}

int OctSystem::RetinaTrack::getLastImageIndexGrabbed(void)
{
	return getImpl().grabbedImageIdx;
}

void OctSystem::RetinaTrack::setLastImageIndexGrabbed(int index)
{
	getImpl().grabbedImageIdx = index;
	return;
}

bool OctSystem::RetinaTrack::waitForTargetRegistered(void)
{
	const int RETINA_NEXT_FRAME_DELAY = 75;

	for (int i = 0; i < 20; i++) {
		if (isTargetRegistered()) {
			return true;
		}
		this_thread::sleep_for(chrono::milliseconds(RETINA_NEXT_FRAME_DELAY));
	}
	return false;
}


RetFocus::TrackImage & OctSystem::RetinaTrack::getTrackCamera(void)
{
	return getImpl().cframe;
}


RetFocus::TrackImage & OctSystem::RetinaTrack::getTrackTarget(void)
{
	return getImpl().target;
}

RetinaTrack::RetinaTrackImpl & OctSystem::RetinaTrack::getImpl(void)
{
	return *d_ptr;
}
