#include "stdafx.h"
#include "OclSystem.h"
#include "OclKernel.h"
#include "ChainSetup.h"
#include "Resampler.h"

#include <vector>

#include "CL\cl.h"
#include "openCL.misc.h"

#ifndef _NCLFFT
#include "clFFT.h"
#endif

#include "CppUtil2.h"
#include "SigProc2.h"

using namespace SigChain;
using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct OclSystem::OclSystemImpl
{
	bool initiated;
	bool prepared;

	std::vector<cl_device_id> clDeviceIds;
	cl_context clContext;
	cl_command_queue clQueue;

#ifndef _NCLFFT
	std::unique_ptr<clfftSetupData> clfftSetup;
	clfftPlanHandle clfftPlan;
	clfftPlanHandle clfftPlanC2C;
	clfftPlanHandle clfftPlanFwd;
	clfftPlanHandle clfftPlanBwd;
	clfftPlanHandle clfftPlanZero;
#endif

	cl_uint numOfInputBuffers;
	cl_uint numOfOutputBuffers;
	cl_mem clInputMemBuffers[2] = { NULL };
	cl_mem clOutputMemBuffers[3] = { NULL };
	size_t inputBufferSizeInBytes;
	size_t outputBufferSizeInBytes;

	size_t inputLineSize;
	size_t inputLinesNum;
	size_t outputLineSize;
	size_t outputLineSizePadded;
	size_t outputLinesNum;

	cl_mem clInputZeroPaddMemBuffers[2] = { NULL };
	cl_mem clOutputZeroPaddMemBuffers[2] = { NULL };
	size_t inputZeroPaddBufferSizeInBytes;
	size_t outputZeroPaddBufferSizeInBytes;

	size_t inputZeroLineSize;
	size_t inputZeroLinesNum;
	size_t outputZeroLineSize;
	size_t outputZeroLinesNum;

	int histColumnStep ;
	size_t histColumnLines;

	int magnPeakIndexAverage;
	int magnPeakLevelAverage;
	float magnSignalNoiseRatio;
	float magnSignalLevelLog;
	float magnNoiseLevelLog;

	size_t imageWidth;
	size_t imageHeight;
	float imageSNR;
	float imageQuality;
	int imageReferPoint;

	size_t phaseInputBufferSize;
	size_t phaseOutputBufferSize;

	cl_mem clPhaseInputMemBuffers[1] = { NULL };
	cl_mem clPhaseOutputMemBuffers[2] = { NULL };
	cl_mem clPhaseShiftMemBuffer[1] = { NULL };

	cl_program clOutputProgram;
	cl_kernel clOutputRealKernel;
	cl_kernel clOutputImagKernel;

	cl_program clResampleProgram; 
	cl_kernel clResampleKernel;
	cl_mem clResampleSplineMemBufer[4] = { NULL };
	cl_mem clResampleIndexMemBuffer[1] = { NULL };
	cl_mem clResampleKValueMemBuffer[4] = { NULL };
	cl_mem clResampleInputMemBuffer[1] = { NULL };
	cl_mem clResampleOutputMemBuffer[1] = { NULL };

	size_t resampleSplineBufferSize;
	size_t resampleIndexBufferSize;
	size_t resampleKValueBufferSize;
	size_t resampleOutputBufferSize;

	cl_program clSubtractProgram;
	cl_kernel clSubtractKernel;
	cl_kernel clSubtractKernel2;
	cl_mem clSubtractBackgroundBuffer[1] = { NULL };

	size_t subtractBackgroundBufferSize;

	cl_program clReductionProgram;
	cl_kernel clReductionRealKernel;
	cl_kernel clReductionImagKernel;

	cl_program clMagnitudeProgram;
	cl_kernel clMagnitudeKernel;
	cl_kernel clMagnitudeKernel2;

	cl_program clHistogramProgram;
	cl_kernel clHistogramKernel;
	cl_mem clHistogramMemBuffer[1] = { NULL };
	size_t histogramBufferSizeInBytes;

	cl_program clGrayScaleProgram;
	cl_kernel clGrayScaleRangeKernel;
	cl_kernel clGrayScaleOutputKernel;
	cl_mem clImageMemBuffer[1] = { NULL };
	size_t imageBufferSizeInBytes;


	OclSystemImpl() : initiated(false), prepared(false) 	
	{
		clOutputProgram = NULL;
		clOutputRealKernel = NULL;
		clOutputImagKernel = NULL;

		clSubtractProgram = NULL;
		clSubtractKernel = NULL;
		clSubtractKernel2 = NULL;

		clReductionProgram = NULL;
		clReductionImagKernel = NULL;
		clReductionRealKernel = NULL;

		clMagnitudeProgram = NULL;
		clMagnitudeKernel = NULL;
		clMagnitudeKernel2 = NULL;

		clHistogramProgram = NULL;
		clHistogramKernel = NULL;

		clGrayScaleProgram = NULL;
		clGrayScaleRangeKernel = NULL;
		clGrayScaleOutputKernel = NULL;

		clResampleProgram = NULL;
		clResampleKernel = NULL;
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<OclSystem::OclSystemImpl> OclSystem::d_ptr(new OclSystemImpl());


OclSystem::OclSystem()
{
}


OclSystem::~OclSystem()
{
}


OclSystem::OclSystemImpl & SigChain::OclSystem::getImpl(void)
{
	return *d_ptr;
}


bool SigChain::OclSystem::initialize(void)
{
#ifndef _NCLFFT
	if (isInitiated()) {
		return true;
	}

	cl_int deviceType = CL_DEVICE_TYPE_GPU;
	cl_int deviceId = 0;
	cl_int platformId = 0;

	try {
		getImpl().clDeviceIds = initializeCL(deviceType, deviceId, platformId, getImpl().clContext, false);
	}
	catch (std::exception) {
		deviceId = -1;
		DebugOut2() << "Failed to initialize OpenCL with gpu device!";
	}

	// Use CPU device platform as a fallback.
	if (deviceId < 0) {
		try {
			deviceType = CL_DEVICE_TYPE_CPU;
			getImpl().clDeviceIds = initializeCL(deviceType, deviceId, platformId, getImpl().clContext, false);
		}
		catch (std::exception) {
			DebugOut2() << "Failed to initialize OpenCL with cpu device!";
			return false;
		}
	}

	getImpl().initiated = true;
	getImpl().prepared = false;
#endif

	return true;
}


bool SigChain::OclSystem::isInitiated(void)
{
	return getImpl().initiated;
}


bool SigChain::OclSystem::initDevice(void)
{
	if (!initialize()) {
		return false;
	}
	return true;
}


bool SigChain::OclSystem::prepareDevice(int dataLen, int batchSize)
{
#ifndef _NCLFFT
	if (!initialize()) {
		return false;
	}

	// Release command queue and buffers. 
	// releaseDevice(false);

	// Input and output buffer sizes for FFT transform of Real to Hermittian planar.
	size_t inputLineSize = dataLen; // LINE_CAMERA_CCD_PIXELS;
	size_t inputLinesNum = batchSize; // BSCAN_LATERAL_SIZE_MAX;
	size_t inputBatchSize = (inputLineSize * inputLinesNum);
	size_t outputLineSize = (1 + inputLineSize / 2);
	size_t outputLinesNum = batchSize;
	size_t outputBatchSize = (outputLineSize * outputLinesNum);

	size_t outputTransposedLineSize = (outputLineSize > outputLinesNum ? outputLineSize : outputLinesNum);
	size_t outputTransposedLinesNum = outputLineSize;
	size_t outputTransposedBatchSize = (outputTransposedLineSize * outputTransposedLinesNum);

	// Output buffer size for FFT C2C transform.
	size_t outputTransposedBufferSize = (outputTransposedLineSize * inputLineSize);


	getImpl().inputLineSize = inputLineSize;
	getImpl().inputLinesNum = inputLinesNum;

	// Transposed output matrix dimension.
	getImpl().outputLineSize = outputLinesNum;
	getImpl().outputLineSizePadded = outputTransposedLineSize;
	getImpl().outputLinesNum = outputTransposedLinesNum;

	getImpl().imageWidth = (int) getImpl().outputLineSize;
	getImpl().imageHeight = FFT_IMAGE_ROW_SIZE;

	getImpl().inputBufferSizeInBytes = (inputBatchSize * sizeof(unsigned short));
	getImpl().outputBufferSizeInBytes = (outputTransposedBufferSize * sizeof(float)); // (outputTransposedBatchSize * sizeof(float));

	getImpl().numOfInputBuffers = 1;
	getImpl().numOfOutputBuffers = 3;

	// Scan lines and offset for making magnitude histogram.
	getImpl().histColumnStep = (int)(getImpl().imageWidth / MAGNITUDE_SCAN_LINES_NUM);
	getImpl().histColumnLines = MAGNITUDE_SCAN_LINES_NUM;

	try {
		// Create command queue and in-out opencl buffers.
		cl_uint command_queue_flags = 0;
		createOpenCLCommandQueue(getImpl().clContext,
			command_queue_flags, getImpl().clQueue,
			getImpl().clDeviceIds,
			getImpl().inputBufferSizeInBytes, getImpl().numOfInputBuffers, getImpl().clInputMemBuffers,
			getImpl().outputBufferSizeInBytes, getImpl().numOfOutputBuffers, getImpl().clOutputMemBuffers);

		// Create FFT buffer with zero paddings.  
		createZeroPaddingBuffers();

		// Create a kernle to background subtraction. 
		createBackgroundSubtractionKernel();

		// Create a kernel to resampling.
		createResamplingInputKernel();

		// Set clFFT execution plan. 
		createClFFTSetupPlan(inputLineSize, outputLineSize, inputLinesNum);

		// Set clFFT execution plan for zero padding.
		createClFFTZerosPlan();

		// Prepare phase shift input and output buffers and user data.
		createPhaseShiftBuffers();

		// Set clFFT execution plan for phase shift. 
		createClFFTPhasePlan(inputLineSize, inputLinesNum);

		// Create a kernel to manipulate fft output data.
		// createRestrictOutputValueKernel();

		// Create a kernel to fixed pattern noise reduction.
		createFixedNoiseReductionKernel();

		// Create a kernel to complex magnitude.
		createComplexMagnitudeKernel();

		// Create a kernel to make a magnitude histogram.
		// createMagnitudeHistogramKernel();
		createMagnitudeHistogramKernel2();

		// Create a kernel to adaptive gray scaling.
		// createAdaptiveGrayScaleKernel();
		createAdaptiveGrayScaleKernel2();

		// Load intermediate data parameters into device. 
		prepareResamplingParamters();
		prepareSubtractionParameters();
		// preparePhaseShiftParameters();
	} 
	catch (std::exception) {
		return false;
	}

	getImpl().prepared = true;

#endif
	return true;
}


void SigChain::OclSystem::releaseDevice(bool context)
{
#ifndef _NCLFFT
	if (!getImpl().prepared) {
		return;
	}

	try {
		destroyClFFTPhasePlan();
		destroyClFFTZerosPlan();
		destroyClFFTSetupPlan();

		// Release programs and kernels
		releaseZeroPaddingBuffers();
		releaseBackgroundSubtractionKernel();
		releasePhaseShiftBuffers();
		releaseResamplingInputKernel();
		// releaseRestrictOutputValueKernel();
		releaseFixedNoiseReductionKernel();
		releaseComplexMagnitudeKernel();
		
		// releaseMagnitudeHistogramKernel();
		// releaseAdaptiveGrayScaleKernel();
		releaseMagnitudeHistogramKernel2();
		releaseAdaptiveGrayScaleKernel2();

		if (context) {
			cleanupCL(&getImpl().clContext, &getImpl().clQueue,
				getImpl().numOfInputBuffers, getImpl().clInputMemBuffers,
				getImpl().numOfOutputBuffers, getImpl().clOutputMemBuffers,
				NULL);

			// Device should be reinitalized after context has been released. 
			getImpl().initiated = false;
		}
		else {
			cleanupCL(NULL, &getImpl().clQueue,
				getImpl().numOfInputBuffers, getImpl().clInputMemBuffers,
				getImpl().numOfOutputBuffers, getImpl().clOutputMemBuffers,
				NULL);
		}
	} 
	catch (std::exception) {
	}

	getImpl().prepared = false;
	return ;
#endif
}


bool SigChain::OclSystem::executeTransform(const unsigned short * input, unsigned char* output, 
										unsigned int* width, unsigned int* height, float* quality, int* refPoint)
{
	if (!getImpl().prepared) {
		return false;
	}

	try {
		if (DEBUG_OUT) {
			CppUtil::ClockTimer::start();
		}

		if (ChainSetup::useSpectrumDataCallback()) {
			(*ChainSetup::getSpectrumDataCallback())(const_cast<unsigned short*>(input), *width, *height);
			if (DEBUG_OUT) {
				DebugOut2() << "Spectrum data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}

		preparePhaseShiftParameters();

		performBackgroundSubtraction(input);
		performResamplingInput();
		performPhaseShiftTransform();

		performZeroPaddingInput();
		performInverseTransform();
		performZeroPaddingOutput();

		performFixedNoiseReduction();
		performMagnitudeOfComplex();
		// performMagnitudeHistogram();
		// perfromAdaptiveGrayScale(output);
		performMagnitudeHistogram2();
		perfromAdaptiveGrayScale2(output);

		if (width != nullptr) {
			*width = (int)getImpl().imageWidth;
		}
		if (height != nullptr) {
			*height = (int)getImpl().imageHeight;
		}
		if (quality != nullptr) {
			*quality = (float)getImpl().imageQuality;
		}
		if (refPoint != nullptr) { 
			*refPoint = (int)getImpl().imageReferPoint;
		}
	}
	catch (std::exception) {
		return false;
	}

	return true;
}


bool SigChain::OclSystem::createClFFTZerosPlan(void)
{
#ifndef _NCLFFT

	if (ChainSetup::useFFTZeroPadding() == false) {
		return true;
	}

	size_t inputSize = getImpl().inputZeroLineSize;
	size_t outputSize = inputSize;// (inputSize / 2 + 1);// getImpl().outputZeroLinesNum;
	size_t batchSize = getImpl().inputZeroLinesNum;

	clfftDim fftDim = CLFFT_1D;
	size_t lengths[1] = { inputSize };

	OPENCL_V_THROW(clfftSetup(getImpl().clfftSetup.get()), "clfftSetup failed");
	OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanZero, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanZero, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanZero, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanZero, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanZero, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	size_t i_strides[4] = { 1, 0, 0, 0 };
	size_t o_strides[4] = { 1, 0, 0, 0 };

	i_strides[1] = i_strides[2] = i_strides[3] = inputSize;
	o_strides[1] = o_strides[2] = o_strides[3] = outputSize;

	// Transposed output (from row major array to column major array).
	o_strides[0] = outputSize;
	o_strides[1] = o_strides[2] = o_strides[3] = 1;

	OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanZero, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanZero, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanZero, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanZero, 1, &getImpl().clQueue, NULL, NULL), "clfftBakePlan failed");
#endif
	return true;
}


bool SigChain::OclSystem::createClFFTSetupPlan(size_t inputSize, size_t outputSize, size_t batchSize)
{
#ifndef _NCLFFT
	clfftDim fftDim = CLFFT_1D;
	size_t lengths[1] = { inputSize };

	OPENCL_V_THROW(clfftSetup(getImpl().clfftSetup.get()), "clfftSetup failed");
	OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlan, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlan, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlan, CLFFT_REAL, CLFFT_HERMITIAN_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlan, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlan, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	size_t i_strides[4] = { 1, 0, 0, 0 };
	size_t o_strides[4] = { 1, 0, 0, 0 };

	i_strides[1] = i_strides[2] = i_strides[3] = inputSize;
	o_strides[1] = o_strides[2] = o_strides[3] = outputSize;

	// Transposed output (from row major array to column major array).
	o_strides[0] = outputSize;
	o_strides[1] = o_strides[2] = o_strides[3] = 1;

	OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlan, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlan, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlan, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");


	// OPENCL_V_THROW(clfftSetPlanCallback(getImpl().clfftPlan, "convert16To32bit", OclKernel::getClFFTPreCallback(), 0, PRECALLBACK, NULL, 0), "clfftSetPlanCallback failed");
	
	// TBD: Even empty post callback causes memory access violation. why??
	// OPENCL_V_THROW(clfftSetPlanCallback(getImpl().clfftPlan, "restrictOutputValue", getClFFTPostCallback(), 0, POSTCALLBACK, NULL, 0), "clfftSetPlanCallback failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlan, 1, &getImpl().clQueue, NULL, NULL), "clfftBakePlan failed");

	/*
	// Get the buffersize
	size_t buffersize = 0;
	OPENCL_V_THROW(clfftGetTmpBufSize(getImpl().clfftPlan, &buffersize), "clfftGetTmpBufSize failed");

	// Allocate the intermediate buffer
	cl_mem clMedBuffer = NULL;

	if (buffersize) {
		cl_int medstatus;
		clMedBuffer = clCreateBuffer(getImpl().clContext, CL_MEM_READ_WRITE, buffersize, 0, &medstatus);
		OPENCL_V_THROW(medstatus, "Creating intmediate Buffer failed");
	}
	*/

	OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanC2C, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanC2C, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanC2C, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanC2C, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanC2C, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanC2C, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanC2C, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanC2C, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanC2C, 1, &getImpl().clQueue, NULL, NULL), "clfftBakePlan failed");
#endif

	return true;
}


bool SigChain::OclSystem::createClFFTPhasePlan(size_t inputSize, size_t batchSize)
{
#ifndef _NCLFFT
	clfftDim fftDim = CLFFT_1D;
	size_t lengths[1] = { inputSize };

	// Foward direction FFT. 
	OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanFwd, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanFwd, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanFwd, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanFwd, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanFwd, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	size_t i_strides[4] = { 1, 0, 0, 0 };
	size_t o_strides[4] = { 1, 0, 0, 0 };

	// Row major indexed array.
	i_strides[1] = i_strides[2] = i_strides[3] = inputSize;
	o_strides[1] = o_strides[2] = o_strides[3] = inputSize;

	OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanFwd, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanFwd, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanFwd, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

	// Post callback function.
	OPENCL_V_THROW(clfftSetPlanCallback(getImpl().clfftPlanFwd, "hilbertTransform", OclKernel::getHilbertTransformSource(), 0, POSTCALLBACK, NULL, 0), "clfftSetPlanCallback failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanFwd, 1, &getImpl().clQueue, NULL, NULL), "clfftBakePlan failed");


	// Backward direction FFT. 
	OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanBwd, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanBwd, CLFFT_INPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanBwd, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanBwd, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanBwd, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanBwd, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanBwd, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanBwd, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

	// Post callback function.
	OPENCL_V_THROW(clfftSetPlanCallback(getImpl().clfftPlanBwd, "hilbertPhaseShift", OclKernel::getHilbertPhaseShiftSource(), 0, POSTCALLBACK, &getImpl().clPhaseShiftMemBuffer[0], 1), "clfftSetPlanCallback failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanBwd, 1, &getImpl().clQueue, NULL, NULL), "clfftBakePlan failed");
#endif

	return true;
}


bool SigChain::OclSystem::destroyClFFTSetupPlan(void)
{
#ifndef _NCLFFT
	/*
	// Release the intermediate buffer.
	if (clMedBuffer) {
		clReleaseMemObject(clMedBuffer);
	}
	*/

	OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlan), "clfftDestroyPlan failed");
	OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanC2C), "clfftDestroyPlan failed");
	//OPENCL_V_THROW(clfftTeardown(), "clfftTeardown failed");
#endif
	return true;
}


bool SigChain::OclSystem::destroyClFFTPhasePlan(void)
{
#ifndef _NCLFFT
	OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanFwd), "clfftDestroyPlan failed");
	OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanBwd), "clfftDestroyPlan failed");
#endif
	return true;
}


bool SigChain::OclSystem::destroyClFFTZerosPlan(void)
{
#ifndef _NCLFFT
	OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanZero), "clfftDestroyPlan failed");
#endif
	return true;
}


bool SigChain::OclSystem::prepareResamplingParamters(void)
{
#ifndef _NCLFFT
	try {
		Resampler* resampler = ChainSetup::getResampler();

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleSplineMemBufer[0], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize, resampler->getCubicSplineA(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleSplineMemBufer[1], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize, resampler->getCubicSplineB(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleSplineMemBufer[2], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize, resampler->getCubicSplineC(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleSplineMemBufer[3], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize, resampler->getCubicSplineD(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleIndexMemBuffer[0], CL_TRUE, 0,
			getImpl().resampleIndexBufferSize, resampler->getLowerPixelIndex(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleKValueMemBuffer[0], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize, resampler->getkValueBetweenUpperAndLower(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleKValueMemBuffer[1], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize, resampler->getkValueBetweenLowerAndPrevLower(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleKValueMemBuffer[2], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize, resampler->getkValueBetweenNextUpperAndUpper(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clResampleKValueMemBuffer[3], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize, resampler->getkValueLinearized(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::prepareSubtractionParameters(void)
{
#ifndef _NCLFFT
	try {
		unsigned short* background = ChainSetup::getBackgroundSpectrum();

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clSubtractBackgroundBuffer[0], CL_TRUE, 0,
			getImpl().subtractBackgroundBufferSize, background, 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::preparePhaseShiftParameters(void)
{
#ifndef _NCLFFT
	try {
		float* shiftVals = ChainSetup::prepareDispersionCorrection();

		SigProc::PhaseCorrector* phaseCorr = ChainSetup::getPhaseCorrector();
		size_t valueSize = sizeof(float) * RESAMPLE_DETECTOR_PIXELS;

		// The amount of Phase shift values along wave numbers linearized around wavelength 
		// center should be prepared at scan operation initialization. 
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clPhaseShiftMemBuffer[0], CL_TRUE, 0,
			valueSize, shiftVals, 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performBackgroundSubtraction(const unsigned short * input)
{
#ifndef _NCLFFT
	try {
		// Copy input host data into device buffer.
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clInputMemBuffers[0], CL_TRUE, 0,
			getImpl().inputBufferSizeInBytes, &input[0], 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Line input data copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		if (ChainSetup::useBackgroundSubtraction())
		{
			const size_t globalWorkSize[2] = { getImpl().inputLineSize, getImpl().inputLinesNum };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);
			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		}
		else 
		{
			const size_t globalWorkSize[2] = { getImpl().inputLineSize, getImpl().inputLinesNum };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel2, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);
			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		}

		if (DEBUG_OUT) {
			DebugOut2() << "Background subtracted: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performResamplingInput(void)
{
#ifndef _NCLFFT
	try {
		/*
		// Copy input host data into device buffer.
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clInputMemBuffers[0], CL_TRUE, 0,
			getImpl().inputBufferSizeInBytes, &input[0], 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Line input data copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
		*/

		const size_t globalWorkSize[2] = { getImpl().inputLineSize, getImpl().inputLinesNum };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clResampleKernel, 2,
								NULL, globalWorkSize, NULL, 0, NULL, NULL);
		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Resampling input data: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		if (ChainSetup::useResampleDataCallback()) 
		{
			float* output = ChainSetup::getResampleFrameData();
			uint32_t width = (uint32_t)getImpl().inputLineSize;
			uint32_t height = (uint32_t)getImpl().inputLinesNum;

			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				getImpl().clResampleOutputMemBuffer[0], CL_TRUE, 0, getImpl().resampleOutputBufferSize,
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");
			
			(*ChainSetup::getResampleDataCallback())(output, width, height);
			if (DEBUG_OUT) {
				DebugOut2() << "Resampling data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performZeroPaddingInput(void)
{
#ifndef _NCLFFT
	try {
		if (ChainSetup::useFFTZeroPadding() == false) {
			return true;
		}

		size_t src_orgin[3] = { 0 };
		size_t dst_orgin[3] = { 0 };
		size_t region[3] = { 0 };
		region[0] = getImpl().inputLineSize * sizeof(float);
		region[1] = getImpl().inputLinesNum ;
		region[2] = 1;

		size_t src_row_pitch = getImpl().inputLineSize * sizeof(float);
		size_t src_slice_pitch = 0;// src_row_pitch * getImpl().inputLinesNum;
		size_t dst_row_pitch = getImpl().inputZeroLineSize * sizeof(float);
		size_t dst_slice_pitch = 0;// dst_row_pitch * getImpl().inputZeroLinesNum;

		if (ChainSetup::useDispersionCompensation()) 
		{
			::clEnqueueCopyBufferRect(getImpl().clQueue,
				getImpl().clPhaseOutputMemBuffers[0], getImpl().clInputZeroPaddMemBuffers[0],
				src_orgin, dst_orgin, region,
				src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
				0, NULL, NULL);

			::clEnqueueCopyBufferRect(getImpl().clQueue,
				getImpl().clPhaseOutputMemBuffers[1], getImpl().clInputZeroPaddMemBuffers[1],
				src_orgin, dst_orgin, region,
				src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
				0, NULL, NULL);
		}
		else 
		{
			::clEnqueueCopyBufferRect(getImpl().clQueue,
				getImpl().clResampleOutputMemBuffer[0], getImpl().clInputZeroPaddMemBuffers[0],
				src_orgin, dst_orgin, region,
				src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
				0, NULL, NULL);
		}

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Zero padded input copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performZeroPaddingOutput(void)
{
#ifndef _NCLFFT
	try {
		if (ChainSetup::useFFTZeroPadding() == false) {
			return true;
		}

		size_t src_orgin[3] = { 0 };
		size_t dst_orgin[3] = { 0 };
		size_t region[3] = { 0 };
		region[0] = getImpl().outputLineSize * sizeof(float);
		region[1] = getImpl().outputLinesNum;
		region[2] = 1;

		size_t src_row_pitch = getImpl().outputZeroLineSize * sizeof(float);
		size_t src_slice_pitch = 0;//src_row_pitch * getImpl().outputZeroLinesNum;
		size_t dst_row_pitch = getImpl().outputLineSizePadded * sizeof(float);
		size_t dst_slice_pitch = 0;// dst_row_pitch * getImpl().outputLinesNum;

		::clEnqueueCopyBufferRect(getImpl().clQueue,
			getImpl().clOutputZeroPaddMemBuffers[0], getImpl().clOutputMemBuffers[0],
			src_orgin, dst_orgin, region,
			src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
			0, NULL, NULL);

		::clEnqueueCopyBufferRect(getImpl().clQueue,
			getImpl().clOutputZeroPaddMemBuffers[1], getImpl().clOutputMemBuffers[1],
			src_orgin, dst_orgin, region,
			src_row_pitch, src_slice_pitch, dst_row_pitch, dst_slice_pitch,
			0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Zero padded output copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performPhaseShiftTransform(void)
{
#ifndef _NCLFFT
	try {
		if (ChainSetup::useDispersionCompensation() == false) {
			return true;
		}

		// Execute fft forward transform.
		cl_mem inputBuffers[2] = { getImpl().clResampleOutputMemBuffer[0], getImpl().clPhaseInputMemBuffers[0] };
		cl_mem outputBuffers[2] = { getImpl().clPhaseOutputMemBuffers[0], getImpl().clPhaseOutputMemBuffers[1] };

		OPENCL_V_THROW(clfftEnqueueTransform(getImpl().clfftPlanFwd, CLFFT_FORWARD, 1,
			&getImpl().clQueue, 0, NULL, NULL,
			&inputBuffers[0], outputBuffers, NULL), //clMedBuffer),
			"clfftEnqueueTransform failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Phase shift input data: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clfftEnqueueTransform(getImpl().clfftPlanBwd, CLFFT_BACKWARD, 1,
			&getImpl().clQueue, 0, NULL, NULL,
			outputBuffers, outputBuffers, NULL), //clMedBuffer),
			"clfftEnqueueTransform failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		/*
		SigProc::PhaseCorrector* phaseCorr = ChainSetup::getPhaseCorrector();
		float* shifts = phaseCorr->getPhaseShiftValues();

		int bufferLen = getImpl().inputLineSize * getImpl().inputLinesNum;
		float* bufferRe = new float[bufferLen];
		float* bufferIm = new float[bufferLen];

		size_t byteOffset = 0;
		size_t byteCount = bufferLen * sizeof(float);
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[0], CL_TRUE, byteOffset, byteCount, 
			&bufferRe[0], 0, NULL, NULL),
			"Reading the result buffer failed");
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[1], CL_TRUE, byteOffset, byteCount,
			&bufferIm[0], 0, NULL, NULL),
			"Reading the result buffer failed");

		int index;
		double phase, magnt, shift;
		for (int r = 0; r < getImpl().inputLinesNum; r++) {
			for (int c = 0; c < getImpl().inputLineSize; c++) {
				index = r * getImpl().inputLineSize + c;
				phase = atan2((double)bufferIm[index], (double)bufferRe[index]);
				magnt = hypot((double)bufferIm[index], (double)bufferRe[index]);
				shift = phase - shifts[c];

				bufferRe[index] = (float)(magnt * cos(shift));
				bufferIm[index] = (float)(magnt * sin(shift));
			}
		}

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[0], CL_TRUE, 0,
			byteCount, &bufferRe[0], 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[1], CL_TRUE, 0,
			byteCount, &bufferIm[0], 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		delete[] bufferRe;
		delete[] bufferIm;
		*/

		/*
		size_t byteOffset = sizeof(float) * 2048*128;
		size_t byteCount = sizeof(float) * 2048; // getImpl().outputLinesNum * getImpl().outputLineSizePadded * sizeof(float);
		float outputRe[2048];
		float outputIm[2048];

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[0], CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
			&outputRe[0], 0, NULL, NULL),
			"Reading the result buffer failed");

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			getImpl().clPhaseOutputMemBuffers[1], CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
			&outputIm[0], 0, NULL, NULL),
			"Reading the result buffer failed");
		*/

		if (DEBUG_OUT) {
			DebugOut2() << "Phase shift transformed: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performInverseTransform(void)
{
#ifndef _NCLFFT
	try {
		if (ChainSetup::useFFTZeroPadding())
		{
			cl_mem *inputBuffers = &getImpl().clInputZeroPaddMemBuffers[0];
			cl_mem *outputBuffers = &getImpl().clOutputZeroPaddMemBuffers[0];

			OPENCL_V_THROW(clfftEnqueueTransform(getImpl().clfftPlanZero, CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				inputBuffers, outputBuffers, NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		}
		else if (ChainSetup::useDispersionCompensation())
		{
			cl_mem *inputBuffers = &getImpl().clPhaseOutputMemBuffers[0];
			cl_mem *outputBuffers = &getImpl().clOutputMemBuffers[0];

			OPENCL_V_THROW(clfftEnqueueTransform(getImpl().clfftPlanC2C, CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				inputBuffers, outputBuffers, NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		}
		else {
			// Execute fft forward transform.'
			cl_mem *inputBuffers = &getImpl().clResampleOutputMemBuffer[0];
			cl_mem *outputBuffers = &getImpl().clOutputMemBuffers[0];

			size_t byteOffset = 0; // 512 * getImpl().outputLineSizePadded * sizeof(float);
			size_t byteCount = 2048 * 64 * sizeof(float);// getImpl().outputLinesNum * getImpl().outputLineSizePadded * sizeof(float);
			float* output = ChainSetup::getMagnitudeFrameData();

			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				inputBuffers[0], CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");

			CString data, text;
			text.Empty();
			for (int j = 0; j < 2048; j++) {
				data.Format(_T("%04d "), j);
				text += data;
			}
			text += _T("\n");
			DebugOut2() << wtoa(text);
		
			for (int i = 0; i < 64; i++) {
				text.Empty();
				for (int j = 0; j < 2048; j++) {
					data.Format(_T("%04.0f "), output[i * 2048 + j]);
					text += data;
				}
				text += _T("\n");
				DebugOut2() << wtoa(text);
			}
			text = _T("\n\n\n");
			DebugOut2() << wtoa(text);
	
			OPENCL_V_THROW(clfftEnqueueTransform(getImpl().clfftPlan, CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				inputBuffers, outputBuffers, NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

			byteCount = 1025 * 64 * sizeof(float);
			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				outputBuffers[0], CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");

			text.Empty();
			for (int j = 0; j < 2048; j++) {
				data.Format(_T("%04d "), j);
				text += data;
			}
			text += _T("\n");
			DebugOut2() << wtoa(text);

			for (int i = 0; i < 64; i++) {
				text.Empty();
				for (int j = 0; j < 1025; j++) {
					data.Format(_T("%04.0f "), output[i * 1025 + j]);
					text += data;
				}
				text += _T("\n");
				DebugOut2() << wtoa(text);
			}
			text = _T("\n\n\n");
			DebugOut2() << wtoa(text);
		}

		if (DEBUG_OUT) {
			DebugOut2() << "FFT transformed: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performFixedNoiseReduction(void)
{
#ifndef _NCLFFT
	try {
		// Read output data. 
		/*
		const size_t globalWorkSize[2] = { getImpl().outputLineSize, getImpl().outputLinesNum };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clOutputRealKernel, 2,
		NULL, globalWorkSize, NULL, 0, NULL, NULL);

		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clOutputImagKernel, 2,
		NULL, globalWorkSize, NULL, 0, NULL, NULL);
		*/

		if (ChainSetup::useFixedNoiseReduction() == false) {
			return true;
		}

		const size_t globalWorkSize[1] = { getImpl().imageHeight };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clReductionRealKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL);

		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clReductionImagKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Fixed noise reduction: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performMagnitudeOfComplex(void)
{
#ifndef _NCLFFT
	try {
		if (ChainSetup::useIntensityDataCallback())
		{
			const size_t globalWorkSize[2] = { getImpl().outputLineSize, getImpl().outputLinesNum };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clMagnitudeKernel2, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
			if (DEBUG_OUT) {
				DebugOut2() << "Complex magnitude: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}

			// int byteOffset = FFT_IMAGE_ROW_OFFSET * getImpl().outputLineSizePadded * sizeof(float);
			// int byteReads = FFT_IMAGE_ROW_SIZE * getImpl().outputLineSizePadded * sizeof(float);
			size_t byteOffset = 0; // 512 * getImpl().outputLineSizePadded * sizeof(float);
			size_t byteCount = getImpl().outputLinesNum * getImpl().outputLineSizePadded * sizeof(float);

			float* output = ChainSetup::getMagnitudeFrameData();
			uint32_t width = (uint32_t)getImpl().outputLineSize; // getImpl().outputLineSizePadded;
			uint32_t height = (uint32_t)getImpl().outputLinesNum;

			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				getImpl().clOutputMemBuffers[2], CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");

			(*ChainSetup::getIntensityDataCallback())(output, width, height);
			if (DEBUG_OUT) {
				DebugOut2() << "Intensity data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
		else
		{
			const size_t globalWorkSize[2] = { getImpl().imageWidth, getImpl().imageHeight };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clMagnitudeKernel, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
			if (DEBUG_OUT) {
				DebugOut2() << "Complex magnitude: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performMagnitudeHistogram(void)
{
#ifndef _NCLFFT
	try {
		// The global histogram as the first row should be initialized with zero. 
		unsigned int zero = 0;
		::clEnqueueFillBuffer(getImpl().clQueue, getImpl().clHistogramMemBuffer[0], &zero,
							sizeof(unsigned int), 0, sizeof(unsigned int)*MAGNITUDE_HISTOGRAM_BINS, 0, NULL, NULL);

		// const size_t globalWorkSize2[1] = { getImpl().imageHeight };
		const size_t globalWorkSize2[1] = { getImpl().histColumnLines };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clHistogramKernel, 1,
								NULL, globalWorkSize2, NULL, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Magnitude histogram: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		/*
		unsigned int hist[GRAY_SCALE_HISTOGRAM_BINS];
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
		getImpl().clHistogramMemBuffer[0], CL_TRUE, 0, getImpl().histogramBufferSizeInBytes,
		&hist, 0, NULL, NULL),
		"Reading the result buffer failed");

		if (DEBUG_OUT) {
			float sum = 0.0f;
			for (int i = 0; i < 512; i++) {
				sum += hist[i];
			}

			DebugOut2() << "Sum: " << sum;
			float acc = 0.0f;
			for (int i = 0; i < 512; i++) {
				acc += ((float)hist[i] / sum);
				DebugOut2() << "[" << i << "] " << hist[i] << ", " << ((float)hist[i] / sum) << ", " << acc;
			}
		}
		*/
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::performMagnitudeHistogram2(void)
{
#ifndef _NCLFFT
	try {
		// const size_t globalWorkSize2[1] = { getImpl().imageHeight };
		const size_t globalWorkSize2[1] = { getImpl().histColumnLines };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clHistogramKernel, 1,
			NULL, globalWorkSize2, NULL, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Magnitude histogram: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		unsigned int blocks[MAGNITUDE_BLOCK_BUFFER_SIZE];
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
						getImpl().clHistogramMemBuffer[0], CL_TRUE, 0, getImpl().histogramBufferSizeInBytes,
						&blocks, 0, NULL, NULL),
						"Reading the result buffer failed");

		size_t lines = getImpl().histColumnLines;
		unsigned int sum[4] = { 0 };

		for (int i = 0; i < lines; i++) {
			sum[0] += blocks[i*MAGNITUDE_BLOCK_ITEMS];
			sum[1] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 1];
			sum[2] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 2];
			sum[3] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 3];

			// DebugOut2() << blocks[i*MAGNITUDE_BLOCK_ITEMS] << ", " << blocks[i*MAGNITUDE_BLOCK_ITEMS+1] << ", " << blocks[i*MAGNITUDE_BLOCK_ITEMS+2] << ", " << blocks[i*MAGNITUDE_BLOCK_ITEMS+3];
		}

		getImpl().magnPeakIndexAverage = (unsigned int)(sum[0] / lines);
		getImpl().magnPeakLevelAverage = (unsigned int)(sum[1] / lines);

		float bgdAvg = (float) sum[2] / lines;
		float sigAvg = (float) sum[3] / lines;
		float snrRate, quality;

		if (bgdAvg <= 0.0f) {
			snrRate = 0.0f;		
			quality = snrRate;
		}
		else {
			snrRate = (float)(MAGNITUDE_SNR_BASE * log10(sigAvg / bgdAvg));

			quality = snrRate;
			quality = (quality < IMAGE_QUALITY_SNR_MIN ? IMAGE_QUALITY_SNR_MIN : quality);
			quality = (quality > IMAGE_QUALITY_SNR_MAX ? IMAGE_QUALITY_SNR_MAX : quality);
			quality = ((quality - IMAGE_QUALITY_SNR_MIN) / (IMAGE_QUALITY_SNR_MAX - IMAGE_QUALITY_SNR_MIN)) * IMAGE_QUALITY_INDEX_SIZE;
		}

		getImpl().magnSignalNoiseRatio = snrRate;
		getImpl().magnSignalLevelLog = log10(sigAvg);
		getImpl().magnNoiseLevelLog = log10(bgdAvg);

		getImpl().imageQuality = quality;
		getImpl().imageReferPoint = getImpl().magnPeakIndexAverage;

		if (DEBUG_OUT) {
			DebugOut2() << "Magnitude peak index: " << getImpl().magnPeakIndexAverage;
			DebugOut2() << "Magnitude peak level: " << getImpl().magnPeakLevelAverage;
			DebugOut2() << "Signal / Noise ratio: " << getImpl().magnSignalNoiseRatio;
			DebugOut2() << "Signal level: " << sigAvg << ", log10: " << getImpl().magnSignalLevelLog;
			DebugOut2() << "Noise level: " << bgdAvg << ", log10: " << getImpl().magnNoiseLevelLog;
			DebugOut2() << "Image quality: " << getImpl().imageQuality;
			DebugOut2() << "Magnitude range: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::perfromAdaptiveGrayScale(unsigned char* output)
{
#ifndef _NCLFFT
	try {
		const size_t globalWorkSize[1] = { 2 };
		const size_t localWorkSize[1] = { 2 };
		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clGrayScaleRangeKernel, 1,
								NULL, globalWorkSize, localWorkSize, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Gray scale range: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		unsigned int hist[MAGNITUDE_HISTOGRAM_SIZE];
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
									getImpl().clHistogramMemBuffer[0], CL_TRUE, 0, getImpl().histogramBufferSizeInBytes,
									&hist, 0, NULL, NULL),
									"Reading the result buffer failed");

		// Signal to Ratio of magnituides.
		float noise = (float)(hist[2] < 1.0f ? 1.0f : hist[2]);
		float signal = (float)hist[3];
		float snrRate = (float)(MAGNITUDE_SNR_BASE * log10(signal / noise));
		snrRate = snrRate - MAGNITUDE_SNR_BASE;
		snrRate = (snrRate < MAGNITUDE_SNR_MIN ? MAGNITUDE_SNR_MIN : snrRate);
		snrRate = (snrRate > MAGNITUDE_SNR_MAX ? MAGNITUDE_SNR_MAX : snrRate);
		getImpl().imageSNR = snrRate;

		if (DEBUG_OUT) {
			DebugOut2() << "Histogram: " << hist[0] << ", " << hist[1] << ", " << hist[2] << ", " << hist[3] << ", " << hist[4] << ", " << hist[5];
			DebugOut2() << "SNR Ratio: " << snrRate;
		}

		const size_t globalWorkSize2[2] = { getImpl().imageWidth, getImpl().imageHeight };
		OPENCL_V_THROW(clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clGrayScaleOutputKernel, 2,
									NULL, globalWorkSize2, NULL, 0, NULL, NULL),
									"Gray scale output failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Gray scale output: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
									getImpl().clImageMemBuffer[0], CL_TRUE, 0, getImpl().imageBufferSizeInBytes,
									&output[0], 0, NULL, NULL),
									"Reading the result buffer failed");

		/*
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
		outBuffers[1], CL_TRUE, 0, getImpl().outputBufferSizeInBytes,
		&outputImag[0], 0, NULL, NULL),
		"Reading the result buffer failed");
		*/
		if (DEBUG_OUT) {
			DebugOut2() << "Output image copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::perfromAdaptiveGrayScale2(unsigned char * output)
{
#ifndef _NCLFFT
	try {
		// Scaling parameters should be updated at running time. 
		float sigLog = getImpl().magnSignalLevelLog;
		float bgdLog = getImpl().magnNoiseLevelLog;

		if (ChainSetup::useAdaptiveGrayscale()) {
			if ((sigLog - bgdLog) < GRAY_SCALE_LOG_RANGE_MIN) {
				sigLog = bgdLog + GRAY_SCALE_LOG_RANGE_MIN;
			}
		}
		else {
			sigLog = GRAY_SCALE_LOG_VALUE_MAX;
			bgdLog = GRAY_SCALE_LOG_VALUE_MIN;
		}

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 7, sizeof(cl_float), &sigLog), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 8, sizeof(cl_float), &bgdLog), "clKernel failed");

		const size_t globalWorkSize[2] = { getImpl().imageWidth, getImpl().imageHeight };
		OPENCL_V_THROW(clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clGrayScaleOutputKernel, 2,
			NULL, globalWorkSize, NULL, 0, NULL, NULL),
			"Gray scale output failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Gray scale output: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			getImpl().clImageMemBuffer[0], CL_TRUE, 0, getImpl().imageBufferSizeInBytes,
			&output[0], 0, NULL, NULL),
			"Reading the result buffer failed");

		/*
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
		outBuffers[1], CL_TRUE, 0, getImpl().outputBufferSizeInBytes,
		&outputImag[0], 0, NULL, NULL),
		"Reading the result buffer failed");
		*/
		if (DEBUG_OUT) {
			DebugOut2() << "Output image copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createZeroPaddingBuffers(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	if (!ChainSetup::useFFTZeroPadding()) {
		return true;
	}

	try {
		int multiSize = ChainSetup::getMultipleOfZeroPaddingSize();
		size_t inputSize = getImpl().inputLineSize * multiSize;
		getImpl().inputZeroLineSize = inputSize;
		getImpl().inputZeroLinesNum = getImpl().inputLinesNum;

		size_t outputSize = inputSize; // getImpl().inputZeroLineSize / 2 + 1;
		size_t batchSize = getImpl().inputZeroLinesNum;

		getImpl().outputZeroLineSize = (outputSize > batchSize ? outputSize : batchSize);
		getImpl().outputZeroLinesNum = outputSize;

		size_t inputBufferSize = getImpl().inputZeroLineSize * getImpl().inputZeroLinesNum * sizeof(float);
		size_t outputBufferSize = getImpl().outputZeroLineSize * getImpl().outputZeroLinesNum * sizeof(float);

		createOpenCLMemoryBuffer(getImpl().clContext,
			inputBufferSize,
			2,
			getImpl().clInputZeroPaddMemBuffers,
			CL_MEM_READ_WRITE);

		createOpenCLMemoryBuffer(getImpl().clContext,
			outputBufferSize,
			2,
			getImpl().clOutputZeroPaddMemBuffers,
			CL_MEM_READ_WRITE);


		// Padded area should be initialized with zeros. 
		float zero = 0.0f;
		::clEnqueueFillBuffer(getImpl().clQueue, getImpl().clInputZeroPaddMemBuffers[0], &zero,
			sizeof(float), 0, inputBufferSize, 0, NULL, NULL);
		::clEnqueueFillBuffer(getImpl().clQueue, getImpl().clInputZeroPaddMemBuffers[1], &zero,
			sizeof(float), 0, inputBufferSize, 0, NULL, NULL);

	}
	catch (std::exception) {
		releaseZeroPaddingBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createPhaseShiftBuffers(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		SigProc::PhaseCorrector* phaseCorr = ChainSetup::getPhaseCorrector();

		unsigned int numRows = (unsigned int)getImpl().inputLinesNum;
		unsigned int numCols = (unsigned int)getImpl().inputLineSize;
		size_t bufferSize = sizeof(float) * numRows * numCols;
		size_t valueSize = sizeof(float) * RESAMPLE_DETECTOR_PIXELS;

		getImpl().phaseInputBufferSize = bufferSize;
		getImpl().phaseOutputBufferSize = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clPhaseInputMemBuffers,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			2,
			getImpl().clPhaseOutputMemBuffers,
			CL_MEM_READ_WRITE);

		createOpenCLMemoryBuffer(getImpl().clContext,
			valueSize,
			1,
			getImpl().clPhaseShiftMemBuffer,
			CL_MEM_READ_ONLY);

		/*
		SigProc::PhaseCorrector* phaseCorr = ChainSetup::getPhaseCorrector();
		size_t valueSize = sizeof(float) * RESAMPLE_DETECTOR_PIXELS;
		// The amount of Phase shift values along wave numbers linearized around wavelength 
		// center should be prepared at scan operation initialization. 
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().clPhaseShiftMemBuffer[0], CL_TRUE, 0,
			valueSize, phaseCorr->getPhaseShiftValues(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		*/

		// Imaginary part of real valued input signal is filled with zero for complex fft. 
		// Note that real part input buffer would be the resampling output. 
		float zero = 0.0f;
		::clEnqueueFillBuffer(getImpl().clQueue, getImpl().clPhaseInputMemBuffers[0], &zero,
								sizeof(float), 0, sizeof(float)*(numRows * numCols), 0, NULL, NULL);
	}
	catch (std::exception) {
		releasePhaseShiftBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createRestrictOutputValueKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clOutputProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getRestrictOutputValueSource());

		createOpenCLKernel(getImpl().clOutputProgram,
			getImpl().clOutputRealKernel, "restrictOutputValue");

		createOpenCLKernel(getImpl().clOutputProgram,
			getImpl().clOutputImagKernel, "restrictOutputValue");

		int lineSize = (int)getImpl().outputLineSizePadded;
		int numLines = (int)getImpl().outputLinesNum;
		float minVal = FFT_OUTPUT_REAL_VALUE_MIN;
		float maxVal = FFT_OUTPUT_REAL_VALUE_MAX;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputRealKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputRealKernel, 1, sizeof(cl_int), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputRealKernel, 2, sizeof(cl_int), &numLines), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputRealKernel, 3, sizeof(cl_float), &minVal), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputRealKernel, 4, sizeof(cl_float), &maxVal), "clKernel failed");

		minVal = FFT_OUTPUT_IMAG_VALUE_MIN;
		maxVal = FFT_OUTPUT_IMAG_VALUE_MAX;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputImagKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputImagKernel, 1, sizeof(cl_int), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputImagKernel, 2, sizeof(cl_int), &numLines), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputImagKernel, 3, sizeof(cl_float), &minVal), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clOutputImagKernel, 4, sizeof(cl_float), &maxVal), "clKernel failed");
	}
	catch (std::exception) {
		releaseRestrictOutputValueKernel();
		return false;
	}
#endif
	return true;
}


void SigChain::OclSystem::releaseZeroPaddingBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(2, getImpl().clInputZeroPaddMemBuffers);
	releaseOpenCLMemBuffer(2, getImpl().clOutputZeroPaddMemBuffers);
	return;
#endif
}


void SigChain::OclSystem::releasePhaseShiftBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().clPhaseInputMemBuffers);
	releaseOpenCLMemBuffer(2, getImpl().clPhaseOutputMemBuffers);
	releaseOpenCLMemBuffer(1, getImpl().clPhaseShiftMemBuffer);
	return ;
#endif
}


void SigChain::OclSystem::releaseRestrictOutputValueKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clOutputRealKernel);
	releaseOpenCLKernel(getImpl().clOutputImagKernel);
	releaseOpenCLProgram(getImpl().clOutputProgram);
	return;
#endif
}


bool SigChain::OclSystem::createBackgroundSubtractionKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clSubtractProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getBackgroundSubtractionSource());

		createOpenCLKernel(getImpl().clSubtractProgram,
			getImpl().clSubtractKernel, "backgroundSubtraction");

		createOpenCLKernel(getImpl().clSubtractProgram,
			getImpl().clSubtractKernel2, "backgroundSubtraction2");

		unsigned int numRows = (unsigned int)getImpl().inputLinesNum;
		unsigned int numCols = (unsigned int)getImpl().inputLineSize;

		size_t bufferSize = sizeof(unsigned short) * numCols;
		getImpl().subtractBackgroundBufferSize = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clSubtractBackgroundBuffer,
			CL_MEM_READ_ONLY);

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 0, sizeof(cl_mem), &getImpl().clInputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 1, sizeof(cl_mem), &getImpl().clSubtractBackgroundBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 2, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 3, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 4, sizeof(cl_uint), &numCols), "clKernel failed");

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 0, sizeof(cl_mem), &getImpl().clInputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 1, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 3, sizeof(cl_uint), &numCols), "clKernel failed");

	}
	catch (std::exception) {
		releaseBackgroundSubtractionKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createResamplingInputKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clResampleProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getResamplingInputSource());

		createOpenCLKernel(getImpl().clResampleProgram,
			getImpl().clResampleKernel, "resamplingInput");

		unsigned int numRows = (unsigned int)getImpl().inputLinesNum;
		unsigned int numCols = (unsigned int)getImpl().inputLineSize;
		float kInterval = ChainSetup().getResampler()->getLinearizedKValueInterval();

		size_t indexSize = sizeof(unsigned short) * RESAMPLE_DETECTOR_PIXELS;
		size_t valueSize = sizeof(float) * RESAMPLE_DETECTOR_PIXELS;
		size_t inputSize = sizeof(unsigned short) * numRows * numCols;
		size_t outputSize = sizeof(float) * numRows * numCols;

		getImpl().resampleKValueBufferSize = valueSize;
		getImpl().resampleIndexBufferSize = indexSize;
		getImpl().resampleSplineBufferSize = valueSize;
		getImpl().resampleOutputBufferSize = outputSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			indexSize,
			1,
			getImpl().clResampleIndexMemBuffer,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(getImpl().clContext,
			valueSize,
			4,
			getImpl().clResampleKValueMemBuffer,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(getImpl().clContext,
			valueSize,
			4,
			getImpl().clResampleSplineMemBufer,
			CL_MEM_READ_ONLY);

		/*
		createOpenCLMemoryBuffer(getImpl().clContext,
		inputSize,
		1,
		getImpl().clResampleInputMemBuffer,
		CL_MEM_READ_WRITE);
		*/

		createOpenCLMemoryBuffer(getImpl().clContext,
			outputSize,
			1,
			getImpl().clResampleOutputMemBuffer,
			CL_MEM_READ_WRITE);


		// The argument data pointed by arg_value is copied and the arg_value pointer can 
		// therefore be resused by the application after clSetKernelArg returns.
		// The argument values specified is the value used by all API calls that enqueue 
		// kernel until the argument values is changed by a call to clSetKernelArg for kernel.
		// https://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clSetKernelArg.html
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 1, sizeof(cl_mem), &getImpl().clResampleOutputMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 4, sizeof(cl_mem), &getImpl().clResampleSplineMemBufer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 5, sizeof(cl_mem), &getImpl().clResampleSplineMemBufer[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 6, sizeof(cl_mem), &getImpl().clResampleSplineMemBufer[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 7, sizeof(cl_mem), &getImpl().clResampleSplineMemBufer[3]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 8, sizeof(cl_mem), &getImpl().clResampleIndexMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 9, sizeof(cl_mem), &getImpl().clResampleKValueMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 10, sizeof(cl_mem), &getImpl().clResampleKValueMemBuffer[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 11, sizeof(cl_mem), &getImpl().clResampleKValueMemBuffer[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 12, sizeof(cl_float), &kInterval), "clKernel failed");
	}
	catch (std::exception) {
		releaseResamplingInputKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createFixedNoiseReductionKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clReductionProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getFixedNoiseReductionSource());

		createOpenCLKernel(getImpl().clReductionProgram,
			getImpl().clReductionRealKernel, "fixedNoiseReduction");

		createOpenCLKernel(getImpl().clReductionProgram,
			getImpl().clReductionImagKernel, "fixedNoiseReduction");

		int lineSize = (int)getImpl().outputLineSizePadded;
		int numRows = (int)getImpl().outputLinesNum;
		int numCols = (int)getImpl().outputLineSize;
		int rowOffset = ChainSetup::getFFTImageRowOffset();;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 1, sizeof(cl_int), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 2, sizeof(cl_int), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 3, sizeof(cl_int), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 4, sizeof(cl_int), &lineSize), "clKernel failed");

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 1, sizeof(cl_int), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 2, sizeof(cl_int), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 3, sizeof(cl_int), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 4, sizeof(cl_int), &lineSize), "clKernel failed");
	}
	catch (std::exception) {
		releaseFixedNoiseReductionKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createComplexMagnitudeKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clMagnitudeProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getComplexMagnitudeSource());

		createOpenCLKernel(getImpl().clMagnitudeProgram,
			getImpl().clMagnitudeKernel, "complexMagnitude");

		createOpenCLKernel(getImpl().clMagnitudeProgram,
			getImpl().clMagnitudeKernel2, "complexMagnitude");

		int lineSize = (int)getImpl().outputLineSizePadded;
		int rowOffset = ChainSetup::getFFTImageRowOffset();
		int numCols = (int)getImpl().imageWidth;
		int numRows = (int)getImpl().imageHeight;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 1, sizeof(cl_mem), &getImpl().clOutputMemBuffers[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 2, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 3, sizeof(cl_int), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 4, sizeof(cl_int), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 5, sizeof(cl_int), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 6, sizeof(cl_int), &lineSize), "clKernel failed");

		rowOffset = 0;
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 1, sizeof(cl_mem), &getImpl().clOutputMemBuffers[1]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 2, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 3, sizeof(cl_int), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 4, sizeof(cl_int), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 5, sizeof(cl_int), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 6, sizeof(cl_int), &lineSize), "clKernel failed");

	}
	catch (std::exception) {
		releaseComplexMagnitudeKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createMagnitudeHistogramKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clHistogramProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getMagnitudeHistogramSource());

		createOpenCLKernel(getImpl().clHistogramProgram,
			getImpl().clHistogramKernel, "magnitudeHistogram");

		size_t bufferSize = sizeof(unsigned int) * MAGNITUDE_HISTOGRAM_SIZE * MAGNITUDE_SCAN_LINES_NUM;
		getImpl().histogramBufferSizeInBytes = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clHistogramMemBuffer,
			CL_MEM_READ_WRITE);

		int lineSize = (int)getImpl().outputLineSizePadded;
		int numRows = (int)getImpl().imageHeight; //getImpl().outputLinesNum;
		int numCols = (int)getImpl().imageWidth; // getImpl().outputLineSize;
		int rowOffset = ChainSetup::getFFTImageRowOffset();;
		int columnStep = getImpl().histColumnStep;
		int histBins = MAGNITUDE_HISTOGRAM_BINS;
		int divider = MAGNITUDE_VALUE_DIVIDER;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 1, sizeof(cl_mem), &getImpl().clHistogramMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 2, sizeof(cl_int), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 3, sizeof(cl_int), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 4, sizeof(cl_int), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 5, sizeof(cl_int), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 6, sizeof(cl_int), &columnStep), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 7, sizeof(cl_int), &histBins), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 8, sizeof(cl_int), &divider), "clKernel failed");
	}
	catch (std::exception) {
		releaseMagnitudeHistogramKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createMagnitudeHistogramKernel2(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clHistogramProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getMagnitudeHistogramSource2());

		createOpenCLKernel(getImpl().clHistogramProgram,
			getImpl().clHistogramKernel, "magnitudeHistogram");

		size_t bufferSize = sizeof(unsigned int) * MAGNITUDE_BLOCK_BUFFER_SIZE;
		getImpl().histogramBufferSizeInBytes = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clHistogramMemBuffer,
			CL_MEM_READ_WRITE);

		unsigned int numRows = (unsigned int) getImpl().imageHeight; //getImpl().outputLinesNum;
		unsigned int numCols = (unsigned int) getImpl().imageWidth; // getImpl().outputLineSize;
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();;
		unsigned int lineSize = (unsigned int) getImpl().outputLineSizePadded;

		unsigned int columnStep = getImpl().histColumnStep;
		unsigned int blockSize = MAGNITUDE_BLOCK_ITEMS;
		unsigned int histBins = MAGNITUDE_HISTOGRAM_BINS;
		unsigned int divider = MAGNITUDE_VALUE_DIVIDER;

		unsigned int bgdSize = (unsigned int)(numRows * GRAY_SCALE_GROUND_SIZE_RATIO);
		unsigned int sigSize = (unsigned int)(numRows * GRAY_SCALE_SIGNAL_SIZE_RATIO);

		sigSize = (sigSize <= 0 ? 1 : sigSize);

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 1, sizeof(cl_mem), &getImpl().clHistogramMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 4, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 5, sizeof(cl_uint), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 6, sizeof(cl_uint), &columnStep), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 7, sizeof(cl_uint), &blockSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 8, sizeof(cl_uint), &histBins), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 9, sizeof(cl_uint), &divider), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 10, sizeof(cl_uint), &sigSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 11, sizeof(cl_uint), &bgdSize), "clKernel failed");
	}
	catch (std::exception) {
		releaseMagnitudeHistogramKernel2();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createAdaptiveGrayScaleKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clGrayScaleProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getAdaptiveGrayScaleSource());

		createOpenCLKernel(getImpl().clGrayScaleProgram,
			getImpl().clGrayScaleRangeKernel, "makeGrayScaleRange");

		unsigned int histBins = MAGNITUDE_HISTOGRAM_BINS;
		unsigned int divider = MAGNITUDE_VALUE_DIVIDER;
		unsigned int numRows = (unsigned int)getImpl().imageHeight;
		unsigned int numCols = (unsigned int)getImpl().imageWidth;
		unsigned int imgSize = numRows * MAGNITUDE_SCAN_LINES_NUM; // (numCols / GRAY_SCALE_COLUMN_STEP);
		unsigned int bgdSize = (unsigned int)(imgSize * GRAY_SCALE_GROUND_SIZE_RATIO);
		unsigned int sigSize = (unsigned int)(imgSize * GRAY_SCALE_SIGNAL_SIZE_RATIO);
		float minRange = GRAY_SCALE_LOG_RANGE_MIN;

		sigSize = (sigSize <= 0 ? 1 : sigSize);

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 0, sizeof(cl_mem), &getImpl().clHistogramMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 1, sizeof(cl_uint), &histBins), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 2, sizeof(cl_uint), &divider), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 3, sizeof(cl_uint), &bgdSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 4, sizeof(cl_uint), &sigSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleRangeKernel, 5, sizeof(cl_float), &minRange), "clKernel failed");


		createOpenCLKernel(getImpl().clGrayScaleProgram,
			getImpl().clGrayScaleOutputKernel, "adaptiveGrayScale");

		size_t bufferSize = sizeof(unsigned char) * numRows * numCols ;
		getImpl().imageBufferSizeInBytes = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clImageMemBuffer,
			CL_MEM_WRITE_ONLY);

		unsigned int lineSize = (unsigned int)getImpl().outputLineSizePadded;
		unsigned int linesNum = (unsigned int)getImpl().outputLinesNum;
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();;

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 1, sizeof(cl_mem), &getImpl().clImageMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 2, sizeof(cl_mem), &getImpl().clHistogramMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 3, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 4, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 5, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 6, sizeof(cl_uint), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 7, sizeof(cl_uint), &linesNum), "clKernel failed");
	}
	catch (std::exception) {
		releaseAdaptiveGrayScaleKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclSystem::createAdaptiveGrayScaleKernel2(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clGrayScaleProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getAdaptiveGrayScaleSource2());

		createOpenCLKernel(getImpl().clGrayScaleProgram,
			getImpl().clGrayScaleOutputKernel, "adaptiveGrayScale");

		unsigned int numRows = (unsigned int)getImpl().imageHeight;
		unsigned int numCols = (unsigned int)getImpl().imageWidth;

		size_t bufferSize = sizeof(unsigned char) * numRows * numCols;
		getImpl().imageBufferSizeInBytes = bufferSize;

		createOpenCLMemoryBuffer(getImpl().clContext,
			bufferSize,
			1,
			getImpl().clImageMemBuffer,
			CL_MEM_WRITE_ONLY);

		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();;
		unsigned int lineSize = (unsigned int)getImpl().outputLineSizePadded;
		unsigned int linesNum = (unsigned int)getImpl().outputLinesNum;
		float sigLog = getImpl().magnSignalLevelLog;
		float bgdLog = getImpl().magnNoiseLevelLog;
	
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 0, sizeof(cl_mem), &getImpl().clOutputMemBuffers[2]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 1, sizeof(cl_mem), &getImpl().clImageMemBuffer[0]), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 4, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 5, sizeof(cl_uint), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 6, sizeof(cl_uint), &linesNum), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 7, sizeof(cl_float), &sigLog), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 8, sizeof(cl_float), &bgdLog), "clKernel failed");

	}
	catch (std::exception) {
		releaseAdaptiveGrayScaleKernel2();
		return false;
	}
#endif
	return true;
}


void SigChain::OclSystem::releaseBackgroundSubtractionKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().clSubtractBackgroundBuffer);
	releaseOpenCLKernel(getImpl().clSubtractKernel);
	releaseOpenCLKernel(getImpl().clSubtractKernel2);
	releaseOpenCLProgram(getImpl().clSubtractProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseFixedNoiseReductionKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clReductionRealKernel);
	releaseOpenCLKernel(getImpl().clReductionImagKernel);
	releaseOpenCLProgram(getImpl().clReductionProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseComplexMagnitudeKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clMagnitudeKernel);
	releaseOpenCLKernel(getImpl().clMagnitudeKernel2);
	releaseOpenCLProgram(getImpl().clMagnitudeProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseMagnitudeHistogramKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().clHistogramMemBuffer);
	releaseOpenCLKernel(getImpl().clHistogramKernel);
	releaseOpenCLProgram(getImpl().clHistogramProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseMagnitudeHistogramKernel2(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().clHistogramMemBuffer);
	releaseOpenCLKernel(getImpl().clHistogramKernel);
	releaseOpenCLProgram(getImpl().clHistogramProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseAdaptiveGrayScaleKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clGrayScaleRangeKernel);
	releaseOpenCLKernel(getImpl().clGrayScaleOutputKernel);
	releaseOpenCLProgram(getImpl().clGrayScaleProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseAdaptiveGrayScaleKernel2(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clGrayScaleOutputKernel);
	releaseOpenCLProgram(getImpl().clGrayScaleProgram);
	return;
#endif
}


void SigChain::OclSystem::releaseResamplingInputKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().clResampleIndexMemBuffer);
	releaseOpenCLMemBuffer(1, getImpl().clResampleInputMemBuffer);
	releaseOpenCLMemBuffer(1, getImpl().clResampleOutputMemBuffer);
	releaseOpenCLMemBuffer(4, getImpl().clResampleSplineMemBufer);
	releaseOpenCLMemBuffer(4, getImpl().clResampleKValueMemBuffer);

	releaseOpenCLKernel(getImpl().clResampleKernel);
	releaseOpenCLProgram(getImpl().clResampleProgram);
	return;
#endif
}
