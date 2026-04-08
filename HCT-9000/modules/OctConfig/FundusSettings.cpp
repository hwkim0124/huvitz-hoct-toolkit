#include "stdafx.h"
#include "FundusSettings.h"


using namespace OctConfig;
using namespace std;



struct FundusSettings::FundusSettingsImpl
{
	vector<double> awbParams;
	vector<int> correctXs;
	vector<int> correctYs;
	vector<int> correctXs_FILR;
	vector<int> correctYs_FILR;
	std::pair<int, int> centerROI;
	int radiusROI;

	int flashLevelNormal;
	int flashLevelMin;
	int flashLevelMax;

	int flashDefLevels[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashSizeBases[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashSizeHighs[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashFocusInts[AUTO_FLASH_LEVEL_PRESET_SIZE];

	int flashSplitFocus[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashRetinaIr[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashWorkDot1[AUTO_FLASH_LEVEL_PRESET_SIZE];
	int flashWorkDot2[AUTO_FLASH_LEVEL_PRESET_SIZE];

	float radialRatio;
	float autoBright;
	float autoContrast;
	float autoUB;
	float autoVR;

	int edgeKernelSize;
	int fundusImageType;

	int removeReflection1;
	int removeReflection2;
	int removeReflection3;

	bool useColorCorrect;
	bool useWhiteBalance;
	bool useROICorrds;
	bool useAutoFlash;
	bool useRadialCorrect;
	bool useRemoveReflection;

	FundusSettingsImpl() {
	}
};


FundusSettings::FundusSettings() :
	d_ptr(make_unique<FundusSettingsImpl>())
{
	initialize();
}


OctConfig::FundusSettings::~FundusSettings() = default;
OctConfig::FundusSettings::FundusSettings(FundusSettings && rhs) = default;
FundusSettings & OctConfig::FundusSettings::operator=(FundusSettings && rhs) = default;


OctConfig::FundusSettings::FundusSettings(const FundusSettings & rhs)
	: d_ptr(make_unique<FundusSettingsImpl>(*rhs.d_ptr))
{
}


FundusSettings & OctConfig::FundusSettings::operator=(const FundusSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::FundusSettings::initialize(void)
{
	getImpl().awbParams.assign(FUNDUS_AWB_PARAMS_SIZE, 0.0);
	getImpl().correctXs.assign(FUNDUS_RADIAL_CORRECT_PARAMS_SIZE, 0);
	getImpl().correctYs.assign(FUNDUS_RADIAL_CORRECT_PARAMS_SIZE, 0);

	getImpl().correctXs_FILR.assign(FUNDUS_FILR_RADIAL_CORRECT_PARAMS_SIZE, 0);
	getImpl().correctYs_FILR.assign(FUNDUS_FILR_RADIAL_CORRECT_PARAMS_SIZE, 0);

	resetToDefaultValues();
	return;
}


void OctConfig::FundusSettings::resetToDefaultValues(void)
{
	getImpl().correctXs[0] = FUNDUS_RADIAL_CORRECT_INIT_PARAM1_X;
	getImpl().correctXs[1] = FUNDUS_RADIAL_CORRECT_INIT_PARAM2_X;
	getImpl().correctXs[2] = FUNDUS_RADIAL_CORRECT_INIT_PARAM3_X;
	getImpl().correctXs[3] = FUNDUS_RADIAL_CORRECT_INIT_PARAM4_X;

	getImpl().correctYs[0] = FUNDUS_RADIAL_CORRECT_INIT_PARAM1_Y;
	getImpl().correctYs[1] = FUNDUS_RADIAL_CORRECT_INIT_PARAM2_Y;
	getImpl().correctYs[2] = FUNDUS_RADIAL_CORRECT_INIT_PARAM3_Y;
	getImpl().correctYs[3] = FUNDUS_RADIAL_CORRECT_INIT_PARAM4_Y;

	getImpl().correctXs_FILR[0] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM1_X;
	getImpl().correctXs_FILR[1] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM2_X;
	getImpl().correctXs_FILR[2] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM3_X;
	getImpl().correctXs_FILR[3] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM4_X;
	getImpl().correctXs_FILR[4] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM5_X;
	getImpl().correctXs_FILR[5] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM6_X;
	getImpl().correctXs_FILR[6] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM7_X;

	getImpl().correctYs_FILR[0] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM1_Y;
	getImpl().correctYs_FILR[1] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM2_Y;
	getImpl().correctYs_FILR[2] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM3_Y;
	getImpl().correctYs_FILR[3] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM4_Y;
	getImpl().correctYs_FILR[4] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM5_Y;
	getImpl().correctYs_FILR[5] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM6_Y;
	getImpl().correctYs_FILR[6] = FUNDUS_FILR_RADIAL_CORRECT_INIT_PARAM7_Y;

	if (OctGlobal::GlobalSettings::useFundusFILR_Enable()) {
		getImpl().awbParams[0] = FUNDUS_FILR_AWB_INIT_PARAM1;
		getImpl().awbParams[1] = FUNDUS_FILR_AWB_INIT_PARAM2;

		getImpl().centerROI = std::pair<int, int>(FUNDUS_FILR_ROI_INIT_CENTER_X, FUNDUS_FILR_ROI_INIT_CENTER_Y);
		getImpl().radiusROI = FUNDUS_FILR_ROI_INIT_RADIUS;

		getImpl().removeReflection1 = FUNDUS_FILR_REMOVE_REFLECTION_INIT_LIGHT;
		getImpl().removeReflection2 = FUNDUS_FILR_REMOVE_REFLECTION_INIT_SHADOW;
		getImpl().removeReflection3 = FUNDUS_FILR_REMOVE_REFLECTION_INIT_OUTER;

	}
	else {
		getImpl().awbParams[0] = FUNDUS_AWB_INIT_PARAM1;
		getImpl().awbParams[1] = FUNDUS_AWB_INIT_PARAM2;

		getImpl().centerROI = std::pair<int, int>(FUNDUS_ROI_INIT_CENTER_X, FUNDUS_ROI_INIT_CENTER_Y);
		getImpl().radiusROI = FUNDUS_ROI_INIT_RADIUS;

		getImpl().removeReflection1 = FUNDUS_REMOVE_REFLECTION_INIT_LIGHT;
		getImpl().removeReflection2 = FUNDUS_REMOVE_REFLECTION_INIT_SHADOW;
		getImpl().removeReflection3 = FUNDUS_REMOVE_REFLECTION_INIT_OUTER;
	}

	getImpl().flashLevelMax = 30;
	getImpl().flashLevelMin = 30;
	getImpl().flashLevelNormal = 60;

	for (int i = 0; i < AUTO_FLASH_LEVEL_PRESET_SIZE; i++) {
		getImpl().flashSizeBases[i] = FlashPresetSizeBases[i];
		getImpl().flashSizeHighs[i] = FlashPresetSizeHighs[i];
		getImpl().flashFocusInts[i] = FlashPresetFocusInts[i];
		getImpl().flashDefLevels[i] = FlashPresetDefLevels[i];

		getImpl().flashSplitFocus[i] = LED_SPLIT_FOCUS_INIT_VALUE;
		getImpl().flashWorkDot1[i] = LED_WORKING_DOTS_INIT_VALUE;
		getImpl().flashWorkDot2[i] = LED_WORKING_DOTS_INIT_VALUE;
		getImpl().flashRetinaIr[i] = LED_RETINA_IR_INIT_VALUE;
	}

	getImpl().autoBright = -50.0f;
	getImpl().autoContrast = 20.0f;
	getImpl().autoUB = 0.75f;
	getImpl().autoVR = 1.29f;
	getImpl().radialRatio = 1.0f;
	getImpl().edgeKernelSize = 19;
	getImpl().fundusImageType = 1;

	getImpl().useColorCorrect = true;
	getImpl().useWhiteBalance = true;
	getImpl().useROICorrds = true;
	getImpl().useAutoFlash = false;
	getImpl().useRadialCorrect = true;
	getImpl().useRemoveReflection = true;

	return;
}


std::vector<double> OctConfig::FundusSettings::getWhiteBalanceParameters(void) const
{
	return getImpl().awbParams;
}


std::vector<int> OctConfig::FundusSettings::getCorrectionCoordsX(void) const
{
	return getImpl().correctXs;
}

std::vector<int> OctConfig::FundusSettings::getCorrectionCoordsY(void) const
{
	return getImpl().correctYs;
}

std::vector<int> OctConfig::FundusSettings::getCorrectionCoordsX_FILR(void) const
{
	return getImpl().correctXs_FILR;
}

std::vector<int> OctConfig::FundusSettings::getCorrectionCoordsY_FILR(void) const
{
	return getImpl().correctYs_FILR;
}

std::pair<int, int> OctConfig::FundusSettings::getCenterPointOfROI(void) const
{
	return getImpl().centerROI;
}


int OctConfig::FundusSettings::getRadiusOfROI(void) const
{
	return getImpl().radiusROI;
}

int OctConfig::FundusSettings::getAutoFlashLevelNormal(void) const
{
	return getImpl().flashLevelNormal;
}

int OctConfig::FundusSettings::getAutoFlashLevelMin(void) const
{
	return getImpl().flashLevelMin;
}

int OctConfig::FundusSettings::getAutoFlashLevelMax(void) const
{
	return getImpl().flashLevelMax;
}

int OctConfig::FundusSettings::getAutoFlashSizeBase(int index) const
{
	return getImpl().flashSizeBases[index];
}

int OctConfig::FundusSettings::getAutoFlashSizeHigh(int index) const
{
	return getImpl().flashSizeHighs[index];
}

int OctConfig::FundusSettings::getAutoFlashFocusInt(int index) const
{
	return getImpl().flashFocusInts[index];
}

int OctConfig::FundusSettings::getAutoFlashDefLevel(int index) const
{
	return getImpl().flashDefLevels[index];
}

int OctConfig::FundusSettings::getAutoFlashSplitFocus(int index) const
{
	return getImpl().flashSplitFocus[index];
}

int OctConfig::FundusSettings::getAutoFlashRetinaIr(int index) const
{
	return getImpl().flashRetinaIr[index];
}

int OctConfig::FundusSettings::getAutoFlashWorkDot1(int index) const
{
	return getImpl().flashWorkDot1[index];
}

int OctConfig::FundusSettings::getAutoFlashWorkDot2(int index) const
{
	return getImpl().flashWorkDot2[index];
}

int OctConfig::FundusSettings::getRemoveReflection1(void) const
{
	return getImpl().removeReflection1;
}

int OctConfig::FundusSettings::getRemoveReflection2(void) const
{
	return getImpl().removeReflection2;
}

int OctConfig::FundusSettings::getRemoveReflection3(void) const
{
	return getImpl().removeReflection3;
}

float OctConfig::FundusSettings::autoBright(void) const
{
	return getImpl().autoBright;
}

float OctConfig::FundusSettings::autoContrast(void) const
{
	return getImpl().autoContrast;
}

float OctConfig::FundusSettings::autoUB(void) const
{
	return getImpl().autoUB;
}

float OctConfig::FundusSettings::autoVR(void) const
{
	return getImpl().autoVR;
}

float OctConfig::FundusSettings::radialCorrectionRatio(void) const
{
	return getImpl().radialRatio;
}

int OctConfig::FundusSettings::edgeKernelSize(void) const
{
	return getImpl().edgeKernelSize;
}

int OctConfig::FundusSettings::fundusImageType(int index) const
{
	return getImpl().fundusImageType;
}

void OctConfig::FundusSettings::setWhiteBalanceParameters(std::vector<double> params)
{
	getImpl().awbParams = params;
	return;
}


void OctConfig::FundusSettings::setCorrectionCoordsX(std::vector<int> xs)
{
	getImpl().correctXs = xs;
	return;
}


void OctConfig::FundusSettings::setCorrectionCoordsY(std::vector<int> ys)
{
	getImpl().correctYs = ys;
	return;
}

void OctConfig::FundusSettings::setCorrectionCoordsX_FILR(std::vector<int> xs)
{
	getImpl().correctXs_FILR = xs;
	return;
}


void OctConfig::FundusSettings::setCorrectionCoordsY_FILR(std::vector<int> ys)
{
	getImpl().correctYs_FILR = ys;
	return;
}


void OctConfig::FundusSettings::setCenterPointOfROI(std::pair<int, int> point)
{
	getImpl().centerROI = point;
	return;
}


void OctConfig::FundusSettings::setRadiusOfROI(int radius)
{
	getImpl().radiusROI = radius;
	return;
}

void OctConfig::FundusSettings::setRemoveReflection1(int radius)
{
	getImpl().removeReflection1 = radius;
	return;
}

void OctConfig::FundusSettings::setRemoveReflection2(int radius)
{
	getImpl().removeReflection2 = radius;
	return;
}

void OctConfig::FundusSettings::setRemoveReflection3(int radius)
{
	getImpl().removeReflection3 = radius;
	return;
}

void OctConfig::FundusSettings::setAutoFlashLevelNormal(int level)
{
	getImpl().flashLevelNormal = min(max(level, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashLevelMin(int level)
{
	getImpl().flashLevelMin = min(max(level, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashLevelMax(int level)
{
	getImpl().flashLevelMax = min(max(level, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashSizeBase(int value, int index)
{
	getImpl().flashSizeBases[index] = min(max(value, FLASH_PRESET_SIZE_MIN), FLASH_PRESET_SIZE_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashSizeHigh(int value, int index)
{
	getImpl().flashSizeHighs[index] = min(max(value, FLASH_PRESET_SIZE_MIN), FLASH_PRESET_SIZE_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashFocusInt(int value, int index)
{
	getImpl().flashFocusInts[index] = min(max(value, FLASH_PRESET_INT_MIN), FLASH_PRESET_INT_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashDefLevel(int value, int index)
{
	getImpl().flashDefLevels[index] = min(max(value, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
	return;
}

void OctConfig::FundusSettings::setAutoFlashSplitFocus(int value, int index)
{
	getImpl().flashSplitFocus[index] = min(max(value, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
}

void OctConfig::FundusSettings::setAutoFlashRetinaIr(int value, int index)
{
	getImpl().flashRetinaIr[index] = min(max(value, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
}

void OctConfig::FundusSettings::setAutoFlashWorkDot1(int value, int index)
{
	getImpl().flashWorkDot1[index] = min(max(value, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
}

void OctConfig::FundusSettings::setAutoFlashWorkDot2(int value, int index)
{
	getImpl().flashWorkDot2[index] = min(max(value, FLASH_LEVEL_MIN), FLASH_LEVEL_MAX);
}

void OctConfig::FundusSettings::setAutoBright(float value)
{
	getImpl().autoBright = value;
	return;
}

void OctConfig::FundusSettings::setAutoContrast(float value)
{
	getImpl().autoContrast = value;
	return;
}

void OctConfig::FundusSettings::setAutoUB(float value)
{
	getImpl().autoUB = value;
	return;
}

void OctConfig::FundusSettings::setAutoVR(float value)
{
	getImpl().autoVR = value;
	return;
}

void OctConfig::FundusSettings::setRadialCorrectionRatio(float value)
{
	getImpl().radialRatio = value;
	return;
}

void OctConfig::FundusSettings::setEdgeKernelSize(int size)
{
	getImpl().edgeKernelSize = size;
	return;
}

void OctConfig::FundusSettings::setFundusImageType(int value)
{
	getImpl().fundusImageType = value;
	return;
}

bool OctConfig::FundusSettings::useWhiteBalance(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useWhiteBalance = flag;
	}
	return getImpl().useWhiteBalance;
}


bool OctConfig::FundusSettings::useColorCorrection(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useColorCorrect = flag;
	}
	return getImpl().useColorCorrect;
}


bool OctConfig::FundusSettings::useROICoordinates(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useROICorrds = flag;
	}
	return getImpl().useROICorrds;
}

bool OctConfig::FundusSettings::useAutoFlashLevel(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useAutoFlash = flag;
	}
	return getImpl().useAutoFlash;
}

bool OctConfig::FundusSettings::useRadialCorrection(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useRadialCorrect = flag;
	}
	return getImpl().useRadialCorrect;
}

bool OctConfig::FundusSettings::useRemoveReflectionLight(bool isset, bool flag) const
{
	if (isset) {
		getImpl().useRemoveReflection = flag;
	}
	return getImpl().useRemoveReflection;
}

FundusSettings::FundusSettingsImpl& OctConfig::FundusSettings::getImpl(void) const
{
	return *d_ptr;
}
