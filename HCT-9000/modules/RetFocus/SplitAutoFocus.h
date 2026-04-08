#pragma once

#include "RetFocusDef.h"
#include "ScanOptimizer.h"

#include <memory>
#include <string>

namespace OctDevice
{
	class MainBoard;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API SplitAutoFocus : public ScanOptimizer
	{
	public:
		SplitAutoFocus();
		virtual ~SplitAutoFocus();

	public:
		static bool initialize(OctDevice::MainBoard * board);
		static bool isInitialized(void);

		static bool start(void);
		static void cancel(bool wait);
		static bool isRunning(void);
		static bool isCancelling(void);
		static bool isCompleted(void);

	protected:
		static void resetSplitStatus(void);
		static void threadFunction(void);
		static bool processAlignment(void);

		static bool renewSplitOffset(void);
		static bool isSplitFocusBeingClose(void);
		static bool isSplitFocusOnTarget(void);
		static bool isSplitFocusOnAlignment(void);

		static void updateDiopter(void);
		static void updateFocusStep(void);
		static bool moveFocusMotor(void);
		static bool moveFocusMotorToInit(void);

	private:
		struct SplitAutoFocusImpl;
		static std::unique_ptr<SplitAutoFocusImpl> d_ptr;
		static SplitAutoFocusImpl& getImpl(void);

		static OctDevice::MainBoard* getMainBoard(void);
	};
}
