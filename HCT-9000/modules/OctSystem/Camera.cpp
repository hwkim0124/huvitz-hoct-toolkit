#include "stdafx.h"
#include "Camera.h"
#include "Measure.h"
#include "RetinaAlign.h"

#include "OctDevice2.h"
#include "CppUtil2.h"
#include "OctResult2.h"

#include <mutex>
#include <atomic>
#include <functional>

using namespace OctSystem;
using namespace OctDevice;
using namespace std;
using namespace CppUtil;


struct Camera::CameraImpl
{
	MainBoard* board;
	bool initiated;

	mutex mutexGrab;
	atomic<bool> posterRun;
	thread threadPost;

	CameraImpl() : initiated(false), board(nullptr), posterRun(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Camera::CameraImpl> Camera::d_ptr(new CameraImpl());


Camera::Camera()
{
}


Camera::~Camera()
{
}


bool OctSystem::Camera::initCamera(OctDevice::MainBoard * board)
{
	getImpl().board = board;
	getImpl().initiated = true;
	return true;
}


bool OctSystem::Camera::isInitiated(void)
{
	return getImpl().initiated;
}


bool OctSystem::Camera::isPosting(void)
{
	return getImpl().posterRun;
}


bool OctSystem::Camera::takeColorFundus(const OctFundusMeasure & measure, bool imageProc)
{
	beginFundusMeasure(measure);

	if (!takePicture()) {
		return false;
	}

	if (!postPicture()) {
		return false;
	}
	return true;
}


bool OctSystem::Camera::prepareShot(void)
{
	return false;
}


bool OctSystem::Camera::takePicture(void)
{
	int row, col;
	getMainBoard()->getLcdFixationOn(row, col);
	getMainBoard()->setLcdFixationOn(false);
	getMainBoard()->getRetinaIrLed()->lightOff();
	getMainBoard()->getWorkingDotsLed()->lightOff();
	getMainBoard()->getWorkingDot2Led()->lightOff();

	auto splitValue = getMainBoard()->getSplitFocusLed()->getIntensity();
	getMainBoard()->getSplitFocusLed()->setIntensity(0);

	getMainBoard()->setFundusCameraCapture();

	LogD() << "Color fundus taking picture";
	getMainBoard()->getColorCamera()->takePicture();

	getMainBoard()->setLcdFixationOn(true, row, col);
	getMainBoard()->getRetinaIrLed()->lightOn();
	getMainBoard()->getWorkingDotsLed()->lightOn();
	getMainBoard()->getWorkingDot2Led()->lightOn();

	// getMainBoard()->getSplitFocusLed()->lightOn();
	getMainBoard()->getSplitFocusLed()->setIntensity(splitValue);

	getMainBoard()->setQuickReturnMirrorIn(true);
	getMainBoard()->setSplitFocusMirrorIn(true);

	LogD() << "Color fundus image acquired, callback";
	GlobalRegister::runFundusImageAcquiredCallback(true);
	return true;
}


bool OctSystem::Camera::postPicture(void)
{
	bool result = startFundusPost();
	return result;
}


void OctSystem::Camera::beginFundusMeasure(const OctFundusMeasure & measure)
{
	// getImpl().fundusMeasure = measure;
	Measure::initiateFundusResult(measure);

	//getMainBoard()->getCorneaCamera()->pause();
	//getMainBoard()->getRetinaCamera()->pause();

	OctCorneaImage cornea;
	getMainBoard()->getCorneaCamera()->captureFrame(cornea);
	Measure::assignCorneaImageToFundusResult(cornea);

	LogD() << "Ir cornea image assigned";

	// OctRetinaImage retina;
	// getMainBoard()->getRetinaCamera()->captureFrame(retina);
	// Measure::assignRetinaImageToFundusResult(retina);
	Measure::assignRetinaImageToFundusResult(RetinaAlign::getRetinaImage());

	LogD() << "Ir retina image assigned";
	return;
}


void OctSystem::Camera::completeFundusMeasure(void)
{
	try {
		const std::uint8_t* data = getMainBoard()->getColorCamera()->getImageBuffer();
		const std::uint8_t* raws = getMainBoard()->getColorCamera()->getFrameBuffer();
		int frm_w = getMainBoard()->getColorCamera()->getFrameWidth();
		int frm_h = getMainBoard()->getColorCamera()->getFrameHeight();
		int img_w = getMainBoard()->getColorCamera()->getImageWidth();
		int img_h = getMainBoard()->getColorCamera()->getImageHeight();

		OctFundusSection section;
		OctFundusImage image(data, img_w, img_h);
		OctFundusFrame frame(raws, frm_w, frm_h);
		Measure::assignFundusImage(section, image, frame);
		LogD() << "Fundus image size: " << img_w << ", " << img_h << ", frame size: " << frm_w << ", " << frm_h;

		Measure::completeFundusResult();
		LogD() << "Color fundus image completed, callback";
		GlobalRegister::runFundusImageCompletedCallback(true);
	}
	catch (const std::exception& e) {
		GlobalRegister::runFundusImageCompletedCallback(false);
	}
	return;
}


bool OctSystem::Camera::startFundusPost(void)
{
	if (!isInitiated()) {
		return false;
	}

	closeFundusPost();

	if (getImpl().threadPost.joinable()) {
		getImpl().threadPost.join();
	}

	getImpl().threadPost = thread{ &Camera::threadPostFunction };
	return true;
}


bool OctSystem::Camera::closeFundusPost(void)
{
	if (isPosting()) {
		getImpl().posterRun = false;

		if (getImpl().threadPost.joinable()) {
			getImpl().threadPost.join();
		}
	}
	return true;
}


void OctSystem::Camera::threadPostFunction(void)
{
	getImpl().posterRun = true;

	CppUtil::ClockTimer::start();
	bool processResult = getMainBoard()->getColorCamera()->processImage();
	CppUtil::ClockTimer::elapsedMsec();

	if (!processResult) {
		getImpl().posterRun = false;
		GlobalRegister::runFundusImageCompletedCallback(false);
		return;
	}

	completeFundusMeasure();

	getImpl().posterRun = false;
	return;
}


OctDevice::MainBoard * OctSystem::Camera::getMainBoard(void)
{
	return d_ptr->board;
}


Camera::CameraImpl & OctSystem::Camera::getImpl(void)
{
	return *d_ptr;
}
