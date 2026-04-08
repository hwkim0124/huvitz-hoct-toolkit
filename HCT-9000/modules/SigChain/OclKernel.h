#pragma once

#include "SigChainDef.h"


namespace SigChain
{
	class OclKernel
	{
	public:
		OclKernel();
		virtual ~OclKernel();

	public:
		static const char* getClFFTPreCallback(void);
		static const char* getClFFTPostCallback(void);

		static const char* getHilbertTransformSource(void);
		static const char* getHilbertPhaseShiftSource(void);

		static const char* getRestrictOutputValueSource(void);
		static const char* getResamplingInputSource(void);
		static const char* getBackgroundSubtractionSource(void);
		static const char* getWindowingSpectrumSource(void);
		static const char* getFixedNoiseReductionSource(void);
		static const char* getComplexMagnitudeSource(void);
		static const char* getMagnitudeHistogramSource(void);
		static const char* getMagnitudeHistogramSource2(void);
		static const char* getMagnitudeHistogramSource3(void);
		static const char* getMagnitudeHistogramSource4(void);

		static const char* getAdaptiveGrayScaleSource(void);
		static const char* getAdaptiveGrayScaleSource2(void);
		static const char* getIntensityAmplitudeSource(void);
		static const char* getAxialProjectionSource(void);
	};
}
