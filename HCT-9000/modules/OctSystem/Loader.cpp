#include "stdafx.h"
#include "Loader.h"
#include "Controller.h"
#include "Scanner.h"
#include "Camera.h"
#include "CorneaAlign.h"
#include "RetinaAlign.h"
#include "RetinaTracker.h"
#include "RetinaTracking.h"
#include "Optimizer.h"
#include "PatternHelper.h"
#include "Measure.h"
#include "Normative.h"
#include "NormativeWide.h"

#include <sstream>
#include <json\json.hpp>
#include <boost/format.hpp>
#include <fstream>

#include "OctDevice2.h"
#include "OctConfig2.h"
#include "OctGrab2.h"
#include "CppUtil2.h"
#include "SigChain2.h"
#include "ImgProc2.h"

using namespace OctSystem;
using namespace OctDevice;
using namespace OctConfig;
using namespace OctGrab;
using namespace CppUtil;
using namespace SigChain;
using namespace ImgProc;
using namespace std;


struct Loader::LoaderImpl
{
	bool initiated;
	bool useNoDevice;
	bool useNoLeds;
	bool useNoCameras;
	bool useNoMotors;
	bool useNoGrabber;
	bool useNoColor;

	bool useDcfFile;
	bool logSrcLine;
	bool logTimeTag;
	bool logFileKeep;
	int numWarnings;

	Logger::LogLevel logLevel;


	wstring dcfFilePath;

	LoaderImpl() : initiated(false), useNoDevice(false), useDcfFile(false),
					useNoCameras(false), useNoLeds(false), useNoMotors(false), useNoColor(false),
					useNoGrabber(false), logLevel(Logger::LogLevel::LEVEL_INFO), 
					logSrcLine(false), logTimeTag(false), numWarnings(0), logFileKeep(false)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Loader::LoaderImpl> Loader::d_ptr(new LoaderImpl());



Loader::Loader()
{
}


Loader::~Loader()
{
}


bool OctSystem::Loader::initializeSystem(LogMsgCallback* cbLogMsg)
{
	wstring wstr;
	bool result = true;
	int numWarns = 0;
	int totalWarns = 0;

	GlobalLogger::setLogMsgCallback(cbLogMsg);

	try
	{
		MainBoard* board = Controller::getMainBoard();

		initSystemBootMode();
		if (!MainPlatform::initMainPlatform(board)) {
			GlobalLogger::error("Main platform init failed!");
			return false;
		}
	
		if (!board->initMainBoard(&numWarns)) {
			GlobalLogger::error("Mainboard init failed!");
			return false;
		}
		else {
			GlobalLogger::info("Initializing mainboard ... ok!");
			totalWarns += numWarns;
			applyConfiguration();
		}

		if (!loadSystemSetup()) {
			GlobalLogger::warn("System profile load from mainboard failed!");
			totalWarns++;
			if (!loadSetupProfile(true, false)) {
				GlobalLogger::warn("System profile load from file failed!");
			}
		}
		else {
			applySystemProfile(false);
			GlobalLogger::info("Loading system profile data ... ok!");
		}

		if (!board->initDevices(&numWarns)) {
			GlobalLogger::error("Devices init failed!");
			return false;
		}
		else {
			applySystemProfile(true);
			GlobalLogger::info("Devices init ... ok!");
			totalWarns += numWarns;
		}
		if (GlobalSettings::useNewGpioRutine()) {
			auto status = MainPlatform::gpio_init();
			if (status == 0xff){
				GlobalLogger::error("Gpio init failed!");
			}
			else {
				GlobalLogger::info("Gpio init ... ok!");
			}
			MainPlatform::startGpioThread();
			GlobalLogger::info("New Gpio Rutine ... ok!");
		}

		if (GlobalSettings::useOctEnable()) {
			Scanner* scanner = Controller::getScanner();

			if (GlobalSettings::isOctUsbCameraGrabberType()) {
				GlobalSettings::useUsbCmosCameraEnable(true, true);
			} 

retry_grabber_init:
			if (GlobalSettings::useUsbCmosCameraEnable()) {
				Usb3Grabber* usb3Grab = Controller::getUsb3Grabber();
				if (!useNoGrabber()) {
					if (!usb3Grab->initializeUsb3Grabber()) {
						GlobalLogger::error("Usb3 grabber init failed!");
						totalWarns++;
						//result = false;
					}
					else {
						result = true;
						if (GlobalSettings::isOctUsbLineCamera80k()) {
							GlobalLogger::info("usb3 grabber init (80k) ... ok!");
						}
						else if (GlobalSettings::isOctUsbLineCamera120k()) {
							GlobalLogger::info("usb3 grabber init (120k) ... ok!");
						}
						else {
							GlobalLogger::info("usb3 grabber init ... ok!");
						}
					}
				}

				if (!scanner->initScanner(board, usb3Grab)) {
					GlobalLogger::error("Scanner process init failed!");
					totalWarns++;
					result = false;
				}
				else {
					GlobalLogger::info("Scanner process init ... ok!");
				}
			}
			else {
				FrameGrabber* grabber = Controller::getFrameGrabber();
				if (!useNoGrabber()) {
					wstring path = (useDcfFile() ? getDcfFilePath() : DCF_FILE_NAME_TO_SPEED_FASTEST);
					grabber->initFrameGrabber(board);
					if (!grabber->startFrameGrabber(path)) {
						GlobalLogger::error("Frame grabber init failed!");
						totalWarns++;
						result = false;
						GlobalSettings::useUsbCmosCameraEnable(true, true);
						goto retry_grabber_init;
					}
					else {
						GlobalLogger::info("Frame grabber init ... ok!");
					}
				}
				
				if (!scanner->initScanner(board, grabber)) {
					GlobalLogger::error("Scanner process init failed!");
					totalWarns++;
					result = false;
				}
				else {
					GlobalLogger::info("Scanner process init ... ok!");
				}
			}

			RetinaAlign* retAlign = Controller::getRetinaAlign();
			retAlign->initRetinaAlign(board);
			Optimizer::initOptimizer(board, scanner, retAlign);
		}

		if (GlobalSettings::useFundusEnable() || GlobalSettings::useFundusFILR_Enable()) {
			Camera* camera = Controller::getCamera();
			if (!camera->initCamera(board)) {
				GlobalLogger::error("Fundus camera init failed!");
				totalWarns++;
				result = false;
			}
			else {
				GlobalLogger::info("Fundus camera init ... ok!");
			}
		}

		CorneaAlign* corAlign = Controller::getCorneaAlign();
		corAlign->initCorneaAlign(board);

		RetinaAlign* retAlign = Controller::getRetinaAlign();
		retAlign->initRetinaAlign(board);
		Optimizer::initOptimizer(board, retAlign);

		auto* rett = RetinaTracking::getInstance();
		if (rett) {
			rett->initializeRetinaTracking(board);
		}

		if (!corAlign->loadKeratoSetupData()) {
			GlobalLogger::warn("Kerato setup load from mainboard failed!");
			totalWarns++;
			if (!corAlign->importKeratoSetupData()) {
				GlobalLogger::warn("Kerato setup import from file failed!");
			}
			else {
				GlobalLogger::info("Importing kerato setup data ... ok!");
			}
		}
		else {
			GlobalLogger::info("Loading kerato setup data ... ok!");
		}

		if (!GlobalSettings::useOctEnable()) {
			RetinaAlign* retAlign = Controller::getRetinaAlign();
			retAlign->initRetinaAlign(board);
			Optimizer::initOptimizer(board, retAlign);
		}

		if (!Normative::initNormative()) {
			GlobalLogger::error("Normative Database init failed!");
			if (GlobalSettings::isUserModeSettings()) {
				result = false;
			}
		}
		else {
			if (!NormativeWide::initNormative()) {
			}
		}

		if (loadTopoCalibration()) {
			GlobalLogger::error("Topography Calibration data does not exist");
		}

		if (!GlobalSettings::useUsbCmosCameraEnable()) {
			if (GlobalSettings::useOctEnable()) {
				if (!checkCommport()) {
					totalWarns++;
					GlobalLogger::error("Communications Port (COM4) init failed!");
				}
			}
		}

		ostringstream msg;
		msg << "Initializing Oct system ... " << (result ? (totalWarns == 0 ? "ok" : "check") : "failed");
		GlobalLogger::info(msg.str());

		getImpl().numWarnings = totalWarns;
		getImpl().initiated = result;
	}
	catch (const std::exception&)
	{
		getImpl().initiated = false;
		GlobalLogger::error("Exception occurred during system init!");
	}

	return isInitiated();
}


bool OctSystem::Loader::isInitiated(void)
{
	return getImpl().initiated;
}


void OctSystem::Loader::releaseSystem(void)
{
	try {
		// saveSystemProfile();

		Controller::getFrameGrabber()->releaseFrameGrabber(true);
		Controller::getUsb3Grabber()->releaseUsb3Grabber();

		Controller::getScanner()->release();
		Controller::getMainBoard()->release();
		MainPlatform::releaseMainPlatform();

		// GlobalLogger::info("Oct system released");
		Logger::releaseLogger();

		
	}
	catch (...) {
	}
	return;
}


int OctSystem::Loader::getLogMsgLevel(void)
{
	return static_cast<int>(d_ptr->logLevel);
}


std::string OctSystem::Loader::getToolkitVersion(void)
{
	std::string version = TOOLKIT_SW_VERSION;
	return version;
}


std::string OctSystem::Loader::getFirmwareVersion(void)
{
	return Controller::getMainBoard()->getApplicationVersion();
}


std::string OctSystem::Loader::getFPGAVersion(void)
{
	return Controller::getMainBoard()->getFPGAVersion();
}


bool OctSystem::Loader::loadConfiguration(void)
{
	if (Controller::getMainBoard()->loadSystemConfiguration()) {
		if (SystemConfig::isValidChecksum()) {
			applyConfiguration();
			return true;
		}
		LogE() << "Configure checksum invalid!, load: " << SystemConfig::getCheckSum() << ", calc: " << SystemConfig::calculateCheckSum();
	}
	return false;
}

bool OctSystem::Loader::loadTopoCalibration(void)
{
	using json = nlohmann::json;
	json j;

	wstring path, name;

	for (int i = 0; i < TOPO_BSCAN_COUNT; i++) {
		name = (boost::wformat(L"%03d") % i).str();
		path = (boost::wformat(L"./Calibration//%s.json") % name).str();

	
		try {
			std::ifstream f(path);
			f >> j;
			f.close();

			std::vector<double> points;
			points = j.at("Calibration_data").get<std::vector<double>>();

			GlobalSettings::topoCalibrationData(i) = points;
		}
		catch (...) {
			LogE() << "Calibration_data load failure!";
		}
	}

	name = (boost::wformat(L"setting")).str();
	path = (boost::wformat(L"./Calibration//%s.json") % name).str();

	try {
		std::ifstream f(path);
		f >> j;
		f.close();

		std::vector<double> modelEyeR, measuredR;
		modelEyeR = j.at("model").get<std::vector<double>>();
		measuredR = j.at("measured").get<std::vector<double>>();

		GlobalSettings::topoModelData() = modelEyeR;
		GlobalSettings::topoMeasuredData() = measuredR;

	}
	catch (...) {
		LogE() << "offset load failure!";
	}
		return false;
}

bool OctSystem::Loader::saveConfiguration(void)
{
	return Controller::getMainBoard()->saveSystemConfiguration(true);
}


bool OctSystem::Loader::applyConfiguration(void)
{
	ChainSetup::updateSpectrometerParameters();
	ChainSetup::updateDispersionParameters();

	GlobalSettings::setInAirScanAxialResolution(ChainSetup::getAxialPixelResolution(INAIR_REFLECTIVE_INDEX));
	GlobalSettings::setCorneaScanAxialResolution(ChainSetup::getAxialPixelResolution(CORNEA_REFLECTIVE_INDEX));
	GlobalSettings::setRetinaScanAxialResolution(ChainSetup::getAxialPixelResolution(RETINA_REFLECTIVE_INDEX));
	
	return true;
}


bool OctSystem::Loader::loadSystemSetup(void)
{
	if (Controller::getMainBoard()->loadSystemProfile()) {
		return true;
	}
	else {
		return false;
	}
}


bool OctSystem::Loader::saveSystemSetup(void)
{
	if (Controller::getMainBoard()->saveSystemProfile()) {
		return true;
	}
	else {
		return false;
	}
}


bool OctSystem::Loader::loadSetupProfile(bool fromfile, bool device)
{
	if (fromfile) {
		if (!SystemProfile::loadConfigFile()) {
			SystemProfile::resetToDefaultValues();
		}
	}
	else {
		if (!loadSystemSetup()) {
			if (!SystemProfile::loadConfigFile()) {
				SystemProfile::resetToDefaultValues();
			}
		}
	}

	applySystemProfile(device);
	return true;
}


bool OctSystem::Loader::saveSetupProfile(bool update, bool tofile)
{
	if (update) {
		captureSystemProfile();
	}

	bool result = false;
	if (tofile) {
		result = SystemProfile::saveConfigFile();
	}
	else {
		result = saveSystemSetup();
	}
	return result;
}


void OctSystem::Loader::applySystemProfile(bool device)
{
	if (device) {
		DeviceSettings* dset = SystemProfile::getDeviceSettings();
		Controller::getMainBoard()->loadDeviceSettings(dset);
		CameraSettings* cset = SystemProfile::getCameraSettings();
		Controller::getMainBoard()->loadCameraSettings(cset);
	}

	AngioSettings* nset = SystemProfile::getAngioSettings();
	GlobalSettings::angioDecorThreshold() = nset->getDecorThreshold();
	GlobalSettings::angioAverageOffset() = nset->getAverageOffset();
	GlobalSettings::angioMotionThreshold() = nset->getMotionThreshold();
	GlobalSettings::angioMotionOverPoints() = nset->getMotionOverPoints();
	GlobalSettings::angioMotionDistRatio() = nset->getMotionDistRatio();
	GlobalSettings::angioFilterOrients() = nset->getFilterOrients();
	GlobalSettings::angioFilterSigma() = nset->getFilterSigma();
	GlobalSettings::angioFilterDivider() = nset->getFilterDivider();
	GlobalSettings::angioFilterWeight() = nset->getFilterWeight();
	GlobalSettings::angioEnhanceParam() = nset->getEnhanceParam();
	GlobalSettings::angioBiasFieldSigma() = nset->getBiasFieldSigma();

	FundusSettings* fset = SystemProfile::getFundusSettings();
	GlobalSettings::whiteBalanceParameters() = fset->getWhiteBalanceParameters();
	GlobalSettings::useWhiteBalance(true, fset->useWhiteBalance());

	GlobalSettings::levelCorrectionCoordsX() = fset->getCorrectionCoordsX();
	GlobalSettings::levelCorrectionCoordsY() = fset->getCorrectionCoordsY();
	GlobalSettings::levelCorrectionCoordsX_FILR() = fset->getCorrectionCoordsX_FILR();
	GlobalSettings::levelCorrectionCoordsY_FILR() = fset->getCorrectionCoordsY_FILR();
	GlobalSettings::useFundusLevelCorrection(true, fset->useColorCorrection());

	GlobalSettings::fundusAutoBrightness() = fset->autoBright();
	GlobalSettings::fundusAutoContrast() = fset->autoContrast();
	GlobalSettings::fundusAutoUB() = fset->autoUB();
	GlobalSettings::fundusAutoVR() = fset->autoVR();
	GlobalSettings::fundusRadialCorrectionRatio() = fset->radialCorrectionRatio();
	GlobalSettings::fundusEdgeKernelSize() = fset->edgeKernelSize();
	GlobalSettings::useFundusRadialCorrection(true, fset->useRadialCorrection());

	GlobalSettings::centerOfFundusROI() = fset->getCenterPointOfROI();
	GlobalSettings::radiusOfFundusROI() = fset->getRadiusOfROI();
	GlobalSettings::useFundusROI(true, fset->useROICoordinates());

	GlobalSettings::removeReflectionLight1() = fset->getRemoveReflection1();
	GlobalSettings::removeReflectionLight2() = fset->getRemoveReflection2();
	GlobalSettings::removeReflectionLight3() = fset->getRemoveReflection3();
	GlobalSettings::useRemoveLight(true, fset->useRemoveReflectionLight());

	GlobalSettings::autoFlashLevelNormal() = fset->getAutoFlashLevelNormal();
	GlobalSettings::autoFlashLevelMax() = fset->getAutoFlashLevelMax();
	GlobalSettings::autoFlashLevelMin() = fset->getAutoFlashLevelMin();

	for (int k = 0; k < AUTO_FLASH_LEVEL_PRESET_SIZE; k++) {
		GlobalSettings::flashPresetFocusInt(k) = fset->getAutoFlashFocusInt(k);
		GlobalSettings::flashPresetSizeBase(k) = fset->getAutoFlashSizeBase(k);
		GlobalSettings::flashPresetSizeHigh(k) = fset->getAutoFlashSizeHigh(k);
		GlobalSettings::flashPresetDefLevel(k) = fset->getAutoFlashDefLevel(k);
	}

	GlobalSettings::useAutoFlashLevel(true, fset->useAutoFlashLevel());

	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	GlobalSettings::useRetinaImageEnhance(true, rset->useImageEnhance());
	GlobalSettings::retinaImageClipLimit() = rset->getEnhanceClipLimit();
	GlobalSettings::retinaImageClipScalar() = rset->getEnhanceClipScalar();

	GlobalSettings::splitFocusCenter() = rset->getSplitCenter();
	GlobalSettings::centerOfRetinaROI() = rset->getCenterPointOfROI();
	GlobalSettings::radiusOfRetinaROI() = rset->getRadiusOfROI();
	GlobalSettings::useRetinaROI(true, rset->useROI());

	GlobalSettings::centerOfRetinaMask() = rset->getCenterPointOfMask();
	GlobalSettings::retinaCenterMaskSize() = rset->getCenterMaskSize();
	GlobalSettings::useRetinaCenterMask(true, rset->useCenterMask());

	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	GlobalSettings::corneaPixelsPerMM() = aset->getPixelsPerMM();
	GlobalSettings::corneaSmallPupilSize() = aset->getSmallPupilSize();

	FixationSettings* xset = SystemProfile::getFixationSettings();
	GlobalSettings::useLcdFixation(true, xset->useLcdFixation());

	GlobalSettings::fixationCenterOD() = xset->getCenterOD();
	GlobalSettings::fixationCenterOS() = xset->getCenterOS();
	GlobalSettings::fixationFundusOD() = xset->getFundusOD();
	GlobalSettings::fixationFundusOS() = xset->getFundusOS();
	GlobalSettings::fixationScanDiskOD() = xset->getScanDiskOD();
	GlobalSettings::fixationScanDiskOS() = xset->getScanDiskOS();

	GlobalSettings::fixationLeftSideOD(0) = xset->getLeftSideOD(0);
	GlobalSettings::fixationLeftSideOD(1) = xset->getLeftSideOD(1);
	GlobalSettings::fixationLeftSideOD(2) = xset->getLeftSideOD(2);
	GlobalSettings::fixationLeftSideOS(0) = xset->getLeftSideOS(0);
	GlobalSettings::fixationLeftSideOS(1) = xset->getLeftSideOS(1);
	GlobalSettings::fixationLeftSideOS(2) = xset->getLeftSideOS(2);

	GlobalSettings::fixationRightSideOD(0) = xset->getRightSideOD(0);
	GlobalSettings::fixationRightSideOD(1) = xset->getRightSideOD(1);
	GlobalSettings::fixationRightSideOD(2) = xset->getRightSideOD(2);
	GlobalSettings::fixationRightSideOS(0) = xset->getRightSideOS(0);
	GlobalSettings::fixationRightSideOS(1) = xset->getRightSideOS(1);
	GlobalSettings::fixationRightSideOS(2) = xset->getRightSideOS(2);

	GlobalSettings::useLcdBlinkOn(true, xset->useLcdBlinkOn());
	GlobalSettings::fixationBlinkPeriod() = xset->getBlinkPeriod();
	GlobalSettings::fixationBlinkOnTime() = xset->getBlinkOnTime();
	GlobalSettings::fixationBrightness() = xset->getBrightness();
	GlobalSettings::fixationType() = xset->getFixationType();

	Controller::getMainBoard()->updateLcdParameters(GlobalSettings::fixationBrightness(),
		GlobalSettings::useLcdBlinkOn(), GlobalSettings::fixationBlinkPeriod(),
		GlobalSettings::fixationBlinkOnTime(), 
		GlobalSettings::fixationType());

	SystemSettings* sset = SystemProfile::getSystemSettings();
	GlobalSettings::checkMainboardAtStartup() = sset->checkMainboardAtStartup();
	GlobalSettings::setModelType(sset->getModelType());
	GlobalSettings::setOctGrabberType(sset->getOctGrabberType());
	GlobalSettings::setOctUsbLineCameraMode(sset->getOctLineCameraMode());
	GlobalSettings::setRetinaTrackingSpeed(sset->getRetinaTrackingSpeed());
	GlobalSettings::setSldFaultDetection(sset->getSldFaultDetection());
	GlobalSettings::setSldFaultTimer(sset->getSldFaultTimer());
	GlobalSettings::setSldFaultThreshold(sset->getSldFaultThreshold());

	for (int i = 0; i < 3; i++) {
		GlobalSettings::setTriggerForePadd(i, sset->getTriggerForePadd(i));
		GlobalSettings::setTriggerPostPadd(i, sset->getTriggerPostPadd(i));
	}

	PatternSettings* tset = SystemProfile::getPatternSettings();
	for (int i = 0; i < 3; i++) {
		GlobalSettings::retinaPatternScale(i) = tset->retinaPatternScale(i);
		GlobalSettings::corneaPatternScale(i) = tset->corneaPatternScale(i);
		GlobalSettings::topographyPatternScale(i) = tset->topographyPatternScale(i);
		if (tset->topographyPatternScale(i).first != 1.0f || tset->topographyPatternScale(i).second != 1.0f) {
			GlobalSettings::useTopoCalibration(true, true);
		}
		GlobalSettings::retinaPatternOffset(i) = tset->retinaPatternOffset(i);
		GlobalSettings::corneaPatternOffset(i) = tset->corneaPatternOffset(i);
		GlobalSettings::topographyPatternOffset(i) = tset->topographyPatternOffset(i);
		if (tset->topographyPatternOffset(i).first != 0.0f || tset->topographyPatternOffset(i).second != 0.0f) {
			GlobalSettings::useTopoCalibration(true, true);
		}
	}

	if (device) {
		applySldFaultDetection();
	}
	return;
}


void OctSystem::Loader::captureSystemProfile(void)
{
	DeviceSettings* dset = SystemProfile::getDeviceSettings();
	Controller::getMainBoard()->saveDeviceSettings(dset);

	CameraSettings* cset = SystemProfile::getCameraSettings();
	Controller::getMainBoard()->saveCameraSettings(cset);

	FundusSettings* fset = SystemProfile::getFundusSettings();
	fset->setWhiteBalanceParameters(GlobalSettings::whiteBalanceParameters());
	fset->useWhiteBalance(true, GlobalSettings::useWhiteBalance());

	fset->setCorrectionCoordsX(GlobalSettings::levelCorrectionCoordsX());
	fset->setCorrectionCoordsY(GlobalSettings::levelCorrectionCoordsY());
	fset->setCorrectionCoordsX_FILR(GlobalSettings::levelCorrectionCoordsX_FILR());
	fset->setCorrectionCoordsY_FILR(GlobalSettings::levelCorrectionCoordsY_FILR());
	fset->useColorCorrection(true, GlobalSettings::useFundusLevelCorrection());

	fset->setAutoBright(GlobalSettings::fundusAutoBrightness());
	fset->setAutoContrast(GlobalSettings::fundusAutoContrast());
	fset->setAutoUB(GlobalSettings::fundusAutoUB());
	fset->setAutoVR(GlobalSettings::fundusAutoVR());
	fset->setRadialCorrectionRatio(GlobalSettings::fundusRadialCorrectionRatio());
	fset->setEdgeKernelSize(GlobalSettings::fundusEdgeKernelSize());
	fset->useRadialCorrection(true, GlobalSettings::useFundusRadialCorrection());

	fset->setCenterPointOfROI(GlobalSettings::centerOfFundusROI());
	fset->setRadiusOfROI(GlobalSettings::radiusOfFundusROI());
	fset->useROICoordinates(true, GlobalSettings::useFundusROI());

	fset->setRemoveReflection1(GlobalSettings::removeReflectionLight1());
	fset->setRemoveReflection2(GlobalSettings::removeReflectionLight2());
	fset->setRemoveReflection3(GlobalSettings::removeReflectionLight3());
	fset->useRemoveReflectionLight(true, GlobalSettings::useRemoveLight());

	fset->setAutoFlashLevelNormal(GlobalSettings::autoFlashLevelNormal());
	fset->setAutoFlashLevelMax(GlobalSettings::autoFlashLevelMax());
	fset->setAutoFlashLevelMin(GlobalSettings::autoFlashLevelMin());

	for (int k = 0; k < AUTO_FLASH_LEVEL_PRESET_SIZE; k++) {
		fset->setAutoFlashFocusInt(GlobalSettings::flashPresetFocusInt(k), k);
		fset->setAutoFlashSizeBase(GlobalSettings::flashPresetSizeBase(k), k);
		fset->setAutoFlashSizeHigh(GlobalSettings::flashPresetSizeHigh(k), k);
		fset->setAutoFlashDefLevel(GlobalSettings::flashPresetDefLevel(k), k);
	}
	fset->useAutoFlashLevel(true, GlobalSettings::useAutoFlashLevel());

	RetinaSettings* rset = SystemProfile::getRetinaSettings();
	rset->setSplitCenter(GlobalSettings::splitFocusCenter());
	// GlobalSettings::splitFocusKernel() = rset->getSplitKernel();
	rset->setCenterPointOfROI(GlobalSettings::centerOfRetinaROI());
	rset->setRadiusOfROI(GlobalSettings::radiusOfRetinaROI());
	rset->useROI(GlobalSettings::useRetinaROI());
	rset->useImageEnhance(GlobalSettings::useRetinaImageEnhance());
	rset->setEnhanceClipLimit(GlobalSettings::retinaImageClipLimit());
	rset->setEnhanceClipScalar(GlobalSettings::retinaImageClipScalar());

	rset->setCenterPointOfMask(GlobalSettings::centerOfRetinaMask());
	rset->setCenterMaskSize(GlobalSettings::retinaCenterMaskSize());
	rset->useCenterMask(GlobalSettings::useRetinaCenterMask());

	CorneaSettings* aset = SystemProfile::getCorneaSettings();
	aset->setPixelsPerMM(GlobalSettings::corneaPixelsPerMM());
	aset->setSmallPupilSize(GlobalSettings::corneaSmallPupilSize());

	FixationSettings* xset = SystemProfile::getFixationSettings();
	xset->setCenterOD(GlobalSettings::fixationCenterOD());
	xset->setCenterOS(GlobalSettings::fixationCenterOS());
	xset->setFundusOD(GlobalSettings::fixationFundusOD());
	xset->setFundusOS(GlobalSettings::fixationFundusOS());
	xset->setScanDiskOD(GlobalSettings::fixationScanDiskOD());
	xset->setScanDiskOS(GlobalSettings::fixationScanDiskOS());

	xset->setLeftSideOD(0, GlobalSettings::fixationLeftSideOD(0));
	xset->setLeftSideOD(1, GlobalSettings::fixationLeftSideOD(1));
	xset->setLeftSideOD(2, GlobalSettings::fixationLeftSideOD(2));
	xset->setLeftSideOS(0, GlobalSettings::fixationLeftSideOS(0));
	xset->setLeftSideOS(1, GlobalSettings::fixationLeftSideOS(1));
	xset->setLeftSideOS(2, GlobalSettings::fixationLeftSideOS(2));

	xset->setRightSideOD(0, GlobalSettings::fixationRightSideOD(0));
	xset->setRightSideOD(1, GlobalSettings::fixationRightSideOD(1));
	xset->setRightSideOD(2, GlobalSettings::fixationRightSideOD(2));
	xset->setRightSideOS(0, GlobalSettings::fixationRightSideOS(0));
	xset->setRightSideOS(1, GlobalSettings::fixationRightSideOS(1));
	xset->setRightSideOS(2, GlobalSettings::fixationRightSideOS(2));

	xset->useLcdBlinkOn(true, GlobalSettings::useLcdBlinkOn());
	xset->setBlinkPeriod(GlobalSettings::fixationBlinkPeriod());
	xset->setBlinkOnTime(GlobalSettings::fixationBlinkOnTime());
	xset->setBrightness(GlobalSettings::fixationBrightness());

	SystemSettings* sset = SystemProfile::getSystemSettings();
	sset->checkMainboardAtStartup(true, GlobalSettings::checkMainboardAtStartup());
//	sset->useOctSoloVersion(true, GlobalSettings::isOctSoloVersion());
	sset->setModelType(GlobalSettings::getModelType());
	sset->setOctGrabberType(GlobalSettings::getOctGrabberType());
	sset->setOctLineCameraMode(GlobalSettings::getOctUsbLineCameraMode());
	sset->setRetinaTrackingSpeed(GlobalSettings::getRetinaTrackingSpeed());
	sset->setSldFaultDetection(GlobalSettings::getSldFaultDetection());
	sset->setSldFaultTimer(GlobalSettings::getSldFaultTimer());
	sset->setSldFaultThreshold(GlobalSettings::getSldFaultThreshold());

	for (int i = 0; i < 3; i++) {
		sset->setTriggerForePadd(i, GlobalSettings::getTriggerForePadd(i));
		sset->setTriggerPostPadd(i, GlobalSettings::getTriggerPostPadd(i));
	}

	PatternSettings* tset = SystemProfile::getPatternSettings();
	for (int i = 0; i < 3; i++) {
		tset->retinaPatternScale(i) = GlobalSettings::retinaPatternScale(i);
		tset->corneaPatternScale(i) = GlobalSettings::corneaPatternScale(i);
		tset->topographyPatternScale(i) = GlobalSettings::topographyPatternScale(i);
		tset->retinaPatternOffset(i) = GlobalSettings::retinaPatternOffset(i);
		tset->corneaPatternOffset(i) = GlobalSettings::corneaPatternOffset(i);
		tset->topographyPatternOffset(i) = GlobalSettings::topographyPatternOffset(i);
	}
	return;
}


bool OctSystem::Loader::applySldFaultDetection(void)
{
	int sett = GlobalSettings::getSldFaultDetection();
	if ((sett == SLD_FAULT_DETECTION_ENABLED && GlobalSettings::isUserModeSettings()) || 
		(GlobalSettings::useSldFaultOverride() && GlobalSettings::isEngineerModeSettings())) {
		Controller::getMainBoard()->setSldWarningEnabled(1);
		auto timer = (std::uint16_t)GlobalSettings::getSldFaultTimer();
		auto thresh = (std::uint16_t)GlobalSettings::getSldFaultThreshold();
		Controller::getMainBoard()->updateScannerFaultParameters(timer, thresh);
		LogD() << "Enabled SLD fault detection, timer: " << timer << ", threshold: " << thresh;
	}
	else {
		Controller::getMainBoard()->setSldWarningEnabled(0);
		LogD() << "Disabled SLD fault detection";
	}
	return true;
}

bool OctSystem::Loader::loadDefectPixelsOfColorCamera(void)
{
	auto ccam = Controller::getMainBoard()->getColorCamera();
	bool ret = ccam->loadDefectPixels(PixelCorrect::getBadPixelCoords());
	return ret;
}


bool OctSystem::Loader::saveDefectPixelsOfColorCamera(void)
{
	auto ccam = Controller::getMainBoard()->getColorCamera();
	bool ret = ccam->saveDefectPixels(PixelCorrect::getBadPixelCoords());
	return ret;
}


void OctSystem::Loader::getRetinaCameraShift(int & shiftX, int & shiftY)
{
	CameraSettings* cset = SystemProfile::getCameraSettings();
	shiftX = cset->getRetinaCamera()->shiftX();
	shiftY = cset->getRetinaCamera()->shiftY();
	return;
}


void OctSystem::Loader::getCorneaCameraShift(int & shiftX, int & shiftY)
{
	CameraSettings* cset = SystemProfile::getCameraSettings();
	shiftX = cset->getCorneaCamera()->shiftX();
	shiftY = cset->getCorneaCamera()->shiftY();
	return;
}

bool OctSystem::Loader::checkCommport(void)
{
	TCHAR lpTargetPath[5000];
	DWORD isComport;

	isComport = QueryDosDevice(_T("COM4"), (LPWSTR)lpTargetPath, 5000);

	CString truePath = _T("\\Device\\Serial0");

	bool compareCommportName = truePath.CompareNoCase((LPCTSTR)lpTargetPath) != 0;

	if (isComport) {
		if (!compareCommportName) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

int OctSystem::Loader::getNumberOfWarnings(void)
{
	return getImpl().numWarnings;
}

bool OctSystem::Loader::setupUsbCmosCamera(void)
{
	GlobalSettings::useUsbCmosCameraEnable(true, true);
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		Usb3Grabber* usb3Grab = Controller::getUsb3Grabber();
		if (!usb3Grab->initializeUsb3Grabber()) {
			GlobalLogger::error("Usb3 grabber init failed!");
			return false;
		}
		else {
			Controller::getScanner()->initScanner(Controller::getMainBoard(), usb3Grab);
			GlobalLogger::info("usb3 grabber init ... ok!");
		}
	}
	return true;
}

void OctSystem::Loader::releaseUsbCmosCamera(void)
{
	if (GlobalSettings::useUsbCmosCameraEnable()) {
		Usb3Grabber* usb3Grab = Controller::getUsb3Grabber();
		usb3Grab->releaseUsb3Grabber();
	}
	return;
}



bool OctSystem::Loader::useNoDevice(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoDevice = flag;
	}
	return getImpl().useNoDevice;
}


bool OctSystem::Loader::useNoCameras(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoCameras = flag;
	}
	return getImpl().useNoCameras;
}


bool OctSystem::Loader::useNoLeds(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoLeds = flag;
	}
	return getImpl().useNoLeds;
}


bool OctSystem::Loader::useNoMotors(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoMotors = flag;
	}
	return getImpl().useNoMotors;
}


bool OctSystem::Loader::useNoGrabber(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoGrabber = flag;
	}
	return getImpl().useNoGrabber;
}


bool OctSystem::Loader::useNoColor(bool isset, bool flag)
{
	if (isset) {
		getImpl().useNoColor = flag;
	}
	return getImpl().useNoColor;
}


bool OctSystem::Loader::useDcfFile(bool isSet, bool flag, const wchar_t * path)
{
	if (isSet) {
		getImpl().useDcfFile = flag;
		if (flag) {
			getImpl().dcfFilePath = path;
		}
	}
	return getImpl().useDcfFile;
}


const wchar_t * OctSystem::Loader::getDcfFilePath(void)
{
	return getImpl().dcfFilePath.c_str();
}


void OctSystem::Loader::initSystemBootMode(void)
{
	updatePropertiesFromCommandLine();

	Logger::initLogger(d_ptr->logLevel, d_ptr->logSrcLine, true, d_ptr->logTimeTag, d_ptr->logFileKeep);
	GlobalLogger::info("Start Oct system");

	MainBoard* board = Controller::getMainBoard();
	board->setDeviceMode(useNoDevice(), useNoCameras(), useNoLeds(), useNoMotors(), useNoColor());

	std::ostringstream ss;
	if (useNoDevice()) {
		ss << "-nodevice ";
	}
	if (useNoCameras()) {
		ss << "-nocamera ";
	}
	if (useNoLeds()) {
		ss << "-noled ";
	}
	if (useNoMotors()) {
		ss << "-nomotor ";
	}
	if (useNoColor()) {
		ss << "-nocolor ";
	}
	if (useNoGrabber()) {
		ss << "-nograbber ";
	}

	if (!ss.str().empty()) {
		ostringstream msg;
		msg << "Command options: " << ss.str();
		GlobalLogger::info(msg.str());
	}

	if (useDcfFile()) {
		ostringstream msg;
		msg << "DCF file path: " << wtoa(getDcfFilePath());
		GlobalLogger::info(msg.str());
	}
	return;
}


Loader::LoaderImpl & OctSystem::Loader::getImpl(void)
{
	return *d_ptr;
}


bool OctSystem::Loader::updatePropertiesFromCommandLine(void)
{
	CFileFind finder;
	CString option, param;

	if (__argc > 1)
	{
		for (int i = 1; i < __argc; i++)
		{
			option.SetString(__targv[i]);

			if (!option.CompareNoCase(_T("-nograbber")) || !option.CompareNoCase(_T("-nograb"))) {
				useNoGrabber(true, true);
			}
			else if (!option.CompareNoCase(_T("-nodevice"))) {
				useNoDevice(true, true);
			}			
			else if (!option.CompareNoCase(_T("-nocam")) || !option.CompareNoCase(_T("-nocamera"))) {
				useNoCameras(true, true);
			}
			else if (!option.CompareNoCase(_T("-noled"))) {
				useNoLeds(true, true);
			}
			else if (!option.CompareNoCase(_T("-nomotor"))) {
				useNoMotors(true, true);
			}
			else if (!option.CompareNoCase(_T("-nocolor"))) {
				useNoColor(true, true);
			}
			/*	
			else if (!option.CompareNoCase(_T("-octsolo"))) {
				useOctSolo(true, true);
			}*/
			/*
			else if (!option.CompareNoCase(_T("-80k"))) {
				GlobalSettings::setOctUsbLineCameraSpeed((int)OctUsbLineCameraSpeed::LINE_CAMERA_80k);
			}
			else if (!option.CompareNoCase(_T("-120k"))) {
				GlobalSettings::setOctUsbLineCameraSpeed((int)OctUsbLineCameraSpeed::LINE_CAMERA_120k);
			}
			else if (!option.CompareNoCase(_T("-15fps"))) {
				GlobalSettings::setRetinaIrHighSpeed(false);
			}
			*/
			else if (!option.CompareNoCase(_T("-angio_camera"))) {
				AngioSetup::setSavingFramesToFiles(true);
			}
			else if (!option.CompareNoCase(_T("-angio_buffer"))) {
				AngioSetup::setSavingBuffersToFiles(true);
			}
			else if (!option.CompareNoCase(_T("-no_klinear"))) {
				ChainSetup::useKLinearResampling(true, false);
			}
			else if (!option.CompareNoCase(_T("-usbcmos"))) {
				GlobalSettings::useUsbCmosCameraEnable(true, true);
			}
			else if (!option.CompareNoCase(_T("-sldfault"))) {
				GlobalSettings::useSldFaultOverride(true, true);
			}
			else if (!option.CompareNoCase(_T("-trace"))) {
				d_ptr->logLevel = Logger::LogLevel::LEVEL_TRACE;
			}
			else if (!option.CompareNoCase(_T("-debug"))) {
				d_ptr->logLevel = Logger::LogLevel::LEVEL_DEBUG;
			}
			else if (!option.CompareNoCase(_T("-info"))) {
				d_ptr->logLevel = Logger::LogLevel::LEVEL_INFO;
			}
			else if (!option.CompareNoCase(_T("-warn"))) {
				d_ptr->logLevel = Logger::LogLevel::LEVEL_WARNING;
			}
			else if (!option.CompareNoCase(_T("-error"))) {
				d_ptr->logLevel = Logger::LogLevel::LEVEL_ERROR;
			}
			else if (!option.CompareNoCase(_T("-srcline"))) {
				d_ptr->logSrcLine = true;
			}
			else if (!option.CompareNoCase(_T("-timetag"))) {
				d_ptr->logTimeTag = true;
			}
			else if (!option.CompareNoCase(_T("-logkeep"))) {
				d_ptr->logFileKeep = true;
			}
			else if (!option.CompareNoCase(_T("-dcf"))) {
				i++;
				param.SetString(__targv[i]);
				if (finder.FindFile(param) != NULL) {
					useDcfFile(true, true, __targv[i]);
				}
			}
		}
	}

	// d_ptr->logLevel = Logger::LogLevel::LEVEL_DEBUG;
	d_ptr->logTimeTag = true;
	return true;
}
