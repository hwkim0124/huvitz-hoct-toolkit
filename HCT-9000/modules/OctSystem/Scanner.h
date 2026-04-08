#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctPattern {
	class PatternPlan;
	class LineTrace;
	class PatternFrame;
}

namespace OctGrab {
	class FrameGrabber;
	class Usb3Grabber;
}

namespace OctDevice {
	class MainBoard;
	class Galvanometer;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API Scanner
	{
	public:
		Scanner();
		virtual ~Scanner();

	public:
		static bool initScanner(OctDevice::MainBoard* board, OctGrab::FrameGrabber* grabber);
		static bool initScanner(OctDevice::MainBoard* board, OctGrab::Usb3Grabber* grabber);

		static bool isInitiated(void);
		static void release(void);

		static bool startMeasure(EyeSide side, PatternDomain domain, PatternType type, int numPoints, int numLines = 1,
								float rangeX = 6.0f, float rangeY = 0.0f, int overlaps = 1, float lineSpace = 0.0f,
								bool useEnface = false, bool usePattern = false, bool useFaster = false);
		static bool startMeasure(OctScanMeasure& measure);
		static bool setupMeasure(OctScanMeasure& measure);
		static bool closePattern(bool measure);
		static void cancelMeasure(void);

		static bool checkMeasureDescript(OctScanMeasure& measure);
		static float getAcquisitionTime(void);

		static bool setupEnface(int numPoints = PATTERN_ENFACE_ASCAN_POINTS,
								int numLines = PATTERN_ENFACE_BSCAN_LINES,
								float rangeX = PATTERN_ENFACE_RANGE_X,
								float rangeY = PATTERN_ENFACE_RANGE_Y);

		static bool setupDispacement(float offsetX = 0.0f, float offsetY = 0.0f, float angle = 0.0f,
									 float scaleX = 0.0f, float scaleY = 0.0f);

		static bool changeScanSpeedToFastest(bool noImageGrab = false);
		static bool changeScanSpeedToFaster(bool noImageGrab = false);
		static bool changeScanSpeedToNormal(bool noImageGrab = false);
		static bool changeScanSpeedToCustom(bool noImageGrab = false);
		static bool isScanSpeedFastest(void);
		static bool isScanSpeedFaster(void);
		static bool isScanSpeedNormal(void);
		static bool isScanSpeedCustom(void);

		static void exportGalvanoPositions(std::string filename = "galv_points.txt");

		static bool isAnteriorMode(void);

		static float getCameraExposureTime(void);
		static void setCameraExposureTime(float expTime);
		static void setTriggerTimeStep(float timeStep);
		static void setTriggerTimeDelay(uint32_t timeDelay);
		static void setTriggerForePaddings(ScanSpeed speed, uint32_t padds);
		static void setTriggerPostPaddings(ScanSpeed speed, uint32_t padds);

		static float getCameraAnalogGain(void);
		static bool setCameraAnalogGain(float gain);

		static float getTriggerTimeStep(void);
		static uint32_t getTriggerTimeDelay(void);
		static uint32_t getTriggerForePaddings(ScanSpeed speed);
		static uint32_t getTriggerPostPaddings(ScanSpeed speed);

		static float getDefaultTriggerTimeStep(ScanSpeed speed);
		static float getDefaultCameraExposureTime(ScanSpeed speed);

		static bool isPreviewing(void);
		static bool isGrabbing(void);
		static bool isMeasuring(void);
		static bool isPosting(void);
		static bool isCancelling(void);
		
	protected:
		static void initializeCallbacks(void);

		static bool startPreview(bool enface);
		static bool closePreview(bool measure);
		static void stopGrabbing(bool measure);
		static bool startProcess(bool enface);

		static bool initiateMeasureResult(void);
		static bool captureMeasureResult(void);

		static bool initPattern(void);
		static bool updatePatternLayout(bool enface);

		static void callbackGrabPreviewBuffer(unsigned short* buffer, int sizeX, int sizeY, int index);
		static void callbackGrabMeasureBuffer(unsigned short* buffer, int sizeX, int sizeY, int index);
		static void callbackGrabEnfaceBuffer(unsigned short* buffer, int sizeX, int sizeY, int index);

		static void receivePreviewImageCompleted(unsigned char* data, unsigned int width, unsigned int height,
			float qindex, float sigRatio, int refPoint, int idxImage);
		static void receiveMeasureImageCompleted(unsigned char* data, unsigned int width, unsigned int height,
			float qindex, float sigRatio, int refPoint, int idxImage, unsigned char* lateral);
		static void receiveEnfaceImageCompleted(unsigned char* data, unsigned int width, unsigned int height);

		static OctPattern::PatternPlan& getPattern(void);
		static OctGrab::FrameGrabber* getGrabber(void);
		static OctGrab::Usb3Grabber* getUsb3Grabber(void);
		static OctDevice::Galvanometer* getGalvanometer(void);
		static OctDevice::MainBoard* getMainboard(void);

	private:
		struct ScannerImpl;
		static std::unique_ptr<ScannerImpl> d_ptr;
		static ScannerImpl& getImpl(void);
	};
}

