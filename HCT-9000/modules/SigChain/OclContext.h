#pragma once

#include "SigChainDef.h"

#include <memory>
#include <vector>

#include "CL\cl.h"
#include "openCL.misc.h"

#ifndef _NCLFFT
#include "clFFT.h"
#endif

namespace SigChain
{
	class OclSystem2;
	class OclTaskPlan;


	class SIGCHAIN_DLL_API OclContext
	{
	public:
		OclContext();
		virtual ~OclContext();

	public:
		static bool initializeContext(void);
		static bool isInitiated(void);
		static void releaseContext(void);
		static bool prepareContext(void);
		static bool executeContext(OclTaskPlan* plan, const std::uint16_t* input, std::uint8_t* output, std::uint8_t* laterals, float* output2 = nullptr);

		static OclTaskPlan* requestTaskPlanForPreview(int dataLen, int batchSize);
		static OclTaskPlan* requestTaskPlanForEnface(int dataLen, int batchSize);
		static OclTaskPlan* requestTaskPlanForMeasure(int dataLen, int batchSize);
		static OclTaskPlan* getTaskPlanForPreview(int dataLen, int batchSize);
		static OclTaskPlan* getTaskPlanForEnface(int dataLen, int batchSize);
		static OclTaskPlan* getTaskPlanForMeasure(int dataLen, int batchSize);
		static void releaseTaskPlans(void);

	protected:
		static OclTaskPlan* requestTaskPlan(int dataLen, int batchSize, bool unique = false);
		static int insertTaskPlan(int dataLen, int batchSize);

		static OclTaskPlan* getCurrentTaskPlan(void);
		static void setCurrentTaskPlan(OclTaskPlan* plan);
		static bool realizeTaskPlan(OclTaskPlan* plan);

		static bool prepareResamplingParameters(void);
		static bool prepareSubtractionParameters(void);
		static bool prepareWindowingParameters(void);
		static bool preparePhaseShiftParameters(OclTaskPlan* plan);

		static bool performSpectrumDataCallback(OclTaskPlan* plan, const std::uint16_t* input);
		static bool performBackgroundSubtraction(OclTaskPlan* plan, const std::uint16_t* input);
		static bool performWindowingSpectrum(OclTaskPlan* plan);
		static bool performResamplingInput(OclTaskPlan* plan);
		static bool performPhaseShiftTransform(OclTaskPlan* plan);
		static bool performZeroPaddingInput(OclTaskPlan* plan);
		static bool performInverseTransform(OclTaskPlan* plan);
		static bool performZeroPaddingOutput(OclTaskPlan* plan);
		static bool performFixedNoiseReduction(OclTaskPlan* plan);
		static bool performMagnitudeOfComplex(OclTaskPlan* plan);
		static bool performMagnitudeHistogram(OclTaskPlan* plan);
		static bool performAdaptiveGrayScale(OclTaskPlan* plan, std::uint8_t* output);
		static bool performIntensityAmplitude(OclTaskPlan* plan, float* output);
		static bool performAxialProjection(OclTaskPlan* plan, std::uint8_t* laterals);
		static bool performPhaseShiftOptimization(OclTaskPlan* plan);

		static bool initializeKernels(void);
		static void releaseKernels(void);
		static bool createCommandQueue(void);
		static bool createBackgroundSubtractionKernel(void);
		static bool createResamplingInputKernel(void);
		static bool createWindowingSpectrumKernel(void);
		static bool createFixedNoiseReductionKernel(void);
		static bool createComplexMagnitudeKernel(void);
		static bool createMagnitudeHistogramKernel(void);
		static bool createAdaptiveGrayScaleKernel(void);
		static bool createIntensityAmplitudeKernel(void);
		static bool createAxialProjectionKernel(void);

		static void releaseCommandQueue(void);
		static void releaseBackgroundSubtractionKernel(void);
		static void releaseWindowingSpectrumKernel(void);
		static void releaseResamplingInputKernel(void);
		static void releaseFixedNoiseReductionKernel(void);
		static void releaseComplexMagnitudeKernel(void);
		static void releaseMagnitudeHistogramKernel(void);
		static void releaseAdaptiveGrayScaleKernel(void);
		static void releaseIntensityAmplitudeKernel(void);
		static void releaseAxialProjectionKernel(void);

	private:
		struct OclContextImpl;
		static std::unique_ptr<OclContextImpl> d_ptr;
		static OclContextImpl& getImpl(void);

		friend class OclSystem2;
	};
}

