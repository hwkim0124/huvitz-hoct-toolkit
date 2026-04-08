#include "stdafx.h"
#include "Controller.h"
#include "Scanner.h"
#include "Measure.h"
#include "CorneaAlign.h"
#include "RetinaAlign.h"
#include "RetinaTrack.h"
#include "RetinaTrack2.h"
#include "RetinaTracker.h"
#include "RetinaTracking.h"
#include "Optimizer.h"
#include "Camera.h"

#include "OctDevice2.h"
#include "OctConfig2.h"
#include "OctGrab2.h"
#include "SigChain2.h"
#include "CppUtil2.h"
#include "CorTopo2.h"


#include <vector>

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace OctGrab;
using namespace CorTopo;
using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct Controller::ControllerImpl
{
	MainBoard mainBoard;
	MainPlatform mainPlatform;
	FrameGrabber grabber;
	Usb3Grabber usb3Grab;
	Scanner scanner;
	Camera camera;

	CorneaAlign corneaAlign;
	RetinaAlign retinaAlign;
	CorTracker corneaTracker;

	OctFundusMeasure fundusMeasure;
	OctScanMeasure scanMeasure;
	OctScanOffset scanOffset;

	CompensationLensMode compenLensMode;

	bool initiated;
	bool isSmallPupilMode;
	bool isRetinaTrackMode = false;
	bool isRetinaTrackOn = false;
	bool isCorneaTrackMode = false;

	bool isSplitFocusOn = false;
	bool isWorkingDotsOn = false;
	bool isSplitMirrorIn = false;

	ControllerImpl() : initiated(false), isSmallPupilMode(false), compenLensMode(CompensationLensMode::Zero)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Controller::ControllerImpl> Controller::d_ptr(new ControllerImpl());


Controller::Controller()
{
	initializeController();
}


Controller::~Controller()
{
}


Controller::ControllerImpl & OctSystem::Controller::getImpl(void) 
{
	return *d_ptr;
}


bool OctSystem::Controller::startAutoDiopterFocus(void)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	bool res = Optimizer::startAutoDiopterFocus();
	return res;
}


bool OctSystem::Controller::startAutoPolarization(void)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	bool res = Optimizer::startAutoPolarization();
	return res;
}


bool OctSystem::Controller::startAutoReference(void)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	bool res = Optimizer::startAutoReference(nullptr, true, true, true, true);
	return res;
}


void OctSystem::Controller::cancelAutoDiopterFocus(void)
{
	Optimizer::cancelAutoDiopterFocus(true);
	return;
}


void OctSystem::Controller::cancelAutoPolarization(void)
{
	Optimizer::cancelAutoPolarization(true);
	return;
}


void OctSystem::Controller::cancelAutoReference(void)
{
	Optimizer::cancelAutoReference(true);
	return;
}


bool OctSystem::Controller::isAutoDiopterFocusing(void)
{
	return Optimizer::isAutoDiopterFocusing();
}


bool OctSystem::Controller::isAutoPolarizing(void)
{
	return Optimizer::isAutoPolarizing();
}


bool OctSystem::Controller::isAutoReferencing(void)
{
	return Optimizer::isAutoReferencing();
}


bool OctSystem::Controller::startAutoPosition(AutoPositionCompletedCallback * callback)
{
	LogD() << "Auto position started, grabbing: " << isScanGrabbing();
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	GlobalRegister::setAutoPositionCompletedCallback(callback);
	bool res = Optimizer::startAutoPosition();
	return res;
}


bool OctSystem::Controller::startAutoOptimize(bool noFundus, AutoOptimizeCompletedCallback * callback, bool dioptFocus, bool topogrphyMode)
{
	if (isAutoOptimizing()) {
		return false;
	}

	LogD() << "Auto optimize started, noFundus: " << noFundus << ", dioptFocus: " << dioptFocus << ", grabbing: " << isScanGrabbing();
	GlobalRegister::setAutoOptimizeCompletedCallback(callback);
	bool res = Optimizer::startAutoOptimize(noFundus, dioptFocus, topogrphyMode);
	return res;
}


void OctSystem::Controller::cancelAutoOptimize(void)
{
	LogD() << "Auto optimize canceled, optimizing: " << Optimizer::isOptimizing();
	if (Optimizer::isOptimizing()) {
		Optimizer::cancel();
	}
	return ;
}


void OctSystem::Controller::cancelAutoPosition(void)
{
	LogD() << "Auto position canceled, optimizing: " << Optimizer::isOptimizing();
	if (Optimizer::isOptimizing()) {
		Optimizer::cancel();
	}
	return;
}


bool OctSystem::Controller::isAutoOptimizing(void)
{
	return Optimizer::isOptimizing() ;
}


bool OctSystem::Controller::startAutoMeasure(AutoMeasureCompletedCallback * callback, bool macular, bool cornea)
{
	LogD() << "Auto measure started, macular: " << macular << ", cornea: " << cornea;
	GlobalRegister::setAutoMeasureCompletedCallback(callback);
	bool res = Optimizer::startAutoMeasure(macular, cornea);
	return res;
}


void OctSystem::Controller::cancelAutoMeasure(void)
{
	LogD() << "Auto measure canceled, optimizing: " << Optimizer::isOptimizing();
	if (Optimizer::isOptimizing()) {
		Optimizer::cancel();
	}
	return;
}


bool OctSystem::Controller::isAutoMeasuring(void)
{
	return Optimizer::isOptimizing();
}


bool OctSystem::Controller::startAutoCorneaFocus(AutoCorneaFocusCompletedCallback * callback, bool compenLens)
{
	GlobalRegister::setAutoCorneaFocusCompletedCallback(callback);

	if (compenLens) {
		changeOctDiopterLens(CompensationLensMode::Plus);
	}

	bool res = Optimizer::startAutoCorneaFocus(compenLens);
	return res;
}


bool OctSystem::Controller::startAutoRetinaFocus(AutoRetinaFocusCompletedCallback * callback, bool compenLens)
{
	GlobalRegister::setAutoRetinaFocusCompletedCallback(callback);

	if (compenLens) {
		changeOctDiopterLens(CompensationLensMode::Zero);
	}

	bool res = Optimizer::startAutoRetinaFocus(compenLens);
	return res;
}


bool OctSystem::Controller::startAutoAxialLength(AutoCorneaFocusCompletedCallback * cornea, AutoRetinaFocusCompletedCallback * retina, int measureCount, bool keepOnError)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	GlobalRegister::setAutoCorneaFocusCompletedCallback(cornea);
	GlobalRegister::setAutoRetinaFocusCompletedCallback(retina);

	bool res = Optimizer::startAutoAxialLength(measureCount, keepOnError);
	return res;
}


bool OctSystem::Controller::startAutoLensThickness(AutoCorneaFocusCompletedCallback* cornea, AutoLensFrontFocusCompletedCallback * front, AutoLensBackFocusCompletedCallback * back, int measureCount, bool keepOnError)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	GlobalRegister::setAutoCorneaFocusCompletedCallback(cornea);
	GlobalRegister::setAutoLensFrontFocusCompletedCallback(front);
	GlobalRegister::setAutoLensBackFocusCompletedCallback(back);

	bool res = Optimizer::startAutoLensThickness(measureCount, keepOnError);
	return res;
}


bool OctSystem::Controller::startAutoLensThickness2(AutoCorneaFocusCompletedCallback * cornea, AutoLensFrontFocusCompletedCallback * front, int measureCount, bool keepOnError, bool referFixed)
{
	if (!isScanGrabbing()) {
		return false;
	}
	if (isAutoOptimizing()) {
		return false;
	}

	GlobalRegister::setAutoCorneaFocusCompletedCallback(cornea);
	GlobalRegister::setAutoLensFrontFocusCompletedCallback(front);

	bool res = Optimizer::startAutoLensThickness2(measureCount, keepOnError, referFixed);
	return res;
}


void OctSystem::Controller::cancelAutoMeasureFocus(void)
{
	cancelAutoOptimize();
	return;
}


bool OctSystem::Controller::isAutoMeasureFocusing(void)
{
	return isAutoOptimizing();
}


bool OctSystem::Controller::startAutoLensFrontFocus(AutoLensFrontFocusCompletedCallback * callback, int corneaReferPos, float corneaDioptPos)
{
	GlobalRegister::setAutoLensFrontFocusCompletedCallback(callback);

	bool result = Optimizer::startAutoLensFrontFocus(corneaReferPos, corneaDioptPos);
	return result;
}


bool OctSystem::Controller::startAutoLensBackFocus(AutoLensBackFocusCompletedCallback * callback, int frontReferPos, float frontDioptPos)
{
	GlobalRegister::setAutoLensBackFocusCompletedCallback(callback);

	bool result = Optimizer::startAutoLensBackFocus(frontReferPos, frontDioptPos);
	return result;
}


bool OctSystem::Controller::isAutoPositioning(void)
{
	return Optimizer::isOptimizing();
}


void OctSystem::Controller::startAutoAlignment(CorneaAlignTarget target)
{
	getCorneaAlign()->startAutoAlignment(target);
	return;
}


void OctSystem::Controller::cancelAutoAlignment(void)
{
	getCorneaAlign()->cancelAutoAlignment();
	return;
}


bool OctSystem::Controller::isAutoAligning(void)
{
	return getCorneaAlign()->isAutoAligning();
}


void OctSystem::Controller::startAutoFundusFocus(void)
{
	// getRetinaAlign()->startAutoAlignment();
	Optimizer::startAutoSplitFocus();
	return;
}


void OctSystem::Controller::cancelAutoFundusFocus(void)
{
	// getRetinaAlign()->cancelAutoAlignment();
	Optimizer::cancelAutoSplitFocus(true);
	return;
}


bool OctSystem::Controller::isAutoFundusFocusing(void)
{
	// return getRetinaAlign()->isAutoAligning();
	return Optimizer::isAutoSplitFocusing();
}


void OctSystem::Controller::cancelAutoControls(void)
{
	Controller::cancelAutoOptimize();
	Controller::cancelAutoAlignment();
	Controller::cancelAutoDiopterFocus();
	Controller::cancelAutoFundusFocus();
	Controller::cancelAutoPolarization();
	Controller::cancelAutoReference();
	return;
}


bool OctSystem::Controller::startCorneaCamera(CorneaCameraImageCallback * callback)
{
	setCorneaCameraImageCallback(callback);

	if (!getMainBoard()->getCorneaIrCamera()->play()) {
		return false;
	}
	return true;
}


void OctSystem::Controller::closeCorneaCamera(void)
{
	getMainBoard()->getCorneaIrCamera()->pause();
	setCorneaCameraImageCallback(nullptr);
	return;
}

int OctSystem::Controller::getErrorCountCorneaCamera(void)
{
	return getMainBoard()->getCorneaIrCamera()->getErrorCount();
}

bool OctSystem::Controller::startRetinaCamera(RetinaCameraImageCallback * callback)
{
	setRetinaCameraImageCallback(callback);

	if (!getMainBoard()->getRetinaIrCamera()->play()) {
		return false;
	}
	return true;
}


void OctSystem::Controller::closeRetinaCamera(void)
{
	getMainBoard()->getRetinaIrCamera()->pause();
	setRetinaCameraImageCallback(nullptr);
	return;
}

int OctSystem::Controller::getErrorCountRetinaCamera(void)
{
	return getMainBoard()->getRetinaIrCamera()->getErrorCount();
}

bool OctSystem::Controller::resetIrCameras()
{
	return getMainBoard()->openFTDIdevices(true, true);
}

void OctSystem::Controller::applyRetinaCameraPreset(int preset)
{
	getMainBoard()->getRetinaCamera()->applyCameraPreset(preset);
	GlobalSettings::setRetinaCameraPreset(preset);

	int index = preset;
	FundusSettings* fset = SystemProfile::getFundusSettings();
	getMainBoard()->getSplitFocusLed()->setIntensity(fset->getAutoFlashSplitFocus(index));
	getMainBoard()->getRetinaIrLed()->setIntensity(fset->getAutoFlashRetinaIr(index));
	getMainBoard()->getWorkingDotsLed()->setIntensity(fset->getAutoFlashWorkDot1(index));
	getMainBoard()->getWorkingDot2Led()->setIntensity(fset->getAutoFlashWorkDot2(index));
	return;
}


int OctSystem::Controller::getCurrentRetinaCameraPreset(void)
{
	return GlobalSettings::getRetinaCameraPreset();
}


int OctSystem::Controller::getAutoFlashDefaultLevel(int preset)
{
	return GlobalSettings::flashPresetDefLevel(preset);
}

int OctSystem::Controller::getRetinaCameraEnhanceLevel(void)
{
	return (int)(GlobalSettings::retinaImageClipLimit() * 10);
}

void OctSystem::Controller::setRetinaCameraEnhanceLevel(int level, bool reset)
{
	float value = level / 10.0f;
	auto rset = SystemProfile::getRetinaSettings();
	value = (reset ? rset->getEnhanceClipLimit() : max(min(10.0f, value), 0.0f));
	GlobalSettings::retinaImageClipLimit() = value;
	return;
}

bool OctSystem::Controller::presetScanEnface(EnfaceImageCallback * cbEnfaceImage,
								int numPoints, int numLines, float rangeX, float rangeY)
{
	setEnfaceImageCallback(cbEnfaceImage);

	if (!getScanner()->setupEnface(numPoints, numLines, rangeX, rangeY)) {
		return false;
	}
	return true;
}


bool OctSystem::Controller::presetScanOffset(float offsetX, float offsetY, float angle, float scaleX, float scaleY)
{
	getImpl().scanOffset.set(offsetX, offsetY, angle, scaleX, scaleY);

	if (!getScanner()->setupDispacement(offsetX, offsetY, angle, scaleX, scaleY)) {
		return false;
	}
	return true;
}


bool OctSystem::Controller::presetScanEnfaceAsDefault(void)
{
	OctScanPattern& enface = d_ptr->scanMeasure.getEnface();
	enface.setupAsEnfaceDefault();

	if (!getScanner()->setupEnface(enface._numPoints, enface._numLines, enface._rangeX, enface._rangeY)) {
		return false;
	}
	return true;
}


bool OctSystem::Controller::presetScanEnface(OctScanPattern pattern)
{
	OctScanPattern& enface = d_ptr->scanMeasure.getEnface();
	enface = pattern;

	if (!getScanner()->setupEnface(enface._numPoints, enface._numLines, enface._rangeX, enface._rangeY)) {
		return false;
	}
	return true;
}


bool OctSystem::Controller::moveScanCenterPosition(float mmPosX, float mmPosY, bool isCornea)
{
	/*
	if (!isScanning()) {
		return false;
	}
	*/

	if (getScanPattern().isCorneaScan()) {
		isCornea = true;
	}
	else {
		if (isScanGrabbing()) {
			// Prevent from alteration in galvanometer offset setting during tracking going on. 
			if (getImpl().isRetinaTrackOn) {
				return false;
			}
		}
	}

	LogD() << "Scanning center position moved to " << mmPosX << ", " << mmPosY;

	bool result = getMainBoard()->setGalvanoDynamicOffset(mmPosX, mmPosY, isCornea);
	if (result) {
		// Scanning offset is applied to the initial scanning center position 
		// only when the scanning pattern is started.
		d_ptr->scanOffset.set(mmPosX, mmPosY);

		// It is called just in case the relocation of scanning center is being required. 
		// If retina tracking mode, the relative location to the optic feature, 
		// which has been initialized at scan starts, should be newly obtained. 

		// Note that the relative movements to the previous scanning center are cleared. 
		if (getImpl().isRetinaTrackMode) {
			auto * track = RetinaTracking::getInstance();
			if (track) {
				track->setupScanningRegion(mmPosX, mmPosY);
			}
		}
	}
	return result;
}


void OctSystem::Controller::getScanCenterPosition(float & mmPosX, float & mmPosY)
{
	mmPosX = d_ptr->scanOffset.getScanOffsetX();
	mmPosY = d_ptr->scanOffset.getScanOffsetY();
	return;
}


bool OctSystem::Controller::prepareScan(void)
{
	if (isScanning()) {
		closeScan(false);
	}

	return true;
}


bool OctSystem::Controller::prepareScan(const OctScanMeasure& measure)
{
	getImpl().scanMeasure = measure;

	if (!getScanner()->checkMeasureDescript(getImpl().scanMeasure)) {
		return false;
	}

	// Move internal fixation into ready position for engineer mode. 
	if (GlobalSettings::isEngineerModeSettings()) {
		if (getImpl().isRetinaTrackMode) {
			if (!moveInternalFixationToRetinaTrack()) {
				return false;
			}
		}
		else {
			if (!moveInternalFixationToStartPattern()) {
				return false;
			}
		}
	}

	if (GlobalSettings::isUserModeSettings()) {
		readyMotorPositionsToScan(false, measure.isAnteriorReadyPosition());
	}

	if (!changeScanSpeedToPreview()) {
		return false;
	}

	if (GlobalSettings::useUsbCmosCameraEnable()) {
		// Galvanometer slew settings for USB grabber only.
		if (getScanPattern().getScanRangeX() >= 9.0f || getScanPattern().getScanRangeY() >= 9.0f) {
			getMainBoard()->getGalvanometer()->setAutoSlew(1);
		}
		else {
			getMainBoard()->getGalvanometer()->setAutoSlew(0);
			getMainBoard()->getGalvanometer()->setSlewRate(40);
		}
	}

	if (getScanPattern().isCorneaScan()) {
		prepareLEDsForAnteriorScan(true);
	}
	else {
		if (getImpl().isRetinaTrackMode) {
			prepareLEDsForRetinaTrack(true);
			auto* track = RetinaTracking::getInstance();
			if (track) {
				track->requestFrameAdjustment(true);
			}
		}
	}
	return true;
}


bool OctSystem::Controller::releaseScan(void)
{
	if (getScanPattern().isCorneaScan()) {
		prepareLEDsForAnteriorScan(false);
	}
	else {
		releaseRetinaTracking();
	}
	return true;
}


bool OctSystem::Controller::initializeRetinaTrackMode(const OctScanMeasure & measure, bool retinaTrack)
{
	getImpl().isRetinaTrackOn = false;
	if (!measure.pattern().isCorneaScan()) {
		if (retinaTrack) {
			auto* track = RetinaTracking::getInstance();
			if (track) {
				track->prepareRetinaTracking(measure);
			}
		}
		getImpl().isRetinaTrackMode = retinaTrack;
	}
	else {
		getImpl().isRetinaTrackMode = false;
	}
	return getImpl().isRetinaTrackMode;
}


void OctSystem::Controller::releaseRetinaTracking(void)
{
	auto* track = RetinaTracking::getInstance();
	if (track) {
		if (getImpl().isRetinaTrackMode) {
			track->cancelRetinaTracking(true);
		}
		else {
			track->cancelRetinaTracking(false);
		}
	}


	if (getImpl().isRetinaTrackMode) {
		prepareLEDsForRetinaTrack(false);

		getImpl().isRetinaTrackMode = false;
		getImpl().isRetinaTrackOn = false;
	}
	return;
}


bool OctSystem::Controller::initializeCorneaTrack(const OctScanMeasure & measure)
{
	getImpl().isCorneaTrackMode = false;

	if (measure._pattern.isCorneaScan()) {
		getImpl().corneaTracker.initialize(measure._preview);
		setPreviewCorneaCallback(getImpl().corneaTracker.getCallbackFunction());
		getImpl().isCorneaTrackMode = true;
	}
	return getImpl().isCorneaTrackMode;
}


bool OctSystem::Controller::prepareLEDsForAnteriorScan(bool start)
{
	if (getScanPattern().isCorneaScan())
	{
		static unsigned short splitValue = 0;

		if (start) {
			getMainBoard()->getKeratoRingLed()->lightOff();
			getMainBoard()->getKeratoFocusLed()->lightOff();
			getMainBoard()->getWorkingDotsLed()->lightOff();
			getMainBoard()->getWorkingDot2Led()->lightOff();

			// getMainBoard()->getSplitFocusLed()->lightOff();
			splitValue = getMainBoard()->getSplitFocusLed()->getIntensity();
			getMainBoard()->getSplitFocusLed()->setIntensity(0);
		}
		else {
			getMainBoard()->getKeratoRingLed()->lightOn();
			getMainBoard()->getKeratoFocusLed()->lightOn();
			getMainBoard()->getWorkingDotsLed()->lightOn();
			getMainBoard()->getWorkingDot2Led()->lightOn();
			
			// getMainBoard()->getSplitFocusLed()->lightOn();
			getMainBoard()->getSplitFocusLed()->setIntensity(splitValue);
		}
	}
	return true;
}


bool OctSystem::Controller::prepareLEDsForRetinaTrack(bool start)
{
	if (start) {
		// Illumination lights should be turned on till optimization started. 
		// turnOnSplitFocus(false);
		// turnOnWorkingDots(false);

		bool splitOn = getMainBoard()->getSplitFocusLed()->isLightOn();
		bool wdotsOn = getMainBoard()->getWorkingDotsLed()->isLightOn();
		bool splitIn = getMainBoard()->getSplitFocusMotor()->isPositionAtMirrorIn();

		getImpl().isSplitFocusOn = splitOn;
		getImpl().isWorkingDotsOn = wdotsOn;
		getImpl().isSplitMirrorIn = splitIn;
		
		auto* track = RetinaTracking::getInstance();
		if (track) {
			track->setGuideLedsInitStatus(splitOn, wdotsOn);
			track->setMirrorMotorInitStatus(splitIn);
		}

		// getMainBoard()->getPupilMaskMotor()->updatePositionToSmall();
		// getMainBoard()->getSplitFocusMotor()->updatePositionToMirrorOut();
	}
	else {
		bool splitOn = getImpl().isSplitFocusOn;
		bool wdotsOn = getImpl().isWorkingDotsOn;
		bool splitIn = getImpl().isSplitMirrorIn;

		turnOnSplitFocus(splitOn);
		turnOnWorkingDots(wdotsOn);

		if (splitIn) {
			getMainBoard()->getSplitFocusMotor()->updatePositionToMirrorIn();
		}
		// getMainBoard()->getPupilMaskMotor()->updatePositionToNormal();
		// getMainBoard()->getSplitFocusMotor()->updatePositionToMirrorIn();
	}
	return true;
}


bool OctSystem::Controller::readyMotorPositionsToScan(bool init, bool anteriorOrigin)
{
	if (init) {
		moveReferenceToOrigin(false);
		float diopter = (isAtSideOd() ? Measure::getPatient().diopterOD() : Measure::getPatient().diopterOS());
		moveScanFocusToDiopter(diopter);
		movePolarizationToDegree(0.0f);
	}
	else {
		static bool anterior = false;

		if (getScanPattern().isMeasureScan()) {
			moveScanFocusToDiopter(0.0f);
			movePolarizationToDegree(0.0f);

			changeOctDiopterLens(CompensationLensMode::Zero);
		}

		if (getScanPattern().isCorneaScan()) {
			if (getScanPattern().getPatternName() == PatternName::Topography) {
				moveReferenceToOrigin(true, true);
			}
			else {
				if (anteriorOrigin) {
					moveReferenceToOrigin(true);
				}
			}
			anterior = true;
		}
		else {
			if (anterior) {
				moveReferenceToOrigin(false);
				anterior = false;
			}
		}
	}
	return true;
}


bool OctSystem::Controller::readyMotorPositionsToFundus(bool init)
{
	if (init) {
		float diopter = (isAtSideOd() ? Measure::getPatient().diopterOD() : Measure::getPatient().diopterOS());
		moveFundusFocusToDiopter(diopter);
	}
	return true;
}


bool OctSystem::Controller::moveInternalFixationToStartPattern(void)
{
	OctScanPattern& pattern = getImpl().scanMeasure.getPattern();
	EyeRegion region = pattern.getEyeRegion();
	EyeSide side = getImpl().scanMeasure.getEyeSide(); 

	if (getImpl().scanMeasure.useTargetFundus()) {
		region = EyeRegion::Fundus;
	}

	bool res = turnOnInternalFixationAtCenter(side, region);
	
	return res;
}


bool OctSystem::Controller::moveInternalFixationToRetinaTrack(void)
{
	OctScanPattern& pattern = getImpl().scanMeasure.getPattern();
	EyeRegion region = pattern.getEyeRegion();
	EyeSide side = getImpl().scanMeasure.getEyeSide();
	FixationTarget target = (region == EyeRegion::OpticDisc ? FixationTarget::OpticDisc : FixationTarget::Fundus);

	bool res = turnOnInternalFixation(side, target);
	return res;
}


bool OctSystem::Controller::changeScanSpeedToPreview(void)
{
	bool noImageGrab = getImpl().scanMeasure.useNoImageGrab();

	if (getImpl().scanMeasure.getPattern().isNormalSpeed()) {
		bool result = getScanner()->changeScanSpeedToNormal(noImageGrab);
		LogD() << "Preview scan set to normal speed => " << result;
		return result;
	}
	else if (getImpl().scanMeasure.getPattern().isFasterSpeed()) {
		bool result = getScanner()->changeScanSpeedToFaster(noImageGrab);
		LogD() << "Preview scan set to faster speed => " << result;
		return result;
	}
	else if (getImpl().scanMeasure.getPattern().isCustomSpeed()) {
		bool result = getScanner()->changeScanSpeedToCustom(noImageGrab);
		LogD() << "Preview scan set to custom speed => " << result;
		return result;
	}
	else {
		bool result = getScanner()->changeScanSpeedToFastest(noImageGrab);
		LogD() << "Preview scan set to fastest speed => " << result;
		return result;
	}
	return true;
}


bool OctSystem::Controller::changeScanSpeedToMeasure(void)
{
	bool noImageGrab = getImpl().scanMeasure.useNoImageGrab();

	if (getImpl().scanMeasure.getPattern().isNormalSpeed()) {
		if (!getScanner()->changeScanSpeedToNormal(noImageGrab)) {
			return false;
		}
		DebugOut2() << "Measure scan, normal speed";
	}
	else {
		if (!getScanner()->changeScanSpeedToFastest(noImageGrab)) {
			return false;
		}
		DebugOut2() << "Measure scan, fastest speed";
	}
	return true;
}


bool OctSystem::Controller::dispatchScanningCenter(bool clear)
{
	float offsetX = 0.0f, offsetY = 0.0f;

	if (getImpl().isRetinaTrackMode) {
		auto* track = RetinaTracking::getInstance();

		if (clear) {
			track->getScanningCenterPosition(offsetX, offsetY);
			LogD() << "Scanning center is dispatched with tracking, offset: " << offsetX << ", " << offsetY;
		}
		else {
			getScanCenterPosition(offsetX, offsetY);
			LogD() << "Scanning center is dispatched with previous, offset: " << offsetX << ", " << offsetY;
		}

		if (!moveScanCenterPosition(offsetX, offsetY)) {
			return false;
		}
	}
	else {
		if (clear) {
			if (getImpl().scanMeasure.useTargetFundus()) {
				offsetX = (getImpl().scanMeasure.isOD() ? FUNDUS_SCAN_CENTER_OFFSET_X_OD : FUNDUS_SCAN_CENTER_OFFSET_X_OS);
				offsetY = FUNDUS_SCAN_CENTER_OFFSET_Y;

				LogD() << "Scanning center is dispatched with target fundus, offset: " << offsetX << ", " << offsetY;
				if (!moveScanCenterPosition(offsetX, offsetY)) {
					return false;
				}
			}
			else {
				LogD() << "Scanning center is dispatched, offset: " << offsetX << ", " << offsetY;
				if (!moveScanCenterPosition(0.0f, 0.0f)) {
					return false;
				}
			}
		}
		else {
			getScanCenterPosition(offsetX, offsetY);
			LogD() << "Scanning center is dispatched with previous, offset: " << offsetX << ", " << offsetY;
			if (!moveScanCenterPosition(offsetX, offsetY)) {
				return false;
			}
		}
	}
	return true;
}


void OctSystem::Controller::callbackDeviceStatus(void)
{

}


bool OctSystem::Controller::startScan(EyeSide side, PatternDomain domain, PatternType type, 
									int numPoints, int numLines, float rangeX, float rangeY, 
									int direction, int overlaps, float lineSpace, 
									bool useEnface, bool usePattern, bool useFaster, 
									PreviewImageCallback * cbPreviewImage)
{
	if (isScanning()) {
		return false;
	}

	LogD() << "\n\n---------------------------------------------------------------------------------------";
	LogD() << "Oct scanning started";

	getImpl().isRetinaTrackMode = false;
	getImpl().isRetinaTrackOn = false;

	if (!prepareScan()) {
		// return false;
	}

	if (cbPreviewImage != nullptr) {
		setPreviewImageCallback(cbPreviewImage);
	}

	OctScanPattern pattern;
	pattern.setup(domain, type, numPoints, numLines, rangeX, rangeY, direction, overlaps, lineSpace);
	pattern.setScanOffset(d_ptr->scanOffset);

	d_ptr->scanMeasure.setEyeSide(side);
	d_ptr->scanMeasure.setPattern(pattern);
	d_ptr->scanMeasure.usePhaseEnface(true, useEnface);
	d_ptr->scanMeasure.useFastRaster(true, useFaster);
	d_ptr->scanMeasure.usePreviewPattern(true, usePattern);

	if (!getScanner()->startMeasure(side, domain, type, numPoints, numLines, rangeX, rangeY,
									overlaps, lineSpace, useEnface, usePattern)) {
		return false;
	}

	Optimizer::clearAutoMeasureResult();
	return true;
}


bool OctSystem::Controller::startScan(const OctScanMeasure& measure, 
						PreviewImageCallback * cbPreviewImage, EnfaceImageCallback* cbEnfaceImage, 
						bool clearPreset, bool retinaTrack)
{
	if (isScanning()) {
		return false;
	}

	LogD() << "\n\n---------------------------------------------------------------------------------------";
	LogD() << "Oct scanning started";
	LogD() << "Clear preset: " << clearPreset << ", retina tracking: " << retinaTrack << ", edi scanning: " << false;

	initializeRetinaTrackMode(measure, retinaTrack);
	// initializeCorneaTrack(measure);

	if (!prepareScan(measure)) {
		// return false;
	}

	setPreviewImageCallback(cbPreviewImage);
	setPreviewImageCallback2(nullptr);
	setEnfaceImageCallback(cbEnfaceImage);

	if (!getScanner()->startMeasure(getImpl().scanMeasure)) {
		return false;
	}

	dispatchScanningCenter(clearPreset);
	Optimizer::clearAutoMeasureResult();
	return true;
}


bool OctSystem::Controller::startScan2(const OctScanMeasure & measure, PreviewImageCallback2 * cbPreviewImage, 
										EnfaceImageCallback * cbEnfaceImage, bool clearPreset, bool retinaTrack, bool ediScanning)
{
	if (isScanning()) {
		return false;
	}

	LogD() << "\n\n---------------------------------------------------------------------------------------";
	LogD() << "Oct scanning started";
	LogD() << "Clear preset: " << clearPreset << ", retina tracking: " << retinaTrack << ", edi scanning: " << ediScanning;

	initializeRetinaTrackMode(measure, retinaTrack);
	// initializeCorneaTrack(measure);

	AngioSetup::setEdiScanningMode(ediScanning);

	if (!prepareScan(measure)) {
		// return false;
	}

	setPreviewImageCallback(nullptr);
	setPreviewImageCallback2(cbPreviewImage);
	setEnfaceImageCallback(cbEnfaceImage);

	if (!getScanner()->startMeasure(getImpl().scanMeasure)) {
		return false;
	}

	dispatchScanningCenter(clearPreset);
	Optimizer::clearAutoMeasureResult();
	return true;
}


bool OctSystem::Controller::closeScan(bool measure, ScanPatternAcquiredCallback* cbAcquired, ScanPatternCompletedCallback* cbCompleted, bool retinaTrack)
{
	if (!isScanning()) {
		return false;
	}

	LogD() << "Closing oct scanning, measure: " << measure;

	GlobalRegister::setScanPatternAcquiredCallback(cbAcquired);
	GlobalRegister::setScanPatternCompletedCallback(cbCompleted);

	cancelAutoPosition();
	cancelAutoOptimize();

	if (measure) {
		if (getImpl().isRetinaTrackMode) {
			auto* track = RetinaTracking::getInstance();
			bool track_on = false;
			if (track) {
				track->completeFrameAdjustment();
				if (retinaTrack) {
					track->requestTargetRegistration();
					if (track->waitForTargetRegistration()) {
						if (track->startRetinaTracking()) {
							track_on = true;
						}
					}
				}
				if (!track_on) {
					track->pauseRetinaTracking();
				}
				getImpl().isRetinaTrackOn = track_on;
			}
			LogD() << "Retina tracking to measure, launched: " << getImpl().isRetinaTrackOn;
		}
	}
		
	if (!getScanner()->closePattern(measure)) {
		// return false;
	}

	if (!releaseScan()) {
		// return false;
	}

	LogD() << "Scanning pattern released";

	setSpectrumDataCallback(nullptr);
	setResampleDataCallback(nullptr);
	setPreviewImageCallback(nullptr);
	setPreviewImageCallback2(nullptr);
	setIntensityDataCallback(nullptr);
	return true;
}


void OctSystem::Controller::cancelScanMeasure(void)
{
	if (!isScanning()) {
		return ;
	}

	getScanner()->cancelMeasure();
	return;
}


bool OctSystem::Controller::isScanning(void)
{
	return (getScanner()->isGrabbing() || getScanner()->isPosting());
}


bool OctSystem::Controller::isScanGrabbing(void)
{
	return getScanner()->isGrabbing();
}


bool OctSystem::Controller::isScanProcessing(void)
{
	return getScanner()->isPosting();
}

bool OctSystem::Controller::isScanCancelling(void)
{
	return getScanner()->isCancelling();
}


const OctScanMeasure & OctSystem::Controller::getScanMeasure(void)
{
	return getImpl().scanMeasure;
}


const OctScanPattern& OctSystem::Controller::getScanPattern(void)
{
	return getImpl().scanMeasure.getPattern();
}


const OctScanPattern & OctSystem::Controller::getScanEnface(void)
{
	return getImpl().scanMeasure.getEnface();
}


bool OctSystem::Controller::takeColorFundus(const OctFundusMeasure & measure, 
						FundusImageAcquiredCallback* cbAcquired, FundusImageCompletedCallback* cbCompleted,
						ColorCameraImageCallback * cbColorImage, ColorCameraFrameCallback* cbColorFrame, 
						bool imageProc)
{
	LogD() << "Color fundus start";

	setColorCameraImageCallback(cbColorImage);
	setColorCameraFrameCallback(cbColorFrame);

	GlobalRegister::setFundusImageAcquiredCallback(cbAcquired);
	GlobalRegister::setFundusImageCompletedCallback(cbCompleted);

	if (!imageProc) {
		// Test mode for checking image processing results. 
	}
	else {
		// This is user mode. 
		if (GlobalSettings::useAutoFlashLevel()) {
			setFlashLightLevel(0, true);
		}
	}

	bool result = getCamera()->takeColorFundus(measure, imageProc);	
	return result;
}


void OctSystem::Controller::setCorneaCameraImageCallback(CorneaCameraImageCallback * callback)
{
	getCorneaAlign()->setCameraImageCallback(callback);
	return;
}


void OctSystem::Controller::setRetinaCameraImageCallback(RetinaCameraImageCallback * callback)
{
	getRetinaAlign()->setCameraImageCallback(callback);
	return;
}


void OctSystem::Controller::setColorCameraFrameCallback(ColorCameraFrameCallback * callback)
{
	getMainBoard()->getColorCamera()->setFrameCallback(callback);
	return;
}


void OctSystem::Controller::setColorCameraImageCallback(ColorCameraImageCallback * callback)
{
	getMainBoard()->getColorCamera()->setImageCallback(callback);
	return;
}


void OctSystem::Controller::setColorCameraUsingLv4(bool enable)
{
	getMainBoard()->getColorCamera()->setUsingLv4(enable);
}


void OctSystem::Controller::setJoystickEventCallback(JoystickEventCallback * callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setJoystickEventCallback(callback);
	}
	else {
		getFrameGrabber()->setJoystickEventCallback(callback);
	}
	
	return;
}


void OctSystem::Controller::setEyeSideEventCallback(EyeSideEventCallback * callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setEyeSideEventCallback(callback);
	}
	else {
		getFrameGrabber()->setEyeSideEventCallback(callback);
	}
	return;
}


void OctSystem::Controller::setOptimizeKeyEventCallback(OptimizeKeyEventCallback * callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setOptimizeKeyEventCallback(callback);
	}
	else {
		getFrameGrabber()->setOptimizeKeyEventCallback(callback);
	}
	return;
}


void OctSystem::Controller::setBacklightOnEventCallback(BacklightOnEventCallback * callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setBacklightOnEventCallback(callback);
	}
	else {
		getFrameGrabber()->setBacklightOnEventCallback(callback);
	}
	return;
}

void OctSystem::Controller::setSldWarningEventCallback(SldWarningEventCallback* callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setSldWarningEventCallback(callback);
	}
	return;
}

void OctSystem::Controller::setScannerFaultEventCallback(ScannerFaultEventCallback* callback)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->setScannerFaultEventCallback(callback);
	}
	return;
}

void OctSystem::Controller::setPreviewImageCallback(PreviewImageCallback * callback)
{
	ChainSetup::usePreviewImageCallback(true, callback);
	return;
}


void OctSystem::Controller::setPreviewImageCallback2(PreviewImageCallback2 * callback)
{
	ChainSetup::usePreviewImageCallback2(true, callback);
	return;
}


void OctSystem::Controller::setPreviewCorneaCallback(PreviewCorneaCallback * callback)
{
	ChainSetup::usePreviewCorneaCallback(true, callback);
	return;
}


void OctSystem::Controller::setEnfaceImageCallback(EnfaceImageCallback * callback)
{
	ChainSetup::useEnfaceImageCallback(true, callback);
	return;
}


void OctSystem::Controller::setSpectrumDataCallback(SpectrumDataCallback * callback)
{
	ChainSetup::useSpectrumDataCallback(true, callback);
	return;
}


void OctSystem::Controller::setResampleDataCallback(ResampleDataCallback * callback)
{
	ChainSetup::useResampleDataCallback(true, callback);
	return;
}


void OctSystem::Controller::setIntensityDataCallback(IntensityDataCallback * callback)
{
	ChainSetup::useIntensityDataCallback(true, callback);
	return;
}


MainBoard * OctSystem::Controller::getMainBoard(void)
{
	return &getImpl().mainBoard;
}

MainPlatform * OctSystem::Controller::getMainPlatform(void)
{
	return &getImpl().mainPlatform;
}

OctGrab::FrameGrabber * OctSystem::Controller::getFrameGrabber(void)
{
	return &getImpl().grabber;
}

OctGrab::Usb3Grabber * OctSystem::Controller::getUsb3Grabber(void)
{
	return &getImpl().usb3Grab;
}


OctSystem::Scanner * OctSystem::Controller::getScanner(void)
{
	return &getImpl().scanner;
}


OctSystem::Camera * OctSystem::Controller::getCamera(void)
{
	return &getImpl().camera;
}


OctSystem::CorneaAlign * OctSystem::Controller::getCorneaAlign(void)
{
	return &getImpl().corneaAlign;
}


OctSystem::RetinaAlign * OctSystem::Controller::getRetinaAlign(void)
{
	return &getImpl().retinaAlign;
}


bool OctSystem::Controller::initializeController(void)
{
	try
	{
		// loadConfiguration();
		setPreviewImageCallback(nullptr);
		presetScanEnfaceAsDefault();

		getImpl().initiated = true;
	}
	catch (const std::exception&)
	{
		getImpl().initiated = false;
	}
	return getImpl().initiated;
}


bool OctSystem::Controller::isInitiated(void)
{
	return getImpl().initiated;
}


void OctSystem::Controller::initiateMeasureMode(const OctPatient & patient)
{
	Measure::setPatient(patient);

	readyMotorPositionsToScan(true);
	readyMotorPositionsToFundus(true);
	return;
}


void OctSystem::Controller::performPackagingMode(void)
{
	getMainBoard()->performPackagingMode();
	return;
}


bool OctSystem::Controller::moveScanFocusToDiopter(float diopt)
{
	float target = diopt;

	target -= getFundusFocusDiopterOrigin();
	if (target < FUNDUS_ZERO_LENS_DIOPT_MIN || target > FUNDUS_ZERO_LENS_DIOPT_MAX) {
		return false;
	}

	bool result = getMainBoard()->getOctFocusMotor()->updateDiopter(target);
	return result;
}

bool OctSystem::Controller::moveScanFocusToTopography(void)
{
	bool result = getMainBoard()->getOctFocusMotor()->updateTopographyDiopter();
	return result;
}

bool OctSystem::Controller::moveFundusFocusToDiopter(float diopt)
{
	float target = diopt;

	target -= getFundusFocusDiopterOrigin();
	if (target < FUNDUS_ZERO_LENS_DIOPT_MIN || target > FUNDUS_ZERO_LENS_DIOPT_MAX) {
		return false;
	}

	bool result = getMainBoard()->getFundusFocusMotor()->updateDiopter(target);
	return result;
}


bool OctSystem::Controller::movePolarizationToDegree(float degree)
{
	bool result = getMainBoard()->getPolarizationMotor()->updateDegree(degree);
	return result;
}


float OctSystem::Controller::getScanFocusDiopter(void)
{
	float diopt = getMainBoard()->getOctFocusMotor()->getCurrentDiopter();
	diopt += getFundusFocusDiopterOrigin();
	return diopt;
}


float OctSystem::Controller::getScanFocusDiopterMax(void)
{
	return getFundusFocusDiopterMax();
}


float OctSystem::Controller::getScanFocusDiopterMin(void)
{
	return getFundusFocusDiopterMin();
}


float OctSystem::Controller::getFundusFocusDiopter(void)
{
	float diopt = getMainBoard()->getFundusFocusMotor()->getCurrentDiopter();
	diopt += getFundusFocusDiopterOrigin();
	return diopt;
}


float OctSystem::Controller::getFundusFocusDiopterOrigin(void)
{
	switch (getImpl().compenLensMode) {
	case CompensationLensMode::Minus:
		return FUNDUS_MINUS_LENS_DIOPT;
	case CompensationLensMode::Plus:
		return FUNDUS_PLUS_LENS_DIOPT;
	default:
		return FUNDUS_ZERO_LENS_DIOPT;
	}
}

float OctSystem::Controller::getTopographyFocusOrigin(void)
{
	float diopt = (float)getMainBoard()->getFundusFocusMotor()->getPositionOfTopographyDiopter();

	return diopt;
}


float OctSystem::Controller::getFundusFocusDiopterMax(void)
{
	switch (getImpl().compenLensMode) {
	case CompensationLensMode::Minus:
		return FUNDUS_MINUS_LENS_DIOPT_MAX;
	case CompensationLensMode::Plus:
		return FUNDUS_PLUS_LENS_DIOPT_MAX;
	default:
		return FUNDUS_ZERO_LENS_DIOPT_MAX;
	}
}


float OctSystem::Controller::getFundusFocusDiopterMin(void)
{
	switch (getImpl().compenLensMode) {
	case CompensationLensMode::Minus:
		return FUNDUS_MINUS_LENS_DIOPT_MIN;
	case CompensationLensMode::Plus:
		return FUNDUS_PLUS_LENS_DIOPT_MIN;
	default:
		return FUNDUS_ZERO_LENS_DIOPT_MIN;
	}
}


float OctSystem::Controller::getPolarizationDegree(void)
{
	float degree = getMainBoard()->getPolarizationMotor()->getCurrentDegree();
	return degree;
}


int OctSystem::Controller::getPolarizationPosition(void)
{
	return getMainBoard()->getPolarizationMotor()->getPosition();
}


int OctSystem::Controller::getPolarizationMin(void)
{
	return getMainBoard()->getPolarizationMotor()->getRangeMin();
}


int OctSystem::Controller::getPolarizationMax(void)
{
	return getMainBoard()->getPolarizationMotor()->getRangeMax();
}


int OctSystem::Controller::getReferencePosition(void)
{
	return getMainBoard()->getReferenceMotor()->getPosition();
}


int OctSystem::Controller::getReferencePositionMin(void)
{
	return getMainBoard()->getReferenceMotor()->getRangeMin();
}


int OctSystem::Controller::getReferencePositionMax(void)
{
	return getMainBoard()->getReferenceMotor()->getRangeMax();
}


bool OctSystem::Controller::moveScanPositionByOffset(int offset)
{
	bool result = getMainBoard()->getReferenceMotor()->updatePositionByOffset(offset);
	return result;
}


bool OctSystem::Controller::moveReferenceToOrigin(bool cornea, bool isTopo)
{
	/* For animal */
	bool isAnimalEye = false;
	bool result;

	if (isAnimalEye) {
		if (cornea) {
			auto pos = (getMainBoard()->getReferenceMotor()->getPositionOfRetinaOrigin());
			result = getMainBoard()->getReferenceMotor()->updatePosition(pos);
		}
		else {
			result = getMainBoard()->getReferenceMotor()->updatePositionToLowerEnd();
		}
		return result;
	}
	else {
		if (cornea) {
			if (isTopo) {
				result = getMainBoard()->getReferenceMotor()->updatePositionToCorneaOrigin();
			}
			else {
				auto pos = (getMainBoard()->getReferenceMotor()->getPositionOfRetinaOrigin()) - 6200;
				result = getMainBoard()->getReferenceMotor()->updatePosition(pos);
				//Wide Anterior Lens의 working distance는 17.4mm이나
				//인증문서에 15mm로 기재됨 이를 맞추기 위해 Topo 촬영을 제외하고
				//나머지 Anterior 촬영은 상수값으로 reference 위치를 보정함.
			}
		}
		else {
			result = getMainBoard()->getReferenceMotor()->updatePositionToRetinaOrigin();
		}
	}
	return result;
}



bool OctSystem::Controller::moveReferenceToPosition(int pos)
{
	return getMainBoard()->getReferenceMotor()->updatePosition(pos);
}


bool OctSystem::Controller::movePolarizationToPosition(int pos)
{
	return getMainBoard()->getPolarizationMotor()->updatePosition(pos);
}


bool OctSystem::Controller::turnOnInternalFixation(int row, int col)
{
	bool result;
	
	if (GlobalSettings::useLcdFixation()) {
		result = getMainBoard()->setLcdFixationOn(true, row, col);

		if (0) { // result) {
			result = getMainBoard()->updateLcdParameters(GlobalSettings::fixationBrightness(),
						GlobalSettings::useLcdBlinkOn(), GlobalSettings::fixationBlinkPeriod(),
						GlobalSettings::fixationBlinkOnTime());
		}
	}
	else {
		result = getMainBoard()->setInternalFixationOn(true, row, col);
	}
	return result;
}


bool OctSystem::Controller::turnOnInternalFixation(EyeSide side, FixationTarget target, bool isMacularWide)
{
	std::pair<int, int> center;
	bool result = false;

	int row, col;

	if (!getInternalFixationAtTarget(side, target, row, col)) {
		return false;
	}

	center.first = row;
	center.second = col;

	center.first += SystemConfig::intFixationOffsetX();
	center.second += SystemConfig::intFixationOffsetY();

	if (isMacularWide) {
		if (side == EyeSide::OD) {
			center.first += 3;
		}
		else {
			center.first -= 3;
		}
	}

	result = turnOnInternalFixation(center.first, center.second);
	LogT() << "LcdFixation on: " << center.first << ", " << center.second << " => " << result;

	return result;
}


bool OctSystem::Controller::turnOnInternalFixationAtCenter(EyeSide side, EyeRegion region, bool isFundus)
{
	bool result;

	if (GlobalSettings::useLcdFixation())
	{
		std::pair<int, int> center;

		if (side == EyeSide::OD) {
			center = GlobalSettings::fixationCenterOD();
			if (isFundus || region == EyeRegion::Fundus) {
				center.first += GlobalSettings::fixationFundusOD().first;
				center.second += GlobalSettings::fixationFundusOD().second;
			}
			else if (region == EyeRegion::OpticDisc) {
				center.first += GlobalSettings::fixationScanDiskOD().first;
				center.second += GlobalSettings::fixationScanDiskOD().second;
			}
		}
		else {
			center = GlobalSettings::fixationCenterOS();
			if (isFundus || region == EyeRegion::Fundus) {
				center.first += GlobalSettings::fixationFundusOS().first;
				center.second += GlobalSettings::fixationFundusOS().second;
			}
			else if (region == EyeRegion::OpticDisc) {
				center.first += GlobalSettings::fixationScanDiskOS().first;
				center.second += GlobalSettings::fixationScanDiskOS().second;
			}
		}

		center.first += SystemConfig::intFixationOffsetX();
		center.second += SystemConfig::intFixationOffsetY();

		result = turnOnInternalFixation(center.first, center.second);
		
		LogT() << "LcdFixation on: " << center.first << ", " << center.second << " => " << result;
	}
	else {
		if (side == EyeSide::OD) {
			if (isFundus) {
				result = turnOnInternalFixation(FIXATION_ROW_OD_FUNDUS, FIXATION_COL_OD_FUNDUS);
			}
			else if (region == EyeRegion::OpticDisc) {
				result = turnOnInternalFixation(FIXATION_ROW_OD_DISC, FIXATION_COL_OD_DISC);
			}
			else {
				result = turnOnInternalFixation(FIXATION_CENTER_ROW, FIXATION_CENTER_COL);
			}
		}
		else {
			if (isFundus) {
				result = turnOnInternalFixation(FIXATION_ROW_OS_FUNDUS, FIXATION_COL_OS_FUNDUS);
			}
			else if (region == EyeRegion::OpticDisc) {
				result = turnOnInternalFixation(FIXATION_ROW_OS_DISC, FIXATION_COL_OS_DISC);
			}
			else {
				result = turnOnInternalFixation(FIXATION_CENTER_ROW, FIXATION_CENTER_COL);
			}
		}
	}
	return result;
}


bool OctSystem::Controller::turnOffInternalFixation(void)
{
	bool result;
	if (GlobalSettings::useLcdFixation()) {
		result = getMainBoard()->setLcdFixationOn(false);
	}
	else {
		result = getMainBoard()->setInternalFixationOn(false);
	}
	return result;
}


bool OctSystem::Controller::getInternalFixationOn(int & row, int & col)
{
	bool result;
	if (GlobalSettings::useLcdFixation()) {
		result = getMainBoard()->getLcdFixationOn(row, col);
	}
	else {
		result = getMainBoard()->getInternalFixationOn(row, col);
	}
	return result;
}


bool OctSystem::Controller::getInternalFixationAtTarget(EyeSide side, FixationTarget target, int & row, int & col)
{
	std::pair<int, int> center;

	row = col = 0;

	if (side == EyeSide::OD) {
		switch (target) {
		case FixationTarget::Center:
			center = GlobalSettings::fixationCenterOD();
			break;
		case FixationTarget::Fundus:
			center = GlobalSettings::fixationFundusOD();
			break;
		case FixationTarget::OpticDisc:
			center = GlobalSettings::fixationScanDiskOD();
			break;
		case FixationTarget::LeftSide:
			center = GlobalSettings::fixationLeftSideOD(0);
			break;
		case FixationTarget::LeftUp:
			center = GlobalSettings::fixationLeftSideOD(1);
			break;
		case FixationTarget::LeftDown:
			center = GlobalSettings::fixationLeftSideOD(2);
			break;
		case FixationTarget::RightSide:
			center = GlobalSettings::fixationRightSideOD(0);
			break;
		case FixationTarget::RightUp:
			center = GlobalSettings::fixationRightSideOD(1);
			break;
		case FixationTarget::RightDown:
			center = GlobalSettings::fixationRightSideOD(2);
			break;
		default:
			return false;
		}

		if (target > FixationTarget::OpticDisc) {
			center.first += (GlobalSettings::fixationFundusOD().first + GlobalSettings::fixationCenterOD().first);
			center.second += (GlobalSettings::fixationFundusOD().second + GlobalSettings::fixationCenterOD().second);
		}
		else if (target > FixationTarget::Center) {
			center.first += GlobalSettings::fixationCenterOD().first;
			center.second += GlobalSettings::fixationCenterOD().second;
		}
	}
	else {
		switch (target) {
		case FixationTarget::Center:
			center = GlobalSettings::fixationCenterOS();
			break;
		case FixationTarget::Fundus:
			center = GlobalSettings::fixationFundusOS();
			break;
		case FixationTarget::OpticDisc:
			center = GlobalSettings::fixationScanDiskOS();
			break;
		case FixationTarget::LeftSide:
			center = GlobalSettings::fixationLeftSideOS(0);
			break;
		case FixationTarget::LeftUp:
			center = GlobalSettings::fixationLeftSideOS(1);
			break;
		case FixationTarget::LeftDown:
			center = GlobalSettings::fixationLeftSideOS(2);
			break;
		case FixationTarget::RightSide:
			center = GlobalSettings::fixationRightSideOS(0);
			break;
		case FixationTarget::RightUp:
			center = GlobalSettings::fixationRightSideOS(1);
			break;
		case FixationTarget::RightDown:
			center = GlobalSettings::fixationRightSideOS(2);
			break;
		default:
			return false;
		}

		if (target > FixationTarget::OpticDisc) {
			center.first += (GlobalSettings::fixationFundusOS().first + GlobalSettings::fixationCenterOS().first);
			center.second += (GlobalSettings::fixationFundusOS().second + GlobalSettings::fixationCenterOS().second);
		}
		else if (target > FixationTarget::Center) {
			center.first += GlobalSettings::fixationCenterOS().first;
			center.second += GlobalSettings::fixationCenterOS().second;
		}
	}

	row = center.first;
	col = center.second;
	return true;
}


bool OctSystem::Controller::changeInternalFixationType(int type)
{
	bool result = getMainBoard()->updateLcdParameters(
		GlobalSettings::fixationBrightness(),
		GlobalSettings::useLcdBlinkOn(),
		GlobalSettings::fixationBlinkPeriod(),
		GlobalSettings::fixationBlinkOnTime(),
		(std::uint8_t)type);
	return result;
}


bool OctSystem::Controller::setFlashLightLevel(int level, bool autoFlash)
{

	bool result;
	
	if (!autoFlash) {
		result = getMainBoard()->getFundusFlashLed()->setIntensity(level);
		LogD() << "Flash level set to " << level;
	}
	else {
		int levelMin = GlobalSettings::autoFlashLevelMin();
		int levelMax = GlobalSettings::autoFlashLevelMax();
		int levelStd = GlobalSettings::autoFlashLevelNormal();

		/*
		float factor = GlobalSettings::autoFlashSizeFactor();
		float extent = 1.0f;
		int level;

		GlobalRecord::readFlashPupilExtent(extent);
		level = (int)((1.0f + (1.0f - extent) * factor) * levelStd);
		level = min(max(level, levelMin), levelMax);
		setFlashLightLevel(level);

		LogD() << "Auto flash light, pupil extent: " << extent << ", level: " << levelStd << " => " << level;
		*/


		int preset = GlobalSettings::getRetinaCameraPreset();
		int focusInt = GlobalSettings::flashPresetFocusInt(preset);
		int sizeHigh = GlobalSettings::flashPresetSizeHigh(preset);
		int sizeBase = GlobalSettings::flashPresetSizeBase(preset);
		int defLevel = GlobalSettings::flashPresetDefLevel(preset);
		int minLevel = min(max(defLevel - levelMin, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
		int maxLevel = min(max(defLevel + levelMax, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);

		float splitSize, splitPeak;
		GlobalRecord::readSplitFocusSize(splitSize, splitPeak);

		float ratio;
		int level = levelStd;
		if (splitPeak >= focusInt) {
			if (splitSize > sizeBase) {
				ratio = (float)(defLevel - minLevel) / (float)(sizeHigh - sizeBase);
				level = (int)(defLevel - ratio * (splitSize - sizeBase));
			}
		}
		else {
			ratio = (float)(maxLevel - defLevel) / (float)(focusInt - 55);
			level = (int)(defLevel + ratio * (focusInt - splitPeak));
		}

		level = min(max(level, minLevel), maxLevel);
		result = setFlashLightLevel(level, false);

		LogD() << "Auto flash preset: " << preset << ", defLevel: " << defLevel << ", maxLevel: " << maxLevel << ", minLevel: " << minLevel;
		LogD() << "Auto flash params: focusInt: " << focusInt << ", sizeBase: " << sizeBase << ", sizeHigh: " << sizeHigh;
		LogD() << "Split focus, size: " << splitSize << ", peak: " << splitPeak << " => flash level: " << level;
	}
	return result;
}


int OctSystem::Controller::getFlashLightLevel(void)
{
	int level = getMainBoard()->getFundusFlashLed()->getIntensity();
	return level;
}


bool OctSystem::Controller::turnOnBacklight(bool flag)
{
	return MainPlatform::setBacklightEnable(flag);
}


bool OctSystem::Controller::isAtSideOd(void)
{
	if (GlobalSettings::useNewGpioRutine()) {
		getMainPlatform()->updateCurrentEyeSide();
		return getMainPlatform()->isAtSideOd();
	}
	else {
		return getFrameGrabber()->isAtSideOd();
	}
}


EyeSide OctSystem::Controller::getEyeSide(void)
{
	if (GlobalSettings::useNewGpioRutine()) {
		return (getMainPlatform()->isAtSideOd() ? EyeSide::OD : EyeSide::OS);
	}
	else {
		if (GlobalSettings::useUsbCmosCameraEnable()) {
			return (getUsb3Grabber()->isAtSideOd() ? EyeSide::OD : EyeSide::OS);
		}
		else {
			return (getFrameGrabber()->isAtSideOd() ? EyeSide::OD : EyeSide::OS);
		}
	}
}

bool OctSystem::Controller::isStageAtLeftEnd(void)
{
	getMainBoard()->getStageXMotor()->updateStatus();
	return getMainBoard()->getStageXMotor()->isEndOfUpperPosition();
}


bool OctSystem::Controller::isStageAtRightEnd(void)
{
	getMainBoard()->getStageXMotor()->updateStatus();
	return getMainBoard()->getStageXMotor()->isEndOfLowerPosition();
}


bool OctSystem::Controller::isStageAtUpperEnd(void)
{
	getMainBoard()->getStageYMotor()->updateStatus();
	return getMainBoard()->getStageYMotor()->isEndOfUpperPosition();
}


bool OctSystem::Controller::isStageAtLowerEnd(void)
{
	getMainBoard()->getStageYMotor()->updateStatus();
	return getMainBoard()->getStageYMotor()->isEndOfLowerPosition();
}


bool OctSystem::Controller::isStageAtFrontEnd(void)
{
	getMainBoard()->getStageZMotor()->updateStatus();
	return getMainBoard()->getStageZMotor()->isEndOfUpperPosition();
}


bool OctSystem::Controller::isStageAtRearEnd(void)
{
	getMainBoard()->getStageZMotor()->updateStatus();
	return getMainBoard()->getStageZMotor()->isEndOfLowerPosition();
}


bool OctSystem::Controller::changeCompensationLens(CompensationLensMode mode)
{
	OctDiopterMotor* motor = getMainBoard()->getOctDiopterMotor();
	bool result;

	if (mode == CompensationLensMode::Minus) {
		result = motor->updatePositionToMinusLens();
	}
	else if (mode == CompensationLensMode::Plus) {
		result = motor->updatePositionToPlusLens();
	}
	else {
		result = motor->updatePositionToZeroLens();
	}

	FundusDiopterMotor* fundus = getMainBoard()->getFundusDiopterMotor();
	if (mode == CompensationLensMode::Minus) {
		result = fundus->updatePositionToMinusLens();
	}
	else if (mode == CompensationLensMode::Plus) {
		result = fundus->updatePositionToPlusLens();
	}
	else {
		result = fundus->updatePositionToZeroLens();
	}

	getImpl().compenLensMode = mode;
	return result;
}


bool OctSystem::Controller::changeOctDiopterLens(CompensationLensMode mode)
{
	OctDiopterMotor* motor = getMainBoard()->getOctDiopterMotor();
	bool result = true;

	if (mode == CompensationLensMode::Minus) {
		if (!motor->isMinusLensMode()) {
			result = motor->updatePositionToMinusLens();
		}
	}
	else if (mode == CompensationLensMode::Plus) {
		if (!motor->isPlusLensMode()) {
			result = motor->updatePositionToPlusLens();
		}
	}
	else {
		if (!motor->isZeroLensMode()) {
			result = motor->updatePositionToZeroLens();
		}
	}

	return result;
}


bool OctSystem::Controller::changeSmallPupilMode(bool flag)
{
	if (!GlobalSettings::useFundusEnable()) {
		if (!GlobalSettings::useFundusFILR_Enable()){
			return false;
		}
	}
	bool result = getMainBoard()->setSmallPupilMask(flag);
	return result;
}


bool OctSystem::Controller::isSmallPupilMode(void)
{
	return getImpl().isSmallPupilMode;
}


bool OctSystem::Controller::getCorneaPupilSize(float & pupSize, bool & isSmall)
{
	int width, pixels;
	float sizeMM, smallMM;

	GlobalRecord::readFlashPupilWidth(width);
	pixels = GlobalSettings::corneaPixelsPerMM();
	smallMM = GlobalSettings::corneaSmallPupilSize();

	if (width > 0 && pixels > 0 && smallMM > 0.0f) {
		sizeMM = (float)width / (float)pixels;
		pupSize = sizeMM;
		isSmall = sizeMM < smallMM;

		// changeSmallPupilMode(result);
		// LogD() << "Auto small pupil: width: " << width << ", pixsMM: " << pixels << ", small: " << smallMM << " => " << pupSize << ", mode: " << isSmall;
		return true;
	}
	else {
		pupSize = 0.0f;
		isSmall = false;
		// LogD() << "Auto small pupil: width: " << width << ", pixsMM: " << pixels << ", small: " << smallMM;
		return false;
	}
}


bool OctSystem::Controller::turnOnSplitFocus(bool flag)
{
	bool result = getMainBoard()->getSplitFocusLed()->control(flag);
	return result;
}


bool OctSystem::Controller::turnOnWorkingDots(bool flag)
{
	bool result = getMainBoard()->getWorkingDotsLed()->control(flag);
	result = getMainBoard()->getWorkingDot2Led()->control(flag);
	return result;
}


bool OctSystem::Controller::turnOnRetinaIrLed(bool flag, bool bright)
{
	bool result = false;
	result = getMainBoard()->getRetinaIrLed()->control(flag);

	if (result && flag) {
		if (!bright) {
			auto value = getMainBoard()->getRetinaIrLed()->getValueSet1();
			result = getMainBoard()->getRetinaIrLed()->setIntensity(value);
		}
		else {
			auto value = getMainBoard()->getRetinaIrLed()->getValueSet2();
			result = getMainBoard()->getRetinaIrLed()->setIntensity(value);
		}
	}
 	return result;
}


bool OctSystem::Controller::setRetinaIrLedIntensity(int value)
{
	return getMainBoard()->getRetinaIrLed()->setIntensity(value);
}


int OctSystem::Controller::getRetinaIrLedIntensity(void)
{
	return getMainBoard()->getRetinaIrLed()->getIntensity();
}

