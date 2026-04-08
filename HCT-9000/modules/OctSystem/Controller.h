#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice 
{
	class MainBoard;
	class MainPlatform;
}


namespace OctGrab
{
	class FrameGrabber;
	class Usb3Grabber;
}


namespace OctSystem
{
	class Scanner;
	class Camera;
	class CorneaAlign;
	class RetinaAlign;


	class OCTSYSTEM_DLL_API Controller
	{
	public:
		Controller();
		virtual ~Controller();

	public:
		static bool initializeController(void);
		static bool isInitiated(void);

		static void initiateMeasureMode(const OctPatient& patient);
		static void performPackagingMode(void);

		static bool moveScanFocusToDiopter(float diopt);
		static bool moveFundusFocusToDiopter(float diopt);
		static bool moveScanFocusToTopography(void);
		static bool movePolarizationToDegree(float degree);
		static bool moveScanPositionByOffset(int offset);
		static bool moveReferenceToOrigin(bool cornea, bool isTopo = false);

		static bool moveReferenceToPosition(int pos);
		static bool movePolarizationToPosition(int pos);

		static float getScanFocusDiopter(void);
		static float getScanFocusDiopterMax(void);
		static float getScanFocusDiopterMin(void);
		static float getFundusFocusDiopter(void);
		static float getFundusFocusDiopterOrigin(void);
		static float getTopographyFocusOrigin(void);
		static float getFundusFocusDiopterMax(void);
		static float getFundusFocusDiopterMin(void);
		static float getPolarizationDegree(void);

		static int getPolarizationPosition(void);
		static int getPolarizationMin(void);
		static int getPolarizationMax(void);

		static int getReferencePosition(void);
		static int getReferencePositionMin(void);
		static int getReferencePositionMax(void);

		static bool changeCompensationLens(CompensationLensMode mode);
		static bool changeOctDiopterLens(CompensationLensMode mode);
		static bool changeSmallPupilMode(bool flag);
		static bool isSmallPupilMode(void);
		static bool getCorneaPupilSize(float& pupSize, bool& isSmall);

		static bool turnOnSplitFocus(bool flag);
		static bool turnOnWorkingDots(bool flag);
		static bool turnOnRetinaIrLed(bool flag, bool bright=false);
		static bool setRetinaIrLedIntensity(int value);
		static int getRetinaIrLedIntensity(void);

		static bool turnOnInternalFixation(int row, int col);
		static bool turnOnInternalFixation(EyeSide side, FixationTarget target, bool isMacularWide = false);
		static bool turnOnInternalFixationAtCenter(EyeSide side = EyeSide::OD, EyeRegion region = EyeRegion::Macular, bool isFundus = false);
		static bool turnOffInternalFixation(void);
		static bool getInternalFixationOn(int& row, int& col);
		static bool getInternalFixationAtTarget(EyeSide side, FixationTarget target, int& row, int& col);
		static bool changeInternalFixationType(int type);

		static bool setFlashLightLevel(int level, bool autoFlash = false);
		static int getFlashLightLevel(void);
		static bool turnOnBacklight(bool flag);

		static bool isAtSideOd(void);
		static EyeSide getEyeSide(void);

		static bool isStageAtLeftEnd(void);
		static bool isStageAtRightEnd(void);
		static bool isStageAtUpperEnd(void);
		static bool isStageAtLowerEnd(void);
		static bool isStageAtFrontEnd(void);
		static bool isStageAtRearEnd(void);

		static bool startAutoDiopterFocus(void);
		static bool startAutoPolarization(void);
		static bool startAutoReference(void);

		static void cancelAutoDiopterFocus(void);
		static void cancelAutoPolarization(void);
		static void cancelAutoReference(void);

		static bool isAutoDiopterFocusing(void);
		static bool isAutoPolarizing(void);
		static bool isAutoReferencing(void);

		static bool startAutoPosition(AutoPositionCompletedCallback* callback = nullptr);
		static void cancelAutoPosition(void);
		static bool isAutoPositioning(void);

		static bool startAutoOptimize(bool noFundus, AutoOptimizeCompletedCallback* callback = nullptr, bool dioptFocus = false, bool topogrphyMode = false);
		static void cancelAutoOptimize(void);
		static bool isAutoOptimizing(void);

		static bool startAutoMeasure(AutoMeasureCompletedCallback* callback = nullptr, bool macular=true, bool cornea=true);
		static void cancelAutoMeasure(void);
		static bool isAutoMeasuring(void);
		
		static bool startAutoCorneaFocus(AutoCorneaFocusCompletedCallback* callback = nullptr, bool compenLens = false);
		static bool startAutoRetinaFocus(AutoRetinaFocusCompletedCallback* callback = nullptr, bool compenLens = false);
		static bool startAutoLensFrontFocus(AutoLensFrontFocusCompletedCallback* callback = nullptr, int corneaReferPos = -1, float corneaDioptPos = 0.0f);
		static bool startAutoLensBackFocus(AutoLensBackFocusCompletedCallback* callback = nullptr, int frontReferPos = -1, float frontDioptPos = 0.0f);

		static bool startAutoAxialLength(AutoCorneaFocusCompletedCallback* cornea, AutoRetinaFocusCompletedCallback* retina, int measureCount = 5, bool keepOnError = false);
		static bool startAutoLensThickness(AutoCorneaFocusCompletedCallback* cornea, AutoLensFrontFocusCompletedCallback* front, AutoLensBackFocusCompletedCallback* back, int measureCount = 5, bool keepOnError = false);
		static bool startAutoLensThickness2(AutoCorneaFocusCompletedCallback* cornea, AutoLensFrontFocusCompletedCallback* front, int measureCount = 5, bool keepOnError = false, bool referFixed = false);

		static void cancelAutoMeasureFocus(void);
		static bool isAutoMeasureFocusing(void);

		static void startAutoAlignment(CorneaAlignTarget target = CorneaAlignTarget::CENTER_LEFT);
		static void cancelAutoAlignment(void);
		static bool isAutoAligning(void);

		static void startAutoFundusFocus(void);
		static void cancelAutoFundusFocus(void);
		static bool isAutoFundusFocusing(void);

		static void cancelAutoControls(void);

		static bool startCorneaCamera(CorneaCameraImageCallback* callback = nullptr);
		static void closeCorneaCamera(void);
		static int getErrorCountCorneaCamera(void);
		static bool startRetinaCamera(RetinaCameraImageCallback* callback = nullptr);
		static void closeRetinaCamera(void);
		static int getErrorCountRetinaCamera(void);
		static bool resetIrCameras();

		static void applyRetinaCameraPreset(int preset);
		static int getCurrentRetinaCameraPreset(void);
		static int getAutoFlashDefaultLevel(int preset);
		static int getRetinaCameraEnhanceLevel(void);
		static void setRetinaCameraEnhanceLevel(int level, bool reset=false);

		static bool presetScanEnface(EnfaceImageCallback* cbEnfaceImage = nullptr,
								int numPoints = PATTERN_ENFACE_ASCAN_POINTS,
								int numLines = PATTERN_ENFACE_BSCAN_LINES,
								float rangeX = PATTERN_ENFACE_RANGE_X,
								float rangeY = PATTERN_ENFACE_RANGE_Y);

		static bool presetScanOffset(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
								float scaleX = 1.0f, float scaleY = 1.0f);

		static bool presetScanEnfaceAsDefault(void);
		static bool presetScanEnface(OctScanPattern pattern);

		static bool moveScanCenterPosition(float mmPosX, float mmPosY, bool isCornea = false);
		static void getScanCenterPosition(float& mmPosX, float& mmPosY);
	
		static bool startScan(EyeSide side, PatternDomain domain, PatternType type,
								int numPoints = 1024, int numLines = 1,
								float rangeX = 6.0f, float rangeY = 0.0f,
								int direction = 0, int overlaps = 1, float lineSpace = 0.0f,
								bool useEnface = false, bool usePattern = false, bool useFaster = false,
								PreviewImageCallback* cbPreviewImage = nullptr);

		static bool startScan(const OctScanMeasure& measure,
								PreviewImageCallback* cbPreviewImage = nullptr, 
								EnfaceImageCallback* cbEnfaceImage = nullptr, 
								bool clearPreset = false, bool retinaTrack = false);

		static bool startScan2(const OctScanMeasure& measure,
								PreviewImageCallback2* cbPreviewImage = nullptr,
								EnfaceImageCallback* cbEnfaceImage = nullptr,
								bool clearPreset = false, bool retinaTrack = false, bool ediScanning = false);

		static bool closeScan(bool measure, ScanPatternAcquiredCallback* cbAcquired = nullptr, 
								ScanPatternCompletedCallback* cbCompleted = nullptr, bool retinaTrack = false);

		static void cancelScanMeasure(void);

		static bool isScanning(void);
		static bool isScanGrabbing(void);
		static bool isScanProcessing(void);
		static bool isScanCancelling(void);

		static const OctScanMeasure& getScanMeasure(void);
		static const OctScanPattern& getScanPattern(void);
		static const OctScanPattern& getScanEnface(void);

		static bool takeColorFundus(const OctFundusMeasure& measure,
									FundusImageAcquiredCallback* cbAcquired = nullptr,
									FundusImageCompletedCallback* cbCompleted = nullptr,
									ColorCameraImageCallback* cbColorImage = nullptr,
									ColorCameraFrameCallback* cbColorFrame = nullptr, 
									bool imageProc = true);

		static void setCorneaCameraImageCallback(CorneaCameraImageCallback* callback);
		static void setRetinaCameraImageCallback(RetinaCameraImageCallback* callback);
		static void setColorCameraFrameCallback(ColorCameraFrameCallback* callback);
		static void setColorCameraImageCallback(ColorCameraImageCallback* callback);
		static void setColorCameraUsingLv4(bool enable);

		static void setJoystickEventCallback(JoystickEventCallback* callback);
		static void setEyeSideEventCallback(EyeSideEventCallback* callback);
		static void setOptimizeKeyEventCallback(OptimizeKeyEventCallback* callback);
		static void setBacklightOnEventCallback(BacklightOnEventCallback* callback);
		static void setSldWarningEventCallback(SldWarningEventCallback* callback);
		static void setScannerFaultEventCallback(ScannerFaultEventCallback* callback);

		static void setPreviewImageCallback(PreviewImageCallback* callback);
		static void setPreviewImageCallback2(PreviewImageCallback2* callback);
		static void setPreviewCorneaCallback(PreviewCorneaCallback* callback);

		static void setEnfaceImageCallback(EnfaceImageCallback* callback);
		static void setSpectrumDataCallback(SpectrumDataCallback* callback);
		static void setResampleDataCallback(ResampleDataCallback* callback);
		static void setIntensityDataCallback(IntensityDataCallback* callback);
				
		static OctDevice::MainBoard* getMainBoard(void);
		static OctDevice::MainPlatform * getMainPlatform(void);
		static OctGrab::FrameGrabber* getFrameGrabber(void);
		static OctGrab::Usb3Grabber* getUsb3Grabber(void);
		static OctSystem::Scanner* getScanner(void);
		static OctSystem::Camera* getCamera(void);
		static OctSystem::CorneaAlign* getCorneaAlign(void);
		static OctSystem::RetinaAlign* getRetinaAlign(void);

		static bool prepareLEDsForAnteriorScan(bool start);
		static bool prepareLEDsForRetinaTrack(bool start);

		static bool readyMotorPositionsToScan(bool init, bool anteriorOrigin = true);
		static bool readyMotorPositionsToFundus(bool init);

	protected:
		static bool prepareScan(void);
		static bool prepareScan(const OctScanMeasure& measure);
		static bool releaseScan(void);

		static bool initializeRetinaTrackMode(const OctScanMeasure & measure, bool retinaTrack);
		static void releaseRetinaTracking(void);
		static bool initializeCorneaTrack(const OctScanMeasure & measure);
		
		static bool moveInternalFixationToStartPattern(void);
		static bool moveInternalFixationToRetinaTrack(void);

		static bool changeScanSpeedToPreview(void);
		static bool changeScanSpeedToMeasure(void);
		static bool dispatchScanningCenter(bool clear);

		static void callbackDeviceStatus(void);

	private:
		struct ControllerImpl;
		static std::unique_ptr<ControllerImpl> d_ptr;
		static ControllerImpl& getImpl(void) ;
	};
}
