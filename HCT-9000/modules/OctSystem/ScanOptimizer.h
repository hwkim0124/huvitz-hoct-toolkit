#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;
}


namespace OctSystem
{
	class Scanner;

	class OCTSYSTEM_DLL_API ScanOptimizer
	{
	public:
		ScanOptimizer();
		virtual ~ScanOptimizer();

		static bool initialize(OctDevice::MainBoard* board, Scanner* scanner);
		static bool isInitiated(void);

		static bool startAutoOptimize(void);
		static bool startAutoPosition(void);

		static bool startAutoFocus(AutoFocusCompleteCallback* callback = nullptr);
		static bool startAutoPolarization(AutoPolarizationCompleteCallback* callback=nullptr);
		static bool startAutoReference(AutoReferenceCompleteCallback* callback=nullptr);

		static bool isOptimizing(void);
		static bool isCancelling(void);
		static bool isCompleted(void);
		static void cancel(void);

	protected:
		struct ScanOptimizerImpl;
		static std::unique_ptr<ScanOptimizerImpl> d_ptr;
		static ScanOptimizerImpl& getImpl(void);

		static OctSystem::Scanner* getScanner(void);
		static OctDevice::MainBoard* getMainBoard(void);

		static void threadAutoOptimizeFunction(void);
		static void threadAutoPositionFunction(void);

		static void threadAutoFocusFunction(void);
		static void threadAutoPolarFunction(void);
		static void threadAutoReferFunction(void);

		static bool phaseAutoFocusProcess(void);
		static bool phaseAutoFocus_Initiate(void);
		static bool phaseAutoFocus_NoSignal(void);
		static bool phaseAutoFocus_FirstStep(void);
		static bool phaseAutoFocus_Forward(void);
		static bool phaseAutoFocus_Retrace(void);
		static bool phaseAutoFocus_Complete(void);
		static bool phaseAutoFocus_Canceled(void);

		static bool renewFocusQualityFromPreview(bool next);

		static float getMotorStepToFocus(void);
		static float getMotorOffsetToFocus(void);
		static bool moveMotorPositionOfFocus(void);

		static bool updateFocusMoveStep(void);
		static bool updateFocusDiopter(void);
		static bool returnToFocusDiopter(void);

		static bool isHeadingForPlusEndOfFocusDiopter(void);
		static bool isHeadingForMinusEndOfFocusDiopter(void);
		static bool isAtPlusSideOfFocusDiopter(void);
		static bool isAtMinusSideOfFocusDiopter(void);
		static bool isAtPlusEndOfFocusDiopter(void);
		static bool isAtMinusEndOfFocusDiopter(void);

		static bool isTargetFoundOfFocus(void);
		static bool isTargetCompleteOfFocus(void);

		static void resetRetryCountToFocus(void);
		static bool checkRetryOverToFocus(void);
		static bool isRetryCountToFocus(void);

		static bool phaseAutoPolarProcess(void);
		static bool phaseAutoPolar_Initiate(void);
		static bool phaseAutoPolar_NoSignal(void);
		static bool phaseAutoPolar_FirstStep(void);
		static bool phaseAutoPolar_Forward(void);
		static bool phaseAutoPolar_Retrace(void);
		static bool phaseAutoPolar_Complete(void);
		static bool phaseAutoPolar_Canceled(void);

		static bool renewPolarQualityFromPreview(bool next);

		static float getMotorStepToPolar(void);
		static float getMotorOffsetToPolar(void);
		static bool moveMotorPositionOfPolar(void);

		static bool updatePolarMoveStep(void);
		static bool updatePolarDegree(void);
		static bool returnToPolarDegree(void);

		static bool isHeadingForUpperEndOfPolarDegree(void);
		static bool isHeadingForLowerEndOfPolarDegree(void);
		static bool isAtUpperSideOfPolarDegree(void);
		static bool isAtLowerSideOfPolarDegree(void);
		static bool isAtUpperEndOfPolarDegree(void);
		static bool isAtLowerEndOfPolarDegree(void);

		static bool isTargetFoundOfPolar(void);
		static bool isTargetCompleteOfPolar(void);

		static void resetRetryCountToPolar(void);
		static bool checkRetryOverToPolar(void);
		static bool isRetryCountToPolar(void);

		static bool phaseAutoReferProcess(void);
		static bool phaseAutoRefer_Initiate(void);
		static bool phaseAutoRefer_NoSignal(void);
		static bool phaseAutoRefer_Forward(void);
		static bool phaseAutoRefer_Retrace(void);
		static bool phaseAutoRefer_Center(void);
		static bool phaseAutoRefer_Complete(void);
		static bool phaseAutoRefer_Canceled(void);

		static bool obtainQualityIndexOfPreview(float& qidx, bool next = true);
		static bool obtainReferencePointOfPreview(int& refPoint, bool next = true);

		static float getFocusDiopterStep(float qidx, bool init = false);
		static float getFocusDiopterOffset(void);

		static float getPolarDegreeStep(float qidx);

		static bool isQualityToComplete(float qidx);
		static bool isQualityToSignal(float qidx);

		static bool renewReferenceQualityFromPreview(bool next);
		static bool renewReferencePointFromPreview(bool next);

		static int getMotorStepToReference(void);
		static int getMotorOffsetToReference(void);
		static bool moveMotorPositionOfReference(void);

		static bool updateReferenceMoveStep(void);
		static bool updateReferencePosition(void);
		static bool returnToReferencePosition(void);

		static bool isTargetFoundOfReference(void);
		static bool isHeadingForUpperEndOfReference(void);
		static bool isHeadingForLowerEndOfReference(void);
		static bool isAtUpperSideOfReference(void);
		static bool isAtLowerSideOfReference(void);
		static bool isAtUpperEndOfReference(void);
		static bool isAtLowerEndOfReference(void);

		static int getTargetDistanceFromReferCenter(void);
		static int getTargetOffsetFromReferCenter(void);
		static bool isTargetBelowReferenceCenter(void);
		static bool isTargetAboveReferenceCenter(void);
		static bool isTargetAtReferenceCenter(void);

		static void resetRetryCountToReference(void);
		static bool checkRetryOverToReference(void);
		static bool isRetryCountToReference(void);
	};
}
