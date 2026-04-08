#pragma once

#include "OctGlobalDef.h"
#include "GlobalScanDef.h"

#include <memory>


namespace OctGlobal
{
	class OCTGLOBAL_DLL_API GlobalRegister
	{
	public:
		GlobalRegister();
		virtual ~GlobalRegister();

	public:
		static void setPreviewImageCompletedEvent(PreviewImageCompletedEvent* callback);
		static void setEnfaceImageCompletedEvent(EnfaceImageCompletedEvent* callback);
		static void setMeasureImageCompletedEvent(MeasureImageCompletedEvent* callback);

		static void triggerPreviewImageCompleted(unsigned char* data, unsigned int width, unsigned height,
												float qindex, float sigRatio, int refPoint, int idxImage);
		static void triggerMeasureImageCompleted(unsigned char* data, unsigned int width, unsigned height,
												float qindex, float sigRatio, int refPoint, int idxImage, 
												unsigned char* lateral);
		static void triggerEnfaceImageCompleted(unsigned char* data, unsigned int width, unsigned height);

		static void setAutoOptimizeCompletedCallback(AutoOptimizeCompletedCallback* callback);
		static void setAutoPositionCompletedCallback(AutoPositionCompletedCallback* callback);
		static void setAutoMeasureCompletedCallback(AutoMeasureCompletedCallback* callback);
		static void setAutoCorneaFocusCompletedCallback(AutoCorneaFocusCompletedCallback* callback);
		static void setAutoRetinaFocusCompletedCallback(AutoRetinaFocusCompletedCallback* callback);
		static void setAutoLensFrontFocusCompletedCallback(AutoLensFrontFocusCompletedCallback* callback);
		static void setAutoLensBackFocusCompletedCallback(AutoLensBackFocusCompletedCallback* callback);

		static void runAutoOptimizeCompletedCallback(bool result);
		static void runAutoPositionCompletedCallback(bool result);
		static void runAutoMeasureCompletedCallback(bool result, int posMacular, int posCornea, float length);
		static void runAutoCorneaFocusCompletedCallback(bool result, int referPos, float dioptPos);
		static void runAutoRetinaFocusCompletedCallback(bool result, int referPos, float dioptPos);
		static void runAutoLensFrontFocusCompletedCallback(bool result, int referPos, float dioptPos);
		static void runAutoLensBackFocusCompletedCallback(bool result, int referPos, float dioptPos);

		static bool isAutoLensFrontFocusCompletedCallback(void);

		static void setFundusImageAcquiredCallback(FundusImageAcquiredCallback* callback);
		static void setFundusImageCompletedCallback(FundusImageCompletedCallback* callback);
		static void setScanPatternAcquiredCallback(ScanPatternAcquiredCallback* callback);
		static void setScanPatternCompletedCallback(ScanPatternCompletedCallback* callback);

		static void runFundusImageAcquiredCallback(bool result);
		static void runFundusImageCompletedCallback(bool result);
		static void runScanPatternAcquiredCallback(bool result);
		static void runScanPatternCompletedCallback(bool result);

	private:
		struct GlobalRegisterImpl;
		static std::unique_ptr<GlobalRegisterImpl> d_ptr;
		static GlobalRegisterImpl& getImpl(void);
	};
}

