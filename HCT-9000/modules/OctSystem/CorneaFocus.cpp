#include "stdafx.h"
#include "CorneaFocus.h"

#include <mutex>

#include "OctDevice2.h"
#include "CppUtil2.h"
#include "KerFocus2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace KerFocus;
using namespace std;


struct CorneaFocus::CorneaFocusImpl
{
	bool initiated;
	MainBoard* board;
	KeratoFocus kerFocus;

	bool autoAlign;

	IrCameraFrameCallback cbFrameFunc;
	CorneaCameraImageCallback* cbImageFunc;
	mutex mutexLock;

	CorneaFocusImpl() : initiated(false), board(nullptr), cbFrameFunc(nullptr), cbImageFunc(nullptr), 
		autoAlign(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<CorneaFocus::CorneaFocusImpl> CorneaFocus::d_ptr(new CorneaFocusImpl());


CorneaFocus::CorneaFocus()
{
}


CorneaFocus::~CorneaFocus()
{
}


bool OctSystem::CorneaFocus::initialize(MainBoard * board)
{
	getImpl().board = board;
	getImpl().initiated = true;

	// The type of a pointer to static member function looks like a pointer to non-member function.
	// It doesn't require this argument since static functions are bound to class not to the object. 
	getImpl().cbFrameFunc = std::bind(&CorneaFocus::callbackIrCameraFrame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	board->getCorneaIrCamera()->setCallback(&getImpl().cbFrameFunc);
	getImpl().cbImageFunc = nullptr;
	return true;
}


bool OctSystem::CorneaFocus::loadKeratoSetupData(const char * path)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	return getImpl().kerFocus.getKeratoSetup().loadDataFile(path);
}


bool OctSystem::CorneaFocus::saveKeratoSetupData(const char * path)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	return getImpl().kerFocus.getKeratoSetup().saveDataFile(path);
}


bool OctSystem::CorneaFocus::getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoSetup& setup = getImpl().kerFocus.getKeratoSetup();
	if (setup.getFocusStepData(diopt, istep, mlen, flen, fdif, m, n, a)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaFocus::updateFocusStepDataFromImage(int diopt, int istep)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoFocus& focus = getImpl().kerFocus;
	if (focus.updateFocusStepDataFromImage(diopt, istep)) {
		return true;
	}
	return false;
}


void OctSystem::CorneaFocus::clearFocusStepData(int diopt, int istep)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoSetup& setup = getImpl().kerFocus.getKeratoSetup();
	setup.clearFocusStepData(diopt, istep);
	return ;
}


bool OctSystem::CorneaFocus::getFocusDistance(float& dist)
{
	unique_lock<mutex> lock(getImpl().mutexLock);

	KeratoFocus& focus = getImpl().kerFocus;
	if (focus.getFocusDistance(dist)) {
		return true;
	}
	return false;
}


KerFocus::KeratoFocus * OctSystem::CorneaFocus::getKeratoFocus(void)
{
	return &getImpl().kerFocus;
}


KerFocus::KeratoSetup * OctSystem::CorneaFocus::getKeratoSetup(void)
{
	return &getImpl().kerFocus.getKeratoSetup();
}


void OctSystem::CorneaFocus::setCameraImageCallback(CorneaCameraImageCallback * callback)
{
	getImpl().cbImageFunc = callback;
	return;
}


void OctSystem::CorneaFocus::startAutoAlignment(void)
{
	getImpl().autoAlign = true;
	return ;
}


void OctSystem::CorneaFocus::cancelAutoAlignment(void)
{
	getImpl().autoAlign = false;

	StageXMotor* xMotor = getImpl().board->getStageXMotor();
	StageYMotor* yMotor = getImpl().board->getStageYMotor();
	StageZMotor* zMotor = getImpl().board->getStageZMotor();
	xMotor->controlStop();
	zMotor->controlStop();
	return;
}


bool OctSystem::CorneaFocus::isAutoAligning(void)
{
	return getImpl().autoAlign;
}


CorneaFocus::CorneaFocusImpl & OctSystem::CorneaFocus::getImpl(void)
{
	return *d_ptr;
}


void OctSystem::CorneaFocus::callbackIrCameraFrame(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	unique_lock<mutex> lock(getImpl().mutexLock);

	float distZ = 0.0f;
	float distX = 0.0f;
	float distY = 0.0f;
	bool isFocus = false;
	bool isCenter = false;
	if (getKeratoFocus()->loadImage(data, width, height)) {
		isFocus = getKeratoFocus()->getFocusDistance(distZ);
		isCenter = getKeratoFocus()->getCenterOffset(distX, distY);
	}

	// Pass the cornea ir image with focus information. 
	if (getImpl().cbImageFunc != nullptr) {
		(*getImpl().cbImageFunc)(data, width, height, isCenter, distX, distY, isFocus, distZ);
	}

	if (isAutoAligning()) 
	{
		if (isCenter) {
			StageXMotor* xMotor = getImpl().board->getStageXMotor();
			StageYMotor* yMotor = getImpl().board->getStageYMotor();
			StageZMotor* zMotor = getImpl().board->getStageZMotor();
			int delta;

			if (!isTargetCenterAligned(distX, distY)) 
			{
				if (isTargetAlignedX(distX)) {
					xMotor->controlStop();
				}
				else {
					if (distX <= 0.0f) {
						delta = (int)(distX / KER_DIST_PER_ENC_STEP_X1)*-1;
					}
					else {
						delta = (int)(distX / KER_DIST_PER_ENC_STEP_X2)*-1;
					}
					xMotor->updatePositionByOffset(delta);
				}

				if (isTargetAlignedY(distY)) {
				}
				else {
					if (distY <= 0.0f) {
						delta = (int)(distY / KER_DIST_PER_ENC_STEP_Y1)*-1;
					}
					else {
						delta = (int)(distY / KER_DIST_PER_ENC_STEP_Y2)*-1;
					}
					yMotor->updatePositionByOffset(delta);
				}
			}
			else {
				if (isFocus) {
					if (isTargetAlignedZ(distZ)) {
						zMotor->controlStop();
					}
					else {
						if (distZ <= 0.0f) {
							delta = (int)(distZ / KER_DIST_PER_ENC_STEP_Z1)*-1;
						}
						else {
							delta = (int)(distZ / KER_DIST_PER_ENC_STEP_Z2)*-1;
						}
						zMotor->updatePositionByOffset(delta);
					}
				}
			}
		}
	}
	return;
}


bool OctSystem::CorneaFocus::isTargetCenterAligned(float distX, float distY)
{
	if (isTargetAlignedX(distX) && isTargetAlignedY(distY)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaFocus::isTargetFocusAligned(float distZ)
{
	if (isTargetAlignedZ(distZ)) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaFocus::isTargetAlignedX(float distX)
{
	if (distX >= KER_ALIGN_TARGET_RANGE_X1 && distX <= KER_ALIGN_TARGET_RANGE_X2) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaFocus::isTargetAlignedY(float distY)
{
	if (distY >= KER_ALIGN_TARGET_RANGE_Y1 && distY <= KER_ALIGN_TARGET_RANGE_Y2) {
		return true;
	}
	return false;
}


bool OctSystem::CorneaFocus::isTargetAlignedZ(float distZ)
{
	if (distZ >= KER_ALIGN_TARGET_RANGE_Z1 && distZ <= KER_ALIGN_TARGET_RANGE_Z2) {
		return true;
	}
	return false;
}
