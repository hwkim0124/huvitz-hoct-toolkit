#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <functional>

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

namespace SigChain {
	class OctProcess;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API ScanGrab
	{
	public:
		ScanGrab();
		virtual ~ScanGrab();

	public:
		static void initiateScanGrab(OctDevice::MainBoard* board, OctGrab::FrameGrabber* grabber);
		static void initiateScanGrab(OctDevice::MainBoard* board, OctGrab::Usb3Grabber* grabber);
		static bool grabPreview(OctPattern::PatternPlan* pattern, SigChain::OctProcess* process, bool noImageGrab);
		static bool grabMeasure(bool start, OctScanMeasure* desc);

		static float getMeasureAcquisitionTime(void);

	protected:
		static bool isInitiated(void);
		static bool startScanGrab(void);
		static bool closeScanGrab(bool measure);
		static void cancelMeasureGrab(void);

		static bool isGrabbing(void);
		static bool isPreviewing(void);
		static bool isMeasuring(void);
		static bool isMeasureCanceled(void);
		static bool isMeasureCompleted(void);

		static void threadGrabFunction(void);
		static bool waitForPreviewAndEnfaceProcessed(void);
		static bool checkIfScanOptimizing(void);
		static void notifyMeasureFrameReceived(void);

		static bool processGrabHidden(bool& initiate, bool& completed);
		static bool processGrabPreview(bool& initiate);
		static bool processGrabEnface(bool& initiate);
		static bool processGrabPreviewHD(void);
		static bool processGrabMeasure(void);
		static bool processGrabMeasure2(void);
		static bool processGrabMeasure3(void);
		static bool proceedToMeasure(void);

		static bool assignGrabBufferToPreview(int frameIdx, int buffIdx, bool repeat);
		static int assignGrabBufferToMeasure(int frameIdx, int buffIdx);
		static bool assignGrabBufferToEnface(int frameIdx, int buffIdx);
		static int assignGrabBufferOfMeasureFrames(int buffIdx);

		static int processGrabBufferToPreview(int frameIdx, int buffIdx, bool repeat);
		static int processGrabBufferToMeasure(int frameIdx, int buffIdx);
		static int processGrabBufferToEnface(int frameIdx, int buffIdx);
		static int processGrabBuffers(int buffIdx, int numLines);

		static bool triggerLineTracesToPreview(int frameIdx, bool repeat);
		static bool triggerLineTracesToMeasure(int frameIdx, bool guessOffset, std::int16_t dynOffsetX, std::int16_t dynOffsetY, std::int16_t& startX, std::int16_t& startY);
		static bool triggerLineTracesToEnface(int frameIdx);

		static bool triggerTraceProfile(OctPattern::LineTrace& line, short numLines = 1, short offsetX = 0, short offsetY = 0);
	
		static bool updateScanSpeedToMeasure(void);
		static bool changeScanSpeedToFastest(bool noImageGrab = false);
		static bool changeScanSpeedToFaster(bool noImageGrab = false);
		static bool changeScanSpeedToNormal(bool noImageGrab = false);
		static bool changeScanSpeedToCustom(bool noImageGrab = false);

		static bool isScanSpeedFastest(void);
		static bool isScanSpeedFaster(void);
		static bool isScanSpeedNormal(void);
		static bool isScanSpeedCustom(void);
		static bool changeLineCameraScanSpeed(ScanSpeed speed);
		static bool changeLineCameraFrameHeight(int height);
		static bool setUsbCameraExposureTime(float expTime);
		static float getUsbCameraExposureTime(void);

		static void increaseGrabErrorCount(void);
		static void clearGrabErrorCount(void);
		static bool isGrabErrorOverLimit(void);
		static bool isGrabErrorOverDelay(void);

		static bool isPreviewImagesRepeated(void);
		static void resetCountOfPreviewImages(bool repeat);
		static void resetCountOfMeasureImages(void);
		static void resetCountOfEnfaceImages(void);

		static int getIndexOfPreviewImageNext(void);
		static int getIndexOfMeasureImageNext(bool autoinc=true);
		static int getIndexOfEnfaceImageNext(void);
		static void retreatIndexOfMeasureImage(int size);

		static OctPattern::PatternPlan * getPattern(void);
		static SigChain::OctProcess* getProcess(void);
		static OctGrab::FrameGrabber* getGrabber(void);
		static OctGrab::Usb3Grabber* getUsb3Grab(void);
		static OctDevice::MainBoard* getMainboard(void);
		static OctDevice::Galvanometer* getGalvanometer(void);

	private:
		struct ScanGrabImpl;
		static std::unique_ptr<ScanGrabImpl> d_ptr;
		static ScanGrabImpl& getImpl(void);

		friend class Scanner;
	};
}
