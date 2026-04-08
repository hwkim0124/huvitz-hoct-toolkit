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
	class OCTSYSTEM_DLL_API ScanFunc
	{
	public:
		ScanFunc();
		virtual ~ScanFunc();

	public:
		static bool initScanFunc(OctDevice::MainBoard* board, OctGrab::FrameGrabber* grabber);
		static bool initScanFunc(OctDevice::MainBoard* board, OctGrab::Usb3Grabber* grabber);
		static bool isInitiated(void);
		static void registerUsb3Grabber(OctGrab::Usb3Grabber* grabber);

		static bool prepareChainSetup(PatternDomain domain, PatternName name, ScanSpeed speed, bool measure);

		static bool updateLineTrace(OctPattern::LineTrace* line, ScanSpeed speed, bool foreDist = false);

		static bool updatePatternPositions(EyeSide eyeSide, const OctPattern::PatternPlan& pattern, ScanSpeed speed);
		static bool updatePreviewPositions(EyeSide eyeSide, const OctPattern::PatternPlan& pattern, ScanSpeed speed);
		static bool updateMeasurePositions(EyeSide eyeSide, const OctPattern::PatternPlan& pattern, ScanSpeed speed);
		static bool updateEnfacePositions(EyeSide eyeSide, const OctPattern::PatternPlan& pattern, ScanSpeed speed);
		static bool updateTracePositions(EyeSide eyeSide, const OctPattern::PatternPlan& pattern, OctPattern::LineTrace& line, bool cornea);

		static bool exportPatternPositions(const OctPattern::PatternPlan& pattern, std::string filename);

		static bool uploadPatternProfiles(const OctPattern::PatternPlan& pattern, bool measure);
		static bool uploadPreviewProfiles(const OctPattern::PatternPlan& pattern);
		static bool uploadMeasureProfiles(const OctPattern::PatternPlan& pattern, int startIdx = 0, int maxLines = 32);
		static bool uploadEnfaceProfiles(const OctPattern::PatternPlan& pattern);
		static bool uploadTraceProfile(OctPattern::LineTrace& line);

		static bool updatePatternBuffers(const OctPattern::PatternPlan& pattern, bool measure, bool reset=true);
		static bool updatePreviewBuffers(const OctPattern::PatternPlan& pattern);
		static bool updateMeasureBuffers(const OctPattern::PatternPlan& pattern);
		static bool updateEnfaceBuffers(const OctPattern::PatternPlan& pattern);
		static void releaseGrabberBuffers(void);

	

	protected:
		static OctGrab::FrameGrabber* getGrabber(void);
		static OctGrab::Usb3Grabber* getUsb3Grab(void);
		static OctDevice::MainBoard* getMainboard(void);
		static OctDevice::Galvanometer* getGalvanometer(void);

	private:
		struct ScanFuncImpl;
		static std::unique_ptr<ScanFuncImpl> d_ptr;
		static ScanFuncImpl& getImpl(void);
	};
}

