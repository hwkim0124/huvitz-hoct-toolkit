#include "stdafx.h"
#include "SystemSetup.h"
#include "SystemProfile.h"
#include "DeviceSettings.h"
#include "CameraSettings.h"
#include "FundusSettings.h"
#include "RetinaSettings.h"
#include "CorneaSettings.h"
#include "FixationSettings.h"
#include "PatternSettings.h"
#include "SystemSettings.h"

#include "CppUtil2.h"

using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct SystemSetup::SystemSetupImpl
{
	SysCal2_st sysCal2;

	SystemSetupImpl()
	{
		memset(&sysCal2, 0, sizeof(SysCal2_st));
	}
};



// Direct initialization of static smart pointer.
std::unique_ptr<SystemSetup::SystemSetupImpl> SystemSetup::d_ptr(new SystemSetupImpl());


SystemSetup::SystemSetup()
{
}


SystemSetup::~SystemSetup()
{
}


SysCal2_st * OctConfig::SystemSetup::getSysCalc2(bool checksum)
{
	if (checksum) {
		updateCheckSum();
	}
	return &(getImpl().sysCal2);
}


bool OctConfig::SystemSetup::loadDataFromProfile(bool update)
{
	if (update) {
		if (!SystemProfile::loadConfigFile()) {
			SystemProfile::resetToDefaultValues();
		}
	}

	DeviceSettings* dset = SystemProfile::getDeviceSettings();
	auto lights = &(getSysCalc2()->lightLeds);
	lights->retinaIR = dset->getRetinaIrLed()->getValue();
	lights->retinaIR_bright = dset->getRetinaIrLedBright()->getValue();
	lights->corneaIR = dset->getCorneaIrLed()->getValue();
	lights->keratoRing = dset->getKeratoRingLed()->getValue();
	lights->keratoFocus = dset->getKeratoFocusLed()->getValue();
	lights->splitFocus = dset->getSplitFocusLed()->getValue();
	lights->externLed = dset->getExtFixationLed()->getValue();
	lights->workingDot1 = dset->getWorkingDotsLed()->getValue();
	lights->workingDot2 = dset->getWorkingDot2Led()->getValue();

	CameraSettings* cset = SystemProfile::getCameraSettings();
	auto cams = &(getSysCalc2()->cameras);
	cams->retinaIR.dgain = cset->getRetinaCamera()->digitalGain();
	cams->retinaIR.again = cset->getRetinaCamera()->analogGain();
	cams->retinaIR.dgain2 = cset->getRetinaCamera()->digitalGain(1);
	cams->retinaIR.again2 = cset->getRetinaCamera()->analogGain(1);
	cams->retinaIR.dgain3 = cset->getRetinaCamera()->digitalGain(2);
	cams->retinaIR.again3 = cset->getRetinaCamera()->analogGain(2);

	cams->corneaIR.dgain = cset->getCorneaCamera()->digitalGain();
	cams->corneaIR.again = cset->getCorneaCamera()->analogGain();
	cams->fundusColor.dgain = cset->getFundusCamera()->digitalGain();
	cams->fundusColor.again = cset->getFundusCamera()->analogGain();

	FundusSettings* fset = SystemProfile::getFundusSettings();
	auto fcolor = &(getSysCalc2()->fundusColor);
	auto wbs = fset->getWhiteBalanceParameters();
	for (int i = 0; i < wbs.size(); i++) {
		fcolor->whiteBalance.params[i] = (float)wbs[i];
	}
	fcolor->whiteBalance.status = fset->useWhiteBalance();
	auto coord_xs = fset->getCorrectionCoordsX();
	auto coord_ys = fset->getCorrectionCoordsY();
	for (int i = 0; i < coord_xs.size(); i++) {
		fcolor->levelCorr.coordXs[i] = coord_xs[i];
		fcolor->levelCorr.coordYs[i] = coord_ys[i];
	}
	auto coord_xs_FILR = fset->getCorrectionCoordsX_FILR();
	auto coord_ys_FILR = fset->getCorrectionCoordsY_FILR();
	for (int i = 0; i < coord_xs_FILR.size(); i++) {
		fcolor->levelCorr.coordXs_FILR[i] = coord_xs_FILR[i];
		fcolor->levelCorr.coordYs_FILR[i] = coord_ys_FILR[i];
	}

	fcolor->levelCorr.status = fset->useColorCorrection();
	fcolor->radialCorr.ratio = fset->radialCorrectionRatio();
	fcolor->radialCorr.status = fset->useRadialCorrection();

	fcolor->autoEnhance.bright = fset->autoBright();
	fcolor->autoEnhance.contrast = fset->autoContrast();
	fcolor->autoEnhance.ub = fset->autoUB();
	fcolor->autoEnhance.vr = fset->autoVR();
	fcolor->edgeEnhance.kernelSize = fset->edgeKernelSize();
	fcolor->colorRoi.center[0] = fset->getCenterPointOfROI().first;
	fcolor->colorRoi.center[1] = fset->getCenterPointOfROI().second;
	fcolor->colorRoi.radius = fset->getRadiusOfROI();
	fcolor->colorRoi.status = fset->useROICoordinates();

	fcolor->removeReflectionlight.light = fset->getRemoveReflection1();
	fcolor->removeReflectionlight.shadow = fset->getRemoveReflection2();
	fcolor->removeReflectionlight.outer = fset->getRemoveReflection3();
	fcolor->removeReflectionlight.status = fset->useRemoveReflectionLight();

	auto fcapture = &(getSysCalc2()->fundusCapture);
	fcapture->autoFlash.levelNorm = fset->getAutoFlashLevelNormal();
	fcapture->autoFlash.levelMax = fset->getAutoFlashLevelMax();
	fcapture->autoFlash.levelMin = fset->getAutoFlashLevelMin();

	for (int k = 0; k < AUTO_FLASH_LEVEL_PRESET_SIZE; k++) {
		fcapture->autoFlash.focusInt[k] = fset->getAutoFlashFocusInt(k);
		fcapture->autoFlash.sizeBase[k] = fset->getAutoFlashSizeBase(k);
		fcapture->autoFlash.sizeHigh[k] = fset->getAutoFlashSizeHigh(k);
		fcapture->autoFlash.defLevel[k] = fset->getAutoFlashDefLevel(k);
		fcapture->autoFlash.splitFocus[k] = fset->getAutoFlashSplitFocus(k);
		fcapture->autoFlash.retinaIr[k] = fset->getAutoFlashRetinaIr(k);
		fcapture->autoFlash.workDot1[k] = fset->getAutoFlashWorkDot1(k);
		fcapture->autoFlash.workDot2[k] = fset->getAutoFlashWorkDot2(k);
	}

	fcapture->autoFlash.status = fset->useAutoFlashLevel();

	//LogD() << "Auto flash status: " << fcapture->autoFlash.status << ", " << fset->useAutoFlashLevel();

	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	auto retina = &(getSysCalc2()->retinaImage);
	retina->imageEnhance.status = rset->useImageEnhance();
	retina->imageEnhance.clipLimit = rset->getEnhanceClipLimit();

	retina->splitFocus.center[0] = rset->getSplitCenter().first;
	retina->splitFocus.center[1] = rset->getSplitCenter().second;
	retina->retinaRoi.center[0] = rset->getCenterPointOfROI().first;
	retina->retinaRoi.center[1] = rset->getCenterPointOfROI().second;
	retina->retinaRoi.radius = rset->getRadiusOfROI();
	retina->retinaRoi.status = rset->useROI();

	retina->centerMask.center[0] = rset->getCenterPointOfMask().first;
	retina->centerMask.center[1] = rset->getCenterPointOfMask().second;
	retina->centerMask.radius = rset->getCenterMaskSize();
	retina->centerMask.status = rset->useCenterMask();

	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	auto cornea = &(getSysCalc2()->corneaImage);
	cornea->pixelsPerMM = aset->getPixelsPerMM();
	cornea->smallPupilSize = aset->getSmallPupilSize();

	FixationSettings* xset = SystemProfile::getFixationSettings();
	auto intfix = &(getSysCalc2()->internalFix);
	intfix->od.center[0] = xset->getCenterOD().first;
	intfix->od.center[1] = xset->getCenterOD().second;
	intfix->od.fundus[0] = xset->getFundusOD().first;
	intfix->od.fundus[1] = xset->getFundusOD().second;
	intfix->od.scanDisk[0] = xset->getScanDiskOD().first;
	intfix->od.scanDisk[1] = xset->getScanDiskOD().second;

	for (int i = 0; i < 3; i++) {
		intfix->od.leftSide[i][0] = xset->getLeftSideOD(i).first;
		intfix->od.leftSide[i][1] = xset->getLeftSideOD(i).second;
		intfix->od.rightSide[i][0] = xset->getRightSideOD(i).first;
		intfix->od.rightSide[i][1] = xset->getRightSideOD(i).second;
	}

	intfix->os.center[0] = xset->getCenterOS().first;
	intfix->os.center[1] = xset->getCenterOS().second;
	intfix->os.fundus[0] = xset->getFundusOS().first;
	intfix->os.fundus[1] = xset->getFundusOS().second;
	intfix->os.scanDisk[0] = xset->getScanDiskOS().first;
	intfix->os.scanDisk[1] = xset->getScanDiskOS().second;

	for (int i = 0; i < 3; i++) {
		intfix->os.leftSide[i][0] = xset->getLeftSideOS(i).first;
		intfix->os.leftSide[i][1] = xset->getLeftSideOS(i).second;
		intfix->os.rightSide[i][0] = xset->getRightSideOS(i).first;
		intfix->os.rightSide[i][1] = xset->getRightSideOS(i).second;
	}

	intfix->bright = xset->getBrightness();
	intfix->blink = xset->useLcdBlinkOn();
	intfix->onTime = xset->getBlinkOnTime();
	intfix->period = xset->getBlinkPeriod();
	intfix->status = xset->useLcdFixation();
	intfix->type = xset->getFixationType();

	PatternSettings* tset = SystemProfile::getPatternSettings();
	auto pattern = &(getSysCalc2()->patterns);
	for (int i = 0; i < 3; i++) {
		pattern->retina[i].offsetX = tset->retinaPatternOffset(i).first;
		pattern->retina[i].offsetY = tset->retinaPatternOffset(i).second;
		pattern->retina[i].scaleX = tset->retinaPatternScale(i).first;
		pattern->retina[i].scaleY = tset->retinaPatternScale(i).second;
		pattern->cornea[i].offsetX = tset->corneaPatternOffset(i).first;
		pattern->cornea[i].offsetY = tset->corneaPatternOffset(i).second;
		pattern->cornea[i].scaleX = tset->corneaPatternScale(i).first;
		pattern->cornea[i].scaleY = tset->corneaPatternScale(i).second;
		pattern->topography[i].offsetX = tset->topographyPatternOffset(i).first;
		pattern->topography[i].offsetY = tset->topographyPatternOffset(i).second;
		pattern->topography[i].scaleX = tset->topographyPatternScale(i).first;
		pattern->topography[i].scaleY = tset->topographyPatternScale(i).second;
	}

	auto refer = &(getSysCalc2()->referRange);
	refer->lowerRangeSize = tset->getReferenceRangeLowerSize();
	refer->upperRangeSize = tset->getReferenceRangeUpperSize();

	auto system = &(getSysCalc2()->systemSet);
	memset(system->sysInfo.serialNo, 0, SystemSettings::SERIAL_NUMBER_SIZE_MAX);

	SystemSettings* sset = SystemProfile::getSystemSettings();
	std::string sstr = sset->serialNumber();
	for (int i = 0; i < sstr.size(); i++) {
		system->sysInfo.serialNo[i] = sstr[i];
	}
//	system->sysInfo.octSoloVersion = sset->useOctSoloVersion();
	system->sysInfo.modelType = sset->getModelType();
	system->sysInfo.octGrabberType = sset->getOctGrabberType();

	system->sysOptions.octLineCameraMode = sset->getOctLineCameraMode();
	system->sysOptions.sldFaultDetection = sset->getSldFaultDetection();
	system->sysOptions.sldFaultTimer = sset->getSldFaultTimer();
	system->sysOptions.sldFaultThreshold = sset->getSldFaultThreshold();
	system->sysOptions.retinaTrackingSpeed = sset->getRetinaTrackingSpeed();

	for (int i = 0; i < 3; i++) {
		system->sysOptions.triggerForePadd[i] = sset->getTriggerForePadd(i);
		system->sysOptions.triggerPostPadd[i] = sset->getTriggerPostPadd(i);
	}
	return true;
}


bool OctConfig::SystemSetup::saveDataToProfile(void)
{
	DeviceSettings* dset = SystemProfile::getDeviceSettings();
	auto lights = &(getSysCalc2()->lightLeds);
	dset->getRetinaIrLed()->setValue(lights->retinaIR);
	dset->getRetinaIrLedBright()->setValue(lights->retinaIR_bright);
	dset->getCorneaIrLed()->setValue(lights->corneaIR);
	dset->getKeratoRingLed()->setValue(lights->keratoRing);
	dset->getKeratoFocusLed()->setValue(lights->keratoFocus);
	dset->getSplitFocusLed()->setValue(lights->splitFocus);
	dset->getExtFixationLed()->setValue(lights->externLed);
	dset->getWorkingDotsLed()->setValue(lights->workingDot1);
	dset->getWorkingDot2Led()->setValue(lights->workingDot2);

	// LogD() << "retina ir: " << lights->retinaIR << " => " << dset->getRetinaIrLed()->getValue();

	CameraSettings* cset = SystemProfile::getCameraSettings();
	auto cams = &(getSysCalc2()->cameras);
	cset->getRetinaCamera()->setDigitalGain(cams->retinaIR.dgain);
	cset->getRetinaCamera()->setAnalogGain(cams->retinaIR.again);
	cset->getRetinaCamera()->setDigitalGain(cams->retinaIR.dgain2, 1);
	cset->getRetinaCamera()->setAnalogGain(cams->retinaIR.again2, 1) ;
	cset->getRetinaCamera()->setDigitalGain(cams->retinaIR.dgain3, 2) ;
	cset->getRetinaCamera()->setAnalogGain(cams->retinaIR.again3, 2);

	cset->getCorneaCamera()->setDigitalGain(cams->corneaIR.dgain) ;
	cset->getCorneaCamera()->setAnalogGain(cams->corneaIR.again) ;
	cset->getFundusCamera()->setDigitalGain(cams->fundusColor.dgain) ;
	cset->getFundusCamera()->setAnalogGain(cams->fundusColor.again) ;

	// LogD() << "retina gain: " << cams->retinaIR.dgain << " => " << cset->getRetinaCamera()->digitalGain();

	FundusSettings* fset = SystemProfile::getFundusSettings();
	auto fcolor = &(getSysCalc2()->fundusColor);
	auto wbs = std::vector<double>(3);
	for (int i = 0; i < wbs.size(); i++) {
		wbs[i] = fcolor->whiteBalance.params[i];
	}
	fset->setWhiteBalanceParameters(wbs);
	fset->useWhiteBalance(true, fcolor->whiteBalance.status > 0);
	auto coord_xs = std::vector<int>(4);
	auto coord_ys = std::vector<int>(4);
	for (int i = 0; i < coord_xs.size(); i++) {
		coord_xs[i] = fcolor->levelCorr.coordXs[i];
		coord_ys[i] = fcolor->levelCorr.coordYs[i];
	}
	fset->setCorrectionCoordsX(coord_xs);
	fset->setCorrectionCoordsY(coord_ys);
	auto coord_xs_FILR = std::vector<int>(7);
	auto coord_ys_FILR = std::vector<int>(7);
	for (int i = 0; i < coord_xs_FILR.size(); i++) {
		coord_xs_FILR[i] = fcolor->levelCorr.coordXs_FILR[i];
		coord_ys_FILR[i] = fcolor->levelCorr.coordYs_FILR[i];
	}
	fset->setCorrectionCoordsX_FILR(coord_xs_FILR);
	fset->setCorrectionCoordsY_FILR(coord_ys_FILR);
	fset->useColorCorrection(true, fcolor->levelCorr.status > 0);
	fset->setRadialCorrectionRatio(fcolor->radialCorr.ratio);
	fset->useRadialCorrection(true, fcolor->radialCorr.status > 0);

	fset->setAutoBright(fcolor->autoEnhance.bright);
	fset->setAutoContrast(fcolor->autoEnhance.contrast);
	fset->setAutoUB(fcolor->autoEnhance.ub);
	fset->setAutoVR(fcolor->autoEnhance.vr);
	fset->setEdgeKernelSize(fcolor->edgeEnhance.kernelSize);
	fset->setCenterPointOfROI(std::pair<int, int>(fcolor->colorRoi.center[0], fcolor->colorRoi.center[1]));
	fset->setRadiusOfROI(fcolor->colorRoi.radius);
	fset->useROICoordinates(true, fcolor->colorRoi.status > 0);

	auto fcapture = &(getSysCalc2()->fundusCapture);
	fset->setAutoFlashLevelNormal(fcapture->autoFlash.levelNorm);
	fset->setAutoFlashLevelMax(fcapture->autoFlash.levelMax);
	fset->setAutoFlashLevelMin(fcapture->autoFlash.levelMin);

	fset->setRemoveReflection1(fcolor->removeReflectionlight.light);
	fset->setRemoveReflection2(fcolor->removeReflectionlight.shadow);
	fset->setRemoveReflection3(fcolor->removeReflectionlight.outer);
	fset->useRemoveReflectionLight(true, (bool)fcolor->removeReflectionlight.status);

	bool defSplit = false;
	bool defWdot1 = false;
	bool defWdot2 = false;
	bool defRetIr = false;
	if (fcapture->autoFlash.splitFocus[0] != lights->splitFocus) {
		defSplit = true;
	}
	if (fcapture->autoFlash.workDot1[0] != lights->workingDot1) {
		defWdot1 = true;
	}
	if (fcapture->autoFlash.workDot2[0] != lights->workingDot2) {
		defWdot2 = true;
	}
	if (fcapture->autoFlash.retinaIr[0] != lights->retinaIR) {
		defRetIr = true;
	}

	for (int k = 0; k < AUTO_FLASH_LEVEL_PRESET_SIZE; k++) {
		fset->setAutoFlashFocusInt(fcapture->autoFlash.focusInt[k], k);
		fset->setAutoFlashSizeBase(fcapture->autoFlash.sizeBase[k], k);
		fset->setAutoFlashSizeHigh(fcapture->autoFlash.sizeHigh[k], k);
		fset->setAutoFlashDefLevel(fcapture->autoFlash.defLevel[k], k);

		auto split = fcapture->autoFlash.splitFocus[k];
		if (defSplit) {
			split = lights->splitFocus / (k == 1 ? 2 : 1);
		}
		fset->setAutoFlashSplitFocus(split, k);

		auto retIr = fcapture->autoFlash.retinaIr[k];
		if (defRetIr) {
			retIr = lights->retinaIR / (k == 1 ? 2 : 1);
		}
		fset->setAutoFlashRetinaIr(retIr, k);

		auto wdot1 = fcapture->autoFlash.workDot1[k];
		if (defWdot1) {
			wdot1 = lights->workingDot1 / (k == 1 ? 2 : 1);
		}
		fset->setAutoFlashWorkDot1(wdot1, k);

		auto wdot2 = fcapture->autoFlash.workDot2[k];
		if (defWdot2) {
			wdot1 = lights->workingDot2 / (k == 1 ? 2 : 1);
		}
		fset->setAutoFlashWorkDot2(wdot2, k);
	}
	fset->useAutoFlashLevel(true, fcapture->autoFlash.status > 0);

	//LogD() << "Auto flash status: " << fcapture->autoFlash.status << ", " << fset->useAutoFlashLevel();

	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	auto retina = &(getSysCalc2()->retinaImage);
	rset->useImageEnhance(true, retina->imageEnhance.status > 0);
	rset->setSplitCenter(std::pair<int, int>(retina->splitFocus.center[0], retina->splitFocus.center[1]));
	rset->setCenterPointOfROI(std::pair<int, int>(retina->retinaRoi.center[0], retina->retinaRoi.center[1]));
	rset->setRadiusOfROI(retina->retinaRoi.radius);
	rset->useROI(true, retina->retinaRoi.status > 0);

	rset->setCenterPointOfMask(std::pair<int, int>(retina->centerMask.center[0], retina->centerMask.center[1]));
	rset->setCenterMaskSize(retina->centerMask.radius);
	rset->useCenterMask(true, retina->centerMask.status > 0);
	rset->setEnhanceClipLimit(retina->imageEnhance.clipLimit);

	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	auto cornea = &(getSysCalc2()->corneaImage);
	aset->setPixelsPerMM(cornea->pixelsPerMM);
	aset->setSmallPupilSize(cornea->smallPupilSize);

	FixationSettings* xset = SystemProfile::getFixationSettings();
	auto intfix = &(getSysCalc2()->internalFix);
	xset->setCenterOD(std::pair<int, int>(intfix->od.center[0], intfix->od.center[1]));
	xset->setFundusOD(std::pair<int, int>(intfix->od.fundus[0], intfix->od.fundus[1]));
	xset->setScanDiskOD(std::pair<int, int>(intfix->od.scanDisk[0], intfix->od.scanDisk[1]));

	for (int i = 0; i < 3; i++) {
		xset->setLeftSideOD(i, std::pair<int, int>(intfix->od.leftSide[i][0],intfix->od.leftSide[i][1]));
		xset->setRightSideOD(i, std::pair<int, int>(intfix->od.rightSide[i][0], intfix->od.rightSide[i][1]));
	}

	xset->setCenterOS(std::pair<int, int>(intfix->os.center[0], intfix->os.center[1]));
	xset->setFundusOS(std::pair<int, int>(intfix->os.fundus[0], intfix->os.fundus[1]));
	xset->setScanDiskOS(std::pair<int, int>(intfix->os.scanDisk[0], intfix->os.scanDisk[1]));

	for (int i = 0; i < 3; i++) {
		xset->setLeftSideOS(i, std::pair<int, int>(intfix->os.leftSide[i][0], intfix->os.leftSide[i][1]));
		xset->setRightSideOS(i, std::pair<int, int>(intfix->os.rightSide[i][0], intfix->os.rightSide[i][1]));
	}

	xset->setBrightness(intfix->bright);
	xset->useLcdBlinkOn(true, intfix->blink > 0);
	xset->setBlinkOnTime(intfix->onTime);
	xset->setBlinkPeriod(intfix->period);
	xset->useLcdFixation(true, intfix->status > 0);
	xset->setFixationType(intfix->type);

	PatternSettings* tset = SystemProfile::getPatternSettings();
	auto pattern = &(getSysCalc2()->patterns);
	for (int i = 0; i < 3; i++) {
		tset->setRetinaPatternOffset(std::pair<float, float>(pattern->retina[i].offsetX, pattern->retina[i].offsetY), i);
		tset->setRetinaPatternScale(std::pair<float, float>(pattern->retina[i].scaleX, pattern->retina[i].scaleY), i);
		tset->setCorneaPatternOffset(std::pair<float, float>(pattern->cornea[i].offsetX, pattern->cornea[i].offsetY), i);
		tset->setCorneaPatternScale(std::pair<float, float>(pattern->cornea[i].scaleX, pattern->cornea[i].scaleY), i);
	
		if (!isnan(pattern->topography[i].offsetX) && !isnan(pattern->topography[i].offsetY))
			tset->setTopographyPatternOffset(std::pair<float, float>(pattern->topography[i].offsetX, pattern->topography[i].offsetY), i);
		if (!isnan(pattern->topography[i].scaleX) && !isnan(pattern->topography[i].scaleY))
			tset->setTopographyPatternScale(std::pair<float, float>(pattern->topography[i].scaleX, pattern->topography[i].scaleY), i);
	}
	
	auto refer = &(getSysCalc2()->referRange);
	tset->setReferenceRangeLowerSize(refer->lowerRangeSize);
	tset->setReferenceRangeUpperSize(refer->upperRangeSize);

	SystemSettings* sset = SystemProfile::getSystemSettings();
	auto system = &(getSysCalc2()->systemSet);
	std::string sstr;
	for (int i = 0; i < SystemSettings::SERIAL_NUMBER_SIZE_MAX; i++) {
		auto ch = system->sysInfo.serialNo[i];
		if (ch >= 32 && ch <= 126) {
			sstr += ch;
		}
	}
	sset->setSerialNumber(sstr);
	sset->setModelType((int)system->sysInfo.modelType);
	sset->setOctGrabberType((int)system->sysInfo.octGrabberType);

	sset->setOctLineCameraMode((int)system->sysOptions.octLineCameraMode);
	sset->setSldFaultDetection((int)system->sysOptions.sldFaultDetection);
	sset->setSldFaultTimer((int)system->sysOptions.sldFaultTimer);
	sset->setSldFaultThreshold((int)system->sysOptions.sldFaultThreshold);
	sset->setRetinaTrackingSpeed((int)system->sysOptions.retinaTrackingSpeed);

	for (int i = 0; i < 3; i++) {
		sset->setTriggerForePadd(i, (int)system->sysOptions.triggerForePadd[i]);
		sset->setTriggerPostPadd(i, (int)system->sysOptions.triggerPostPadd[i]);
	}
	return true;
}


bool OctConfig::SystemSetup::isValidChecksum(void)
{
	auto chksum = calculateCheckSum();
	if (chksum != 0 && chksum == getCheckSum()) {
		return true;
	}
	return false;
}


std::uint64_t OctConfig::SystemSetup::calculateCheckSum(void)
{
	size_t size = sizeof(d_ptr->sysCal2) - 8;		// except checksum bytes
	uint8_t* ptr = (uint8_t*)&(getImpl().sysCal2);

	uint64_t chksum = 0;
	for (int i = 0; i < size; i++) {
		chksum += ptr[i];
	}

	return chksum;
}


void OctConfig::SystemSetup::updateCheckSum(void)
{
	d_ptr->sysCal2.chksum = calculateCheckSum();
	return ;
}


std::uint64_t OctConfig::SystemSetup::getCheckSum(void)
{
	return getSysCalc2()->chksum;
}


SystemSetup::SystemSetupImpl & OctConfig::SystemSetup::getImpl(void)
{
	return *d_ptr;
}
