#pragma once

#include "SigProcDef.h"


#include <memory>


namespace SigProc
{
	class SIGPROC_DLL_API Resampler
	{
	public:
		Resampler();
		virtual ~Resampler();

		Resampler(Resampler&& rhs);
		Resampler& operator=(Resampler&& rhs);

		// Prevent copy construction and assignment. 
		Resampler(const Resampler& rhs) = delete;
		Resampler& operator=(const Resampler& rhs) = delete;

	public:
		double getWavelengthFunctionCoefficient(int index) const;
		void setWavelengthFunctionCoefficient(int index, double value);
		void setWavelengthFunctionCoefficients(double coeffs[RESAMPLE_WAVELENGTH_FUNCTION_DEGREE]);

		float* getCubicSplineA(void) const;
		float* getCubicSplineB(void) const;
		float* getCubicSplineC(void) const;
		float* getCubicSplineD(void) const;

		float* getkValueBetweenUpperAndLower(void) const;
		float* getkValueBetweenLowerAndPrevLower(void) const;
		float* getkValueBetweenNextUpperAndUpper(void) const;
		float* getkValueLinearized(void) const;

		unsigned short* getLowerPixelIndex(void) const;
		float getLinearizedKValueInterval(void) const;
		double* detectorWaveLength(void) const;

		double getWavenumberResampledAtPixelPosition(int index) const;
		double getWavenumberResampledAtPixelPosition(float index) const;
		double getWavelengthResampledAtPixelPosition(float index) const;

		double getPixelResolutionInAir(void) const;
		double getPixelResolutionInTissue(double fwhm, double refIndex = 1.36) const;
		double getPixelResolution(double refractiveIndex) const;


	private:
		struct ResamplerImpl;
		std::unique_ptr<ResamplerImpl> d_ptr;
		ResamplerImpl& getImpl(void) const;

		void updateDetectorWavelengths(void);
		void updateDetectorKValues(void);
		void updateLinearizedKValues(void);
		void updateResamplingParameters(void);
	};
}
