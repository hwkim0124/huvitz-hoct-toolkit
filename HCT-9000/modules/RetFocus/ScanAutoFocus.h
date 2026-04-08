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
	class RETFOCUS_DLL_API ScanAutoFocus : public ScanOptimizer
	{
	public:
		ScanAutoFocus();
		virtual ~ScanAutoFocus();

	public:
		static bool initialize(OctDevice::MainBoard * board, AutoFocusCompleteCallback* callback = nullptr);
		static bool isInitialized(void);

		static bool startOptimizing(void);
		static void cancelOptimizing(bool wait);
		static bool isRunning(void);
		static bool isCancelling(void);
		static bool isCompleted(void);
		static bool isCancelled(void);

	protected:
		static void initializeFocusStatus(void);
		static void resetTargetTrackCount(void);
		static void threadFunction(void);
		static bool processAutoFocusPhase(void);

		static bool phaseAutoFocus_Initiate(void);
		static bool phaseAutoFocus_NoSignal(void);
		static bool phaseAutoFocus_FirstGuess(void);
		static bool phaseAutoFocus_Forward(void);
		static bool phaseAutoFocus_Retrace(void);
		static bool phaseAutoFocus_Forward2(void);
		static bool phaseAutoFocus_Retrace2(void);

		static bool phaseAutoFocus_Complete(void);
		static bool phaseAutoFocus_Canceled(void);

		static bool changeFocusPhase(AutoFocusPhase phase, bool resetPos = false);

		static bool isRetryCountNotZero(void);
		static bool checkResistOverToForward(void);
		static bool checkResistOverToRetrace(void);
		static bool checkResistOverToForward2(void);
		static bool checkResistOverToRetrace2(void);
		static bool checkLimitsOverToForward2(void);
		static bool checkLimitsOverToRetrace2(void);
		static bool checkRetryOverToForward(void);
		static bool checkRetryOverToRetrace(void);
		static bool checkRetryOverToFirstGuess(void);

		static bool renewQualityIndex(bool next = false);
		static bool isTargetFound(void);
		static bool isTargetComplete(void);

		static float updateMotorMoveStep(void);
		static int updateMotorPosition(void);
		static int updateMotorDirection(void);
		static float getMotorOffsetToMove(void);
		static void updateStartPosition(void);

		static bool moveMotorPosition(void);
		static int getFocusMotorPosition(void);
		static bool returnToStartPosition(void);
		static bool returnToTargetPosition(void);
		static bool returnToSignalPosition(void);

		static bool isMovingToPlusDiopt(void);
		static bool isMovingToMinusDiopt(void);
		static bool isAtPlusDioptEnd(void);
		static bool isAtMinusDioptEnd(void);
		static bool isAnteriorMode(void);

		static float getDiopterRangeMax(void);
		static float getDiopterRangeMin(void);

	private:
		struct ScanAutoFocusImpl;
		static std::unique_ptr<ScanAutoFocusImpl> d_ptr;
		static ScanAutoFocusImpl& impl(void);

		static OctDevice::MainBoard* getMainBoard(void);
	};
}