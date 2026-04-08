#include "stdafx.h"
#include "OclContext.h"
#include "OclKernel.h"
#include "OclTaskPlan.h"
#include "ChainSetup.h"
#include "AngioSetup.h"
#include "Resampler.h"

#include "CppUtil2.h"
#include "SigProc2.h"

using namespace SigChain;
using namespace SigProc;
using namespace CppUtil;
using namespace std;


struct OclContext::OclContextImpl
{
	bool initiated;
	OclTaskPlan* currTaskPlan;

	cl_context clContext;
	cl_command_queue clQueue;
	std::vector<cl_device_id> clDeviceIds;

	cl_program clSubtractProgram;
	cl_kernel clSubtractKernel;
	cl_kernel clSubtractKernel2;
	cl_kernel clSubtractKernel3;
	cl_kernel clSubtractKernel4;
	cl_mem subtractBuffers[1];
	size_t subtractBufferSize;

	cl_program clWindowingProgram;
	cl_kernel clWindowingKernel;
	cl_mem windowingBuffers[1];
	size_t windowingBufferSize;

	cl_program clResampleProgram;
	cl_kernel clResampleKernel;
	cl_mem resampleSplineBuffers[4];
	cl_mem resampleIndexBuffers[1];
	cl_mem resampleKValueBuffers[4];
	size_t resampleSplineBufferSize;
	size_t resampleIndexBufferSize;
	size_t resampleKValueBufferSize;

	cl_program clReductionProgram;
	cl_kernel clReductionRealKernel;
	cl_kernel clReductionImagKernel;

	cl_program clMagnitudeProgram;
	cl_kernel clMagnitudeKernel;
	cl_kernel clMagnitudeKernel2;

	cl_program clHistogramProgram;
	cl_kernel clHistogramKernel;

	cl_program clGrayScaleProgram;
	cl_kernel clGrayScaleOutputKernel;

	cl_program clAmplitudeProgram;
	cl_kernel clAmplitudeOutputKernel;

	cl_program clProjectionProgram;
	cl_kernel clProjectionKernel;


	std::vector<OclTaskPlan> taskPlans;


	OclContextImpl() : initiated(false), currTaskPlan(NULL),
		subtractBuffers{ NULL }, windowingBuffers{ NULL },
		resampleSplineBuffers{ NULL }, resampleIndexBuffers{ NULL }, resampleKValueBuffers{ NULL }
	{
		clSubtractProgram = NULL;
		clSubtractKernel = NULL;
		clSubtractKernel2 = NULL;
		clSubtractKernel3 = NULL;
		clSubtractKernel4 = NULL;

		clWindowingProgram = NULL;
		clWindowingKernel = NULL;

		clResampleProgram = NULL;
		clResampleKernel = NULL;

		clReductionProgram = NULL;
		clReductionImagKernel = NULL;
		clReductionRealKernel = NULL;

		clMagnitudeProgram = NULL;
		clMagnitudeKernel = NULL;
		clMagnitudeKernel2 = NULL;

		clHistogramProgram = NULL;
		clHistogramKernel = NULL;

		clGrayScaleProgram = NULL;
		clGrayScaleOutputKernel = NULL;

		clAmplitudeProgram = NULL;
		clAmplitudeOutputKernel = NULL;

		clProjectionProgram = NULL;
		clProjectionKernel = NULL;
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<OclContext::OclContextImpl> OclContext::d_ptr(new OclContextImpl());


OclContext::OclContext()
{
}


OclContext::~OclContext()
{
}


bool SigChain::OclContext::initializeContext(void)
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
		LogD() << "Failed to initialize OpenCL with gpu device!";
	}

	// Use CPU device platform as a fallback.
	if (deviceId < 0) {
		try {
			deviceType = CL_DEVICE_TYPE_CPU;
			getImpl().clDeviceIds = initializeCL(deviceType, deviceId, platformId, getImpl().clContext, false);
		}
		catch (std::exception) {
			LogD() << "Failed to initialize OpenCL with cpu device!";
			return false;
		}
	}

	getImpl().initiated = true;
#endif

	return true;
}


bool SigChain::OclContext::isInitiated(void)
{
	return getImpl().initiated;
}


void SigChain::OclContext::releaseContext(void)
{
	if (!isInitiated()) {
		return;
	}

	try
	{
		releaseTaskPlans();
		releaseKernels();
	}
	catch (std::exception) {
	}

	getImpl().initiated = false;
	return;
}


bool SigChain::OclContext::prepareContext(void)
{
	if (!isInitiated()) {
		return false;
	}

	if (!prepareResamplingParameters()) {
		return false;
	}

	if (!prepareSubtractionParameters()) {
		return false;
	}

	if (!prepareWindowingParameters()) {
		return false;
	}
	return true;
}


bool SigChain::OclContext::executeContext(OclTaskPlan* plan, const std::uint16_t* input, 
										std::uint8_t* output, std::uint8_t* laterals, float* output2)
{
	if (!isInitiated()) {
		return false;
	}

	if (plan == nullptr) {
		return false;
	}

	if (DEBUG_OUT) {
		CppUtil::ClockTimer::start();
		DebugOut2() << "Starting execution of Ocl context";
	}

	if (!performSpectrumDataCallback(plan, input)) {
		return false;
	}

	if (!performBackgroundSubtraction(plan, input)) {
		return false;
	}

	if (!performResamplingInput(plan)) {
		return false;
	}

	if (!preparePhaseShiftParameters(plan)) {
		return false;
	}

	if (!performPhaseShiftTransform(plan)) {
		return false;
	}

	if (!performZeroPaddingInput(plan)) {
		return false;
	}

	if (!performInverseTransform(plan)) {
		return false;
	}

	if (!performZeroPaddingOutput(plan)) {
		return false;
	}

	if (!performFixedNoiseReduction(plan)) {
		return false;
	}

	if (!performMagnitudeOfComplex(plan)) {
		return false;
	}

	if (!performMagnitudeHistogram(plan)) {
		return false;
	}

	performPhaseShiftOptimization(plan);

	if (!performAdaptiveGrayScale(plan, output)) {
		return false;
	}

	if (!performAxialProjection(plan, laterals)) {
		return false;
	}

	if (output2 != nullptr) {
		if (!performIntensityAmplitude(plan, output2)) {
			return false;
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "End of execution of Ocl context";
	}

	return true;
}


bool SigChain::OclContext::performPhaseShiftOptimization(OclTaskPlan * plan)
{
	if (!plan->isTaskPlanForPreview()) {
		return true;
	}
	if (ChainSetup::useRefreshDispersionParams()) {
		return true;
	}

	auto* corr = ChainSetup::getPhaseCorrector();
	if (!ChainSetup::isDynamicDispersionCorrection()) {
		corr->clearPhaseShiftTargetCounts(true);
		return true;
	}

	auto preset1 = ChainSetup::getCurrentPhaseShiftOffset(0);
	auto preset2 = ChainSetup::getCurrentPhaseShiftOffset(1);
	auto quality = plan->indexOfImageQuality();

	if (corr->updateImageProcessedCount(quality)) {
		corr->clearPhaseShiftSignalQueues();

		auto index = 0;
		auto table = corr->getPhaseShiftDistanceTable(index);
		auto signal = 0.0;
		auto offset1 = corr->getPhaseShiftOffset1();
		auto offset2 = corr->getPhaseShiftOffset2();
		
		for (auto delta : table) {
			auto trial1 = delta + offset1;
			auto trial2 = 0.0 + offset2;

			if (delta == 0.0) {
				signal = plan->ratioOfImageSignal();
				corr->putPhaseShiftSignalResult(index, delta, signal);
			}
			else {
				ChainSetup::prepareDispersionCorrection(true, trial1, trial2);

				if (!preparePhaseShiftParameters(plan)) {
					return false;
				}

				if (!performPhaseShiftTransform(plan)) {
					return false;
				}

				if (!performInverseTransform(plan)) {
					return false;
				}

				if (!performFixedNoiseReduction(plan)) {
					return false;
				}

				if (!performMagnitudeOfComplex(plan)) {
					return false;
				}

				if (!performMagnitudeHistogram(plan)) {
					return false;
				}

				signal = plan->ratioOfImageSignal();
				corr->putPhaseShiftSignalResult(index, delta, signal);
			}
		}

		corr->optimizePhaseShiftOffsets();
		offset1 = corr->getPhaseShiftOffset1();
		offset2 = corr->getPhaseShiftOffset2();
		ChainSetup::prepareDispersionCorrection(true, offset1, offset2);
	}
	return true;
}


OclTaskPlan * SigChain::OclContext::requestTaskPlanForPreview(int dataLen, int batchSize)
{
	OclTaskPlan* plan = requestTaskPlan(dataLen, batchSize, true);

	if (plan != nullptr) {
		plan->isTaskPlanForPreview(true, true);
	}

	if (!plan->isInitiated()) {
		plan->initializeTaskPlan(dataLen, batchSize);
	}
	return plan;
}


OclTaskPlan * SigChain::OclContext::requestTaskPlanForEnface(int dataLen, int batchSize)
{
	OclTaskPlan* plan = requestTaskPlan(dataLen, batchSize, true);

	if (plan != nullptr) {
		plan->isTaskPlanForEnface(true, true);
	}

	if (!plan->isInitiated()) {
		plan->initializeTaskPlan(dataLen, batchSize);
	}
	return plan;
}


OclTaskPlan * SigChain::OclContext::requestTaskPlanForMeasure(int dataLen, int batchSize)
{
	OclTaskPlan* plan = requestTaskPlan(dataLen, batchSize, true);

	if (plan != nullptr) {
		plan->isTaskPlanForMeasure(true, true);
	}

	if (!plan->isInitiated()) {
		plan->initializeTaskPlan(dataLen, batchSize);
	}
	return plan;
}


OclTaskPlan * SigChain::OclContext::getTaskPlanForPreview(int dataLen, int batchSize)
{
	for (OclTaskPlan& plan : d_ptr->taskPlans) {
		if (plan.isTaskPlanForPreview() && plan.isIdenticalDataLayout(dataLen, batchSize)) {
			return &plan;
		}
	}
	return nullptr;
}


OclTaskPlan * SigChain::OclContext::getTaskPlanForEnface(int dataLen, int batchSize)
{
	for (OclTaskPlan& plan : d_ptr->taskPlans) {
		if (plan.isTaskPlanForEnface() && plan.isIdenticalDataLayout(dataLen, batchSize)) {
			return &plan;
		}
	}
	return nullptr;
}


OclTaskPlan * SigChain::OclContext::getTaskPlanForMeasure(int dataLen, int batchSize)
{
	for (OclTaskPlan& plan : d_ptr->taskPlans) {
		if (plan.isTaskPlanForMeasure() && plan.isIdenticalDataLayout(dataLen, batchSize)) {
			return &plan;
		}
	}
	return nullptr;
}


OclTaskPlan* SigChain::OclContext::requestTaskPlan(int dataLen, int batchSize, bool unique)
{
	int index = -1;

	if (!unique) {
		// Allow use of pre-existing task plan with same capacity.
		for (int i = 0; i < d_ptr->taskPlans.size(); i++) {
			auto item = &d_ptr->taskPlans[i];
			if (item->isIdenticalDataLayout(dataLen, batchSize)) {
				index = i;
				break;
			}
		}
	}
	else {
		index = insertTaskPlan(dataLen, batchSize);
	}

	d_ptr->currTaskPlan = &d_ptr->taskPlans[index];
	return getCurrentTaskPlan();
}


int SigChain::OclContext::insertTaskPlan(int dataLen, int batchSize)
{
	OclTaskPlan task;

	d_ptr->taskPlans.push_back(std::move(task));
	int index = (int)(d_ptr->taskPlans.size() - 1);
	return index;
}


void SigChain::OclContext::releaseTaskPlans(void)
{
	for (OclTaskPlan& plan : d_ptr->taskPlans) {
		plan.releaseTaskPlan();
	}

	d_ptr->taskPlans.clear();
	return;
}


OclTaskPlan * SigChain::OclContext::getCurrentTaskPlan(void)
{
	/*
	int index = d_ptr->indexOfCurrentTaskPlan;
	if (index < 0 || index >= d_ptr->taskPlans.size()) {
	return nullptr;
	}

	return &d_ptr->taskPlans[index];
	*/
	return d_ptr->currTaskPlan;
}


void SigChain::OclContext::setCurrentTaskPlan(OclTaskPlan * plan)
{
	d_ptr->currTaskPlan = plan;
	return;
}


bool SigChain::OclContext::realizeTaskPlan(OclTaskPlan * plan)
{
	if (!isInitiated()) {
		return false;
	}

	bool ret = plan->realizeTaskPlan(getImpl().clContext, getImpl().clQueue);

	return ret;
}


bool SigChain::OclContext::prepareResamplingParameters(void)
{
#ifndef _NCLFFT
	try
	{
		Resampler* resampler = ChainSetup::getResampler();

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleSplineBuffers[0], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize,
			resampler->getCubicSplineA(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleSplineBuffers[1], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize,
			resampler->getCubicSplineB(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleSplineBuffers[2], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize,
			resampler->getCubicSplineC(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleSplineBuffers[3], CL_TRUE, 0,
			getImpl().resampleSplineBufferSize,
			resampler->getCubicSplineD(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleIndexBuffers[0], CL_TRUE, 0,
			getImpl().resampleIndexBufferSize,
			resampler->getLowerPixelIndex(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleKValueBuffers[0], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize,
			resampler->getkValueBetweenUpperAndLower(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleKValueBuffers[1], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize,
			resampler->getkValueBetweenLowerAndPrevLower(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleKValueBuffers[2], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize,
			resampler->getkValueBetweenNextUpperAndUpper(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().resampleKValueBuffers[3], CL_TRUE, 0,
			getImpl().resampleKValueBufferSize,
			resampler->getkValueLinearized(), 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::prepareSubtractionParameters(void)
{
#ifndef _NCLFFT
	try {
		unsigned short* spectrum = ChainSetup::getBackgroundSpectrum();

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().subtractBuffers[0], CL_TRUE, 0,
			getImpl().subtractBufferSize, spectrum, 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}

bool SigChain::OclContext::prepareWindowingParameters(void)
{
#ifndef _NCLFFT
	try {
		float* window = ChainSetup::getWindowingFunctionData();

		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			getImpl().windowingBuffers[0], CL_TRUE, 0,
			getImpl().windowingBufferSize, window, 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::preparePhaseShiftParameters(OclTaskPlan* plan)
{
#ifndef _NCLFFT
	try {
		float* shifts = ChainSetup::prepareDispersionCorrection();
		size_t bufferSize = sizeof(float) * LINE_CAMERA_CCD_PIXELS;

		// The amount of Phase shift values along wave numbers linearized around wavelength 
		// center should be prepared at scan operation initialization. 
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			*plan->phaseShiftBuffers(0), CL_TRUE, 0,
			bufferSize, shifts, 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::performSpectrumDataCallback(OclTaskPlan* plan, const std::uint16_t * input)
{
	if (plan->useSpectrumDataCallback())
	{
		int width = (int)plan->inputLineSize();
		int height = (int)plan->inputLinesNum();

		SpectrumDataCallback* callback = ChainSetup::getSpectrumDataCallback();
		if (callback) {
			(*callback)(const_cast<unsigned short*>(input), width, height);
			if (DEBUG_OUT) {
				DebugOut2() << "Spectrum data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
	}
	return true;
}


bool SigChain::OclContext::performBackgroundSubtraction(OclTaskPlan * plan, const std::uint16_t * input)
{
#ifndef _NCLFFT
	try
	{
		// Copy input host data into device buffer.
		OPENCL_V_THROW(clEnqueueWriteBuffer(getImpl().clQueue,
			*plan->inputBuffers(0), CL_TRUE, 0,
			plan->inputBufferSize(), &input[0], 0, NULL, NULL),
			"clEnqueueWriteBuffer failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Input lines data copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		unsigned int numRows = (unsigned int)plan->inputLinesNum();
		unsigned int numCols = (unsigned int)plan->inputLineSize();


		if (plan->useBackgroundSubtraction())
		{
			if (ChainSetup::useFFTWindowing()) {
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel3, 0, sizeof(cl_mem), plan->inputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel3, 1, sizeof(cl_mem), &getImpl().subtractBuffers[0]), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel3, 2, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel3, 3, sizeof(cl_uint), &numRows), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel3, 4, sizeof(cl_uint), &numCols), "clKernel failed");

				const size_t globalWorkSize[2] = { numCols, numRows };
				::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel3, 2,
					NULL, globalWorkSize, NULL, 0, NULL, NULL);
				OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");
			}
			else {
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 0, sizeof(cl_mem), plan->inputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 1, sizeof(cl_mem), &getImpl().subtractBuffers[0]), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 2, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 3, sizeof(cl_uint), &numRows), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel, 4, sizeof(cl_uint), &numCols), "clKernel failed");

				const size_t globalWorkSize[2] = { numCols, numRows };
				::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel, 2,
					NULL, globalWorkSize, NULL, 0, NULL, NULL);
				OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");
			}
		}
		else
		{
			if (ChainSetup::useFFTWindowing()) {
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel4, 0, sizeof(cl_mem), plan->inputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel4, 1, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel4, 2, sizeof(cl_uint), &numRows), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel4, 3, sizeof(cl_uint), &numCols), "clKernel failed");

				const size_t globalWorkSize[2] = { numCols, numRows };
				::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel4, 2,
					NULL, globalWorkSize, NULL, 0, NULL, NULL);
				OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");
			}
			else {
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 0, sizeof(cl_mem), plan->inputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 1, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 2, sizeof(cl_uint), &numRows), "clKernel failed");
				OPENCL_V_THROW(::clSetKernelArg(getImpl().clSubtractKernel2, 3, sizeof(cl_uint), &numCols), "clKernel failed");

				const size_t globalWorkSize[2] = { numCols, numRows };
				::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clSubtractKernel2, 2,
					NULL, globalWorkSize, NULL, 0, NULL, NULL);
				OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");
			}
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

bool SigChain::OclContext::performWindowingSpectrum(OclTaskPlan* plan)
{
#ifndef _NCLFFT
	try
	{
		unsigned int numRows = (unsigned int)plan->inputLinesNum();
		unsigned int numCols = (unsigned int)plan->inputLineSize();

		if (plan->useWindowingFunction())
		{
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clWindowingKernel, 0, sizeof(cl_mem), plan->resampleBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clWindowingKernel, 1, sizeof(cl_mem), &getImpl().windowingBuffers[0]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clWindowingKernel, 2, sizeof(cl_mem), plan->resampleBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clWindowingKernel, 3, sizeof(cl_uint), &numRows), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clWindowingKernel, 4, sizeof(cl_uint), &numCols), "clKernel failed");

			const size_t globalWorkSize[2] = { numCols, numRows };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clWindowingKernel, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);
			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");
		}
		if (DEBUG_OUT) {
			DebugOut2() << "Windowing spectrum: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}

bool SigChain::OclContext::performResamplingInput(OclTaskPlan * plan)
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

		unsigned int numRows = (unsigned int)plan->inputLinesNum();
		unsigned int numCols = (unsigned int)plan->inputLineSize();

		if (plan->useKLinearResampling()) {
			float kInterval = ChainSetup().getResampler()->getLinearizedKValueInterval();

			// The argument data pointed by arg_value is copied and the arg_value pointer can 
			// therefore be resused by the application after clSetKernelArg returns.
			// The argument values specified is the value used by all API calls that enqueue 
			// kernel until the argument values is changed by a call to clSetKernelArg for kernel.
			// https://www.khronos.org/registry/cl/sdk/1.1/docs/man/xhtml/clSetKernelArg.html
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 0, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 1, sizeof(cl_mem), plan->resampleBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 4, sizeof(cl_mem), &getImpl().resampleSplineBuffers[0]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 5, sizeof(cl_mem), &getImpl().resampleSplineBuffers[1]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 6, sizeof(cl_mem), &getImpl().resampleSplineBuffers[2]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 7, sizeof(cl_mem), &getImpl().resampleSplineBuffers[3]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 8, sizeof(cl_mem), &getImpl().resampleIndexBuffers[0]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 9, sizeof(cl_mem), &getImpl().resampleKValueBuffers[0]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 10, sizeof(cl_mem), &getImpl().resampleKValueBuffers[1]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 11, sizeof(cl_mem), &getImpl().resampleKValueBuffers[2]), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clResampleKernel, 12, sizeof(cl_float), &kInterval), "clKernel failed");

			const size_t globalWorkSize[2] = { numCols, numRows };
			::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clResampleKernel, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);
			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed!");

			if (DEBUG_OUT) {
				DebugOut2() << "Resampling input data: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
		else {
			size_t srcOrgin[3] = { 0 };
			size_t dstOrgin[3] = { 0 };
			size_t region[3] = { 0 };

			region[0] = numCols * sizeof(float);
			region[1] = numRows;
			region[2] = 1;

			size_t srcRowPitch = numCols * sizeof(float);
			size_t srcSlicePitch = 0;
			size_t dstRowPitch = numCols * sizeof(float);
			size_t dstSlicePitch = 0;

			::clEnqueueCopyBufferRect(getImpl().clQueue,
				*plan->outputBuffers(0),
				*plan->resampleBuffers(0),
				srcOrgin, dstOrgin, region,
				srcRowPitch, srcSlicePitch,
				dstRowPitch, dstSlicePitch,
				0, NULL, NULL);
		}

		if (!performWindowingSpectrum(plan)) {
			return false;
		}

		if (plan->useResampleDataCallback())
		{
			float* output = ChainSetup::getResampleFrameData();
			uint32_t width = numCols;
			uint32_t height = numRows;

			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				*plan->resampleBuffers(0), CL_TRUE, 0, plan->resampleBufferSize(),
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");

			ResampleDataCallback* callback = ChainSetup::getResampleDataCallback();
			if (callback) {
				(*callback)(output, width, height);
				if (DEBUG_OUT) {
					DebugOut2() << "Resampling data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
				}
			}
		}
	}
	catch (std::exception) {
		return false;
	}

#endif
	return true;
}


bool SigChain::OclContext::performPhaseShiftTransform(OclTaskPlan * plan)
{
#ifndef _NCLFFT
	try {
		if (plan->useDispersionCompensation() == false) {
			return true;
		}

		// CppUtil::ClockTimer::start();
		// LogD() << "Phase shift transform";

		// Execute fft forward transform.
		cl_mem inputBuffers[2] = { *plan->resampleBuffers(0), *plan->phaseInputBuffers(0) };
		cl_mem outputBuffers[2] = { *plan->phaseOutputBuffers(0), *plan->phaseOutputBuffers(1) };

		OPENCL_V_THROW(clfftEnqueueTransform(
			plan->clfftPlanFwd(),
			CLFFT_FORWARD, 1,
			&getImpl().clQueue, 0, NULL, NULL,
			inputBuffers, outputBuffers, NULL), //clMedBuffer),
			"clfftEnqueueTransform failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Phase shift input data: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clfftEnqueueTransform(
			plan->clfftPlanBwd(),
			CLFFT_BACKWARD, 1,
			&getImpl().clQueue, 0, NULL, NULL,
			outputBuffers, outputBuffers, NULL), //clMedBuffer),
			"clfftEnqueueTransform failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		// LogD() << "Phase shift input data: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
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


bool SigChain::OclContext::performZeroPaddingInput(OclTaskPlan * plan)
{
	try {
		if (plan->useZeroPadding() == false) {
			return true;
		}

		unsigned int numRows = (unsigned int)plan->inputLinesNum();
		unsigned int numCols = (unsigned int)plan->inputLineSize();

		size_t srcOrgin[3] = { 0 };
		size_t dstOrgin[3] = { 0 };
		size_t region[3] = { 0 };

		region[0] = numCols * sizeof(float);
		region[1] = numRows;
		region[2] = 1;

		unsigned int numColsZero = (unsigned int)plan->inputZeroLineSize();

		size_t srcRowPitch = numCols * sizeof(float);
		size_t srcSlicePitch = 0;// src_row_pitch * getImpl().inputLinesNum;
		size_t dstRowPitch = numColsZero * sizeof(float);
		size_t dstSlicePitch = 0;// dst_row_pitch * getImpl().inputZeroLinesNum;

		if (plan->useDispersionCompensation())
		{
			::clEnqueueCopyBufferRect(getImpl().clQueue,
				*plan->phaseOutputBuffers(0),
				*plan->inputZeroBuffers(0),
				srcOrgin, dstOrgin, region,
				srcRowPitch, srcSlicePitch,
				dstRowPitch, dstSlicePitch,
				0, NULL, NULL);

			::clEnqueueCopyBufferRect(getImpl().clQueue,
				*plan->phaseOutputBuffers(1),
				*plan->inputZeroBuffers(1),
				srcOrgin, dstOrgin, region,
				srcRowPitch, srcSlicePitch,
				dstRowPitch, dstSlicePitch,
				0, NULL, NULL);
		}
		else
		{
			::clEnqueueCopyBufferRect(getImpl().clQueue,
				*plan->resampleBuffers(0),
				*plan->inputZeroBuffers(0),
				srcOrgin, dstOrgin, region,
				srcRowPitch, srcSlicePitch,
				dstRowPitch, dstSlicePitch,
				0, NULL, NULL);
		}
#ifndef _NCLFFT
		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
#endif
		if (DEBUG_OUT) {
			DebugOut2() << "Zero padded input copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
	return true;
}


bool SigChain::OclContext::performInverseTransform(OclTaskPlan * plan)
{
#ifndef _NCLFFT
	try {
		if (plan->useZeroPadding())
		{
			OPENCL_V_THROW(clfftEnqueueTransform(
				plan->clfftPlanZero(),
				CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				plan->inputZeroBuffers(),
				plan->outputZeroBuffers(),
				NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		}
		else if (plan->useDispersionCompensation())
		{
			// CppUtil::ClockTimer::start();
			// LogD() << "FFT transform";

			OPENCL_V_THROW(clfftEnqueueTransform(
				plan->clfftPlanC2C(),
				CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				plan->phaseOutputBuffers(),
				plan->outputBuffers(),
				NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

			// LogD() << "FFT transformed: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
		else
		{
			// Execute fft forward transform.'
			/*
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
			*/

			OPENCL_V_THROW(clfftEnqueueTransform(
				plan->clfftPlan(),
				CLFFT_FORWARD, 1,
				&getImpl().clQueue, 0, NULL, NULL,
				plan->resampleBuffers(),
				plan->outputBuffers(),
				NULL), //clMedBuffer),
				"clfftEnqueueTransform failed");

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

			/*
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
			*/
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


bool SigChain::OclContext::performZeroPaddingOutput(OclTaskPlan * plan)
{
	try {
		if (plan->useZeroPadding() == false) {
			return true;
		}

		unsigned int numRows = (unsigned int)plan->outputLinesNum();
		unsigned int numCols = (unsigned int)plan->outputLineSize();

		size_t srcOrgin[3] = { 0 };
		size_t dstOrgin[3] = { 0 };
		size_t region[3] = { 0 };

		region[0] = numCols * sizeof(float);
		region[1] = numRows;
		region[2] = 1;

		unsigned int numColsZero = (unsigned int)plan->outputZeroLineSize();
		unsigned int numColsPadd = (unsigned int)plan->outputLineSizePadded();

		// Copy initial part of zero padded result into output buffer.
		size_t srcRowPitch = numColsZero * sizeof(float);
		size_t srcSlicePitch = 0;// src_row_pitch * getImpl().outputZeroLinesNum;
		size_t dstRowPitch = numColsPadd * sizeof(float);
		size_t dstSlicePitch = 0;// dst_row_pitch * getImpl().outputLinesNum;

		::clEnqueueCopyBufferRect(getImpl().clQueue,
			*plan->outputZeroBuffers(0),
			*plan->outputBuffers(0),
			srcOrgin, dstOrgin, region,
			srcRowPitch, srcSlicePitch,
			dstRowPitch, dstSlicePitch,
			0, NULL, NULL);

		::clEnqueueCopyBufferRect(getImpl().clQueue,
			*plan->outputZeroBuffers(1),
			*plan->outputBuffers(1),
			srcOrgin, dstOrgin, region,
			srcRowPitch, srcSlicePitch,
			dstRowPitch, dstSlicePitch,
			0, NULL, NULL);

#ifndef _NCLFFT
		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
#endif

		if (DEBUG_OUT) {
			DebugOut2() << "Zero padded output copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
	return true;
}


bool SigChain::OclContext::performFixedNoiseReduction(OclTaskPlan * plan)
{
#ifndef _NCLFFT
	try {
		if (plan->useFixedNoiseReduction() == false) {
			return true;
		}

		// CppUtil::ClockTimer::start();
		// LogD() << "Fixed noise reduction";

		unsigned int numRows = (unsigned int)plan->outputLinesNum();
		unsigned int numCols = (unsigned int)plan->outputLineSize();
		unsigned int lineSize = (unsigned int)plan->outputLineSizePadded();
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 0, sizeof(cl_mem), plan->outputBuffers()), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 1, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 2, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 3, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionRealKernel, 4, sizeof(cl_uint), &lineSize), "clKernel failed");

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 0, sizeof(cl_mem), plan->outputBuffers(1)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 1, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 2, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 3, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clReductionImagKernel, 4, sizeof(cl_uint), &lineSize), "clKernel failed");


		size_t height = plan->grayScaleImageHeight();
		const size_t globalWorkSize[1] = { height };

		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clReductionRealKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL);

		::clEnqueueNDRangeKernel(getImpl().clQueue, getImpl().clReductionImagKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Fixed noise reduction: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		// LogD() << "Fixed noise reduction: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::performMagnitudeOfComplex(OclTaskPlan * plan)
{
#ifndef _NCLFFT
	try {
		unsigned int numRows = (unsigned int)plan->grayScaleImageHeight();
		unsigned int numCols = (unsigned int)plan->grayScaleImageWidth();
		unsigned int lineSize = (unsigned int)plan->outputLineSizePadded();
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();

		if (plan->useIntensityDataCallback())
		{
			rowOffset = 0;
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 0, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 1, sizeof(cl_mem), plan->outputBuffers(1)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 2, sizeof(cl_mem), plan->outputBuffers(2)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 3, sizeof(cl_uint), &numRows), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 4, sizeof(cl_uint), &numCols), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 5, sizeof(cl_uint), &rowOffset), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel2, 6, sizeof(cl_uint), &lineSize), "clKernel failed");

			unsigned int numRows2 = (unsigned int)plan->outputLinesNum();
			unsigned int numCols2 = (unsigned int)plan->outputLineSize();

			const size_t globalWorkSize[2] = { numCols2, numRows2 };
			::clEnqueueNDRangeKernel(getImpl().clQueue,
				getImpl().clMagnitudeKernel2, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

			if (DEBUG_OUT) {
				DebugOut2() << "Complex magnitude: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}

			// int byteOffset = FFT_IMAGE_ROW_OFFSET * getImpl().outputLineSizePadded * sizeof(float);
			// int byteReads = FFT_IMAGE_ROW_SIZE * getImpl().outputLineSizePadded * sizeof(float);
			size_t byteOffset = 0; // 512 * getImpl().outputLineSizePadded * sizeof(float);
			size_t byteCount = numRows2 * lineSize * sizeof(float);

			float* output = ChainSetup::getMagnitudeFrameData();
			uint32_t width = (uint32_t)numCols2; // getImpl().outputLineSizePadded;
			uint32_t height = (uint32_t)numRows2;

			OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
				*plan->outputBuffers(2),
				CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
				&output[0], 0, NULL, NULL),
				"Reading the result buffer failed");

			IntensityDataCallback* callback = ChainSetup::getIntensityDataCallback();
			if (callback) {
				(*callback)(output, width, height);
			}

			if (DEBUG_OUT) {
				DebugOut2() << "Intensity data callback: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}
		}
		else
		{
			// CppUtil::ClockTimer::start();
			// LogD() << "Complex magnitude";

			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 0, sizeof(cl_mem), plan->outputBuffers(0)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 1, sizeof(cl_mem), plan->outputBuffers(1)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 2, sizeof(cl_mem), plan->outputBuffers(2)), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 3, sizeof(cl_uint), &numRows), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 4, sizeof(cl_uint), &numCols), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 5, sizeof(cl_uint), &rowOffset), "clKernel failed");
			OPENCL_V_THROW(::clSetKernelArg(getImpl().clMagnitudeKernel, 6, sizeof(cl_uint), &lineSize), "clKernel failed");

			const size_t globalWorkSize[2] = { numCols, numRows };
			::clEnqueueNDRangeKernel(getImpl().clQueue,
				getImpl().clMagnitudeKernel, 2,
				NULL, globalWorkSize, NULL, 0, NULL, NULL);

			OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");

			if (DEBUG_OUT) {
				DebugOut2() << "Complex magnitude: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
			}

			if (plan->isTaskPlanForMeasure()) {
				if (AngioSetup::isSavingBuffersToFiles() && ChainSetup::isAngioScan())
				{
					unsigned int numRows2 = (unsigned int)plan->outputLinesNum();
					unsigned int numCols2 = (unsigned int)plan->outputLineSize();

					size_t byteOffset = 0;
					size_t byteCount = numRows2 * lineSize * sizeof(float);
					uint32_t width = (uint32_t)numCols2; // getImpl().outputLineSizePadded;
					uint32_t height = (uint32_t)numRows2;
					uint32_t linePadds = (lineSize - width);
				
					AngioSetup::resetImageBuffers(width, height, 0);
					auto buffer = vector<float>(numRows2 * lineSize, 0.0f);
					auto vflip = AngioSetup::isEdiScanningMode();

					OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
						*plan->outputBuffers(0),
						CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
						&buffer[0], 0, NULL, NULL),
						"Reading real part of the result buffer failed");

					AngioSetup::copyImageRealsToBuffer(&buffer[0], width, height, linePadds, vflip);

					OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
						*plan->outputBuffers(1),
						CL_TRUE, byteOffset, byteCount, //getImpl().outputBufferSizeInBytes,
						&buffer[0], 0, NULL, NULL),
						"Reading real part of the result buffer failed");

					AngioSetup::copyImageImagsToBuffer(&buffer[0], width, height, linePadds, vflip);
				}
			}

			// LogD() << "Complex magnitude: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::performMagnitudeHistogram(OclTaskPlan * plan)
{
#ifndef _NCLFFT
	try {
		unsigned int numRows = (unsigned int)plan->grayScaleImageHeight(); //getImpl().outputLinesNum;
		unsigned int numCols = (unsigned int)plan->grayScaleImageWidth(); // getImpl().outputLineSize;
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();
		unsigned int lineSize = (unsigned int)plan->outputLineSizePadded();

		unsigned int columnStep = (numCols / MAGNITUDE_SCAN_LINES_NUM);
		unsigned int blockSize = MAGNITUDE_BLOCK_ITEMS;
		unsigned int histBins = MAGNITUDE_HISTOGRAM_BINS;
		unsigned int divider = MAGNITUDE_VALUE_DIVIDER;

		float sigRatio = (ChainSetup::isCorneaScan() ? GRAY_SCALE_SIGNAL_SIZE_RATIO_CORNEA : GRAY_SCALE_SIGNAL_SIZE_RATIO_RETINA);
		float bgdRatio = (ChainSetup::isCorneaScan() ? GRAY_SCALE_GROUND_SIZE_RATIO_CORNEA : GRAY_SCALE_GROUND_SIZE_RATIO_RETINA);

		unsigned int bgdSize = (unsigned int)(numRows * bgdRatio);
		unsigned int sigSize = (unsigned int)(numRows * sigRatio);

		sigSize = (sigSize <= 0 ? 1 : sigSize);

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 0, sizeof(cl_mem), plan->outputBuffers(2)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clHistogramKernel, 1, sizeof(cl_mem), plan->histogramBuffers(0)), "clKernel failed");
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

		unsigned int numScanLines = MAGNITUDE_SCAN_LINES_NUM;
		unsigned int blocks[MAGNITUDE_BLOCK_BUFFER_SIZE];

		const size_t globalWorkSize[1] = { numScanLines };
		::clEnqueueNDRangeKernel(getImpl().clQueue,
			getImpl().clHistogramKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL);

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Magnitude histogram: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			*plan->histogramBuffers(0),
			CL_TRUE, 0, plan->histogramBufferSize(),
			&blocks, 0, NULL, NULL),
			"Reading the result buffer failed");

		unsigned int lineStart = (ChainSetup::isCorneaScan() ? MAGNITUDE_START_LINES_CORNEA : MAGNITUDE_START_LINES_RETINA);
		unsigned int lineClose = (ChainSetup::isCorneaScan() ? MAGNITUDE_CLOSE_LINES_CORNEA : MAGNITUDE_CLOSE_LINES_RETINA);

		unsigned int sum[5] = { 0 };
		unsigned int lineCount = 0;
		
		for (unsigned int i = lineStart; i <= lineClose; i++) {
			/*
			if (blocks[i*MAGNITUDE_BLOCK_ITEMS + 2] >= MAGNITUDE_BACKGROUND_MAX) {
				continue;
			}
			*/
			if (ChainSetup::isCorneaScan()) {
				if (i >= MAGNITUDE_START_CENTER_CORNEA && i <= MAGNITUDE_CLOSE_CENTER_CORNEA) {
					continue;
				}
			}
			else {
				if (i >= MAGNITUDE_START_CENTER_RETINA && i <= MAGNITUDE_CLOSE_CENTER_RETINA) {
					continue;
				}
			}

			sum[0] += blocks[i*MAGNITUDE_BLOCK_ITEMS];
			sum[1] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 1];
			sum[2] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 2];
			sum[3] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 3];
			sum[4] += blocks[i*MAGNITUDE_BLOCK_ITEMS + 4];
			lineCount++;
		}

		unsigned int magnPeakIndexAverage = 0; // (unsigned int)(sum[0] / numScanLines);
		unsigned int magnPeakLevelAverage = 0; // (unsigned int)(sum[1] / numScanLines);
		unsigned int peakAvg = (unsigned int)(lineCount <= 0 ? 0.0f : (float)sum[1] / lineCount);

		if (lineCount > 0) {
			unsigned int peakPhaseSum = 0;
			unsigned int peakIndexSum = 0;
			unsigned int peakCount = 0;

			for (unsigned int i = lineStart; i <= lineClose; i++) {
				/*
				if (blocks[i*MAGNITUDE_BLOCK_ITEMS + 2] >= MAGNITUDE_BACKGROUND_MAX) {
					continue;
				}
				*/
				if (ChainSetup::isCorneaScan()) {
					if (i >= MAGNITUDE_START_CENTER_CORNEA && i <= MAGNITUDE_CLOSE_CENTER_CORNEA) {
						continue;
					}
				}
				else {
					if (i >= MAGNITUDE_START_CENTER_RETINA && i <= MAGNITUDE_CLOSE_CENTER_RETINA) {
						continue;
					}
				}

				int peakPhase = (blocks[i*MAGNITUDE_BLOCK_ITEMS + 1] - peakAvg);
				if (peakPhase >= 0) {
					peakPhaseSum += (peakPhase + 1);
					peakIndexSum += (blocks[i*MAGNITUDE_BLOCK_ITEMS] * peakPhase);
					peakCount++;
				}
			}

			if (peakCount > 0) {
				// Closer to genuine peak level on the image. 
				magnPeakIndexAverage = peakIndexSum / peakPhaseSum;
				magnPeakLevelAverage = peakPhaseSum / peakCount + peakAvg;
			}
		}

		float sigAvg = (lineCount <= 0 ? 0.0f : (float)sum[2] / lineCount);
		float bgdAvg = (lineCount <= 0 ? 0.0f : (float)sum[3] / lineCount);
		float bgdStd = (lineCount <= 0 ? 0.0f : (float)sum[4] / lineCount);
		float snrRate = 0.0f, quality = 0.0f, drange = 0.0f;
		
		if (bgdAvg <= 0.0f) {
			snrRate = 0.0f;
			quality = 0.0f;
		}
		else {
			drange = sigAvg - bgdAvg;
			// snrRate = (float)(MAGNITUDE_SNR_BASE * log10((sigAvg / (bgdAvg + 1.0f))));
			snrRate = (float)(MAGNITUDE_SNR_BASE * log10((drange / (bgdStd + 0.01f))));

			// Image quality is determined from the SNR range predefined as exposure.  
			float snrMin, snrMax;
			if (ChainSetup::isSpeedFastest()) {
				snrMin = IMAGE_QUALITY_SNR_MIN_FASTEST;
				snrMax = IMAGE_QUALITY_SNR_MAX_FASTEST;
			}
			else if (ChainSetup::isSpeedFaster()) {
				snrMin = IMAGE_QUALITY_SNR_MIN_FASTER;
				snrMax = IMAGE_QUALITY_SNR_MAX_FASTER;
			}
			else {
				snrMin = IMAGE_QUALITY_SNR_MIN_NORMAL;
				snrMax = IMAGE_QUALITY_SNR_MAX_NORMAL;
			}
			if (ChainSetup::isCorneaScan()) {
				snrMin *= IMAGE_QUALITY_SNR_MIN_CORNEA_RATIO;
				snrMax *= IMAGE_QUALITY_SNR_MAX_CORNEA_RATIO;
			}

			/*
			float rateDiff = snrRate - snrMin;
			if (rateDiff >= 1.0f) {
				float logMin = 0.0f;
				float logMax = log10((snrMax - snrMin) * 0.25f + 0.75f);
				float logRate = log10(rateDiff * 0.25f + 0.75f);
				quality = ((logRate - logMin) / (logMax - logMin)) * IMAGE_QUALITY_INDEX_SIZE;
				quality = max(quality, IMAGE_QUALITY_INDEX_MIN);
				quality = min(quality, IMAGE_QUALITY_INDEX_MAX);
			}
			else {
				quality = 0.0f;
			}
			*/
			quality = snrRate;
			quality = ((quality - snrMin) / (snrMax - snrMin)) * IMAGE_QUALITY_INDEX_SIZE;
			quality = min(max(quality, IMAGE_QUALITY_INDEX_MIN), IMAGE_QUALITY_INDEX_MAX);
		}

		float magnSignalNoiseRatio = snrRate;
		float groundLevel = bgdAvg; // +1.0f;
		float signalLevel = sigAvg; // +1.0f;
		// float signalLevel = peakAvg + 1.0f;

		float magnSignalLevelLog = log10(signalLevel + 0.0001f);
		float magnGroundLevelLog = log10(groundLevel + 0.0001f);

		float imageQuality = quality;
		unsigned int imageReferPoint = magnPeakIndexAverage;

		plan->imageSignalLevel(true, sigAvg);
		plan->imageNoiseLevel(true, bgdStd);
		plan->imageBackgroundLevel(true, bgdAvg);

		plan->logOfImageSignal(true, magnSignalLevelLog);
		plan->logOfImageBackground(true, magnGroundLevelLog);
		plan->indexOfImageQuality(true, imageQuality);
		plan->ratioOfImageSignal(true, magnSignalNoiseRatio);
		plan->referencePoint(true, imageReferPoint);
		
		// if (DEBUG_OUT) {
		/*
		static unsigned long long count = 0;
		if (++count % 100 == 0 || (plan->isTaskPlanForMeasure() || plan->isTaskPlanForPreview())) {
			LogD() << "Magnitude peak index: " << magnPeakIndexAverage;
			LogD() << "Magnitude peak level: " << magnPeakLevelAverage;
			LogD() << "Signal / Noise ratio: " << magnSignalNoiseRatio;
			LogD() << "Level bgd: " << bgdAvg << ", sig: " << sigAvg << ", peak: " << peakAvg;
			LogD() << "Signal level: " << signalLevel << ", log10: " << magnSignalLevelLog;
			LogD() << "Noise level: " << noiseLevel << ", log10: " << magnNoiseLevelLog;
			LogD() << "Image quality: " << imageQuality << ", snrRate: " << snrRate;
			LogD() << "Reference point: " << imageReferPoint;
		}
		*/
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::performAdaptiveGrayScale(OclTaskPlan * plan, std::uint8_t * output)
{
#ifndef _NCLFFT
	try {
		unsigned int numRows = (unsigned int)plan->grayScaleImageHeight(); //getImpl().outputLinesNum;
		unsigned int numCols = (unsigned int)plan->grayScaleImageWidth(); // getImpl().outputLineSize;
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();
		unsigned int lineSize = (unsigned int)plan->outputLineSizePadded();
		unsigned int linesNum = (unsigned int)plan->outputLinesNum();

		float sigLog = plan->logOfImageSignal();
		float bgdLog = plan->logOfImageBackground();

		float rangeMin;

		if (ChainSetup::isCorneaScan()) {
			if (ChainSetup::isSpeedFastest()) {
				rangeMin = GRAY_SCALE_LOG_RANGE_CORNEA_FASTEST;
			}
			else if (ChainSetup::isSpeedFaster()) {
				rangeMin = GRAY_SCALE_LOG_RANGE_CORNEA_FASTER;
			}
			else {
				rangeMin = GRAY_SCALE_LOG_RANGE_CORNEA_NORMAL;
			}
		}
		else {
			if (ChainSetup::isSpeedFastest()) {
				rangeMin = GRAY_SCALE_LOG_RANGE_RETINA_FASTEST;
			}
			else if (ChainSetup::isSpeedFaster()) {
				rangeMin = GRAY_SCALE_LOG_RANGE_RETINA_FASTER;
			}
			else {
				rangeMin = GRAY_SCALE_LOG_RANGE_RETINA_NORMAL;
			}
		}

		// Scaling parameters should be updated at running time. 
		if (plan->useAdaptiveGrayscale())
		{
			/*
			if ((sigLog - bgdLog) < rangeMin) {
				bgdLog += 0.0f;// GRAY_SCALE_LOG_RANGE_LOWER_OFFSET;
				sigLog = bgdLog + rangeMin;
			}
			else {
				bgdLog += GRAY_SCALE_LOG_RANGE_LOWER_OFFSET;
				sigLog += GRAY_SCALE_LOG_RANGE_UPPER_OFFSET;
			}
			*/
			// bgdLog += GRAY_SCALE_LOG_RANGE_LOWER_OFFSET;
			// sigLog += GRAY_SCALE_LOG_RANGE_UPPER_OFFSET;
			// bgdLog += 0.25f;
			if ((sigLog - bgdLog) < rangeMin) {
				sigLog = bgdLog + rangeMin;
			}
			else {
				sigLog += GRAY_SCALE_LOG_RANGE_UPPER_OFFSET;
			}
		}
		else {
			bgdLog = GRAY_SCALE_LOG_VALUE_MIN;
			sigLog = GRAY_SCALE_LOG_VALUE_MAX;
		}

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 0, sizeof(cl_mem), plan->outputBuffers(2)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 1, sizeof(cl_mem), plan->grayScaleBuffers(0)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 4, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 5, sizeof(cl_uint), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 6, sizeof(cl_uint), &linesNum), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 7, sizeof(cl_float), &sigLog), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clGrayScaleOutputKernel, 8, sizeof(cl_float), &bgdLog), "clKernel failed");


		const size_t globalWorkSize[2] = { numCols, numRows };
		OPENCL_V_THROW(clEnqueueNDRangeKernel(getImpl().clQueue,
			getImpl().clGrayScaleOutputKernel, 2,
			NULL, globalWorkSize, NULL, 0, NULL, NULL),
			"Gray scale output failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Gray scale output: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			*plan->grayScaleBuffers(0),
			CL_TRUE, 0, plan->grayScaleBufferSize(),
			&output[0], 0, NULL, NULL),
			"Reading the result buffer failed");

		if (AngioSetup::isEdiScanningMode()) {
			auto temp = vector<uint8_t>(numCols);
			auto size = sizeof(uint8_t) * numCols;
			for (unsigned int i = 0; i < numRows/2; i++) {
				auto *p1 = output + i * numCols;
				auto *p2 = output + (numRows - i - 1) * numCols;
				memcpy(&temp[0], p1, size);
				memcpy(p1, p2, size);
				memcpy(p2, &temp[0], size);
			}
		}

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


bool SigChain::OclContext::performIntensityAmplitude(OclTaskPlan * plan, float * output)
{
#ifndef _NCLFFT
	try {
		unsigned int numRows = (unsigned int)plan->grayScaleImageHeight(); //getImpl().outputLinesNum;
		unsigned int numCols = (unsigned int)plan->grayScaleImageWidth(); // getImpl().outputLineSize;
		unsigned int rowOffset = ChainSetup::getFFTImageRowOffset();
		unsigned int lineSize = (unsigned int)plan->outputLineSizePadded();
		unsigned int linesNum = (unsigned int)plan->outputLinesNum();

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 0, sizeof(cl_mem), plan->outputBuffers(2)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 1, sizeof(cl_mem), plan->amplitudeBuffers(0)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 4, sizeof(cl_uint), &rowOffset), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 5, sizeof(cl_uint), &lineSize), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clAmplitudeOutputKernel, 6, sizeof(cl_uint), &linesNum), "clKernel failed");

		const size_t globalWorkSize[2] = { numCols, numRows };
		OPENCL_V_THROW(clEnqueueNDRangeKernel(getImpl().clQueue,
			getImpl().clAmplitudeOutputKernel, 2,
			NULL, globalWorkSize, NULL, 0, NULL, NULL),
			"Amplitude output failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Amplitude output: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			*plan->amplitudeBuffers(0),
			CL_TRUE, 0, plan->amplitudeBufferSize(),
			&output[0], 0, NULL, NULL),
			"Reading the output buffer failed");

		if (AngioSetup::isEdiScanningMode()) {
			auto temp = vector<float>(numCols);
			auto size = sizeof(float) * numCols;
			for (unsigned int i = 0; i < numRows / 2; i++) {
				auto *p1 = output + i * numCols;
				auto *p2 = output + (numRows - i - 1) * numCols;
				memcpy(&temp[0], p1, size);
				memcpy(p1, p2, size);
				memcpy(p2, &temp[0], size);
			}
		}

		/*
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
		outBuffers[1], CL_TRUE, 0, getImpl().outputBufferSizeInBytes,
		&outputImag[0], 0, NULL, NULL),
		"Reading the result buffer failed");
		*/
		if (DEBUG_OUT) {
			DebugOut2() << "Amplitude image copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}
	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::performAxialProjection(OclTaskPlan * plan, std::uint8_t* laterals)
{
#ifndef _NCLFFT
	if (/* !plan->isTaskPlanForMeasure() ||*/ laterals == nullptr) {
		return true;
	}

	try {
		unsigned int numRows = (unsigned int)plan->grayScaleImageHeight(); //getImpl().outputLinesNum;
		unsigned int numCols = (unsigned int)plan->grayScaleImageWidth(); // getImpl().outputLineSize;
		unsigned int rowStart = 0;
		unsigned int rowClose = numRows - 1;

		if (plan->isTaskPlanForEnface()/* || plan->isTaskPlanForMeasure() */) {
			auto range = GlobalSettings::enfacePreviewRange();
			rowStart = min((unsigned int)max(range.first, 0), numRows - 1);
			rowClose = min((unsigned int)max(range.first, range.second), numRows - 1);
		}

		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 0, sizeof(cl_mem), plan->grayScaleBuffers(0)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 1, sizeof(cl_mem), plan->projectionBuffers(0)), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 2, sizeof(cl_uint), &numRows), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 3, sizeof(cl_uint), &numCols), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 4, sizeof(cl_uint), &rowStart), "clKernel failed");
		OPENCL_V_THROW(::clSetKernelArg(getImpl().clProjectionKernel, 5, sizeof(cl_uint), &rowClose), "clKernel failed");

		const size_t globalWorkSize[1] = { numCols };
		OPENCL_V_THROW(clEnqueueNDRangeKernel(getImpl().clQueue,
			getImpl().clProjectionKernel, 1,
			NULL, globalWorkSize, NULL, 0, NULL, NULL),
			"Axial projection failed");

		OPENCL_V_THROW(clFinish(getImpl().clQueue), "clFinish failed");
		if (DEBUG_OUT) {
			DebugOut2() << "Axial projection output: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

		std::uint8_t* profile = laterals; // plan->lateralProfile();
		OPENCL_V_THROW(clEnqueueReadBuffer(getImpl().clQueue,
			*plan->projectionBuffers(0),
			CL_TRUE, 0, plan->projectionBufferSize(),
			&profile[0], 0, NULL, NULL),
			"Reading the result buffer failed");

		if (DEBUG_OUT) {
			DebugOut2() << "Axial projection copied: " << CppUtil::ClockTimer::elapsedMsec() << " ms";
		}

	}
	catch (std::exception) {
		return false;
	}
#endif
	return true;
}

bool SigChain::OclContext::initializeKernels(void)
{
	if (!createCommandQueue()) {
		return false;
	}
	if (!createBackgroundSubtractionKernel()) {
		return false;
	}
	if (!createWindowingSpectrumKernel()) {
		return false;
	}
	if (!createResamplingInputKernel()) {
		return false;
	}
	if (!createFixedNoiseReductionKernel()) {
		return false;
	}
	if (!createComplexMagnitudeKernel()) {
		return false;
	}
	if (!createMagnitudeHistogramKernel()) {
		return false;
	}
	if (!createAdaptiveGrayScaleKernel()) {
		return false;
	}
	if (!createIntensityAmplitudeKernel()) {
		return false;
	}
	if (!createAxialProjectionKernel()) {
		return false;
	}
	return true;
}


void SigChain::OclContext::releaseKernels(void)
{
	releaseBackgroundSubtractionKernel();
	releaseResamplingInputKernel();
	releaseFixedNoiseReductionKernel();
	releaseComplexMagnitudeKernel();
	releaseMagnitudeHistogramKernel();
	releaseAdaptiveGrayScaleKernel();
	releaseIntensityAmplitudeKernel();
	releaseAxialProjectionKernel();
	releaseCommandQueue();
	return;
}


bool SigChain::OclContext::createCommandQueue(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		// Create command queue and in-out opencl buffers.
		cl_uint command_queue_flags = 0;
		createOpenCLCommandQueue(getImpl().clContext,
			command_queue_flags, getImpl().clQueue,
			getImpl().clDeviceIds, 0, 0, NULL, 0, 0, NULL);
	}
	catch (std::exception) {
		releaseCommandQueue();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createBackgroundSubtractionKernel(void)
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

		createOpenCLKernel(getImpl().clSubtractProgram,
			getImpl().clSubtractKernel3, "backgroundSubtraction3");

		createOpenCLKernel(getImpl().clSubtractProgram,
			getImpl().clSubtractKernel4, "backgroundSubtraction4");

		size_t bufferSize = sizeof(unsigned short) * LINE_CAMERA_CCD_PIXELS;
		getImpl().subtractBufferSize = bufferSize;

		createOpenCLMemoryBuffer(
			getImpl().clContext,
			bufferSize,
			1,
			getImpl().subtractBuffers,
			CL_MEM_READ_ONLY);
	}
	catch (std::exception) {
		releaseBackgroundSubtractionKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createResamplingInputKernel(void)
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

		size_t indexSize = sizeof(unsigned short) * LINE_CAMERA_CCD_PIXELS;
		size_t valueSize = sizeof(float) * LINE_CAMERA_CCD_PIXELS;

		getImpl().resampleKValueBufferSize = valueSize;
		getImpl().resampleIndexBufferSize = indexSize;
		getImpl().resampleSplineBufferSize = valueSize;

		createOpenCLMemoryBuffer(
			getImpl().clContext,
			indexSize,
			1,
			getImpl().resampleIndexBuffers,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(getImpl().clContext,
			valueSize,
			4,
			getImpl().resampleKValueBuffers,
			CL_MEM_READ_ONLY);

		createOpenCLMemoryBuffer(getImpl().clContext,
			valueSize,
			4,
			getImpl().resampleSplineBuffers,
			CL_MEM_READ_ONLY);
	}
	catch (std::exception) {
		releaseResamplingInputKernel();
		return false;
	}
#endif

	return true;
}

bool SigChain::OclContext::createWindowingSpectrumKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clWindowingProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getWindowingSpectrumSource());

		createOpenCLKernel(getImpl().clWindowingProgram,
			getImpl().clWindowingKernel, "windowingSpectrum");

		size_t bufferSize = sizeof(float) * LINE_CAMERA_CCD_PIXELS;
		getImpl().windowingBufferSize = bufferSize;

		createOpenCLMemoryBuffer(
			getImpl().clContext,
			bufferSize,
			1,
			getImpl().windowingBuffers,
			CL_MEM_READ_ONLY);
	}
	catch (std::exception) {
		releaseWindowingSpectrumKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createFixedNoiseReductionKernel(void)
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
	}
	catch (std::exception) {
		releaseFixedNoiseReductionKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createComplexMagnitudeKernel(void)
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
	}
	catch (std::exception) {
		releaseComplexMagnitudeKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createMagnitudeHistogramKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clHistogramProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getMagnitudeHistogramSource4());

		createOpenCLKernel(getImpl().clHistogramProgram,
			getImpl().clHistogramKernel, "magnitudeHistogram");
	}
	catch (std::exception) {
		releaseMagnitudeHistogramKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createAdaptiveGrayScaleKernel(void)
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
	}
	catch (std::exception) {
		releaseAdaptiveGrayScaleKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createIntensityAmplitudeKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clAmplitudeProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getIntensityAmplitudeSource());

		createOpenCLKernel(getImpl().clAmplitudeProgram,
			getImpl().clAmplitudeOutputKernel, "intensityAmplitude");
	}
	catch (std::exception) {
		releaseIntensityAmplitudeKernel();
		return false;
	}
#endif
	return true;
}


bool SigChain::OclContext::createAxialProjectionKernel(void)
{
#ifndef _NCLFFT
	if (!isInitiated()) {
		return false;
	}

	try {
		createOpenCLProgram(getImpl().clProjectionProgram,
			getImpl().clContext, getImpl().clDeviceIds[0],
			OclKernel::getAxialProjectionSource());

		createOpenCLKernel(getImpl().clProjectionProgram,
			getImpl().clProjectionKernel, "axialProjection");
	}
	catch (std::exception) {
		releaseAxialProjectionKernel();
		return false;
	}
#endif
	return true;
}


void SigChain::OclContext::releaseCommandQueue(void)
{
#ifndef _NCLFFT
	cleanupCL(&getImpl().clContext, &getImpl().clQueue,
		0, NULL, 0, NULL, NULL);
	return;
#endif
}


void SigChain::OclContext::releaseBackgroundSubtractionKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().subtractBuffers);
	releaseOpenCLKernel(getImpl().clSubtractKernel);
	releaseOpenCLKernel(getImpl().clSubtractKernel2);
	releaseOpenCLKernel(getImpl().clSubtractKernel3);
	releaseOpenCLKernel(getImpl().clSubtractKernel4);
	releaseOpenCLProgram(getImpl().clSubtractProgram);
	return;
#endif
}

void SigChain::OclContext::releaseWindowingSpectrumKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().windowingBuffers);
	releaseOpenCLKernel(getImpl().clWindowingKernel);
	releaseOpenCLProgram(getImpl().clWindowingProgram);
	return;
#endif
}


void SigChain::OclContext::releaseResamplingInputKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLMemBuffer(1, getImpl().resampleIndexBuffers);
	releaseOpenCLMemBuffer(4, getImpl().resampleSplineBuffers);
	releaseOpenCLMemBuffer(4, getImpl().resampleKValueBuffers);

	releaseOpenCLKernel(getImpl().clResampleKernel);
	releaseOpenCLProgram(getImpl().clResampleProgram);
	return;
#endif
}


void SigChain::OclContext::releaseFixedNoiseReductionKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clReductionRealKernel);
	releaseOpenCLKernel(getImpl().clReductionImagKernel);
	releaseOpenCLProgram(getImpl().clReductionProgram);
	return;
#endif
}


void SigChain::OclContext::releaseComplexMagnitudeKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clMagnitudeKernel);
	releaseOpenCLKernel(getImpl().clMagnitudeKernel2);
	releaseOpenCLProgram(getImpl().clMagnitudeProgram);
	return;
#endif
}


void SigChain::OclContext::releaseMagnitudeHistogramKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clHistogramKernel);
	releaseOpenCLProgram(getImpl().clHistogramProgram);
	return;
#endif
}


void SigChain::OclContext::releaseAdaptiveGrayScaleKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clGrayScaleOutputKernel);
	releaseOpenCLProgram(getImpl().clGrayScaleProgram);
	return;
#endif
}


void SigChain::OclContext::releaseIntensityAmplitudeKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clAmplitudeOutputKernel);
	releaseOpenCLProgram(getImpl().clAmplitudeProgram);
	return;
#endif
}


void SigChain::OclContext::releaseAxialProjectionKernel(void)
{
#ifndef _NCLFFT
	releaseOpenCLKernel(getImpl().clProjectionKernel);
	releaseOpenCLProgram(getImpl().clProjectionProgram);
	return;
#endif
}


OclContext::OclContextImpl & SigChain::OclContext::getImpl(void)
{
	return *d_ptr;
}
