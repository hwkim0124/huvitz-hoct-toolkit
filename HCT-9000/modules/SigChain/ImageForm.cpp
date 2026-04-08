#include "stdafx.h"
#include "ImageForm.h"
#include "ChainSetup.h"
#include "CppUtil2.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>

// Enable OpenCL C++ exceptions. 
#define CL_HPP_ENABLE_EXCEPTIONS

// OpenCL 2.X version of the C++ bindings
#define CL_HPP_TARGET_OPENCL_VERSION	200
#include <CL/cl2.hpp>	
#include "openCL.misc.h"

#ifndef _NCLFFT
#include <clFFT.h>
#endif

using namespace SigChain;


struct ImageForm::ImageFormImpl
{
	bool initialized;

	std::vector<cl_device_id> clDeviceIds;
#ifndef _NCLFFT
	std::unique_ptr<clfftSetupData> clfftSetup;
#endif
	cl_context clContext;

	ImageFormImpl() : initialized(false) {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<ImageForm::ImageFormImpl> ImageForm::d_ptr(new ImageFormImpl());


ImageForm::ImageForm()
{
}


ImageForm::~ImageForm()
{
}


ImageForm::ImageFormImpl & SigChain::ImageForm::getImpl(void)
{
	return *d_ptr;
}


bool SigChain::ImageForm::performFFT(unsigned short * lineBuffer, int ccdPixels, int numOfLines)
{
	try
	{
		//CppUtil::ClockTimer::start();
		if (!initializeOclSystem()) {
			return false;
		}
		DebugOut2() << "Ocl initialized: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

		float *real = nullptr; // ChainSetup::getOutputRealBuffer();
		float *imag = nullptr; // ChainSetup::getOutputImagBuffer();
		int out_length;
		
		
		if (transform_forward(lineBuffer, ccdPixels, numOfLines, real, imag, &out_length)) {
			//ChainSetup::setOutputLineLength(out_length);
			//ChainSetup::setOutputLineCount(numOfLines);
		}
		DebugOut2() << "Image transformed: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

		/*
		if (transform_forward(lineBuffer, ccdPixels, numOfLines, real, imag, &out_length)) {
			ChainSetup::setOutputLineLength(out_length);
			ChainSetup::setOutputLineCount(numOfLines);
		}
		DebugOut2() << "Image transformed: " << CppUtil::ClockTimer::elapsedMsec() << " msec";
		*/
	}
	catch (std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return false;
	}
	return true;
}


bool SigChain::ImageForm::performFFT(float* lineBuffer, int linePixels, int numOfLines)
{
#ifndef _NCLFFT
	try
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		cl_context_properties cprops[] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[0])(), 0
		};
		cl::Context context(CL_DEVICE_TYPE_GPU, cprops);

		std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		cl::CommandQueue queue(context, devices[0], 0);

		clfftSetupData fftSetup;
		cl_int err = clfftInitSetupData(&fftSetup);
		err = clfftSetup(&fftSetup);


		size_t Nx = linePixels;
		size_t lineSize = Nx * sizeof(float);
		size_t inplaceSize = (1 + Nx / 2) * 2;
		size_t batchSize = numOfLines;
		size_t buffSize = batchSize * inplaceSize *sizeof(float);

		// Copy each line into inplace fft buffer with padding. 
		float* hostBuffer = (float*) malloc(buffSize);
		for (int i = 0; i < batchSize; i++) {
			memcpy(hostBuffer + i*inplaceSize, lineBuffer + i*linePixels, lineSize);
		}

		cl::Buffer fftBuffer = cl::Buffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR, 
									buffSize, (void*)&hostBuffer[0], &err);
	
		//cl::Buffer fftBuffer = cl::Buffer(context, CL_MEM_READ_WRITE,
		//							buffSize, (void*)&hostBuffer, &err);

		clfftPlanHandle plan;
		clfftDim fftDim = CLFFT_1D; 
		size_t clLengths[1] = { Nx };

		err = clfftCreateDefaultPlan(&plan, context(), fftDim, clLengths);
		err = clfftSetPlanPrecision(plan, CLFFT_SINGLE);
		err = clfftSetLayout(plan, CLFFT_REAL, CLFFT_HERMITIAN_INTERLEAVED);
		err = clfftSetResultLocation(plan, CLFFT_INPLACE);
		err = clfftSetPlanBatchSize(plan, batchSize);

		err = clfftBakePlan(plan, 1, &queue(), NULL, NULL);
		err = clfftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue(), 0, NULL, NULL, &fftBuffer(), NULL, NULL);
		
		queue.finish();
		hostBuffer = (float*)queue.enqueueMapBuffer(fftBuffer, CL_TRUE, CL_MAP_READ, 0, buffSize);
		err = queue.enqueueUnmapMemObject(fftBuffer, (void*)hostBuffer);

		err = clfftDestroyPlan(&plan);
		clfftTeardown();
		return true;
	}
	catch (cl::Error err) {
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}
#endif
	return true;
}


bool SigChain::ImageForm::initializeOclSystem(void)
{
#ifndef _NCLFFT
	if (getImpl().initialized == false) 
	{
		cl_int deviceType = CL_DEVICE_TYPE_GPU;
		cl_int deviceId = 0;
		cl_int platformId = 0;

		getImpl().clDeviceIds = initializeCL(deviceType, deviceId, platformId, getImpl().clContext, false);

		// Initialize clFFT library. 
		getImpl().clfftSetup.reset(new clfftSetupData);
		OPENCL_V_THROW(clfftInitSetupData(getImpl().clfftSetup.get()), "clfftInitSetupData failed");

		getImpl().initialized = true;
	}

	return getImpl().initialized;
#else
	return false;
#endif
}


bool SigChain::ImageForm::transform_forward(unsigned short* inputBuffer, int dataLength, int batchSize, float* outputReal, float* outputImag, int* outputLength)
{
#ifndef _NCLFFT
	size_t Nx = dataLength;
	size_t Nt = 1 + Nx / 2;

	// Real to Hermitian planar.
	size_t fftVectorSize = Nx;
	size_t fftVectorSizePadded = Nx;
	size_t fftBatchSize = fftVectorSizePadded * batchSize;

	size_t outfftVectorSize = Nx;
	size_t outfftVectorSizePadded = Nt;
	size_t outfftBatchSize = outfftVectorSizePadded * batchSize;

	// It needs two output buffers with real and imag. 
	cl_uint num_of_output_buffers = 2;
	size_t size_of_output_buffers_in_bytes = outfftBatchSize * sizeof(float);

	// Type conversion of input data would be performed via fft pre callback.
	cl_uint num_of_input_buffers = 1;
	size_t size_of_input_buffers_in_bytes = fftBatchSize * sizeof(unsigned short);

	// Create command queue and in-out opencl buffers. 
	cl_command_queue queue;
	cl_uint command_queue_flags = 0;

	cl_mem input_cl_mem_buffers[2] = { NULL, NULL };
	cl_mem output_cl_mem_buffers[2] = { NULL, NULL };

	DebugOut2() << "transform_forward: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	createOpenCLCommandQueue(getImpl().clContext, command_queue_flags, queue,
		getImpl().clDeviceIds, 
		size_of_input_buffers_in_bytes, num_of_input_buffers, input_cl_mem_buffers, 
		size_of_output_buffers_in_bytes, num_of_output_buffers, output_cl_mem_buffers);

	DebugOut2() << "queue created: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	// Copy input host data into device buffer.
	OPENCL_V_THROW(clEnqueueWriteBuffer(queue, input_cl_mem_buffers[0], CL_TRUE,
		0, size_of_input_buffers_in_bytes, &inputBuffer[0], 0, NULL, NULL),
		"clEnqueueWriteBuffer failed");

	DebugOut2() << "FFT output copied: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	clfftPlanHandle plan;
	clfftDim fftDim = CLFFT_1D;
	size_t lengths[1] = { Nx };

	OPENCL_V_THROW(clfftSetup(getImpl().clfftSetup.get()), "clfftSetup failed");
	OPENCL_V_THROW(clfftCreateDefaultPlan(&plan, getImpl().clContext, fftDim, lengths), "clfftCreateDefaultPlan failed");

	DebugOut2() << "FFT setup created: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	// Default plan creates a plan that expects an inPlace transform with interleaved complex numbers
	OPENCL_V_THROW(clfftSetResultLocation(plan, CLFFT_OUTOFPLACE), "clfftSetResultLocation failed");
	OPENCL_V_THROW(clfftSetLayout(plan, CLFFT_REAL, CLFFT_HERMITIAN_PLANAR), "clfftSetLayout failed");
	OPENCL_V_THROW(clfftSetPlanBatchSize(plan, batchSize), "clfftSetPlanBatchSize failed");
	OPENCL_V_THROW(clfftSetPlanPrecision(plan, CLFFT_SINGLE), "clfftSetPlanPrecision failed");

	size_t i_strides[4] = { 1, 0, 0, 0 };
	size_t o_strides[4] = { 1, 0, 0, 0 };

	i_strides[1] = i_strides[2] = i_strides[3] = Nx;
	o_strides[1] = o_strides[2] = o_strides[3] = Nt;

	// Transposed output (from row major array to column major array).
	o_strides[0] = Nt;
	o_strides[1] = o_strides[2] = o_strides[3] = 1;

	OPENCL_V_THROW(clfftSetPlanInStride(plan, fftDim, i_strides), "clfftSetPlanInStride failed");
	OPENCL_V_THROW(clfftSetPlanOutStride(plan, fftDim, o_strides), "clfftSetPlanOutStride failed");
	OPENCL_V_THROW(clfftSetPlanDistance(plan, i_strides[3], o_strides[3]), "clfftSetPlanDistance failed");

	// Set precallback function for input data type conversion.
	const char* precallbackstr = "float convert16To32bit(__global void* input, uint inoffset, __global void* userdata)\n" \
		"{\n" \
		"__global ushort* p = (__global ushort*) input; \n" \
		"return (float) p[inoffset]; \n" \
		"}\n";

	OPENCL_V_THROW(clfftSetPlanCallback(plan, "convert16To32bit", precallbackstr, 0, PRECALLBACK, NULL, 0), "clfftSetPlanCallback failed");

	// Build fft execution plan.
	OPENCL_V_THROW(clfftBakePlan(plan, 1, &queue, NULL, NULL), "clfftBakePlan failed");

	// Get the buffersize
	size_t buffersize = 0;
	OPENCL_V_THROW(clfftGetTmpBufSize(plan, &buffersize), "clfftGetTmpBufSize failed");

	// Allocate the intermediate buffer
	cl_mem clMedBuffer = NULL;

	if (buffersize) {
		cl_int medstatus;
		clMedBuffer = clCreateBuffer(getImpl().clContext, CL_MEM_READ_WRITE, buffersize, 0, &medstatus);
		OPENCL_V_THROW(medstatus, "Creating intmediate Buffer failed");
	}

	DebugOut2() << "FFT prepared: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	// Execute fft forward transform.
	cl_mem *outBuffers = &output_cl_mem_buffers[0];
	OPENCL_V_THROW(clfftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue, 0, NULL, NULL,
		&input_cl_mem_buffers[0], outBuffers, clMedBuffer),
		"clfftEnqueueTransform failed");

	OPENCL_V_THROW(clFinish(queue), "clFinish failed");

	DebugOut2() << "FFT executed: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	
	// Release the intermediate buffer.
	if (clMedBuffer) {
		clReleaseMemObject(clMedBuffer);
	}

	// Read output data. 
	OPENCL_V_THROW(clEnqueueReadBuffer(queue, outBuffers[0], CL_TRUE, 0, size_of_output_buffers_in_bytes, 
		&outputReal[0], 0, NULL, NULL),
		"Reading the result buffer failed");
	OPENCL_V_THROW(clEnqueueReadBuffer(queue, outBuffers[1], CL_TRUE, 0, size_of_output_buffers_in_bytes, 
		&outputImag[0], 0, NULL, NULL),
		"Reading the result buffer failed");

	*outputLength = (int)outfftVectorSizePadded;

	DebugOut2() << "FFT output read: " << CppUtil::ClockTimer::elapsedMsec() << " msec";

	OPENCL_V_THROW(clfftDestroyPlan(&plan), "clfftDestroyPlan failed");
	OPENCL_V_THROW(clfftTeardown(), "clfftTeardown failed");

	cleanupCL(&getImpl().clContext, &queue, countOf(input_cl_mem_buffers), input_cl_mem_buffers, countOf(output_cl_mem_buffers), output_cl_mem_buffers, NULL);

#endif
	return true;
}

/*
bool SigChain::ImageForm::performFFT2(unsigned short* lineBuffer, int lineSize, int numOfLines, float* outBuffer)
{
	try
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		cl_context_properties cprops[] = {
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[0])(), 0
		};
		cl::Context context(CL_DEVICE_TYPE_GPU, cprops);

		std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
		cl::CommandQueue queue(context, devices[0], 0);

		clfftSetupData fftSetup;
		cl_int err = clfftInitSetupData(&fftSetup);
		err = clfftSetup(&fftSetup);

		size_t Nx = lineSize;
		size_t batchSize = numOfLines;
		size_t inBuffSize = Nx * sizeof(unsigned short) * batchSize;

		// Copy line camera data into device memory.
		cl::Buffer dInBuff = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
										inBuffSize, (void*)&lineBuffer[0], &err);

		// Create buffer using host memory for output.
		size_t outBuffSize = (1 + Nx / 2) * sizeof(float) * 2;
		cl::Buffer dOutBuff = cl::Buffer(context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR,
										outBuffSize, (void*)&outBuffer[0], &err);

		
		clfftPlanHandle plan;
		clfftDim fftDim = CLFFT_1D;
		size_t clLengths[1] = { Nx };

		err = clfftCreateDefaultPlan(&plan, context(), fftDim, clLengths);
		err = clfftSetPlanPrecision(plan, CLFFT_SINGLE);
		err = clfftSetLayout(plan, CLFFT_REAL, CLFFT_HERMITIAN_PLANAR);
		err = clfftSetResultLocation(plan, CLFFT_OUTOFPLACE);
		err = clfftSetPlanBatchSize(plan, batchSize);


		const char* precallbackstr = "float convert16To32bit(__global void* in, uint inoffset, __global void* userdata)\n" \
			"{\n" \
			"	__global cl_ushort* p = (__global cl_ushort*) in; \n" \
			"	return (float) p[inoffset]; \n" \
			"}\n";

		err = clfftSetPlanCallback(plan, "convert16To32bit", precallbackstr, 0, PRECALLBACK, NULL, 0);


		err = clfftBakePlan(plan, 1, &queue(), NULL, NULL);
		err = clfftEnqueueTransform(plan, CLFFT_FORWARD, 1, &queue(), 0, NULL, NULL, &dInBuff(), &dOutBuff(), NULL);

		queue.finish();
		hostBuffer = (float*)queue.enqueueMapBuffer(fftBuffer, CL_TRUE, CL_MAP_READ, 0, buffSize);
		err = queue.enqueueUnmapMemObject(fftBuffer, (void*)hostBuffer);

		err = clfftDestroyPlan(&plan);
		clfftTeardown();
		return true;
	}
	catch (cl::Error err) {
		std::cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << std::endl;
		return false;
	}
	return true;
}
*/