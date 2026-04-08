#pragma once

#include "SigProcDef.h"

#include <memory>
#include <vector>


namespace SigProc
{
	class SIGPROC_DLL_API SpectroCalibrator
	{
	public:
		SpectroCalibrator();
		virtual ~SpectroCalibrator();

		SpectroCalibrator(SpectroCalibrator&& rhs);
		SpectroCalibrator& operator=(SpectroCalibrator&& rhs);
		SpectroCalibrator(const SpectroCalibrator& rhs);
		SpectroCalibrator& operator=(const SpectroCalibrator& rhs);

	public:
		void updateWavelengths(int fbgClass);
		int findPeaksOnSpectrum(unsigned short* data, int size);
		bool calculateParameters(int order=WAVELENGTH_FUNCTION_COEFFS);

		int getThresholdOfPeak(void) const;
		int getThresholdOfSubs(void) const;
		void setThresholdOfPeak(int level);
		void setThresholdOfSubs(int level);
		
		int getNumberOfPeaks(void) const;

		std::vector<int>& getPeakIndexs(void) const;
		std::vector<int>& getPeakValues(void) const;
		std::vector<double>& getWavelengths(void) const;
		const std::vector<double>& getParameters(void) const;
		const std::vector<double>& getFbgClass(int index) const;
		const char* getFbgClassName(int index) const;

	private:
		struct SpectroCalibratorImpl;
		std::unique_ptr<SpectroCalibratorImpl> d_ptr;
		SpectroCalibratorImpl& getImpl(void) const;

		void initFbgClasses(void);

		int findPeaksInCenter(unsigned short* data, int size);
		int findPeaksInOutside(unsigned short* data, int size);
	};
}
