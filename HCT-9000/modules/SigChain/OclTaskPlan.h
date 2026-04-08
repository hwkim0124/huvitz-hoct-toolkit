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
	class OclContext;

	class SIGCHAIN_DLL_API OclTaskPlan
	{
	public:
		OclTaskPlan();
		virtual ~OclTaskPlan();

		OclTaskPlan(OclTaskPlan&& rhs);
		OclTaskPlan& operator=(OclTaskPlan&& rhs);

		// Prevent copy construction and assignment. 
		OclTaskPlan(const OclTaskPlan& rhs) = delete;
		OclTaskPlan& operator=(const OclTaskPlan& rhs) = delete;

	public:
		void initializeTaskPlan(int dataLen, int batchSize);
		bool isInitiated(void) const;
		bool isIdenticalDataLayout(int dataLen, int batchSize);
		bool realizeTaskPlan(cl_context& context, cl_command_queue& queue);
		void releaseTaskPlan(void);

		bool isTaskPlanForPreview(bool isset = false, bool flag = false) ;
		bool isTaskPlanForMeasure(bool isset = false, bool flag = false) ;
		bool isTaskPlanForEnface(bool isset = false, bool flag = false) ;

		bool useSpectrumDataCallback(bool isset = false, bool flag = true);
		bool useBackgroundSubtraction(bool isset = false, bool flag = true);
		bool useWindowingFunction(bool isset = false, bool flag = true);
		bool useResampleDataCallback(bool isset = false, bool flag = true);
		bool useDispersionCompensation(bool isset = false, bool flag = true);
		bool useZeroPadding(bool isset = false, bool flag = true, int paddSize = 0);
		bool useFixedNoiseReduction(bool isset = false, bool flag = true);
		bool useIntensityDataCallback(bool isset = false, bool flag = true);
		bool useAdaptiveGrayscale(bool isset = false, bool flag = true);
		bool useKLinearResampling(bool isset = false, bool flag = true);

		int getZeroPaddingSize(void);

		float imageSignalLevel(bool isset = false, float value = 0.0f);
		float imageBackgroundLevel(bool isset = false, float value = 0.0f);
		float imageNoiseLevel(bool isset = false, float value = 0.0f);

		float logOfImageSignal(bool isset = false, float value = 0.0f);
		float logOfImageBackground(bool isset = false, float value = 0.0f);
		float indexOfImageQuality(bool isset = false, float value = 0.0f);
		float ratioOfImageSignal(bool isset = false, float value = 0.0f);
		int referencePoint(bool isset = false, int value = 0);
		uint8_t* lateralProfile(void);

#ifndef _NCLFFT
		clfftPlanHandle& clfftPlan(void) const;
		clfftPlanHandle& clfftPlanC2C(void) const;
		clfftPlanHandle& clfftPlanFwd(void) const;
		clfftPlanHandle& clfftPlanBwd(void) const;
		clfftPlanHandle& clfftPlanZero(void) const;
#endif

		size_t inputLineSize(void) const ;
		size_t inputLinesNum(void) const ;
		size_t outputLineSize(void) const ;
		size_t outputLineSizePadded(void) const;
		size_t outputLinesNum(void) const;

		size_t inputBufferSize(void) const;
		size_t outputBufferSize(void) const;
		size_t inputZeroBufferSize(void) const;
		size_t outputZeroBufferSize(void) const;

		int numberOfInputBuffers(void) const;
		int numberOfOutputBuffers(void) const;
		int numberOfInputZeroBuffers(void) const;
		int numberOfOutputZeroBuffers(void) const;

		size_t inputZeroLineSize(void) const;
		size_t outputZeroLineSize(void) const;

		size_t grayScaleImageWidth(void) const;
		size_t grayScaleImageHeight(void) const;

		cl_mem* inputBuffers(int index = 0) const;
		cl_mem* outputBuffers(int index = 0) const;
		cl_mem* inputZeroBuffers(int index = 0) const;
		cl_mem* outputZeroBuffers(int index = 0) const;

		cl_mem* resampleBuffers(int index = 0) const;
		cl_mem* phaseInputBuffers(int index = 0) const;
		cl_mem* phaseOutputBuffers(int index = 0) const;
		cl_mem* phaseShiftBuffers(int index = 0) const;
		cl_mem* histogramBuffers(int index = 0) const;
		cl_mem* grayScaleBuffers(int index = 0) const;
		cl_mem* amplitudeBuffers(int index = 0) const;
		cl_mem* projectionBuffers(int index = 0) const;

		size_t resampleBufferSize(void);
		size_t histogramBufferSize(void);
		size_t grayScaleBufferSize(void);
		size_t amplitudeBufferSize(void);
		size_t projectionBufferSize(void);


	protected:
		bool createInputOutputBuffers(cl_context& context);
		bool createZeroPaddingBuffers(cl_context& context, cl_command_queue& queue, int paddSize);
		bool createResamplingOutputBuffers(cl_context& context);
		bool createPhaseShiftBuffers(cl_context& context, cl_command_queue& queue);
		bool createMagnitudeHistogramBuffers(cl_context& context);
		bool createGrayScaleImageBuffers(cl_context& context);
		bool createAmplitudeImageBuffers(cl_context& context);
		bool createAxialProjectionBuffers(cl_context& context);

		bool createClFFTSetupPlan(cl_context& context, cl_command_queue& queue);
		bool createClFFTZerosPlan(cl_context& context, cl_command_queue& queue);
		bool createClFFTPhasePlan(cl_context& context, cl_command_queue& queue);

		void releaseInputOutputBuffers(void);
		void releaseZeroPaddingBuffers(void);
		void releaseResamplingOutputBuffers(void);
		void releasePhaseShiftBuffers(void);
		void releaseMagnitudeHistogramBuffers(void);
		void releaseGrayScaleImageBuffers(void);
		void releaseAmplitudeImageBuffers(void);
		void releaseAxialProjectionBuffers(void);

		void releaseClFFTSetupPlan(void);
		void releaseClFFTZerosPlan(void);
		void releaseClFFTPhasePlan(void);

	private:
		struct OclTaskPlanImpl;
		std::unique_ptr<OclTaskPlanImpl> d_ptr;
		OclTaskPlanImpl& getImpl(void) const;

		friend class OclContext;
	};
}
