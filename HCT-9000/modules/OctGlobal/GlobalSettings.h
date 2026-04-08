#pragma once

#include "OctGlobalDef.h"
#include "GlobalMeasureDef.h"

#include <memory>
#include <vector>

namespace OctGlobal
{
	class OCTGLOBAL_DLL_API GlobalSettings
	{
	public:
		GlobalSettings();
		virtual ~GlobalSettings();

	public:
		static void applyUserModeSettings(bool flag);
		static void applyEngineerModeSettings(void);
		static bool isUserModeSettings(void);
		static bool isEngineerModeSettings(void);

		static float& angioDecorThreshold(void);
		static int& angioAverageOffset(void);
		static float& angioMotionThreshold(void);
		static float& angioMotionOverPoints(void);
		static float& angioMotionDistRatio(void);
		static int& angioFilterOrients(void);
		static float& angioFilterSigma(void);
		static float& angioFilterDivider(void);
		static float& angioFilterWeight(void);
		static float& angioEnhanceParam(void);
		static float& angioBiasFieldSigma(void);

		static bool useColorImageCorrection(bool isset = false, bool flag = false);
		static bool useColorImageResizing(bool isset = false, bool flag = false);
		static bool useEnfaceImageCorrection(bool isset = false, bool flag = false);
		static bool useOclTasksDefaultPreset(bool isset = false, bool flag = false);

		static bool useOctEnable(bool isset = false, bool flag = false);
		static bool useUsbCmosCameraEnable(bool isset = false, bool flag = false);
		static bool useFundusEnable(bool isset = false, bool flag = false);
		static bool useAngioEnable(bool isset = false, bool flag = false);
		static bool useFundusFILR_Enable(bool isset = false, bool flag = false);
		static bool useNewGpioRutine(bool isset = false, bool flag = false);
		static bool useRemoveLight(bool isset = false, bool flag = false);
		static bool useTopoCalibration(bool isset = false, bool flag = false);
		static bool useSaveTiff(bool isset = false, bool flag = false);

		static std::vector<double>& whiteBalanceParameters(void);
		static bool useWhiteBalance(bool isset = false, bool flag = false);

		static std::vector<int>& levelCorrectionCoordsX(void);
		static std::vector<int>& levelCorrectionCoordsY(void);
		static std::vector<int>& levelCorrectionCoordsX_FILR(void);
		static std::vector<int>& levelCorrectionCoordsY_FILR(void);
		static bool useFundusLevelCorrection(bool isset = false, bool flag = false);

		static std::pair<int, int> startOfFundusROI(void);
		static std::pair<int, int> closeOfFundusROI(void);
		static std::pair<int, int>& centerOfFundusROI(void);
		static int& radiusOfFundusROI(void);
		static int& removeReflectionLight1(void);
		static int& removeReflectionLight2(void);
		static int& removeReflectionLight3(void);
		static bool useFundusROI(bool isset = false, bool flag = false);

        static std::pair<int, int>& centerOfRetinaROI(void);
		static std::pair<int, int>& centerOfRetinaMask(void);
        static int& radiusOfRetinaROI(void);
		static int& retinaCenterMaskSize(void);
        static bool useRetinaROI(bool isset = false, bool flag = false);
		static bool useRetinaCenterMask(bool isset = false, bool flag = false);

		static int& corneaPixelsPerMM(void);
		static float& corneaSmallPupilSize(void);

		static int& autoFlashLevelNormal(void);
		static int& autoFlashLevelMin(void);
		static int& autoFlashLevelMax(void);
		static int& flashPresetSizeBase(int index = 0);
		static int& flashPresetSizeHigh(int index = 0);
		static int& flashPresetFocusInt(int index = 0);
		static int& flashPresetDefLevel(int index = 0);
		static bool useAutoFlashLevel(bool isset = false, bool flag = false);

		static int getRetinaCameraPreset(void);
		static void setRetinaCameraPreset(int preset);

		static int getModelType(void);
		static int getOctGrabberType(void);
		static int getOctUsbLineCameraMode(void);
		static int getRetinaTrackingSpeed(void);
		static int getSldFaultDetection(void);
		static int getSldFaultTimer(void);
		static int getSldFaultThreshold(void);
		static bool isSldFaultDetectionEnabled(void);
		static int getTriggerForePadd(int index);
		static int getTriggerPostPadd(int index);

		static void setModelType(int index);
		static void setOctGrabberType(int type);
		static void setOctUsbLineCameraMode(int mode);
		static void setRetinaTrackingSpeed(int speed);
		static void setSldFaultDetection(int value);
		static void setSldFaultTimer(int value);
		static void setSldFaultThreshold(int value);
		static bool useSldFaultOverride(bool isset = false, bool flag = false);
		static void setTriggerForePadd(int index, int value);
		static void setTriggerPostPadd(int index, int value);

		static bool isOctUsbLineCamera68k(void);
		static bool isOctUsbLineCamera80k(void);
		static bool isOctUsbLineCamera80kTest(void);
		static bool isOctUsbLineCamera120k(void);
		static bool isRetinaTracking30fps(void);
		static bool isRetinaTracking15fps(void);

		static float getOctUsbTriggerTimeStep(ScanSpeed speed = ScanSpeed::Fastest);
		static float getOctUsbExposureTime(ScanSpeed speed = ScanSpeed::Fastest);

		static bool isOctUsbCameraGrabberType(void);
		static bool isOctMilFrameGrabberType(void);

		static float& fundusAutoBrightness(void);
		static float& fundusAutoContrast(void);
		static float& fundusAutoUB(void);
		static float& fundusAutoVR(void);
		static int& fundusEdgeKernelSize(void);

		static float& fundusRadialCorrectionRatio(void);
		static bool useFundusRadialCorrection(bool isset = false, bool flag = false);

        static std::pair<int, int>& splitFocusCenter(void);
		static std::pair<int, int>& splitFocusKernel(void);
		static bool useRetinaImageEnhance(bool isset = false, bool flag = false);
		static float& retinaImageClipLimit(void);
		static int& retinaImageClipScalar(void);

		static std::pair<int, int>& fixationCenterOD(void);
		static std::pair<int, int>& fixationFundusOD(void);
		static std::pair<int, int>& fixationScanDiskOD(void);
		static std::pair<int, int>& fixationLeftSideOD(int index);
		static std::pair<int, int>& fixationRightSideOD(int index);

		static std::pair<int, int>& fixationCenterOS(void);
		static std::pair<int, int>& fixationFundusOS(void);
		static std::pair<int, int>& fixationScanDiskOS(void);
		static std::pair<int, int>& fixationLeftSideOS(int index);
		static std::pair<int, int>& fixationRightSideOS(int index);

		static bool useLcdFixation(bool isset = false, bool flag = false);
		static bool useLcdBlinkOn(bool isset = false, bool flag = false);
		static int& fixationBlinkPeriod(void);
		static int& fixationBlinkOnTime(void);
		static int& fixationBrightness(void);
		static int& fixationType(void);

		static std::pair<int, int>& enfacePreviewRange(void);

		static std::pair<float, float>& retinaPatternScale(int index);
		static std::pair<float, float>& corneaPatternScale(int index);
		static std::pair<float, float>& topographyPatternScale(int index);
		static std::pair<float, float>& retinaPatternOffset(int index);
		static std::pair<float, float>& corneaPatternOffset(int index);
		static std::pair<float, float>& topographyPatternOffset(int index);

		static int& checkMainboardAtStartup(void);

		static void setRetinaScanAxialResolution(double resol);
		static void setCorneaScanAxialResolution(double resol);
		static void setInAirScanAxialResolution(double resol);
		static double getRetinaScanAxialResolution(void);
		static double getCorneaScanAxialResolution(void);
		static double getInAirScanAxialResolution(void);
		static std::vector<double>& topoCalibrationData(int index);
		static std::vector<double>& topoModelData(void);
		static std::vector<double>& topoMeasuredData(void);


		//============================================================//
		// measure fundus //
		static void setMeasureFundusDomainTransform(bool isDomainTransform);
		static void setMeasureFundusDomainParam(float value);
		static void setMeasureFundusWhiteBalance(bool isWhiteBalance);
		static void setMeasureFundusWhiteBalanceR(int value);
		static void setMeasureFundusWhiteBalanceG(int value);
		static void setMeasureFundusWhiteBalanceB(int value);
		static void setMeasureFundusCentralBrightness(bool isCentralBrightness);
		static void setMeasureFundusBrightness(float value);
		static void setMeasureFundusBrightnessGamma(float value);
		static void setMeasureFundusCosineFallOff(bool isCosineFallOff);
		static void setMeasureFundusInner(int value);
		static void setMeasureFundusOuter(int value);
		static void setMeasureFundusFactor(float value);

		static bool isMeasureFundusDomainTransform();
		static float getMeasureFundusDomainParam();
		static bool isMeasureFundusWhiteBalance();
		static int getMeasureFundusWhiteBalanceR();
		static int getMeasureFundusWhiteBalanceG();
		static int getMeasureFundusWhiteBalanceB();
		static bool isMeasureFundusCentralBrightness();
		static float getMeasureFundusBrightness();
		static float getMeasureFundusBrightnessGamma();
		static bool isMeasureFundusCosineFallOff();
		static int getMeasureFundusInner();
		static int getMeasureFundusOuter();
		static float getMeasureFundusFactor();
		//============================================================//

	private:
		struct GlobalSettingsImpl;
		static std::unique_ptr<GlobalSettingsImpl> d_ptr;
		static GlobalSettingsImpl& getImpl(void);
	};
}
