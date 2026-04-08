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
	class RETFOCUS_DLL_API ScanAutoRefer : public ScanOptimizer
	{
	public:
		ScanAutoRefer();
		virtual ~ScanAutoRefer();

	public:
		static bool initialize(OctDevice::MainBoard * board, AutoReferenceCompleteCallback* callback);
		static bool isInitialized(void);

		static bool startOptimizing(bool isAnterior = false, bool autoCenter = false, bool motorInRange = true, bool narrowTarget=false, 
							bool smallStep=false, bool measureMode=false, bool upperTarget = false);
		static void cancelOptimizing(bool wait);
		static bool isRunning(void);
		static bool isCancelling(void);
		static bool isCompleted(void);
		static bool isCancelled(void);

	protected:
		static void resetReferStatus(void);
		static void threadFunction(void);
		static bool processAutoReferPhase(void);

		static bool phaseAutoRefer_Initiate(void);
		static bool phaseAutoRefer_NoSignal(void);
		static bool phaseAutoRefer_Capture(void);
		static bool phaseAutoRefer_Forward(void);
		static bool phaseAutoRefer_Retrace(void);
		static bool phaseAutoRefer_Center(void);
		static bool phaseAutoRefer_Complete(void);
		static bool phaseAutoRefer_Canceled(void);

		static bool changeReferPhase(AutoReferPhase phase, bool resetPos = false);
		static void resetReferCounts(void);
		static bool isRetryCount(void);

		static bool checkIfTargetExist(void);
		static bool checkTrackOverToCenter(void);
		static bool checkRetryOverToForward(void);
		static bool checkRetryOverToRetrace(void);
		static bool checkRetryOverToCenter(void);

		static bool renewQualityIndex(bool next = false);
		static bool renewReferencePoint(bool next = false);

		static bool isTargetFound(void);
		static bool isTargetAtCenter(void);
		static bool isTargetOutOfCenter(void);
		static bool isTargetAboveCenter(bool shift = false);
		static bool isTargetBelowCenter(bool shift = false);
		static bool isTargetInValidRange(void);
		static int getTargetOffsetFromCenter(bool shift =false);

		static bool isAnteriorMode(void);
		static bool isMeasureMode(void);
		static bool isNarrowTarget(void);
		static bool useSmallMoveStep(void);
		static int updateMotorStep(void);
		static int updateMotorPosition(void);
		static int updateMotorDirection(void);
		static int getMotorOffsetToMove(void);

		static int getReferMotorPosition(void);
		static bool moveReferenceMotor(void);
		static bool moveReferenceMotor(int dist);
		static bool returnToStartPosition(void);
		static bool returnToPeakPosition(void);

		static bool isMovingForward(void);
		static bool isMovingBackward(void);
		static bool isAtForwardEnd(void);
		static bool isAtBackwardEnd(void);
		static bool isAtUpperSideEnd(void);
		static bool isAtLowerSideEnd(void);
		static bool isAtUpperSide(void);
		static bool isAtLowerSide(void);

	private:
		struct ScanAutoReferImpl;
		static std::unique_ptr<ScanAutoReferImpl> d_ptr;
		static ScanAutoReferImpl& getImpl(void);

		static OctDevice::MainBoard* getMainBoard(void);
	};
}

