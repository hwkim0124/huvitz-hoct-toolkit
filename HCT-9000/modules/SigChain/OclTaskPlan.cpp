#include "stdafx.h"
#include "OclTaskPlan.h"
#include "OclKernel.h"

#include "CppUtil2.h"
#include "SigProc2.h"

using namespace SigChain;
using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct OclTaskPlan::OclTaskPlanImpl
{
#ifndef _NCLFFT
	std::unique_ptr<clfftSetupData> clfftSetup;
	clfftPlanHandle clfftPlan = 0;
	clfftPlanHandle clfftPlanC2C = 0;
	clfftPlanHandle clfftPlanFwd = 0;
	clfftPlanHandle clfftPlanBwd = 0;
	clfftPlanHandle clfftPlanZero = 0;
#endif

	size_t inputLineSize;
	size_t inputLinesNum;
	size_t inputDataSize;
	size_t outputLineSize;
	size_t outputLineSizePadded;
	size_t outputLinesNum;
	size_t outputDataSize;
	size_t outputDataSizePadded;

	cl_uint numInputBuffers;
	cl_uint numOutputBuffers;
	cl_mem inputBuffers[2];
	cl_mem outputBuffers[3];
	size_t inputBufferSize;
	size_t outputBufferSize;

	cl_uint numInputZeroBuffers;
	cl_uint numOutputZeroBuffers;
	cl_mem inputZeroBuffers[2];
	cl_mem outputZeroBuffers[2];
	size_t inputZeroBufferSize;
	size_t outputZeroBufferSize;

	size_t inputZeroLineSize;
	size_t inputZeroLinesNum;
	size_t outputZeroLineSize;
	size_t outputZeroLinesNum;

	size_t resampleBufferSize;
	cl_mem resampleBuffers[1];

	size_t histogramBufferSize;
	cl_mem histogramBuffers[1];

	size_t grayScaleImageWidth;
	size_t grayScaleImageHeight;
	size_t grayScaleImageSize;
	size_t grayScaleBufferSize;
	cl_mem grayScaleBuffers[1];
	size_t amplitudeBufferSize;
	cl_mem amplitudeBuffers[1];

	size_t phaseInputBufferSize;
	size_t phaseOutputBufferSize;
	cl_mem phaseInputBuffers[1];
	cl_mem phaseOutputBuffers[2];
	cl_mem phaseShiftBuffers[1];

	cl_mem projectionBuffers[1];
	size_t projectionBufferSize;

	bool initiated;
	bool isPreview;
	bool isMeasure;
	bool isEnface;

	bool useZeroPadding;
	bool useSubtraction;
	bool useWindowing;
	bool useResampleCallback;
	bool useSpectrumCallback;
	bool useDispersion;
	bool useFixedNoise;
	bool useIntensityCallback;
	bool useGrayscale;
	bool useResampling;

	int sizeOfZeroPadding;
	int referencePoint;
	float indexOfImageQuality;
	float ratioOfImageSignal;

	float logOfImageSignal;
	float logOfImageBackground;
	float imageSignalLevel;
	float imageBackgroundLevel;
	float imageNoiseLevel;

	uint8_t lateralProfile[FRAME_DATA_LINES_MAX];


	OclTaskPlanImpl() : initiated(false), 
			isPreview(false), isMeasure(false), isEnface(false),
			useZeroPadding(false), useSubtraction(true), useResampleCallback(false),
			useSpectrumCallback(false), useDispersion(false), useFixedNoise(false),
			useIntensityCallback(false), useGrayscale(false), useResampling(false), useWindowing(false),
			inputBuffers{ NULL }, outputBuffers{ NULL }, inputZeroBuffers{ NULL }, outputZeroBuffers{ NULL }, 
			resampleBuffers{ NULL }, histogramBuffers{ NULL }, grayScaleBuffers{ NULL }, amplitudeBuffers{ NULL },
			phaseInputBuffers{ NULL }, phaseOutputBuffers{ NULL }, phaseShiftBuffers{ NULL }, 
			projectionBuffers{ NULL }, 
			logOfImageSignal(0.0f), logOfImageBackground(0.0f), ratioOfImageSignal(0.0f), 
			imageSignalLevel(0.0f), imageBackgroundLevel(0.0f), imageNoiseLevel(0.0f),
			indexOfImageQuality(0.0f), referencePoint(0), lateralProfile{ 0 }
	{
	}
};


OclTaskPlan::OclTaskPlan() :
	d_ptr(make_unique<OclTaskPlanImpl>())
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::OclTaskPlan::~OclTaskPlan() = default;
SigChain::OclTaskPlan::OclTaskPlan(OclTaskPlan && rhs) = default;
OclTaskPlan & SigChain::OclTaskPlan::operator=(OclTaskPlan && rhs) = default;


void SigChain::OclTaskPlan::initializeTaskPlan(int dataLen, int batchSize)
{
	// Input and output buffer sizes for FFT transform of Real to Hermittian planar.
	size_t inputLineSize = dataLen; // LINE_CAMERA_CCD_PIXELS;
	size_t inputLinesNum = batchSize; // BSCAN_LATERAL_SIZE_MAX;
	size_t inputBatchSize = (inputLineSize * inputLinesNum);
	size_t outputLineSize = (1 + inputLineSize / 2);
	size_t outputLinesNum = batchSize;
	size_t outputBatchSize = (outputLineSize * outputLinesNum);

	// Transposed buffer's layout considering fft output padding.
	size_t outputTransposedLineSize = (outputLineSize > outputLinesNum ? outputLineSize : outputLinesNum);
	size_t outputTransposedLinesNum = outputLineSize;
	size_t outputTransposedBatchSize = (outputTransposedLineSize * outputTransposedLinesNum);

	// Output buffer size for FFT C2C transform.
	size_t outputTransposedBufferSize = (outputTransposedLineSize * inputLineSize);

	// Input data matrix dimension.
	d_ptr->inputLineSize = inputLineSize;
	d_ptr->inputLinesNum = inputLinesNum;

	// Transposed output matrix dimension.
	d_ptr->outputLineSize = outputLinesNum;
	d_ptr->outputLineSizePadded = outputTransposedLineSize;
	d_ptr->outputLinesNum = outputTransposedLinesNum;

	d_ptr->inputDataSize = inputBatchSize;
	d_ptr->outputDataSize = outputBatchSize;
	d_ptr->outputDataSizePadded = outputTransposedBatchSize; // outputLinesNum * outputTransposedLineSize;

	d_ptr->inputBufferSize = (inputBatchSize * sizeof(unsigned short));
	d_ptr->outputBufferSize = (outputTransposedBufferSize * sizeof(float)); // (outputTransposedBatchSize * sizeof(float));
	d_ptr->numInputBuffers = 1;
	d_ptr->numOutputBuffers = 3;

	d_ptr->numInputZeroBuffers = 2;
	d_ptr->numOutputZeroBuffers = 2;

	d_ptr->grayScaleImageWidth = outputLinesNum;
	d_ptr->grayScaleImageHeight = (isTaskPlanForMeasure() ? FFT_IMAGE_ROW_SIZE_FOR_MEASURE : (isTaskPlanForEnface() ? FFT_IMAGE_ROW_SIZE_FOR_ENFACE : FFT_IMAGE_ROW_SIZE_FOR_PREVIEW));
	d_ptr->grayScaleImageSize = (d_ptr->grayScaleImageWidth * d_ptr->grayScaleImageHeight);

	getImpl().initiated = true;
	return;
}


bool SigChain::OclTaskPlan::isInitiated(void) const
{
	return d_ptr->initiated;
}


bool SigChain::OclTaskPlan::isIdenticalDataLayout(int dataLen, int batchSize)
{
	return (d_ptr->inputLineSize == dataLen && d_ptr->inputLinesNum == batchSize);
}


bool SigChain::OclTaskPlan::realizeTaskPlan(cl_context & context, cl_command_queue & queue)
{
	if (!isInitiated()) {
		return false;
	}

	// Release pre allocated buffers before.
	releaseTaskPlan();

	if (!createInputOutputBuffers(context)) {
		return false;
	}

	// To enable zero paddings, task plan should be reinitialized. 
	if (useZeroPadding()) {
		if (!createZeroPaddingBuffers(context, queue, getImpl().sizeOfZeroPadding)) {
			return false;
		}
	}

	if (!createResamplingOutputBuffers(context)) {
		return false;
	}

	if (!createPhaseShiftBuffers(context, queue)) {
		return false;
	}

	if (!createClFFTSetupPlan(context, queue)) {
		return false;
	}

	if (useZeroPadding()) {
		if (!createClFFTZerosPlan(context, queue)) {
			return false;
		}
	}

	if (!createClFFTPhasePlan(context, queue)) {
		return false;
	}

	if (!createMagnitudeHistogramBuffers(context)) {
		return false;
	}

	if (!createGrayScaleImageBuffers(context)) {
		return false;
	}

	if (!createAmplitudeImageBuffers(context)) {
		return false;
	}

	if (!createAxialProjectionBuffers(context)) {
		return false;
	}

	return true;
}


void SigChain::OclTaskPlan::releaseTaskPlan(void)
{
	releaseInputOutputBuffers();
	releaseResamplingOutputBuffers();
	releasePhaseShiftBuffers();
	releaseMagnitudeHistogramBuffers();
	releaseGrayScaleImageBuffers();
	releaseAmplitudeImageBuffers();
	releaseAxialProjectionBuffers();

	//if (useZeroPadding()) {
		releaseZeroPaddingBuffers();
	//}

	releaseClFFTSetupPlan();
	releaseClFFTPhasePlan();

	//if (useZeroPadding()) {
		releaseClFFTZerosPlan();
	//}
	return;
}


bool SigChain::OclTaskPlan::isTaskPlanForPreview(bool isset, bool flag) 
{
	if (isset) {
		getImpl().isPreview = flag;
	}
	return getImpl().isPreview;
}


bool SigChain::OclTaskPlan::isTaskPlanForMeasure(bool isset, bool flag) 
{
	if (isset) {
		getImpl().isMeasure = flag;
	}
	return getImpl().isMeasure;
}


bool SigChain::OclTaskPlan::isTaskPlanForEnface(bool isset, bool flag) 
{
	if (isset) {
		getImpl().isEnface = flag;
	}
	return getImpl().isEnface;
}


bool SigChain::OclTaskPlan::useSpectrumDataCallback(bool isset, bool flag)
{
	if (isset) {
		getImpl().useSpectrumCallback = flag;
	}
	return getImpl().useSpectrumCallback;
}


bool SigChain::OclTaskPlan::useBackgroundSubtraction(bool isset, bool flag)
{
	if (isset) {
		getImpl().useSubtraction = flag;
	}
	return getImpl().useSubtraction;
}

bool SigChain::OclTaskPlan::useWindowingFunction(bool isset, bool flag)
{
	if (isset) {
		getImpl().useWindowing = flag;
	}
	return getImpl().useWindowing;
}

bool SigChain::OclTaskPlan::useResampleDataCallback(bool isset, bool flag)
{
	if (isset) {
		getImpl().useResampleCallback = flag;
	}
	return getImpl().useResampleCallback;
}


bool SigChain::OclTaskPlan::useDispersionCompensation(bool isset, bool flag)
{
	if (isset) {
		getImpl().useDispersion = flag;
	}
	return getImpl().useDispersion;
}


bool SigChain::OclTaskPlan::useZeroPadding(bool isset, bool flag, int paddSize)
{
	if (isset) {
		getImpl().useZeroPadding = flag;
		getImpl().sizeOfZeroPadding = paddSize;
	}
	return getImpl().useZeroPadding;
}


bool SigChain::OclTaskPlan::useFixedNoiseReduction(bool isset, bool flag)
{
	if (isset) {
		getImpl().useFixedNoise = flag;
	}
	return getImpl().useFixedNoise;
}


bool SigChain::OclTaskPlan::useIntensityDataCallback(bool isset, bool flag)
{
	if (isset) {
		getImpl().useIntensityCallback = flag;
	}
	return getImpl().useIntensityCallback;
}


bool SigChain::OclTaskPlan::useAdaptiveGrayscale(bool isset, bool flag)
{
	if (isset) {
		getImpl().useGrayscale = flag;
	}
	return getImpl().useGrayscale;
}

bool SigChain::OclTaskPlan::useKLinearResampling(bool isset, bool flag)
{
	if (isset) {
		getImpl().useResampling = flag;
	}
	return getImpl().useResampling;
}


int SigChain::OclTaskPlan::getZeroPaddingSize(void)
{
	return getImpl().sizeOfZeroPadding;
}

float SigChain::OclTaskPlan::imageSignalLevel(bool isset, float value)
{
	if (isset) {
		getImpl().imageSignalLevel = value;
	}
	return getImpl().imageSignalLevel;
}

float SigChain::OclTaskPlan::imageBackgroundLevel(bool isset, float value)
{
	if (isset) {
		getImpl().imageBackgroundLevel = value;
	}
	return getImpl().imageBackgroundLevel;
}

float SigChain::OclTaskPlan::imageNoiseLevel(bool isset, float value)
{
	if (isset) {
		getImpl().imageNoiseLevel = value;
	}
	return getImpl().imageNoiseLevel;
}


float SigChain::OclTaskPlan::logOfImageSignal(bool isset, float value)
{
	if (isset) {
		getImpl().logOfImageSignal = value;
	}
	return getImpl().logOfImageSignal;
}


float SigChain::OclTaskPlan::logOfImageBackground(bool isset, float value)
{
	if (isset) {
		getImpl().logOfImageBackground = value;
	}
	return getImpl().logOfImageBackground;
}


float SigChain::OclTaskPlan::indexOfImageQuality(bool isset, float value)
{
	if (isset) {
		getImpl().indexOfImageQuality = value;
	}
	return getImpl().indexOfImageQuality;
}


float SigChain::OclTaskPlan::ratioOfImageSignal(bool isset, float value)
{
	if (isset) {
		getImpl().ratioOfImageSignal = value;
	}
	return getImpl().ratioOfImageSignal;
}


int SigChain::OclTaskPlan::referencePoint(bool isset, int value)
{
	if (isset) {
		getImpl().referencePoint = value;
	}
	return getImpl().referencePoint;
}


uint8_t * SigChain::OclTaskPlan::lateralProfile(void)
{
	return getImpl().lateralProfile;
}

#ifndef _NCLFFT
clfftPlanHandle & SigChain::OclTaskPlan::clfftPlan(void) const
{
	return getImpl().clfftPlan;
}


clfftPlanHandle & SigChain::OclTaskPlan::clfftPlanC2C(void) const
{
	return getImpl().clfftPlanC2C;
}


clfftPlanHandle & SigChain::OclTaskPlan::clfftPlanFwd(void) const
{
	return getImpl().clfftPlanFwd;
}


clfftPlanHandle & SigChain::OclTaskPlan::clfftPlanBwd(void) const
{
	return getImpl().clfftPlanBwd;
}


clfftPlanHandle & SigChain::OclTaskPlan::clfftPlanZero(void) const
{
	return getImpl().clfftPlanZero;
}
#endif

OclTaskPlan::OclTaskPlanImpl & SigChain::OclTaskPlan::getImpl(void) const
{
	return *d_ptr;
}


int SigChain::OclTaskPlan::numberOfInputBuffers(void) const
{
	return 2;
}


int SigChain::OclTaskPlan::numberOfOutputBuffers(void) const
{
	return 3;
}


int SigChain::OclTaskPlan::numberOfInputZeroBuffers(void) const
{
	return 2;
}


int SigChain::OclTaskPlan::numberOfOutputZeroBuffers(void) const
{
	return 2;
}


size_t SigChain::OclTaskPlan::inputZeroLineSize(void) const
{
	return d_ptr->inputZeroLineSize;
}


size_t SigChain::OclTaskPlan::outputZeroLineSize(void) const
{
	return d_ptr->outputZeroLineSize;
}


size_t SigChain::OclTaskPlan::grayScaleImageWidth(void) const
{
	return d_ptr->grayScaleImageWidth;
}


size_t SigChain::OclTaskPlan::grayScaleImageHeight(void) const
{
	return d_ptr->grayScaleImageHeight;
}


bool SigChain::OclTaskPlan::createInputOutputBuffers(cl_context& context)
{
#ifndef _NCLFFT
	try
	{
		// Create command queue and in-out opencl buffers.
		createOpenCLMemoryBuffer(
			context,
			d_ptr->inputBufferSize,
			d_ptr->numInputBuffers,
			d_ptr->inputBuffers,
			CL_MEM_READ_WRITE);

		createOpenCLMemoryBuffer(
			context,
			d_ptr->outputBufferSize,
			d_ptr->numOutputBuffers,
			d_ptr->outputBuffers,
			CL_MEM_READ_WRITE);

	}
	catch (std::exception) {
		releaseInputOutputBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createZeroPaddingBuffers(cl_context& context, cl_command_queue& queue, int paddSize)
{
#ifndef _NCLFFT
	// Increase input data line size with multiple of padding size. 
	size_t inputLineSize = d_ptr->inputLineSize * paddSize;
	size_t inputLinesNum = d_ptr->inputLinesNum;

	d_ptr->inputZeroLineSize = inputLineSize;
	d_ptr->inputZeroLinesNum = inputLinesNum;

	// Transposed output data size.
	size_t outputSize = inputLineSize; // getImpl().inputZeroLineSize / 2 + 1;
	size_t batchSize = inputLinesNum;
	size_t outputLineSize = (outputSize > batchSize ? outputSize : batchSize);
	size_t outputLinesNum = outputSize;

	d_ptr->outputZeroLineSize = outputLineSize;
	d_ptr->outputZeroLinesNum = outputLinesNum;

	size_t inputBufferSize = inputLineSize * inputLinesNum * sizeof(float);
	size_t outputBufferSize = outputLineSize * outputLinesNum * sizeof(float);

	try
	{
		createOpenCLMemoryBuffer(
			context,
			inputBufferSize,
			d_ptr->numInputZeroBuffers,
			d_ptr->inputZeroBuffers,
			CL_MEM_READ_WRITE);

		createOpenCLMemoryBuffer(
			context,
			outputBufferSize,
			d_ptr->numOutputZeroBuffers,
			d_ptr->outputZeroBuffers,
			CL_MEM_READ_WRITE);

		// Padded region should be initialized with zeros. 
		cl_mem* buffers = d_ptr->inputZeroBuffers;
		float zero = 0.0f;
		::clEnqueueFillBuffer(queue, buffers[0], &zero,
			sizeof(float), 0, inputBufferSize, 0, NULL, NULL);
		::clEnqueueFillBuffer(queue, buffers[1], &zero,
			sizeof(float), 0, inputBufferSize, 0, NULL, NULL);
	}
	catch (std::exception) {
		releaseZeroPaddingBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createResamplingOutputBuffers(cl_context & context)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(float) * d_ptr->inputDataSize;
	d_ptr->resampleBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->resampleBuffers,
			CL_MEM_READ_WRITE);
	}
	catch (std::exception) {
		releaseResamplingOutputBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createPhaseShiftBuffers(cl_context & context, cl_command_queue& queue)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(float) * d_ptr->inputDataSize;
	size_t indexSize = sizeof(float) * d_ptr->inputLineSize;

	d_ptr->phaseInputBufferSize = bufferSize;
	d_ptr->phaseOutputBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->phaseInputBuffers,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			2,
			d_ptr->phaseOutputBuffers,
			CL_MEM_READ_WRITE);

		createOpenCLMemoryBuffer(
			context,
			indexSize,
			1,
			d_ptr->phaseShiftBuffers,
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
		::clEnqueueFillBuffer(queue, getImpl().phaseInputBuffers[0], &zero,
			sizeof(float), 0, bufferSize, 0, NULL, NULL);

	}
	catch (std::exception) {
		releasePhaseShiftBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createMagnitudeHistogramBuffers(cl_context & context)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(unsigned int) * MAGNITUDE_BLOCK_BUFFER_SIZE;
	d_ptr->histogramBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->histogramBuffers,
			CL_MEM_READ_WRITE);
	}
	catch (std::exception) {
		releaseMagnitudeHistogramBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createGrayScaleImageBuffers(cl_context & context)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(unsigned char) * d_ptr->grayScaleImageSize;
	d_ptr->grayScaleBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->grayScaleBuffers,
			CL_MEM_WRITE_ONLY);
	}
	catch (std::exception) {
		releaseGrayScaleImageBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createAmplitudeImageBuffers(cl_context & context)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(float) * d_ptr->grayScaleImageSize;
	d_ptr->amplitudeBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->amplitudeBuffers,
			CL_MEM_WRITE_ONLY);
	}
	catch (std::exception) {
		releaseAmplitudeImageBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createAxialProjectionBuffers(cl_context & context)
{
#ifndef _NCLFFT
	size_t bufferSize = sizeof(unsigned char) * FRAME_DATA_LINES_MAX;
	d_ptr->projectionBufferSize = bufferSize;

	try
	{
		createOpenCLMemoryBuffer(
			context,
			bufferSize,
			1,
			d_ptr->projectionBuffers,
			CL_MEM_WRITE_ONLY);
	}
	catch (std::exception) {
		releaseAxialProjectionBuffers();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createClFFTSetupPlan(cl_context& context, cl_command_queue& queue)
{
#ifndef _NCLFFT
	try
	{
		size_t batchSize = d_ptr->inputLinesNum;
		size_t inputSize = d_ptr->inputLineSize;
		size_t outputSize = d_ptr->outputLinesNum;

		clfftDim fftDim = CLFFT_1D;
		size_t lengths[1] = { inputSize };

		OPENCL_V_THROW(clfftSetup(getImpl().clfftSetup.get()), "clfftSetup failed");
		OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlan, context, fftDim, lengths), "clfftCreateDefaultPlan failed");

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
		OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlan, 1, &queue, NULL, NULL), "clfftBakePlan failed");

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

		OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanC2C, context, fftDim, lengths), "clfftCreateDefaultPlan failed");

		// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
		OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanC2C, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
		OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanC2C, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
		OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanC2C, batchSize), "clfftSetPlanBatchSize failed");
		OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanC2C, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

		OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanC2C, fftDim, i_strides), "clfftSetPlanInStride failed");
		OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanC2C, fftDim, o_strides), "clfftSetPlanOutStride failed");
		OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanC2C, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

		// Build fft execution plan.
		OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanC2C, 1, &queue, NULL, NULL), "clfftBakePlan failed");

	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createClFFTZerosPlan(cl_context& context, cl_command_queue& queue)
{
#ifndef _NCLFFT
	try
	{
		size_t inputSize = getImpl().inputZeroLineSize;
		size_t outputSize = inputSize;// (inputSize / 2 + 1);// getImpl().outputZeroLinesNum;
		size_t batchSize = getImpl().inputZeroLinesNum;

		clfftDim fftDim = CLFFT_1D;
		size_t lengths[1] = { inputSize };

		OPENCL_V_THROW(clfftSetup(getImpl().clfftSetup.get()), "clfftSetup failed");
		OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanZero, context, fftDim, lengths), "clfftCreateDefaultPlan failed");

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
		OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanZero, 1, &queue, NULL, NULL), "clfftBakePlan failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclTaskPlan::createClFFTPhasePlan(cl_context & context, cl_command_queue & queue)
{
#ifndef _NCLFFT
	try
	{
		size_t batchSize = d_ptr->inputLinesNum;
		size_t inputSize = d_ptr->inputLineSize;

		clfftDim fftDim = CLFFT_1D;
		size_t lengths[1] = { inputSize };

		// Foward direction FFT. 
		OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanFwd, context, fftDim, lengths), "clfftCreateDefaultPlan failed");

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
		OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanFwd, 1, &queue, NULL, NULL), "clfftBakePlan failed");


		// Backward direction FFT. 
		OPENCL_V_THROW(clfftCreateDefaultPlan(&getImpl().clfftPlanBwd, context, fftDim, lengths), "clfftCreateDefaultPlan failed");

		// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
		OPENCL_V_THROW(clfftSetResultLocation(getImpl().clfftPlanBwd, CLFFT_INPLACE), "clfftSetResultLocation failed");
		OPENCL_V_THROW(clfftSetLayout(getImpl().clfftPlanBwd, CLFFT_COMPLEX_PLANAR, CLFFT_COMPLEX_PLANAR), "clfftSetLayout failed");
		OPENCL_V_THROW(clfftSetPlanBatchSize(getImpl().clfftPlanBwd, batchSize), "clfftSetPlanBatchSize failed");
		OPENCL_V_THROW(clfftSetPlanPrecision(getImpl().clfftPlanBwd, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

		OPENCL_V_THROW(clfftSetPlanInStride(getImpl().clfftPlanBwd, fftDim, i_strides), "clfftSetPlanInStride failed");
		OPENCL_V_THROW(clfftSetPlanOutStride(getImpl().clfftPlanBwd, fftDim, o_strides), "clfftSetPlanOutStride failed");
		OPENCL_V_THROW(clfftSetPlanDistance(getImpl().clfftPlanBwd, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

		// Post callback function.
		// Note that phase shift memory buffer should be initialized before.
		OPENCL_V_THROW(clfftSetPlanCallback(getImpl().clfftPlanBwd, "hilbertPhaseShift", OclKernel::getHilbertPhaseShiftSource(), 0, POSTCALLBACK, &d_ptr->phaseShiftBuffers[0], 1), "clfftSetPlanCallback failed");

		// Build fft execution plan.
		OPENCL_V_THROW(clfftBakePlan(getImpl().clfftPlanBwd, 1, &queue, NULL, NULL), "clfftBakePlan failed");

	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


void SigChain::OclTaskPlan::releaseInputOutputBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(getImpl().numInputBuffers, getImpl().inputBuffers);
	releaseOpenCLMemBuffer(getImpl().numOutputBuffers, getImpl().outputBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseZeroPaddingBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(getImpl().numInputZeroBuffers, getImpl().inputZeroBuffers);
	releaseOpenCLMemBuffer(getImpl().numOutputZeroBuffers, getImpl().outputZeroBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseResamplingOutputBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().resampleBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releasePhaseShiftBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().phaseInputBuffers);
	releaseOpenCLMemBuffer(2, getImpl().phaseOutputBuffers);
	releaseOpenCLMemBuffer(1, getImpl().phaseShiftBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseMagnitudeHistogramBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().histogramBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseGrayScaleImageBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().grayScaleBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseAmplitudeImageBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().amplitudeBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseAxialProjectionBuffers(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().projectionBuffers);
	return;
#endif
}


void SigChain::OclTaskPlan::releaseClFFTSetupPlan(void)
{
#ifndef _NCLFFT
	try {
		if (getImpl().clfftPlan > 0) {
			OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlan), "clfftDestroyPlan failed");
			getImpl().clfftPlan = 0;
		}
		if (getImpl().clfftPlanC2C > 0) {
			OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanC2C), "clfftDestroyPlan failed");
			getImpl().clfftPlanC2C = 0;
		}
	}
	catch (std::exception) {
	}
	return;
#endif
}


void SigChain::OclTaskPlan::releaseClFFTZerosPlan(void)
{
#ifndef _NCLFFT
	try {
		if (getImpl().clfftPlanZero > 0) {
			OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanZero), "clfftDestroyPlan failed");
			getImpl().clfftPlanZero = 0;
		}
	}
	catch (std::exception) {
	}
	return;
#endif
}


void SigChain::OclTaskPlan::releaseClFFTPhasePlan(void)
{
#ifndef _NCLFFT
	try {
		if (getImpl().clfftPlanFwd > 0) {
			OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanFwd), "clfftDestroyPlan failed");
			getImpl().clfftPlanFwd = 0;
		}
		if (getImpl().clfftPlanBwd > 0) {
			OPENCL_V_THROW(clfftDestroyPlan(&getImpl().clfftPlanBwd), "clfftDestroyPlan failed");
			getImpl().clfftPlanBwd = 0;
		}
	}
	catch (std::exception) {
	}
	return;
#endif
}


cl_mem * SigChain::OclTaskPlan::resampleBuffers(int index) const
{
	return (d_ptr->resampleBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::phaseInputBuffers(int index) const
{
	return (d_ptr->phaseInputBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::phaseOutputBuffers(int index) const
{
	return (d_ptr->phaseOutputBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::phaseShiftBuffers(int index) const
{
	return (d_ptr->phaseShiftBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::histogramBuffers(int index) const
{
	return (d_ptr->histogramBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::grayScaleBuffers(int index) const
{
	return (d_ptr->grayScaleBuffers + index);
}

cl_mem * SigChain::OclTaskPlan::amplitudeBuffers(int index) const
{
	return (d_ptr->amplitudeBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::projectionBuffers(int index) const
{
	return (d_ptr->projectionBuffers + index);
}


size_t SigChain::OclTaskPlan::resampleBufferSize(void)
{
	return d_ptr->resampleBufferSize;
}


size_t SigChain::OclTaskPlan::histogramBufferSize(void)
{
	return d_ptr->histogramBufferSize;
}


size_t SigChain::OclTaskPlan::grayScaleBufferSize(void)
{
	return d_ptr->grayScaleBufferSize;
}

size_t SigChain::OclTaskPlan::amplitudeBufferSize(void)
{
	return d_ptr->amplitudeBufferSize;
}


size_t SigChain::OclTaskPlan::projectionBufferSize(void)
{
	return d_ptr->projectionBufferSize;
}


size_t SigChain::OclTaskPlan::inputLineSize(void) const
{
	return d_ptr->inputLineSize;
}


size_t SigChain::OclTaskPlan::inputLinesNum(void) const
{
	return d_ptr->inputLinesNum;
}


size_t SigChain::OclTaskPlan::outputLineSize(void) const
{
	return d_ptr->outputLineSize;
}


size_t SigChain::OclTaskPlan::outputLineSizePadded(void) const
{
	return d_ptr->outputLineSizePadded;
}


size_t SigChain::OclTaskPlan::outputLinesNum(void) const
{
	return d_ptr->outputLinesNum;
}


size_t SigChain::OclTaskPlan::inputBufferSize(void) const
{
	return d_ptr->inputBufferSize;
}


size_t SigChain::OclTaskPlan::outputBufferSize(void) const
{
	return d_ptr->outputBufferSize;
}


size_t SigChain::OclTaskPlan::inputZeroBufferSize(void) const
{
	return d_ptr->inputZeroBufferSize;
}


size_t SigChain::OclTaskPlan::outputZeroBufferSize(void) const
{
	return d_ptr->outputZeroBufferSize;
}


cl_mem * SigChain::OclTaskPlan::inputBuffers(int index) const
{
	return (d_ptr->inputBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::outputBuffers(int index) const
{
	return (d_ptr->outputBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::inputZeroBuffers(int index) const
{
	return (d_ptr->inputZeroBuffers + index);
}


cl_mem * SigChain::OclTaskPlan::outputZeroBuffers(int index) const
{
	return (d_ptr->outputZeroBuffers + index);
}
