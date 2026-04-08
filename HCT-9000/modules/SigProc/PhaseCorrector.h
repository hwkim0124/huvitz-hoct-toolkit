#pragma once

#include "SigProcDef.h"

#include <memory>
#include <vector>


namespace SigProc
{
	class SIGPROC_DLL_API PhaseCorrector
	{
	public:
		PhaseCorrector();
		virtual ~PhaseCorrector();

		PhaseCorrector(PhaseCorrector&& rhs);
		PhaseCorrector& operator=(PhaseCorrector&& rhs);
		PhaseCorrector(const PhaseCorrector& rhs) = delete;
		PhaseCorrector& operator=(const PhaseCorrector& rhs) = delete;

	public:
		bool updateImageProcessedCount(float quality);
		bool isReadyForPhaseShiftOptimize(float quality);
		void enablePhaseShiftOptimizer(bool flag);
		void pausePhaseShiftOptimizer(bool flag);
		bool isPhaseShiftOptimizing(void);
		bool isPhaseShiftOptimzerEnabled(void);

		auto resetPhaseShiftOptimizer(void) -> void;
		auto clearPhaseShiftSignalQueues(void) -> void;
		auto clearPhaseShiftTargetCounts(bool reset) -> void;
		auto getPhaseShiftDistanceTable(int index)->std::vector<double>;
		auto putPhaseShiftSignalResult(int index, double delta, double signal) -> void;
		auto fetchPhaseShiftDeltaOfMax(int index) -> double;
		auto optimizePhaseShiftOffsets(void) -> void;
		auto getPhaseShiftOffset1(void) -> double;
		auto getPhaseShiftOffset2(void) -> double;
		bool isPhaseShiftOptimized(void);

		void setCenterOfWavelengths(float center);
		double getCenterOfWavelengths(void) const;
		void setWavenumbers(float* kvalues);
		double* getWavenumbers(void) const;

		void setPhaseShiftParameterToRetina(int index, double value);
		void setPhaseShiftParameterToCornea(int index, double value);
		void setPhaseParameterOffsetToRetina(int index, double value);
		void setPhaseParameterOffsetToCornea(int index, double value);
		auto getPhaseParameterOffsetToRetina(int index) -> double;
		auto getPhaseParameterOffsetToCornea(int index) -> double;
		auto clearPhaseParameterOffsetsOfRetina(void) -> void;
		auto clearPhaseParameterOffsetsOfCornea(void) -> void;

		double getPhaseShiftParameterToRetina(int index) const;
		double getPhaseShiftParameterToCornea(int index) const;
		double getPhaseShiftParameter(bool isRetina, int index) const;

		float* getPhaseShiftValuesToRetina(bool update);
		float* getPhaseShiftValuesToCornea(bool update);
		void updatePhaseShiftValuesToRetina(void);
		void updatePhaseShiftValuesToCornea(void);

	private:
		struct PhaseCorrectorImpl;
		std::unique_ptr<PhaseCorrectorImpl> d_ptr;
		PhaseCorrectorImpl& getImpl(void) const;

	};
}
