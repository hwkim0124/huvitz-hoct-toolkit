#pragma once

#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class SIGCHAIN_DLL_API OclSystem
	{
	public:
		OclSystem();
		virtual ~OclSystem();

	public:
		static bool initDevice(void);
		static bool prepareDevice(int dataLen, int batchSize);
		static void releaseDevice(bool context);
		static bool executeTransform(const unsigned short* input, unsigned char* output,
									unsigned int* width, unsigned int* height, float* quality = nullptr, 
									int* refPoint = nullptr);

	private:
		struct OclSystemImpl;
		static std::unique_ptr<OclSystemImpl> d_ptr;
		static OclSystemImpl& getImpl(void);

		static bool initialize(void);
		static bool isInitiated(void);

		static bool createClFFTSetupPlan(size_t inputSize, size_t outputSize, size_t batchSize);
		static bool createClFFTPhasePlan(size_t inputSize, size_t batchSize);
		static bool createClFFTZerosPlan(void);
		static bool destroyClFFTSetupPlan(void);
		static bool destroyClFFTPhasePlan(void);
		static bool destroyClFFTZerosPlan(void);

		static bool performBackgroundSubtraction(const unsigned short * input);
		static bool performResamplingInput(void);
		static bool performZeroPaddingInput(void);
		static bool performZeroPaddingOutput(void);
		static bool performInverseTransform(void);
		static bool performPhaseShiftTransform(void);
		static bool performFixedNoiseReduction(void);
		static bool performMagnitudeOfComplex(void);
		static bool performMagnitudeHistogram(void);
		static bool performMagnitudeHistogram2(void);
		static bool perfromAdaptiveGrayScale(unsigned char* output);
		static bool perfromAdaptiveGrayScale2(unsigned char* output);

		static bool prepareResamplingParamters(void);
		static bool prepareSubtractionParameters(void);
		static bool preparePhaseShiftParameters(void);

		static bool createZeroPaddingBuffers(void);
		static bool createPhaseShiftBuffers(void);
		static bool createRestrictOutputValueKernel(void);
		static bool createBackgroundSubtractionKernel(void);
		static bool createFixedNoiseReductionKernel(void);
		static bool createComplexMagnitudeKernel(void);
		static bool createMagnitudeHistogramKernel(void);
		static bool createMagnitudeHistogramKernel2(void);
		static bool createAdaptiveGrayScaleKernel(void);
		static bool createAdaptiveGrayScaleKernel2(void);
		static bool createResamplingInputKernel(void);

		static void releaseZeroPaddingBuffers(void);
		static void releasePhaseShiftBuffers(void);
		static void releaseRestrictOutputValueKernel(void);
		static void releaseBackgroundSubtractionKernel(void);
		static void releaseFixedNoiseReductionKernel(void);
		static void releaseComplexMagnitudeKernel(void);
		static void releaseMagnitudeHistogramKernel(void);
		static void releaseMagnitudeHistogramKernel2(void);
		static void releaseAdaptiveGrayScaleKernel(void);
		static void releaseAdaptiveGrayScaleKernel2(void);
		static void releaseResamplingInputKernel(void);
	};
}
