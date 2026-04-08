#include "stdafx.h"
#include "CorneaAlign.h"
#include "RetinaTrack.h"

#include <mutex>
#include <atomic>
#include <functional>

#include "OctDevice2.h"
#include "CppUtil2.h"
#include "OctConfig2.h"
#include "KerFocus2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace KerFocus;
using namespace std;


struct CorneaAlign::CorneaAlignImpl
{
	bool initiated;
	MainBoard* board;
	KeratoFocus kerFocus;
	CorneaAlignTarget targetMode;

	atomic<bool> autoAlign;
	atomic<bool> stageXon;
	atomic<bool> stageYon;
	atomic<bool> stageZon;

	IrCameraFrameCallback cbFrameFunc;
	CorneaCameraImageCallback* cbImageFunc;
	mutex mutexLock;

	CorneaAlignImpl() : initiated(false), board(nullptr), cbFrameFunc(nullptr), cbImageFunc(nullptr), 
		autoAlign(false), stageXon(true), stageYon(true), stageZon(true), targetMode(CorneaAlignTarget::CENTER)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<CorneaAlign::CorneaAlignImpl> CorneaAlign::d_ptr(new CorneaAlignImpl());


CorneaAlign::CorneaAlign()
{
}


CorneaAlign::~CorneaAlign()
{
}


bool OctSystem::CorneaAlign::initCorneaAlign(MainBoard * board)
{
	getImpl().board = board;
	getImpl().initiated = true;

	// The type of a pointer to static member function looks like a pointer to non-member function.
	// It doesn't require this argument since static functions are bound to class not to the object. 
	getImpl().cbFrameFunc = std::bind(&CorneaAlign::callbackIrCameraFrame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	board->getCorneaIrCamera()->setCallback(&getImpl().cbFrameFunc);
	getImpl().cbImageFunc = nullptr;
	return true;
}


bool OctSystem::CorneaAlign::importKeratoSetupData(const char * path)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	std::string cstr = (path == nullptr ? getDefaultKeratoSetupDataPath() : path);
	if (getImpl().kerFocus.getKeratoSetup().loadDataFile(cstr.c_str())) {
		LogI() << "Kerato setup imported, path: " << cstr;
		return true;
	}
	else {
		LogW() << "Kerato setup import failed!, path: " << cstr;
		return false;
	}
}


bool OctSystem::CorneaAlign::exportKeratoSetupData(const char * path)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	std::string cstr = (path == nullptr ? getDefaultKeratoSetupDataPath() : path);
	if (getImpl().kerFocus.getKeratoSetup().saveDataFile(cstr.c_str())) {
		LogI() << "Kerato setup exported, path: " << cstr;
		return true;
	}
	else {
		LogW() << "Kerato setup export failed!, path: " << cstr;
		return false;
	}
}


bool OctSystem::CorneaAlign::loadKeratoSetupData(void)
{
	KeratoSetup setup;
	bool result = true;

	setup.initialize();
	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		auto dptSrc = SystemConfig::keratoSetupDiopter(i);
		auto dptDst = setup.getKerSetupDiopter(i);

		dptDst->diopt = dptSrc->diopt;
		dptDst->mm = dptSrc->mm;
		for (int j = 0; j < KER_SETUP_FOCUS_STEP_NUM; j++) {
			dptDst->steps[j].a = dptSrc->data[j].a;
			dptDst->steps[j].fdif = dptSrc->data[j].diff;
			dptDst->steps[j].flen = dptSrc->data[j].flen;
			dptDst->steps[j].m = dptSrc->data[j].m;
			dptDst->steps[j].mlen = dptSrc->data[j].mlen;
			dptDst->steps[j].n = dptSrc->data[j].n;
			dptDst->steps[j].valid = dptSrc->data[j].valid;

			if (!dptSrc->data[j].valid) {
				result = false;
			}
		}
	}

	if (result) {
		getImpl().kerFocus.setKeratoSetup(std::move(setup));
		LogD() << "Kerato setup loaded from mainboard";
	}
	else {
		LogD() << "Kerato setup loading from mainboard failed!";
	}
	return result;
}


bool OctSystem::CorneaAlign::saveKeratoSetupData(bool write)
{
	bool result = true;

	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		auto dptSrc = getImpl().kerFocus.getKeratoSetup().getKerSetupDiopter(i);
		for (int j = 0; j < KER_SETUP_FOCUS_STEP_NUM; j++) {
			if (!dptSrc->steps[j].valid) {
				result = false;
			}
		}
	}

	for (int i = 0; i < KER_SETUP_DIOPTER_NUM; i++) {
		auto dptDst = SystemConfig::keratoSetupDiopter(i);
		auto dptSrc = getImpl().kerFocus.getKeratoSetup().getKerSetupDiopter(i);

		dptDst->diopt = dptSrc->diopt;
		dptDst->mm = dptSrc->mm;
		for (int j = 0; j < KER_SETUP_FOCUS_STEP_NUM; j++) {
			dptDst->data[j].a = dptSrc->steps[j].a;
			dptDst->data[j].diff = dptSrc->steps[j].fdif;
			dptDst->data[j].flen = dptSrc->steps[j].flen;
			dptDst->data[j].m = dptSrc->steps[j].m;
			dptDst->data[j].mlen = dptSrc->steps[j].mlen;
			dptDst->data[j].n = dptSrc->steps[j].n;
			dptDst->data[j].valid = dptSrc->steps[j].valid;
		}
	}

	if (write) {
		if (!getImpl().board->saveSystemConfiguration(true)) {
			LogD() << "Kerato setup saving to mainboard failed!";
			return false;
		}
	}
	LogD() << "Kerato setup saved to mainboard";
	return true;
}


bool OctSystem::CorneaAlign::getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoSetup& setup = getImpl().kerFocus.getKeratoSetup();
	if (setup.getFocusStepData(diopt, istep, mlen, flen, fdif, m, n, a)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::updateFocusStepDataFromImage(int diopt, int istep)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoFocus& focus = getImpl().kerFocus;
	if (focus.updateFocusStepDataFromImage(diopt, istep)) {
		return true;
	}
	return false;
}


void OctSystem::CorneaAlign::clearFocusStepData(int diopt, int istep)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoSetup& setup = getImpl().kerFocus.getKeratoSetup();
	setup.clearFocusStepData(diopt, istep);
	return ;
}


std::string OctSystem::CorneaAlign::getDefaultKeratoSetupDataPath(void)
{
	std::string path;

	path = OctConfig::SystemProfile::getDefaultConfigDirPath();
	path += "\\";
	path += KER_SETUP_DATA_FILE_NAME;
	return path;
}


bool OctSystem::CorneaAlign::getFocusDistance(float& distZ)
{
	// unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoFocus& focus = getImpl().kerFocus;
	bool result = focus.getFocusDistance(distZ);

	if (result) {
		if (getImpl().targetMode == CorneaAlignTarget::CENTER_LEFT ||
			getImpl().targetMode == CorneaAlignTarget::CENTER_RIGHT) {
			distZ += CORNEA_TRACK_STEREO_ZDIST_OFFSET;
		}
	}
	return result;
}


bool OctSystem::CorneaAlign::getCenterDistance(float & distX, float & distY)
{
	bool result = getKeratoFocus()->getCenterOffset(distX, distY);
	
	if (result) {
		if (getImpl().targetMode == CorneaAlignTarget::CENTER_LEFT) {
			distX += (GlobalSettings::corneaPixelsPerMM() * +1);
		}
		else if (getImpl().targetMode == CorneaAlignTarget::CENTER_RIGHT) {
			distX += (GlobalSettings::corneaPixelsPerMM() * -1);
		}
	}

	return result;
}


KerFocus::KeratoFocus * OctSystem::CorneaAlign::getKeratoFocus(void)
{
	return &getImpl().kerFocus;
}


KerFocus::KeratoSetup * OctSystem::CorneaAlign::getKeratoSetup(void)
{
	return &getImpl().kerFocus.getKeratoSetup();
}


void OctSystem::CorneaAlign::setCameraImageCallback(CorneaCameraImageCallback * callback)
{
	getImpl().cbImageFunc = callback;
	return;
}


void OctSystem::CorneaAlign::startAutoAlignment(CorneaAlignTarget target)
{
	getImpl().autoAlign = true;
	getImpl().targetMode = target;
	return ;
}


void OctSystem::CorneaAlign::cancelAutoAlignment(void)
{
	getImpl().autoAlign = false;

	StageXMotor* xMotor = getImpl().board->getStageXMotor();
	StageYMotor* yMotor = getImpl().board->getStageYMotor();
	StageZMotor* zMotor = getImpl().board->getStageZMotor();
	xMotor->controlStop();
	zMotor->controlStop();
	return;
}


bool OctSystem::CorneaAlign::isAutoAligning(void)
{
	return getImpl().autoAlign;
}


CorneaAlign::CorneaAlignImpl & OctSystem::CorneaAlign::getImpl(void)
{
	return *d_ptr;
}


void OctSystem::CorneaAlign::callbackIrCameraFrame(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	unique_lock<mutex> lock(getImpl().mutexLock);

	//CppUtil::ClockTimer::start();

	float distZ = 0.0f;
	float distX = 0.0f;
	float distY = 0.0f;
	float centX = 0.0f;
	float centY = 0.0f;
	bool isFocus = false;
	bool isCenter = false;

	if (getKeratoFocus()->loadImage(data, width, height)) {
		// isFocus = getKeratoFocus()->getFocusDistance(distZ);
		isFocus = getFocusDistance(distZ);
		isCenter = getCenterDistance(distX, distY);

		GlobalRecord::writeFlashPupilExtent(getKeratoFocus()->getLightExtent());

		int width = getKeratoFocus()->getPupilWidth();
		GlobalRecord::writeFlashPupilWidth(width);
	}

	//double msec = CppUtil::ClockTimer::elapsedMsec();
	// DebugOut2() << "Cornea image processed: " << msec;

	if (isAutoAligning()) {
		doAutoAlignment(isCenter, isFocus, distX, distY, distZ);
	}

	// Pass the cornea ir image with focus information. 
	if (getImpl().cbImageFunc != nullptr) {
		if (!isTargetCenterAligned(distX, distY)) {
			isFocus = false;
		}
		(*getImpl().cbImageFunc)(data, width, height, isCenter, distX, distY, isFocus, distZ);
	}
	return;
}


void OctSystem::CorneaAlign::doAutoAlignment(bool isCenter, bool isFocus, float distX, float distY, float distZ)
{
	bool alignX = false;
	bool alignY = false;
	bool alignZ = false;

	if (isCenter) {
		if (!isTargetCenterAligned(distX, distY)) {
			alignStageX(distX);
			alignStageY(distY);
			alignX = alignY = true;
		}
		else {
			alignStageZ(distZ);
			alignZ = true;
		}
	}

	if (!alignX) {
		getImpl().board->getStageXMotor()->controlStop();
	}
	if (!alignY) {
		getImpl().board->getStageYMotor()->controlStop();
	}
	if (!alignZ) {
		getImpl().board->getStageZMotor()->controlStop();
	}
	return;
}


void OctSystem::CorneaAlign::doScanAlignment(bool isCenter, float distX, float distY)
{
	bool alignX = false;
	bool alignY = false;
	float scale = 0.3f; // 1.0f;

	if (isCenter) {
		if (!isTargetCenterAligned(distX, distY, scale)) {
			alignStageX(distX, scale);
			alignStageY(distY, scale);
			alignX = alignY = true;
		}
	}

	if (!alignX) {
		getImpl().board->getStageXMotor()->controlStop();
	}
	if (!alignY) {
		getImpl().board->getStageYMotor()->controlStop();
	}
	return;
}


void OctSystem::CorneaAlign::alignStageX(float dist, float scale)
{
	StageXMotor* motor = getImpl().board->getStageXMotor();
	static bool stopped = true;

	if (isTargetAlignedX(dist, scale) || !enableStageX()) {
		if (!stopped) {
			motor->controlStop();
			stopped = true;
			LogT() << "Stage X, target dist: " << dist << " => stopped";
		}
	}
	else {
		int delta;
		if (dist <= 0.0f) {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_X1);	// to left
		}
		else {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_X2);	// to right
		}
		delta *= -1;
		motor->updatePositionByDelta(delta);
		// motor->updatePositionByOffset(delta);
		stopped = false;

		LogT() << "Stage X, target dist: " << dist << ", delta: " << delta;

		if (delta > 0 && motor->isAtUpperSideOfPosition()) {
			LogT() << "Stage X, moving on the left side";
		}
		if (delta < 0 && motor->isAtLowerSideOfPosition()) {
			LogT() << "Stage X, moving on the right side";
		}
	}
	return;
}


void OctSystem::CorneaAlign::alignStageY(float dist, float scale)
{
	StageYMotor* motor = getImpl().board->getStageYMotor();
	static bool stopped = true;

	if (isTargetAlignedY(dist, scale) || !enableStageY()) {
		if (!stopped) {
			motor->controlStop();
			stopped = true;
			LogT() << "Stage Y, target dist: " << dist << " => stopped";
		}
	}
	else {
		int delta;
		if (dist <= 0.0f) {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_Y1);	// to upper
		}
		else {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_Y2);	// to lower
		}
		delta *= -1;
		motor->updatePositionByOffset(delta);
		stopped = false;

		LogT() << "Stage Y, target dist: " << dist << ", delta: " << delta;

		if (delta > 0 && motor->isAtUpperSideOfPosition()) {
			LogT() << "Stage Y, moving on the upper side";
		}
		if (delta < 0 && motor->isAtLowerSideOfPosition()) {
			LogT() << "Stage Y, moving on the lower side";
		}
	}
	return;
}


void OctSystem::CorneaAlign::alignStageZ(float dist)
{
	StageZMotor* motor = getImpl().board->getStageZMotor();
	static bool stopped = true;

	if (isTargetAlignedZ(dist) || !enableStageZ()) {
		if (!stopped) {
			motor->controlStop();
			stopped = true;
			LogT() << "Stage Z, target dist: " << dist << " => stopped";
		}
	}
	else {
		int delta;
		if (dist <= 0.0f) {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_Z1);	// to backward
		}
		else {
			delta = (int)(dist / KER_DIST_PER_ENC_STEP_Z2);	// to forward
		}
		delta *= -1;

		motor->updatePositionByDelta(delta);
		// motor->updatePositionByOffset(delta);
		stopped = false;

		LogT() << "Stage Z, target dist: " << dist << ", delta: " << delta;

		if (delta > 0 && motor->isAtUpperSideOfPosition()) {
			LogT() << "Stage Z, moving on the backward side";
		}
		if (delta < 0 && motor->isAtLowerSideOfPosition()) {
			LogT() << "Stage Z, moving on the forward side";
		}
	}
	return;
}


bool OctSystem::CorneaAlign::isTargetCenterAligned(float distX, float distY, float scale)
{
	if (isTargetAlignedX(distX, scale) && isTargetAlignedY(distY, scale)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::isTargetFocusAligned(float distZ)
{
	if (isTargetAlignedZ(distZ)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::isTargetAlignedX(float distX, float scale)
{
	float x1 = KER_ALIGN_TARGET_RANGE_X1 * scale;
	float x2 = KER_ALIGN_TARGET_RANGE_X2 * scale;
	if (distX >= x1 && distX <= x2) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::isTargetAlignedY(float distY, float scale)
{
	float y1 = KER_ALIGN_TARGET_RANGE_Y1 * scale;
	float y2 = KER_ALIGN_TARGET_RANGE_Y2 * scale;
	if (distY >= y1 && distY <= y2) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::isTargetAlignedZ(float distZ)
{
	if (distZ >= KER_ALIGN_TARGET_RANGE_Z1 && distZ <= KER_ALIGN_TARGET_RANGE_Z2) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaAlign::enableStageX(bool isset, bool flag)
{
	if (isset) {
		d_ptr->stageXon = flag;
	}
	return d_ptr->stageXon;
}


bool OctSystem::CorneaAlign::enableStageY(bool isset, bool flag)
{
	if (isset) {
		d_ptr->stageYon = flag;
	}
	return d_ptr->stageYon;
}


bool OctSystem::CorneaAlign::enableStageZ(bool isset, bool flag)
{
	if (isset) {
		d_ptr->stageZon = flag;
	}
	return d_ptr->stageZon;
}
