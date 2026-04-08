#pragma once

#include "SigProcDef.h"

#include <memory>
#include <vector>


namespace SigProc
{
	class SIGPROC_DLL_API PhaseCorrection
	{
	public:
		PhaseCorrection();
		virtual ~PhaseCorrection();

		PhaseCorrection(PhaseCorrection&& rhs) ;
		PhaseCorrection& operator=(PhaseCorrection&& rhs) ;
		PhaseCorrection(const PhaseCorrection& rhs) = delete;
		PhaseCorrection& operator=(const PhaseCorrection& rhs) = delete;

	public:
		bool initialize(void);
		void release(void);

		void setWavenumbers(float* kvalues);
		float* getWavenumbers(void) const;
		void setPhaseShiftParameters(float alpha2, float alpha3);
		void updatePhaseShiftValues(void);
		float* getPhaseShiftValues(void) const;

		int getSampleSize(void);
		bool setSampleSize(int size = LINE_CAMERA_CCD_PIXELS);
		bool setSampleData(float* data, int size = LINE_CAMERA_CCD_PIXELS);
		bool setBackgroundData(float* data, int size = LINE_CAMERA_CCD_PIXELS);
		
		bool performPhaseShift(float alpha2, float alpha3, float* output);
		bool performPhaseShift(float* output);
		bool performPhaseZero(float* output);

		bool saveBackgroundData(const std::wstring& path);
		bool loadBackgroundData(const std::wstring& path);

	private:
		struct PhaseCorrectionImpl;
		std::unique_ptr<PhaseCorrectionImpl> d_ptr;
		PhaseCorrectionImpl& getImpl(void) const;

		void initWaveNumbers(void);
		bool initHilbertBuffer(void);
		bool initFourierBuffer(void);
		void releaseBuffers(void);
		bool prepareInputData(void);
		bool hilbertTransform(void);

	};
}
