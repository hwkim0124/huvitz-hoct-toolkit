#include "stdafx.h"
#include "RetinaAlign.h"
#include "RetinaTrack.h"
#include "RetinaTrack2.h"
#include "RetinaTracker.h"
#include "RetinaTracking.h"

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


struct RetinaAlign::RetinaAlignImpl
{
	bool initiated;
	MainBoard* board;
	RetinaFocus2 retFocus;

    float irGamma;

	CvImage retImage;

	IrCameraFrameCallback cbFrameFunc;
	RetinaCameraImageCallback* cbImageFunc;
	mutex mutexLock;

	RetinaAlignImpl() : initiated(false), board(nullptr), cbFrameFunc(nullptr), cbImageFunc(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<RetinaAlign::RetinaAlignImpl> RetinaAlign::d_ptr(new RetinaAlignImpl());


RetinaAlign::RetinaAlign()
{
}


RetinaAlign::~RetinaAlign()
{
}


bool OctSystem::RetinaAlign::initRetinaAlign(OctDevice::MainBoard * board)
{
	getImpl().board = board;
	getImpl().initiated = true;

	// The type of a pointer to static member function looks like a pointer to non-member function.
	// It doesn't require this argument since static functions are bound to class not to the object. 
	getImpl().cbFrameFunc = std::bind(&RetinaAlign::callbackIrCameraFrame, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	board->getRetinaIrCamera()->setCallback(&getImpl().cbFrameFunc);
	getImpl().cbImageFunc = nullptr;
    getImpl().irGamma = 1.6f;
	return true;
}


void OctSystem::RetinaAlign::setCameraImageCallback(RetinaCameraImageCallback * callback)
{
	getImpl().cbImageFunc = callback;
	return;
}


RetFocus::RetinaFocus2& OctSystem::RetinaAlign::getRetinaFocus(void)
{
	return d_ptr->retFocus;
}


float OctSystem::RetinaAlign::getIrGamma(void)
{
    return getImpl().irGamma;
}


void OctSystem::RetinaAlign::setIrGamma(float g)
{
    getImpl().irGamma = g;
}


CppUtil::CvImage OctSystem::RetinaAlign::getRetinaImage(void)
{
	unique_lock<mutex> lock(getImpl().mutexLock);
	return getImpl().retImage;
}


RetinaAlign::RetinaAlignImpl & OctSystem::RetinaAlign::getImpl(void)
{
	return *d_ptr;
}


void OctSystem::RetinaAlign::processRetinaFocus(unsigned char * data, unsigned int width, unsigned int height)
{
	if (getRetinaFocus().loadImage(data, width, height)) 
	{
		if (getRetinaFocus().isSplitFocus()) {
			float offset = d_ptr->retFocus.getSplitOffset();
			GlobalRecord::writeSplitFocusOffset(offset);

			RetinaFrame& frame = getRetinaFocus().getRetinaFrame();
			float uppSize, uppPeak, lowSize, lowPeak;
			if (frame.isUpperSplit()) {
				uppPeak = frame.getSplitUpperPeak();
				uppSize = frame.getSplitUpperFwhm();
			}
			else {
				uppPeak = uppSize = 0.0f;
			}
			if (frame.isLowerSplit()) {
				lowPeak = frame.getSplitLowerPeak();
				lowSize = frame.getSplitLowerFwhm();
			}
			else {
				lowPeak = lowSize = 0.0f;
			}
			GlobalRecord::writeSplitFocusSize(max(lowSize, uppSize), max(lowPeak, uppPeak));
		}
	}
	return;
}


void OctSystem::RetinaAlign::callbackIrCameraFrame(unsigned char * data, unsigned int width, unsigned int height)
{
	if (data == nullptr) {
		return;
	}

	unique_lock<mutex> lock(getImpl().mutexLock);
	processRetinaFocus(data, width, height);

	auto* track = RetinaTracking::getInstance();
	if (track->isTrackingReady()) {
		CppUtil::CvImage m;
		m.fromBitsDataNoCopy(data, width, height);
		m.equalizeHistogram(1.0f, 8, true);
		track->processTrackingFrame(m);
	}
	else {
		if (GlobalSettings::useRetinaImageEnhance()) {
			CppUtil::CvImage m;
			m.fromBitsDataNoCopy(data, width, height);
			m.equalizeHistogram(GlobalSettings::retinaImageClipLimit(), GlobalSettings::retinaImageClipScalar(), true);
		}
		else {
			CppUtil::CvImage m;
			m.fromBitsDataNoCopy(data, width, height);
			m.correctGamma(getIrGamma());
		}
	}

	if (GlobalSettings::useRetinaCenterMask()) {
		CppUtil::CvImage m;
		m.fromBitsDataNoCopy(data, width, height);

		auto center = GlobalSettings::centerOfRetinaMask();
		if (center.first <= 0 || center.second <= 0) {
			center = GlobalSettings::centerOfRetinaROI();
		}
		auto radius = GlobalSettings::retinaCenterMaskSize();
		m.drawCircleMask(center.first, center.second, radius, true);
	}

	// double msec = CppUtil::ClockTimer::elapsedMsec();
	// DebugOut2() << "Retina image processed: " << msec;

	int wsize = width;
	int hsize = height;
	d_ptr->retImage.fromBitsData(data, wsize, hsize);

	if (GlobalSettings::useRetinaROI()) {
		auto center = GlobalSettings::centerOfRetinaROI();
		auto radius = GlobalSettings::radiusOfRetinaROI();

		wsize = radius * 2;
		hsize = radius * 2;
		int x0 = center.first - radius;
		int y0 = center.second - radius;

		x0 = min(max(0, x0), CAMERA_IMAGE_WIDTH - 1);
		y0 = min(max(0, y0), CAMERA_IMAGE_HEIGHT - 1);

		if ((wsize + x0) > CAMERA_IMAGE_WIDTH) {
			wsize = CAMERA_IMAGE_WIDTH - x0;
		}
		if ((hsize + y0) > CAMERA_IMAGE_HEIGHT) {
			hsize = CAMERA_IMAGE_HEIGHT - y0;
		}

		// image.applyMaskROI(center, radius, true);
		d_ptr->retImage.crop(x0, y0, wsize, hsize);
	}

    // crop IR image to square to be centered on canvas
    if (getImpl().cbImageFunc != nullptr) 
	{
        (*getImpl().cbImageFunc)(d_ptr->retImage.getBitsData(), wsize, hsize);
 	}
	return;
}


OctDevice::MainBoard * OctSystem::RetinaAlign::getMainBoard(void)
{
	return d_ptr->board;
}