#include "stdafx.h"
#include "Scanner.h"
#include "Measure.h"
#include "Optimizer.h"
#include "ScanFunc.h"
#include "ScanGrab.h"
#include "ScanPost.h"
#include "RetinaAlign.h"

#include "OctPattern2.h"
#include "OctDevice2.h"
#include "OctGrab2.h"
#include "SigChain2.h"
#include "CppUtil2.h"

#include <mutex>
#include <atomic>
#include <functional>

using namespace OctSystem;
using namespace OctPattern;
using namespace OctDevice;
using namespace OctGrab;
using namespace SigChain;
using namespace std;
using namespace CppUtil;


struct Scanner::ScannerImpl
{
	MainBoard* board;
	FrameGrabber* grabber;
	Usb3Grabber* usb3Grab;
	Galvanometer* galvano;
	bool initiated;
	bool useEnface;

	OctScanMeasure scanMeasure;

	PatternPlan pattern;
	OctProcess process;

	GrabMeasureBufferCallback cbGrabMeasureBuffer;
	GrabPreviewBufferCallback cbGrabPreviewBuffer;
	GrabEnfaceBufferCallback cbGrabEnfaceBuffer;

	PreviewImageCompletedEvent cbPreviewImageCompleted;
	MeasureImageCompletedEvent cbMeasureImageCompleted;
	EnfaceImageCompletedEvent cbEnfaceImageCompleted;


	ScannerImpl() : galvano(nullptr), grabber(nullptr), board(nullptr), usb3Grab(nullptr),
		initiated(false), useEnface(false), 
		cbGrabMeasureBuffer(nullptr), cbGrabPreviewBuffer(nullptr), cbGrabEnfaceBuffer(nullptr), 
		cbPreviewImageCompleted(nullptr), cbMeasureImageCompleted(nullptr), cbEnfaceImageCompleted(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Scanner::ScannerImpl> Scanner::d_ptr(new ScannerImpl());


Scanner::Scanner()
{
}


Scanner::~Scanner()
{
}


bool OctSystem::Scanner::initScanner(OctDevice::MainBoard* board, OctGrab::FrameGrabber * grabber)
{
	ChainSetup::initializeChainSetup();
	PatternHelper::initializePatternHelper();

	if (grabber->isInitiated()) {
		if (!getImpl().process.initializeProcess()) {
			return false;
		}
	}

	ScanGrab::initiateScanGrab(board, grabber);
	ScanPost::initiateScanPost(board, &getImpl().process);
	ScanFunc::initScanFunc(board, grabber);

	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().grabber = grabber;
	getImpl().initiated = true;

	initializeCallbacks();
	return true;
}


bool OctSystem::Scanner::initScanner(OctDevice::MainBoard * board, OctGrab::Usb3Grabber * grabber)
{
	ChainSetup::initializeChainSetup();
	PatternHelper::initializePatternHelper();

	if (grabber->isInitiated()) {
		if (!getImpl().process.initializeProcess()) {
			return false;
		}
	}

	ScanGrab::initiateScanGrab(board, grabber);
	ScanPost::initiateScanPost(board, &getImpl().process);
	ScanFunc::initScanFunc(board, grabber);

	getImpl().board = board;
	getImpl().galvano = board->getGalvanometer();
	getImpl().usb3Grab = grabber;
	getImpl().initiated = true;

	initializeCallbacks();
	return true;
}


bool OctSystem::Scanner::isInitiated(void)
{
	return getImpl().initiated;
}


void OctSystem::Scanner::release(void)
{
	getImpl().process.releaseProcess();

	getImpl().initiated = false;
	return;
}


bool OctSystem::Scanner::startMeasure(EyeSide side, PatternDomain domain, PatternType type,
									int numPoints, int numLines, float rangeX, float rangeY,
									int overlaps, float lineSpace, bool useEnface, bool usePattern, bool useFaster)
{
	if (!initPattern()) {
		return false;
	}

	getPattern().buildMeasure(domain, type, numPoints, numLines, rangeX, rangeY,
								overlaps, lineSpace, useEnface, usePattern, useFaster);
	
	if (!ScanFunc::updatePatternPositions(side, getPattern(), ScanSpeed::Fastest)) {
		return false;
	}

	if (!ScanFunc::uploadPatternProfiles(getPattern(), false)) {
		return false;
	}

	if (!ScanFunc::updatePatternBuffers(getPattern(), false, true)) {
		return false;
	}

	if (!getImpl().scanMeasure.useNoImageGrab()) {
		if (!getImpl().scanMeasure.usePhaseEnface()) {
			if (!ScanFunc::updatePatternBuffers(getPattern(), true, false)) {
				return false;
			}
		}
	}

	ScanFunc::prepareChainSetup(domain, PatternName::Unknown, ScanSpeed::Fastest, false);

	if (!updatePatternLayout(useEnface)) {
		return false;
	}

	bool ret = startPreview(useEnface);
	return ret;
}


bool OctSystem::Scanner::startMeasure(OctScanMeasure& measure)
{
	getImpl().scanMeasure = measure;

	if (!initPattern()) {
		return false;
	}

	if (!getPattern().buildMeasure(measure)) {
		return false;
	}

	auto msrPat = measure.getPattern();
	ScanFunc::prepareChainSetup(
				msrPat.getPatternDomain(),
				msrPat.getPatternName(),
				msrPat.getScanSpeed(),
				false);

	if (ChainSetup::isAngioScan() || ChainSetup::isScanAmplitudesStored()) {
		AngioSetup::setAngiogramLayout(msrPat.getNumberOfScanLines(),
			msrPat.getNumberOfScanPoints(),
			msrPat.getScanOverlaps(),
			getPattern().getMeasureScan().isVertical());
	}

	PatternPlan& pattern = getPattern();
	if (!ScanFunc::updatePatternPositions(measure.getEyeSide(), pattern, measure.getPattern().getScanSpeed())) {
		return false;
	}

	if (!ScanFunc::uploadPatternProfiles(pattern, false)) {
		return false;
	}

	if (!ScanFunc::updatePatternBuffers(getPattern(), false, true)) {
		return false;
	}

	if (!getImpl().scanMeasure.useNoImageGrab()) {
		if (!getImpl().scanMeasure.usePhaseEnface()) {
			if (!ScanFunc::updatePatternBuffers(getPattern(), true, false)) {
				return false;
			}
		}
	}

	bool useEnface = measure.usePhaseEnface();
	if (!updatePatternLayout(useEnface)) {
		return false;
	}

	bool ret = startPreview(useEnface);
	return ret;
}

bool OctSystem::Scanner::setupMeasure(OctScanMeasure & measure)
{
	getImpl().scanMeasure = measure;

	if (!getPattern().buildMeasure(measure)) {
		return false;
	}

	auto msrPat = measure.getPattern();
	ScanFunc::prepareChainSetup(
		msrPat.getPatternDomain(),
		msrPat.getPatternName(),
		msrPat.getScanSpeed(),
		false);

	if (ChainSetup::isAngioScan() || ChainSetup::isScanAmplitudesStored()) {
		AngioSetup::setAngiogramLayout(msrPat.getNumberOfScanLines(),
			msrPat.getNumberOfScanPoints(),
			msrPat.getScanOverlaps(),
			getPattern().getMeasureScan().isVertical());
	}
	
	bool useEnface = measure.usePhaseEnface();
	if (!updatePatternLayout(useEnface)) {
		return false;
	}


	if (!getImpl().process.initializeProcess()) {
		return false;
	}
	// bool ret = startPreview(useEnface);
	// return ret;
	return true;
}


bool OctSystem::Scanner::closePattern(bool measure)
{ 
	bool ret = false;

	ret = closePreview(measure);

	// It is not necessary for GPU devices to be released after scanning pattern.
	// getImpl().process.release();
	return ret;
}


void OctSystem::Scanner::cancelMeasure(void)
{
	if (!isMeasuring()) {
		return;
	}

	ScanGrab::cancelMeasureGrab();
	return;
}


bool OctSystem::Scanner::checkMeasureDescript(OctScanMeasure & measure)
{
	OctScanPattern& pattern = measure.getPattern();
	int code = PatternHelper::getPatternCodeFromName(pattern._name);
	if (code < 0) {
		LogD() << "Invalid pattern code = " << code;
		return false;
	}

	PatternDomain domain = PatternHelper::getPatternDomainFromCode(code);
	PatternType type = PatternHelper::getPatternTypeFromCode(code, pattern._direction);
	pattern._domain = domain;
	pattern._type = type;
	return true;
}


float OctSystem::Scanner::getAcquisitionTime(void)
{
	return ScanGrab::getMeasureAcquisitionTime();
}


bool OctSystem::Scanner::setupEnface(int numPoints, int numLines, float rangeX, float rangeY)
{
	getPattern().setupEnface(numPoints, numLines, rangeX, rangeY);
	return true;
}


bool OctSystem::Scanner::setupDispacement(float offsetX, float offsetY, float angle, float scaleX, float scaleY)
{
	getPattern().setupDisplacement(offsetX, offsetY, angle, scaleX, scaleY);
	return true;
}


bool OctSystem::Scanner::changeScanSpeedToFastest(bool noImageGrab)
{
	if (!isInitiated()) {
		return false;
	}
	if (isGrabbing() || isPosting()) {
		return false;
	}
	/*
	if (isScanSpeedFastest()) {
		return true;
	}
	*/
	if (!ScanGrab::changeScanSpeedToFastest(noImageGrab)) {
		return false;
	}

	ChainSetup::clearBackgroundSpectrum();
	return true;
}


bool OctSystem::Scanner::changeScanSpeedToFaster(bool noImageGrab)
{
	if (isGrabbing() || isPosting()) {
		return false;
	}
	/*
	if (isScanSpeedFaster()) {
		return true;
	}
	*/
	if (!ScanGrab::changeScanSpeedToFaster(noImageGrab)) {
		return false;
	}
	ChainSetup::clearBackgroundSpectrum();
	return true;
}


bool OctSystem::Scanner::changeScanSpeedToNormal(bool noImageGrab)
{
	if (isGrabbing() || isPosting()) {
		return false;
	}
	/*
	if (isScanSpeedNormal()) {
		return true;
	}
	*/
	if (!ScanGrab::changeScanSpeedToNormal(noImageGrab)) {
		return false;
	}
	ChainSetup::clearBackgroundSpectrum();
	return true;
}


bool OctSystem::Scanner::changeScanSpeedToCustom(bool noImageGrab)
{
	if (isGrabbing() || isPosting()) {
		return false;
	}
	/*
	if (isScanSpeedCustom()) {
		return true;
	}
	*/

	if (!ScanGrab::changeScanSpeedToCustom(noImageGrab)) {
		return false;
	}
	ChainSetup::clearBackgroundSpectrum();
	return true;
}


bool OctSystem::Scanner::isScanSpeedFastest(void)
{
	return ScanGrab::isScanSpeedFastest() ;
}


bool OctSystem::Scanner::isScanSpeedFaster(void)
{
	return ScanGrab::isScanSpeedFaster();
}


bool OctSystem::Scanner::isScanSpeedNormal(void)
{
	return ScanGrab::isScanSpeedNormal();
}


bool OctSystem::Scanner::isScanSpeedCustom(void)
{
	return ScanGrab::isScanSpeedCustom();
}


void OctSystem::Scanner::exportGalvanoPositions(std::string filename)
{
	ScanFunc::exportPatternPositions(getPattern(), filename);
	return;
}


bool OctSystem::Scanner::isAnteriorMode(void)
{
	return getPattern().isCornea();
}


bool OctSystem::Scanner::isPreviewing(void)
{
	return ScanGrab::isPreviewing();
}


bool OctSystem::Scanner::isGrabbing(void)
{
	return ScanGrab::isGrabbing();
}


bool OctSystem::Scanner::isMeasuring(void)
{
	return ScanGrab::isMeasuring();
}


bool OctSystem::Scanner::isPosting(void)
{
	return ScanPost::isPosting();
}

bool OctSystem::Scanner::isCancelling(void)
{
	return ScanGrab::isMeasureCanceled();
}


bool OctSystem::Scanner::startPreview(bool enface)
{
	if (!isInitiated()) {
		return false;
	}

	stopGrabbing(false);

	bool noImageGrab = getImpl().scanMeasure.useNoImageGrab();
	if (!noImageGrab) {
		if (!getImpl().process.startProcess(enface)) {
			return false;
		}
	}

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getImpl().usb3Grab->setGrabPreviewBufferCallback(&getImpl().cbGrabPreviewBuffer);
		getImpl().usb3Grab->setGrabMeasureBufferCallback(&getImpl().cbGrabMeasureBuffer);
		getImpl().usb3Grab->setGrabEnfaceBufferCallback(&getImpl().cbGrabEnfaceBuffer);
	}
	else {
		getImpl().grabber->setGrabPreviewBufferCallback(&getImpl().cbGrabPreviewBuffer);
		getImpl().grabber->setGrabMeasureBufferCallback(&getImpl().cbGrabMeasureBuffer);
		getImpl().grabber->setGrabEnfaceBufferCallback(&getImpl().cbGrabEnfaceBuffer);
	}

	auto pattern = &getImpl().pattern;
	auto process = &getImpl().process;

	if (getImpl().board) {
		if (!getImpl().board->prepareDevicesForScan()) {
			return false;
		}
	}

	if (!ScanGrab::grabPreview(pattern, process, noImageGrab)) {
		return false;
	}

	initiateMeasureResult();
	return true;
}


bool OctSystem::Scanner::closePreview(bool measure)
{
	if (!isInitiated()) {
		return false;
	}

	if (measure) {
		if (!captureMeasureResult()) {
			return false;
		}
	}

	stopGrabbing(measure);
	return true;
}

bool OctSystem::Scanner::startProcess(bool enface)
{
	if (!getImpl().process.startProcess(enface)) {
		return false;
	}
	return true;
}

void OctSystem::Scanner::stopGrabbing(bool measure)
{
	auto scanDesc = &getImpl().scanMeasure;

	// Busy waiting for grabbing finished.
	ScanGrab::grabMeasure(measure, scanDesc);

	if (measure) {
		bool result = ScanGrab::isMeasureCompleted();

		// Start off a thread for image processing. 
		ScanPost::postMeasure(result);
	}
	return;
}


bool OctSystem::Scanner::initiateMeasureResult(void)
{
	// Update eye side right before starting measure. 
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		d_ptr->scanMeasure.setEyeSide(getUsb3Grabber()->getEyeSide());
	}
	else {
		d_ptr->scanMeasure.setEyeSide(getGrabber()->getEyeSide());
	}

	Measure::initiateScannerResult(d_ptr->scanMeasure);
	return true;
}


bool OctSystem::Scanner::captureMeasureResult(void)
{
	// Stop Ir camera threads before scan images processing. 
	// getMainboard()->getCorneaCamera()->pause();
	// getMainboard()->getRetinaCamera()->pause();

	OctCorneaImage cornea;
	getMainboard()->getCorneaCamera()->captureFrame(cornea);
	Measure::assignCorneaImageToScannerResult(cornea);

	LogD() << "Cornea frame image assigned";

	// OctRetinaImage retina;
	// getMainboard()->getRetinaCamera()->captureFrame(retina);
	// Measure::assignRetinaImageToScannerResult(retina);
	auto image = RetinaAlign::getRetinaImage();
	Measure::assignRetinaImageToScannerResult(image);
	LogD() << "Retina frame image assigned";
	return true;
}


void OctSystem::Scanner::initializeCallbacks(void)
{
	getImpl().cbGrabPreviewBuffer = std::bind(&Scanner::callbackGrabPreviewBuffer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	getImpl().cbGrabMeasureBuffer = std::bind(&Scanner::callbackGrabMeasureBuffer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
	getImpl().cbGrabEnfaceBuffer = std::bind(&Scanner::callbackGrabEnfaceBuffer, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	getImpl().cbMeasureImageCompleted = std::bind(&Scanner::receiveMeasureImageCompleted,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, 
		std::placeholders::_7, std::placeholders::_8);
	GlobalRegister::setMeasureImageCompletedEvent(&d_ptr->cbMeasureImageCompleted);

	getImpl().cbPreviewImageCompleted = std::bind(&Scanner::receivePreviewImageCompleted,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
		std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
	GlobalRegister::setPreviewImageCompletedEvent(&d_ptr->cbPreviewImageCompleted);

	getImpl().cbEnfaceImageCompleted = std::bind(&Scanner::receiveEnfaceImageCompleted,
		std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
	GlobalRegister::setEnfaceImageCompletedEvent(&d_ptr->cbEnfaceImageCompleted);
	return;
}

bool OctSystem::Scanner::initPattern(void)
{
	if (!isInitiated()) {
		return false;
	}

	getPattern().clear();

	int fore1 = GlobalSettings::getTriggerForePadd(0);
	int fore2 = GlobalSettings::getTriggerForePadd(1);
	int fore3 = GlobalSettings::getTriggerForePadd(2);
	setTriggerForePaddings(ScanSpeed::Fastest, fore1);
	setTriggerForePaddings(ScanSpeed::Faster, fore2);
	setTriggerForePaddings(ScanSpeed::Normal, fore3);

	int post1 = GlobalSettings::getTriggerPostPadd(0);
	int post2 = GlobalSettings::getTriggerPostPadd(1);
	int post3 = GlobalSettings::getTriggerPostPadd(2);
	setTriggerPostPaddings(ScanSpeed::Fastest, post1);
	setTriggerPostPaddings(ScanSpeed::Faster, post2);
	setTriggerPostPaddings(ScanSpeed::Normal, post3);
	LogD() << "Trigger paddings, fore: " << fore1 << ", " << fore2 << ", " << fore3 << ", post: " << post1 << ", " << post2 << ", " << post3;

	bool ret = true;
	return ret;
}


bool OctSystem::Scanner::updatePatternLayout(bool enface)
{
	int points, lines;

	points = getPattern().getPreviewScanPoints();
	lines = getPattern().getPreviewScanLines();

	if (!getImpl().process.setPreviewFeature(lines, points)) {
		return false;
	}

	points = getPattern().getMeasureScanPoints();
	lines = getPattern().getMeasureScanLines();

	if (!getImpl().process.setMeasureFeature(lines, points)) {
		return false;
	}

	if (enface) {
		points = getPattern().getEnfaceScanPoints();
		lines = getPattern().getEnfaceScanLines();

		if (!getImpl().process.setEnfaceFeature(lines, points)) {
			return false;
		}
	}
	return true;
}


Scanner::ScannerImpl & OctSystem::Scanner::getImpl(void)
{
	return *d_ptr;
}

float OctSystem::Scanner::getCameraExposureTime(void)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		auto expTime = getImpl().usb3Grab->getLineCameraExposureTime();
		return expTime;
	}
	return 0.0f;
}

void OctSystem::Scanner::setCameraExposureTime(float expTime)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		getImpl().usb3Grab->setLineCameraExposureTime(expTime);
	}
}

void OctSystem::Scanner::setTriggerTimeStep(float timeStep)
{
	getGalvanometer()->setTriggerTimeStep(timeStep);
	return;
}


void OctSystem::Scanner::setTriggerTimeDelay(uint32_t timeDelay)
{
	getGalvanometer()->setTriggerTimeDelay(timeDelay);
	return;
}


void OctSystem::Scanner::setTriggerForePaddings(ScanSpeed speed, uint32_t padds)
{
	getGalvanometer()->triggerForePaddings(speed, padds);
	return;
}


void OctSystem::Scanner::setTriggerPostPaddings(ScanSpeed speed, uint32_t padds)
{
	getGalvanometer()->triggerPostPaddings(speed, padds);
	return;
}

float OctSystem::Scanner::getCameraAnalogGain(void)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		auto value = getImpl().usb3Grab->getLineCameraAnalogGain();
		return value;
	}
	return 0.0f;
}

bool OctSystem::Scanner::setCameraAnalogGain(float gain)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		auto flag = getImpl().usb3Grab->setLineCameraAnalogGain(gain);
		return flag;
	}
	return false;
}

float OctSystem::Scanner::getTriggerTimeStep(void)
{
	if (!getGalvanometer()) {
		return 0.0f;
	}
	return getGalvanometer()->getTriggerTimeStep();
}


uint32_t OctSystem::Scanner::getTriggerTimeDelay(void)
{
	if (!getGalvanometer()) {
		return 0;
	}
	return getGalvanometer()->getTriggerTimeDelay();
}


uint32_t OctSystem::Scanner::getTriggerForePaddings(ScanSpeed speed)
{
	if (!getGalvanometer()) {
		return 0;
	}
	return 	getGalvanometer()->triggerForePaddings(speed);
}


uint32_t OctSystem::Scanner::getTriggerPostPaddings(ScanSpeed speed)
{
	if (!getGalvanometer()) {
		return 0;
	}
	return 	getGalvanometer()->triggerPostPaddings(speed);
}

float OctSystem::Scanner::getDefaultTriggerTimeStep(ScanSpeed speed)
{
	if (speed == ScanSpeed::Normal) {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbTriggerTimeStep(speed);
		}
		return TRIGGER_TIME_STEP_AT_NORMAL_SPEED;
	}
	else if (speed == ScanSpeed::Faster) {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbTriggerTimeStep(speed);
		}
		return TRIGGER_TIME_STEP_AT_FASTER_SPEED;
	}
	else {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbTriggerTimeStep(speed);
		}
		return TRIGGER_TIME_STEP_AT_FASTEST_SPEED;
	}
}

float OctSystem::Scanner::getDefaultCameraExposureTime(ScanSpeed speed)
{
	if (speed == ScanSpeed::Normal) {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbExposureTime(speed);
		}
		return TRIGGER_TIME_STEP_AT_NORMAL_SPEED;
	}
	else if (speed == ScanSpeed::Faster) {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbExposureTime(speed);
		}
		return TRIGGER_TIME_STEP_AT_FASTER_SPEED;
	}
	else {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return GlobalSettings::getOctUsbExposureTime(speed);
		}
		return TRIGGER_TIME_STEP_AT_FASTEST_SPEED;
	}
}


void OctSystem::Scanner::callbackGrabPreviewBuffer(unsigned short * buffer, int sizeX, int sizeY, int index)
{
	int idxOfImage = ScanGrab::getIndexOfPreviewImageNext();
	getImpl().process.receiveDataForPreview(buffer, sizeY, idxOfImage);
	return;
}


void OctSystem::Scanner::callbackGrabMeasureBuffer(unsigned short * buffer, int sizeX, int sizeY, int index)
{
	int idxOfImage = ScanGrab::getIndexOfMeasureImageNext();
	getImpl().process.receiveDataForMeasure(buffer, sizeY, idxOfImage);
	return;
}


void OctSystem::Scanner::callbackGrabEnfaceBuffer(unsigned short * buffer, int sizeX, int sizeY, int index)
{
	int idxOfImage = ScanGrab::getIndexOfEnfaceImageNext();
	getImpl().process.receiveDataForEnface(buffer, sizeY, idxOfImage);
	return;
}


void OctSystem::Scanner::receivePreviewImageCompleted(unsigned char * data, unsigned int width, unsigned int height, float qindex, float sigRatio, int refPoint, int idxImage)
{
	bool repeat = ScanGrab::isPreviewImagesRepeated();

	// LogD() << "Line trace in preview image completed, index: " << idxImage << ", repeat: " << repeat;

	auto line = getPattern().getPreviewScan().getLineTraceFromImageIndex(idxImage, repeat, true);
	if (line == nullptr) {
		LogD() << "Invalid line trace in preview image completed, index: " << idxImage << ", repeat: " << repeat;
		return;
	}

	int idxLine = getPattern().getPreviewScan().getLineIndexFromImageIndex(idxImage, repeat, true);
	if (idxLine < 0) {
		LogD() << "Invalid line index in preview image completed, index: " << idxImage;
		return;
	}

	if (repeat) {
		OctRoute route = line->getRouteOfScan();
		OctScanSection section(idxLine, route);
		OctScanImage image(data, width, height, qindex, sigRatio, refPoint);

		if (Measure::assignPreviewImage(section, image)) {
			LogD() << "Preview-hd image assigned, idxLine: " << idxLine << ", idxImage: " << idxImage;
		}
	}

	static int image_count = 0;
	static auto timer_tick = GetTickCount64();

	image_count++;
	if (image_count > 0 && image_count % 100 == 0) {
		auto curr_tick = GetTickCount64();
		LogD() << "Preview 100 frames received in " << (curr_tick - timer_tick) / 1000.0f << "ms";
		timer_tick = curr_tick;
		image_count = 0;
	}

	//if (!Measure::assignPreviewImage(section, image)) {
	// LogD() << "Failed to assign preview image, idxLine: " << idxLine << ", idxImage: " << idxImage;
	//}
	// () << "Preview image, w: " << width << ", h: " << height << ", qindex: " << qindex << ", refPoint: " << refPoint;
	return;
}


void OctSystem::Scanner::receiveMeasureImageCompleted(unsigned char * data, unsigned int width,
									unsigned int height, float qindex, float sigRatio,
									int refPoint, int idxImage, unsigned char* lateral)
{
	auto line = getPattern().getMeasureScan().getLineTraceFromImageIndex(idxImage, true);
	if (line == nullptr) {
		LogD() << "Invalid line trace in measure image completed";
		return;
	}

	int idxLine = getPattern().getMeasureScan().getLineIndexFromImageIndex(idxImage, true);
	if (idxLine < 0) {
		LogD() << "Invalid line index in measure image completed";
		return;
	}

	int idxOverlap = getPattern().getMeasureScan().getOverlapIndexFromImageIndex(idxImage, true);

	OctRoute route = line->getRouteOfScan();
	OctScanSection section(idxLine, route);
	OctScanImage image(data, width, height, qindex, sigRatio, refPoint);

	int numLines = getPattern().getMeasureScan().getNumberOfScanLines();
	int numOverlaps = getPattern().getMeasureScan().getNumberOfScanOverlaps();
	bool isClosed = (numOverlaps == (idxOverlap + 1));
	bool vertical = getPattern().getMeasureScan().isVertical();
	bool reversed = false; // (getPattern().getMeasureScan().isRaster() && (idxLine % 2));
	bool segment = false; // (numOverlaps == 1 || !(numLines % numOverlaps));

	if (idxOverlap > 0 && getImpl().scanMeasure.getPattern().isAngioScan()) { 
		// return;
	}
	else {
		image.setReversed(reversed);
		if (Measure::assignPatternImage(section, image, idxOverlap, segment)) {
			LogD() << "Pattern image assigned, idxLine: " << idxLine << ", idxImage: " << idxImage << ", idxOverlap: " << idxOverlap;
			if (!idxOverlap) {
				ChainOutput::setLateralLineOfEnface(lateral, idxLine, width, numLines, vertical, reversed);
			}
		}
	}

	if (Measure::getCountOfPatternSections() == numLines && isClosed) {
		if (getPattern().getMeasureScan().isScan3D()) {
			int enfaceW = (vertical ? numLines : width);
			int enfaceH = (vertical ? width : numLines);

			ChainOutput::setEnfaceImageResult(enfaceW, enfaceH);
			std::uint8_t* data = ChainOutput::getEnfaceImageBuffer();
			OctEnfaceImage enface(data, enfaceW, enfaceH);
			if (Measure::assignEnfaceImage(enface)) {
				LogD() << "Pattern enface assigned, w: " << enfaceW << ", h: " << enfaceH;
			}
		}
	}

	if (idxOverlap == 0) {
		if (ChainSetup::usePreviewImageCallback2()) {
			PreviewImageCallback2* callback2 = ChainSetup::getPreviewImageCallback2();
			if (callback2) {
				(*callback2)(data, width, height, qindex, sigRatio, refPoint, idxLine);
			}
		}
		else {
			if (ChainSetup::usePreviewImageCallback()) {
				PreviewImageCallback* callback = ChainSetup::getPreviewImageCallback();
				if (callback) {
					(*callback)(data, width, height, qindex, idxLine);
				}
			}
		}
	}
	return;
}


void OctSystem::Scanner::receiveEnfaceImageCompleted(unsigned char * data, unsigned int width, unsigned int height)
{
	return;
}



OctPattern::PatternPlan & OctSystem::Scanner::getPattern(void)
{
	return getImpl().pattern;
}


OctGrab::FrameGrabber * OctSystem::Scanner::getGrabber(void)
{
	return getImpl().grabber;
}


OctGrab::Usb3Grabber * OctSystem::Scanner::getUsb3Grabber(void)
{
	return getImpl().usb3Grab;
}


OctDevice::Galvanometer * OctSystem::Scanner::getGalvanometer(void)
{
	return getImpl().galvano;
}


OctDevice::MainBoard * OctSystem::Scanner::getMainboard(void)
{
	return getImpl().board;
}
