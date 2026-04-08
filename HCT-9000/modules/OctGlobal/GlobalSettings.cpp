#include "stdafx.h"
#include "GlobalSettings.h"
#include "GlobalConfigDef.h"
#include "GlobalFundusDef.h"
#include "GlobalScanDef.h"
#include "GlobalRetinaDef.h"
#include "OctScanDescript.h"
#include "GlobalAnalysisDef.h"
#include "GlobalDeviceDef.h"

#include <vector>

using namespace OctGlobal;
using namespace std;


struct GlobalSettings::GlobalSettingsImpl
{
	int modleType;
	bool octEnable;
	bool fundusEnable;
	bool angioEnable;
	bool fundusFILR_Enable;
	bool newGpioRutine;
	bool usbCmosCameraEnable = false;
	int octGrabberType;
	int octLineCameraMode;
	int retinaTrackSpeed;
	int sldFaultDetection;
	int sldFaultTimer;
	int sldFaultThreshold;
	bool sldFaultOverride;
	int triggerForePadd[3];
	int triggerPostPadd[3];

	bool userMode;
	bool useColorImageCorrection;
	bool useColorImageResizing;
	bool useEnfaceImageCorrection;
	bool useOclTasksDefaultPreset;
	bool useRemoveReflectionLight;
	bool useAutoFlash;
	bool useTopoCalibration;
	bool useSaveTiffImage = false;

	vector<double> awbParams;
	vector<int> correctXs;
	vector<int> correctYs;
	vector<int> correctXs_FILR;
	vector<int> correctYs_FILR;

	bool useWhiteBalance;
	bool useLevelCorrection;
	bool useRadialCorrection;
	bool useFundusROI;
	pair<int, int> centerFundusROI;
	int radiusFundusROI;

	int flashLevelNormal;
	int flashLevelMin;
	int flashLevelMax;
	int removeReflectionLisht1;
	int removeReflectionLisht2;
	int removeReflectionLisht3;
	int flashSizeBases[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashSizeHighs[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashFocusInts[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashDefLevels[AUTO_FLASH_LEVEL_PRESET_SIZE];
	
	int retinaCameraPreset;

	float autoBright;
	float autoContrast;
	float autoUB;
	float autoVR;
	float radialRatio;
	int edgeKernelSize;

    bool useRetinaROI;
	bool useRetinaCenterMask;
	float retinaClipLimit = RETINA_ENHANCE_CLIP_LIMIT;
	int retinaClipScalar = RETINA_ENHANCE_CLIP_SCALAR;

    pair<int, int> centerRetinaROI;
	pair<int, int> centerRetinaMask;
    int radiusRetinaROI;
	int retinaCenterMaskSize;

    bool useRetinaImageEnhance;
	pair<int, int> splitCenter;
	pair<int, int> splitKernel;

	int corneaPixelsPerMM;
	float corneaSmallPupilSize;

	bool useLcdFixation;
	bool useLcdBlinkOn;
	int fixBlinkPeriod;
	int fixBlinkOnTime;
	int fixBrightness;
	int fixationType;

	pair<int, int> centerOD;
	pair<int, int> fundusOD;
	pair<int, int> scanDiskOD;
	pair<int, int> leftSideOD[3];
	pair<int, int> rightSideOD[3];

	pair<int, int> centerOS;
	pair<int, int> fundusOS;
	pair<int, int> scanDiskOS;
	pair<int, int> leftSideOS[3];
	pair<int, int> rightSideOS[3];

	pair<int, int> enfaceRange;

	pair<float, float> retinaPatternScales[3];
	pair<float, float> corneaPatternScales[3];
	pair<float, float> topographyPatternScales[3];
	
	pair<float, float> retinaPatternOffsets[3];
	pair<float, float> corneaPatternOffsets[3];
	pair<float, float> topographyPatternOffsets[3];

	int checkMainboard;

	double corneaPixResol;
	double retinaPixResol;
	double inAirPixResol;

	float decorThreshold = 0.0f; // 0.15f;
	int averageOffset = 3;;
	float motionThreshold = 1.5f;
	float motionOverPoints = 0.5f;
	float motionDistRatio = 1.15f;
	int filterOrients = 12;
	float filterSigma = 3.0f;
	float filterDivider = 1.5f;
	float filterWeight = 0.25f; // 0.5f;
	float enhanceParam = 1.0f;
	float biasFieldSigma = 15.0f;


	//==========================================================================//
	// measure fundus //
	bool bDomainTransform;
	float fDomainParam;
	bool bWhiteBalance;
	int nWhiteBalanceR;
	int nWhiteBalanceG;
	int nWhiteBalanceB;
	bool bCentralBrightness;
	float fBrightness;
	float fBrightnessGamma;
	bool bCosineFallOff;
	int nInner;
	int nOutter;
	float fFactor;
	//==========================================================================//


	vector<vector<double>> topoCalibrationData;
	vector<double> topoModelR;
	vector<double> topoMeasuredR;

	GlobalSettingsImpl() 
		: useColorImageCorrection(true), useColorImageResizing(true), 
			useEnfaceImageCorrection(true), usbCmosCameraEnable(false), octGrabberType(0), octLineCameraMode(0),
		    retinaTrackSpeed(0), 
			sldFaultDetection(0), sldFaultTimer(5000), sldFaultThreshold(10), sldFaultOverride(false),
			useOclTasksDefaultPreset(true), userMode(true), 
			useWhiteBalance(false), useLevelCorrection(false), useRadialCorrection(false),
			useFundusROI(false), useRetinaROI(false), useTopoCalibration(false),
			useRetinaImageEnhance(false), useLcdFixation(false), useLcdBlinkOn(false), useRemoveReflectionLight(false),
			retinaCameraPreset(0), corneaPixResol(CORNEA_SCAN_PIXEL_RESOLUTION), retinaPixResol(RETINA_SCAN_PIXEL_RESOLUTION), inAirPixResol(INAIR_SCAN_PIXEL_RESOLUTION)
	{
		awbParams.assign(FUNDUS_AWB_PARAMS_SIZE, 1.0);
		correctXs.assign(FUNDUS_RADIAL_CORRECT_PARAMS_SIZE, 0);
		correctYs.assign(FUNDUS_RADIAL_CORRECT_PARAMS_SIZE, 0);

		correctXs_FILR.assign(FUNDUS_FILR_RADIAL_CORRECT_PARAMS_SIZE, 0);
		correctYs_FILR.assign(FUNDUS_FILR_RADIAL_CORRECT_PARAMS_SIZE, 0);
		enfaceRange = std::pair<int, int>(ENFACE_IMAGE_DEPTH_START, ENFACE_IMAGE_DEPTH_CLOSE);

		topoCalibrationData.assign(TOPO_BSCAN_COUNT, vector<double>(TOPO_CALIBRATION_DATA_SIZE, 0));
		topoModelR.assign(4, 0);
		topoMeasuredR.assign(4, 0);
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<GlobalSettings::GlobalSettingsImpl> GlobalSettings::d_ptr(new GlobalSettingsImpl());


GlobalSettings::GlobalSettings()
{
}


GlobalSettings::~GlobalSettings()
{
}


void OctGlobal::GlobalSettings::applyUserModeSettings(bool flag)
{
	getImpl().userMode = flag;

	if (flag) {
		useColorImageCorrection(true, true);
		useColorImageResizing(true, true);
		useEnfaceImageCorrection(true, true);
		useOclTasksDefaultPreset(true, true);
	}
	else {
		useColorImageCorrection(true, false);
		useColorImageResizing(true, false);
		useEnfaceImageCorrection(true, true);
		useOclTasksDefaultPreset(true, false);
	}
	return;
}

void OctGlobal::GlobalSettings::applyEngineerModeSettings(void)
{
	applyUserModeSettings(false);
	return;
}


bool OctGlobal::GlobalSettings::isUserModeSettings(void)
{
	return getImpl().userMode;
}

bool OctGlobal::GlobalSettings::isEngineerModeSettings(void)
{
	bool flag = !isUserModeSettings();
	return flag;
}


float & OctGlobal::GlobalSettings::angioDecorThreshold(void)
{
	return getImpl().decorThreshold;
}

int & OctGlobal::GlobalSettings::angioAverageOffset(void)
{
	return getImpl().averageOffset;
}

float & OctGlobal::GlobalSettings::angioMotionThreshold(void)
{
	return getImpl().motionThreshold;
}

float & OctGlobal::GlobalSettings::angioMotionOverPoints(void)
{
	return getImpl().motionOverPoints;
}

float & OctGlobal::GlobalSettings::angioMotionDistRatio(void)
{
	return getImpl().motionDistRatio;
}

int & OctGlobal::GlobalSettings::angioFilterOrients(void)
{
	return getImpl().filterOrients;
}

float & OctGlobal::GlobalSettings::angioFilterSigma(void)
{
	return getImpl().filterSigma;
}

float & OctGlobal::GlobalSettings::angioFilterDivider(void)
{
	return getImpl().filterDivider;
}

float & OctGlobal::GlobalSettings::angioFilterWeight(void)
{
	return getImpl().filterWeight;
}

float & OctGlobal::GlobalSettings::angioEnhanceParam(void)
{
	return getImpl().enhanceParam;
}

float & OctGlobal::GlobalSettings::angioBiasFieldSigma(void)
{
	return getImpl().biasFieldSigma;
}

bool OctGlobal::GlobalSettings::useColorImageCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useColorImageCorrection = flag;
	}
	return getImpl().useColorImageCorrection;
}


bool OctGlobal::GlobalSettings::useColorImageResizing(bool isset, bool flag)
{
	if (isset) {
		getImpl().useColorImageResizing = flag;
	}
	return getImpl().useColorImageResizing;
}


bool OctGlobal::GlobalSettings::useEnfaceImageCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useEnfaceImageCorrection = flag;
	}
	return getImpl().useEnfaceImageCorrection;
}


bool OctGlobal::GlobalSettings::useOclTasksDefaultPreset(bool isset, bool flag)
{
	if (isset) {
		getImpl().useOclTasksDefaultPreset = flag;
	}
	return getImpl().useOclTasksDefaultPreset;
}

bool OctGlobal::GlobalSettings::useOctEnable(bool isset, bool flag)
{
	if (isset) {
		getImpl().octEnable = flag;
	}
	return getImpl().octEnable;
}

bool OctGlobal::GlobalSettings::useUsbCmosCameraEnable(bool isset, bool flag)
{
	if (isset) {
		getImpl().usbCmosCameraEnable = flag;
	}
	return getImpl().usbCmosCameraEnable;
}

bool OctGlobal::GlobalSettings::useFundusEnable(bool isset, bool flag)
{
	if (isset) {
		getImpl().fundusEnable = flag;
	}
	return getImpl().fundusEnable;
}

bool OctGlobal::GlobalSettings::useAngioEnable(bool isset, bool flag)
{
	if (isset) {
		getImpl().angioEnable = flag;
	}
	return getImpl().angioEnable;
}

bool OctGlobal::GlobalSettings::useFundusFILR_Enable(bool isset, bool flag)
{
	if (isset) {
		getImpl().fundusFILR_Enable = flag;
	}
	return getImpl().fundusFILR_Enable;
}

bool OctGlobal::GlobalSettings::useNewGpioRutine(bool isset, bool flag)
{
	if (isset) {
		getImpl().newGpioRutine = flag;
	}
	return getImpl().newGpioRutine;
}

bool OctGlobal::GlobalSettings::useRemoveLight(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRemoveReflectionLight = flag;
	}
	return getImpl().useRemoveReflectionLight;
}

bool OctGlobal::GlobalSettings::useTopoCalibration(bool isset, bool flag)
{
	if (isset) {
		getImpl().useTopoCalibration = flag;
	}
	return getImpl().useTopoCalibration;
}

bool OctGlobal::GlobalSettings::useSaveTiff(bool isset, bool flag)
{
	if (isset) {
		getImpl().useSaveTiffImage = flag;
	}
	return getImpl().useSaveTiffImage;
}


std::vector<double>& OctGlobal::GlobalSettings::whiteBalanceParameters(void)
{
	return getImpl().awbParams;
}


bool OctGlobal::GlobalSettings::useWhiteBalance(bool isset, bool flag)
{
	if (isset) {
		getImpl().useWhiteBalance = flag;
	}
	return getImpl().useWhiteBalance;
}


std::vector<int>& OctGlobal::GlobalSettings::levelCorrectionCoordsX(void)
{
	return getImpl().correctXs;
}


std::vector<int>& OctGlobal::GlobalSettings::levelCorrectionCoordsY(void)
{
	return getImpl().correctYs;
}

std::vector<int>& OctGlobal::GlobalSettings::levelCorrectionCoordsX_FILR(void)
{
	return getImpl().correctXs_FILR;
}


std::vector<int>& OctGlobal::GlobalSettings::levelCorrectionCoordsY_FILR(void)
{
	return getImpl().correctYs_FILR;
}

bool OctGlobal::GlobalSettings::useFundusLevelCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useLevelCorrection = flag;
	}
	return getImpl().useLevelCorrection;
}


std::pair<int, int> OctGlobal::GlobalSettings::startOfFundusROI(void)
{
	auto center = centerOfFundusROI();
	auto radius = radiusOfFundusROI();
	center.first = max(center.first - radius, 0);
	center.second = max(center.second - radius, 0);
	return center;
}


std::pair<int, int> OctGlobal::GlobalSettings::closeOfFundusROI(void)
{
	auto center = centerOfFundusROI();
	auto radius = radiusOfFundusROI();
	if (GlobalSettings::useFundusFILR_Enable()) {
		center.first = min(center.first + radius, FUNDUS_FILR_FRAME_WIDTH - 1);
		center.second = min(center.second + radius, FUNDUS_FILR_FRAME_HEIGHT - 1);
	}
	else {
		center.first = min(center.first + radius, FUNDUS_FILR_FRAME_WIDTH - 1);
		center.second = min(center.second + radius, FUNDUS_FILR_FRAME_HEIGHT - 1);
	}
	return center;
}


std::pair<int, int>& OctGlobal::GlobalSettings::centerOfFundusROI(void)
{
	return getImpl().centerFundusROI;
}


int& OctGlobal::GlobalSettings::radiusOfFundusROI(void)
{
	return getImpl().radiusFundusROI;
}

int& OctGlobal::GlobalSettings::removeReflectionLight1(void)
{
	return getImpl().removeReflectionLisht1;
}

int& OctGlobal::GlobalSettings::removeReflectionLight2(void)
{
	return getImpl().removeReflectionLisht2;
}

int& OctGlobal::GlobalSettings::removeReflectionLight3(void)
{
	return getImpl().removeReflectionLisht3;
}

bool OctGlobal::GlobalSettings::useFundusROI(bool isset, bool flag)
{
	if (isset) {
		getImpl().useFundusROI = flag;
	}
	return getImpl().useFundusROI;
}


std::pair<int, int>& OctGlobal::GlobalSettings::centerOfRetinaROI(void)
{
    return getImpl().centerRetinaROI;
}


std::pair<int, int>& OctGlobal::GlobalSettings::centerOfRetinaMask(void)
{
	return getImpl().centerRetinaMask;
}


int& OctGlobal::GlobalSettings::radiusOfRetinaROI(void)
{
    return getImpl().radiusRetinaROI;
}

int & OctGlobal::GlobalSettings::retinaCenterMaskSize(void)
{
	return getImpl().retinaCenterMaskSize;
}


bool OctGlobal::GlobalSettings::useRetinaROI(bool isset, bool flag)
{
    if (isset) {
        getImpl().useRetinaROI = flag;
    }
    return getImpl().useRetinaROI;
}

bool OctGlobal::GlobalSettings::useRetinaCenterMask(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRetinaCenterMask = flag;
	}
	return getImpl().useRetinaCenterMask;
}

int & OctGlobal::GlobalSettings::corneaPixelsPerMM(void)
{
	return getImpl().corneaPixelsPerMM;
}

float & OctGlobal::GlobalSettings::corneaSmallPupilSize(void)
{
	return getImpl().corneaSmallPupilSize;
}

int & OctGlobal::GlobalSettings::autoFlashLevelNormal(void)
{
	return getImpl().flashLevelNormal;
}


int & OctGlobal::GlobalSettings::autoFlashLevelMin(void)
{
	return getImpl().flashLevelMin;
}


int & OctGlobal::GlobalSettings::autoFlashLevelMax(void)
{
	return getImpl().flashLevelMax;
}

int & OctGlobal::GlobalSettings::flashPresetSizeBase(int index)
{
	return getImpl().flashSizeBases[index];
}

int & OctGlobal::GlobalSettings::flashPresetSizeHigh(int index)
{
	return getImpl().flashSizeHighs[index];
}

int & OctGlobal::GlobalSettings::flashPresetFocusInt(int index)
{
	return getImpl().flashFocusInts[index];
}

int & OctGlobal::GlobalSettings::flashPresetDefLevel(int index)
{
	return getImpl().flashDefLevels[index];
}


bool OctGlobal::GlobalSettings::useAutoFlashLevel(bool isset, bool flag)
{
	if (isset) {
		getImpl().useAutoFlash = flag;
	}
	return getImpl().useAutoFlash;
}

int OctGlobal::GlobalSettings::getRetinaCameraPreset(void)
{
	return getImpl().retinaCameraPreset;
}

void OctGlobal::GlobalSettings::setRetinaCameraPreset(int index)
{
	getImpl().retinaCameraPreset = min(max(index, 0), 2);
	return;
}

int OctGlobal::GlobalSettings::getModelType(void)
{
	return getImpl().modleType;
}

int OctGlobal::GlobalSettings::getOctGrabberType(void)
{
	return getImpl().octGrabberType;
}

int OctGlobal::GlobalSettings::getOctUsbLineCameraMode(void)
{
	return getImpl().octLineCameraMode;
}

int OctGlobal::GlobalSettings::getRetinaTrackingSpeed(void)
{
	return getImpl().retinaTrackSpeed;
}

int OctGlobal::GlobalSettings::getSldFaultDetection(void)
{
	return getImpl().sldFaultDetection;
}

int OctGlobal::GlobalSettings::getSldFaultTimer(void)
{
	return getImpl().sldFaultTimer;
}

int OctGlobal::GlobalSettings::getSldFaultThreshold(void)
{
	return getImpl().sldFaultThreshold;
}

bool OctGlobal::GlobalSettings::isSldFaultDetectionEnabled(void)
{
	auto value = getSldFaultDetection();
	return (value == SLD_FAULT_DETECTION_ENABLED);
}

int OctGlobal::GlobalSettings::getTriggerForePadd(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().triggerForePadd[index];
	}
	else {
		return TRIGGER_FORE_PADDING_POINTS_AT_FASTEST;
	}

}

int OctGlobal::GlobalSettings::getTriggerPostPadd(int index)
{
	if (index >= 0 && index < 3) {
		return getImpl().triggerPostPadd[index];
	}
	else {
		return TRIGGER_POST_PADDING_POINTS_AT_FASTEST;
	}
}

void OctGlobal::GlobalSettings::setModelType(int index)
{
	getImpl().modleType = index;
	return;
}

void OctGlobal::GlobalSettings::setOctGrabberType(int type)
{
	getImpl().octGrabberType = type;
	return;
}

void OctGlobal::GlobalSettings::setOctUsbLineCameraMode(int mode)
{
	getImpl().octLineCameraMode = mode;
	return;
}

void OctGlobal::GlobalSettings::setRetinaTrackingSpeed(int speed)
{
	getImpl().retinaTrackSpeed = speed;
	return;
}

void OctGlobal::GlobalSettings::setSldFaultDetection(int value)
{
	getImpl().sldFaultDetection = value;
	return;
}

void OctGlobal::GlobalSettings::setSldFaultTimer(int value)
{
	getImpl().sldFaultTimer = value;
	return;
}

void OctGlobal::GlobalSettings::setSldFaultThreshold(int value)
{
	getImpl().sldFaultThreshold = value;
	return;
}

bool OctGlobal::GlobalSettings::useSldFaultOverride(bool isset, bool flag)
{
	if (isset) {
		getImpl().sldFaultOverride = flag;
	}
	return getImpl().sldFaultOverride;
}

void OctGlobal::GlobalSettings::setTriggerForePadd(int index, int value)
{
	if (index >= 0 && index < 3) {
		getImpl().triggerForePadd[index] = value;
	}
	return;
}

void OctGlobal::GlobalSettings::setTriggerPostPadd(int index, int value)
{
	if (index >= 0 && index < 3) {
		getImpl().triggerPostPadd[index] = value;
	}
	return;
}

bool OctGlobal::GlobalSettings::isOctUsbLineCamera68k(void)
{
	return (getImpl().octLineCameraMode == (int)OctLineCameraMode::LINE_CAMERA_68K);
}

bool OctGlobal::GlobalSettings::isOctUsbLineCamera80k(void)
{
	return (getImpl().octLineCameraMode == (int)OctLineCameraMode::LINE_CAMERA_80K);
}

bool OctGlobal::GlobalSettings::isOctUsbLineCamera80kTest(void)
{
	return (getImpl().octLineCameraMode == (int)OctLineCameraMode::LINE_CAMERA_80K_TEST);
}

bool OctGlobal::GlobalSettings::isOctUsbLineCamera120k(void)
{
	return (getImpl().octLineCameraMode == (int)OctLineCameraMode::LINE_CAMERA_120K);
}

bool OctGlobal::GlobalSettings::isRetinaTracking30fps(void)
{
	return (getImpl().retinaTrackSpeed == (int)RetinaTrackingSpeed::TRACK_CAMERA_30FPS);
}

bool OctGlobal::GlobalSettings::isRetinaTracking15fps(void)
{
	return (getImpl().retinaTrackSpeed == (int)RetinaTrackingSpeed::TRACK_CAMERA_15FPS);
}

float OctGlobal::GlobalSettings::getOctUsbTriggerTimeStep(ScanSpeed speed)
{
	if (speed == ScanSpeed::Normal) {
		if (isOctUsbLineCamera120k()) {
			return TRIGGER_TIME_STEP_AT_NORMAL_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k()) {
			return TRIGGER_TIME_STEP_AT_NORMAL_SPEED_080K;
		}
		else {
			return TRIGGER_TIME_STEP_AT_NORMAL_SPEED_068K;
		}
	}
	else if (speed == ScanSpeed::Faster) {
		if (isOctUsbLineCamera120k()) {
			return TRIGGER_TIME_STEP_AT_FASTER_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k()) {
			return TRIGGER_TIME_STEP_AT_FASTER_SPEED_080K;
		}
		else {
			return TRIGGER_TIME_STEP_AT_FASTER_SPEED_068K;
		}
	}
	else {
		if (isOctUsbLineCamera120k()) {
			return TRIGGER_TIME_STEP_AT_FASTEST_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k() || isOctUsbLineCamera80kTest()) {
			return TRIGGER_TIME_STEP_AT_FASTEST_SPEED_080K;
		}
		else {
			return TRIGGER_TIME_STEP_AT_FASTEST_SPEED_068K;
		}
	}
}

float OctGlobal::GlobalSettings::getOctUsbExposureTime(ScanSpeed speed)
{
	if (speed == ScanSpeed::Normal) {
		if (isOctUsbLineCamera120k()) {
			return EXPOSURE_TIME_AT_NORMAL_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k()) {
			return EXPOSURE_TIME_AT_NORMAL_SPEED_080K;
		}
		else {
			return EXPOSURE_TIME_AT_NORMAL_SPEED_068K;
		}
	}
	else if (speed == ScanSpeed::Faster) {
		if (isOctUsbLineCamera120k()) {
			return EXPOSURE_TIME_AT_FASTER_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k()) {
			return EXPOSURE_TIME_AT_FASTER_SPEED_080K;
		}
		else {
			return EXPOSURE_TIME_AT_FASTER_SPEED_068K;
		}
	}
	else {
		if (isOctUsbLineCamera120k()) {
			return EXPOSURE_TIME_AT_FASTEST_SPEED_120K;
		}
		else if (isOctUsbLineCamera80k() || isOctUsbLineCamera80kTest()) {
			return EXPOSURE_TIME_AT_FASTEST_SPEED_080K;
		}
		else {
			return EXPOSURE_TIME_AT_FASTEST_SPEED_068K;
		}
	}
}

bool OctGlobal::GlobalSettings::isOctUsbCameraGrabberType(void)
{
	return getImpl().octGrabberType == (int)OctGrabberType::USB_LINE_CAMERA;
}

bool OctGlobal::GlobalSettings::isOctMilFrameGrabberType(void)
{
	return !isOctUsbCameraGrabberType();
}

float & OctGlobal::GlobalSettings::fundusAutoBrightness(void)
{
	return getImpl().autoBright;
}

float & OctGlobal::GlobalSettings::fundusAutoContrast(void)
{
	return getImpl().autoContrast;
}

float & OctGlobal::GlobalSettings::fundusAutoUB(void)
{
	return getImpl().autoUB;
}

float & OctGlobal::GlobalSettings::fundusAutoVR(void)
{
	return getImpl().autoVR;
}

int & OctGlobal::GlobalSettings::fundusEdgeKernelSize(void)
{
	return getImpl().edgeKernelSize;
}

float & OctGlobal::GlobalSettings::fundusRadialCorrectionRatio(void)
{
	return getImpl().radialRatio;
}

bool OctGlobal::GlobalSettings::useFundusRadialCorrection(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRadialCorrection = flag;
	}
	return getImpl().useRadialCorrection;
}


std::pair<int, int>& OctGlobal::GlobalSettings::splitFocusCenter(void)
{
	return getImpl().splitCenter;
}


std::pair<int, int>& OctGlobal::GlobalSettings::splitFocusKernel(void)
{
	return getImpl().splitKernel;
}


bool OctGlobal::GlobalSettings::useRetinaImageEnhance(bool isset, bool flag)
{
	if (isset) {
		getImpl().useRetinaImageEnhance = flag;
	}
	return getImpl().useRetinaImageEnhance;
}


float & OctGlobal::GlobalSettings::retinaImageClipLimit(void)
{
	return getImpl().retinaClipLimit;
}


int & OctGlobal::GlobalSettings::retinaImageClipScalar(void)
{
	return getImpl().retinaClipScalar;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationCenterOD(void)
{
	return getImpl().centerOD;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationFundusOD(void)
{
	return getImpl().fundusOD;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationScanDiskOD(void)
{
	return getImpl().scanDiskOD;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationLeftSideOD(int index)
{
	return getImpl().leftSideOD[index];
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationRightSideOD(int index)
{
	return getImpl().rightSideOD[index];
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationCenterOS(void)
{
	return getImpl().centerOS;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationFundusOS(void)
{
	return getImpl().fundusOS;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationScanDiskOS(void)
{
	return getImpl().scanDiskOS;
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationLeftSideOS(int index)
{
	return getImpl().leftSideOS[index];
}


std::pair<int, int>& OctGlobal::GlobalSettings::fixationRightSideOS(int index)
{
	return getImpl().rightSideOS[index];
}


bool OctGlobal::GlobalSettings::useLcdFixation(bool isset, bool flag)
{
	if (isset) {
		getImpl().useLcdFixation = flag;
	}
	return getImpl().useLcdFixation;
}


bool OctGlobal::GlobalSettings::useLcdBlinkOn(bool isset, bool flag)
{
	if (isset) {
		getImpl().useLcdBlinkOn = flag;
	}
	return getImpl().useLcdBlinkOn;
}


int & OctGlobal::GlobalSettings::fixationBlinkPeriod(void)
{
	return getImpl().fixBlinkPeriod;
}


int & OctGlobal::GlobalSettings::fixationBlinkOnTime(void)
{
	return getImpl().fixBlinkOnTime;
}


int & OctGlobal::GlobalSettings::fixationBrightness(void)
{
	return getImpl().fixBrightness;
}


int & OctGlobal::GlobalSettings::fixationType(void)
{
	return getImpl().fixationType;
}


std::pair<int, int>& OctGlobal::GlobalSettings::enfacePreviewRange(void)
{
	return getImpl().enfaceRange;
}

std::pair<float, float>& OctGlobal::GlobalSettings::retinaPatternScale(int index)
{
	return getImpl().retinaPatternScales[index];
}

std::pair<float, float>& OctGlobal::GlobalSettings::corneaPatternScale(int index)
{
	return getImpl().corneaPatternScales[index];
}

std::pair<float, float>& OctGlobal::GlobalSettings::topographyPatternScale(int index)
{
	return getImpl().topographyPatternScales[index];
}

std::pair<float, float>& OctGlobal::GlobalSettings::retinaPatternOffset(int index)
{
	return getImpl().retinaPatternOffsets[index];
}

std::pair<float, float>& OctGlobal::GlobalSettings::corneaPatternOffset(int index)
{
	return getImpl().corneaPatternOffsets[index];
}

std::pair<float, float>& OctGlobal::GlobalSettings::topographyPatternOffset(int index)
{
	return getImpl().topographyPatternOffsets[index];
}


int & OctGlobal::GlobalSettings::checkMainboardAtStartup(void)
{
	return d_ptr->checkMainboard;
}

void OctGlobal::GlobalSettings::setRetinaScanAxialResolution(double resol)
{
	if (resol < 0.0 || resol > 10.0) {
		resol = RETINA_SCAN_PIXEL_RESOLUTION;
	}
	getImpl().retinaPixResol = resol;
	return;
}

void OctGlobal::GlobalSettings::setCorneaScanAxialResolution(double resol)
{
	if (resol < 0.0 || resol > 10.0) {
		resol = CORNEA_SCAN_PIXEL_RESOLUTION;
	}
	getImpl().corneaPixResol = resol;
	return;
}

void OctGlobal::GlobalSettings::setInAirScanAxialResolution(double resol)
{
	if (resol < 0.0 || resol > 10.0) {
		resol = INAIR_SCAN_PIXEL_RESOLUTION;
	}
	getImpl().inAirPixResol = resol;
	return;
}

double OctGlobal::GlobalSettings::getRetinaScanAxialResolution(void)
{
	return getImpl().retinaPixResol;
}

double OctGlobal::GlobalSettings::getCorneaScanAxialResolution(void)
{
	return getImpl().corneaPixResol;
}

double OctGlobal::GlobalSettings::getInAirScanAxialResolution(void)
{
	return getImpl().inAirPixResol;
}

GlobalSettings::GlobalSettingsImpl & OctGlobal::GlobalSettings::getImpl(void)
{
	return *d_ptr;
}

std::vector<double>& OctGlobal::GlobalSettings::topoCalibrationData(int index)
{
	return getImpl().topoCalibrationData[index];
}

std::vector<double>& OctGlobal::GlobalSettings::topoModelData(void)
{
	return getImpl().topoModelR;
}

std::vector<double>& OctGlobal::GlobalSettings::topoMeasuredData(void)
{
	return getImpl().topoMeasuredR;
}




// measure fundus

void OctGlobal::GlobalSettings::setMeasureFundusDomainTransform(bool isDomainTransform)
{
	getImpl().bDomainTransform = isDomainTransform;
}

void OctGlobal::GlobalSettings::setMeasureFundusDomainParam(float value)
{
	getImpl().fDomainParam = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusWhiteBalance(bool isWhiteBalance)
{
	getImpl().bWhiteBalance = isWhiteBalance;
}

void OctGlobal::GlobalSettings::setMeasureFundusWhiteBalanceR(int value)
{
	getImpl().nWhiteBalanceR = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusWhiteBalanceG(int value)
{
	getImpl().nWhiteBalanceG = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusWhiteBalanceB(int value)
{
	getImpl().nWhiteBalanceB = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusCentralBrightness(bool isCentralBrightness)
{
	getImpl().bCentralBrightness = isCentralBrightness;
}

void OctGlobal::GlobalSettings::setMeasureFundusBrightness(float value)
{
	getImpl().fBrightness = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusBrightnessGamma(float value)
{
	getImpl().fBrightnessGamma = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusCosineFallOff(bool isCosineFallOff)
{
	getImpl().bCosineFallOff = isCosineFallOff;
}

void OctGlobal::GlobalSettings::setMeasureFundusInner(int value)
{
	getImpl().nInner = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusOuter(int value)
{
	getImpl().nOutter = value;
}

void OctGlobal::GlobalSettings::setMeasureFundusFactor(float value)
{
	getImpl().fFactor = value;
}

bool OctGlobal::GlobalSettings::isMeasureFundusDomainTransform()
{
	return getImpl().bDomainTransform;
}

float OctGlobal::GlobalSettings::getMeasureFundusDomainParam()
{
	return getImpl().fDomainParam;
}

bool OctGlobal::GlobalSettings::isMeasureFundusWhiteBalance()
{
	return getImpl().bWhiteBalance;
}

int OctGlobal::GlobalSettings::getMeasureFundusWhiteBalanceR()
{
	return getImpl().nWhiteBalanceR;
}

int OctGlobal::GlobalSettings::getMeasureFundusWhiteBalanceG()
{
	return getImpl().nWhiteBalanceG;
}

int OctGlobal::GlobalSettings::getMeasureFundusWhiteBalanceB()
{
	return getImpl().nWhiteBalanceB;
}

bool OctGlobal::GlobalSettings::isMeasureFundusCentralBrightness()
{
	return getImpl().bCentralBrightness;
}

float OctGlobal::GlobalSettings::getMeasureFundusBrightness()
{
	return getImpl().fBrightness;
}

float OctGlobal::GlobalSettings::getMeasureFundusBrightnessGamma()
{
	return getImpl().fBrightnessGamma;
}

bool OctGlobal::GlobalSettings::isMeasureFundusCosineFallOff()
{
	return getImpl().bCosineFallOff;
}

int OctGlobal::GlobalSettings::getMeasureFundusInner()
{
	return getImpl().nInner;
}

int OctGlobal::GlobalSettings::getMeasureFundusOuter()
{
	return getImpl().nOutter;
}

float OctGlobal::GlobalSettings::getMeasureFundusFactor()
{
	return getImpl().fFactor;
}