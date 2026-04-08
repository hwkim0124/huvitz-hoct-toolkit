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
	class RETFOCUS_DLL_API ScanAutoPolar : public ScanOptimizer
	{
	public:
		ScanAutoPolar();
		virtual ~ScanAutoPolar();

	public:
		static bool initialize(OctDevice::MainBoard * board, AutoPolarizationCompleteCallback* callback = nullptr);
		static bool isInitialized(void);

		static bool startOptimizing(void);
		static void cancelOptimizing(bool wait);
		static bool isRunning(void);
		static bool isCancelling(void);
		static bool isCompleted(void);
		static bool isCancelled(void);

	protected:
		static void initializePolarStatus(void);
		static void threadFunction(void);
		static bool processAutoPolarPhase(void);

		static bool phaseAutoPolar_Initiate(void);
		static bool phaseAutoPolar_NoSignal(void);
		static bool phaseAutoPolar_FirstGuess(void);
		static bool phaseAutoPolar_Forward(void);
		static bool phaseAutoPolar_Retrace(void);
		static bool phaseAutoPolar_Forward2(void);
		static bool phaseAutoPolar_Retrace2(void);
		static bool phaseAutoPolar_Complete(void);
		static bool phaseAutoPolar_Canceled(void);

		static bool changePolarPhase(AutoPolarPhase phase, bool resetPos = false);
		static void resetTargetTrackCount(void);

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
		static int getPolarMotorPosition(void);
		static bool returnToStartPosition(void);
		static bool returnToTargetPosition(void);
		static bool returnToSignalPosition(void);

		static bool isMovingToPlusDegree(void);
		static bool isMovingToMinusDegree(void);
		static bool isAtPlusDegreeEnd(void);
		static bool isAtMinusDegreeEnd(void);

	private:
		struct ScanAutoPolarImpl;
		static std::unique_ptr<ScanAutoPolarImpl> d_ptr;
		static ScanAutoPolarImpl& impl(void);

		static OctDevice::MainBoard* getMainBoard(void);
	};
}

