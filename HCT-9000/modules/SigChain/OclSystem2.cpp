#include "stdafx.h"
#include "OclSystem2.h"
#include "OclKernel.h"
#include "OclContext.h"
#include "OclTaskPlan.h"
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


struct OclSystem2::OclSystem2Impl
{
	bool initiated;

	OclContext context;


	OclSystem2Impl() : initiated(false)
	{

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<OclSystem2::OclSystem2Impl> OclSystem2::d_ptr(new OclSystem2Impl());


OclSystem2::OclSystem2()
{
}


OclSystem2::~OclSystem2()
{
}


bool SigChain::OclSystem2::initializeOclSystem(void)
{
	if (isInitiated()) {
		return true;
	}

	if (!getContext().initializeContext()) {
		return false;
	}

	if (!getContext().initializeKernels()) {
		LogD() << "Failed to initialize OpenCL kernels!";
		return false;
	}

	d_ptr->initiated = true;
	return true;
}


bool SigChain::OclSystem2::isInitiated(void)
{
	return d_ptr->initiated;
}


void SigChain::OclSystem2::releaseOclSystem(void)
{
	if (!isInitiated()) {
		return;
	}

	getContext().releaseContext();
	getImpl().initiated = false;
	return;
}


bool SigChain::OclSystem2::initializeTransform(void)
{
	// If not user mode, all pre-existing tasks are cleared. 
	if (!GlobalSettings::useOclTasksDefaultPreset()) {
		getContext().releaseTaskPlans();
	}

	if (!prepareDefaultParameters()) {
		return false;
	}
	return true;
}


bool SigChain::OclSystem2::prepareDefaultTransforms(void)
{
	if (!prepareTransformForPreview(TASK_PLAN_DATA_WIDTH_PREVIEW, TASK_PLAN_DATA_HEIGHT_PREVIEW)) {
		return false;
	}

	if (!prepareTransformForMeasure(TASK_PLAN_DATA_WIDTH_MEASURE1, TASK_PLAN_DATA_HEIGHT_MEASURE1)) {
		return false;
	}

	if (!prepareTransformForMeasure(TASK_PLAN_DATA_WIDTH_MEASURE2, TASK_PLAN_DATA_HEIGHT_MEASURE2)) {
		return false;
	}

	if (!prepareTransformForMeasure(TASK_PLAN_DATA_WIDTH_MEASURE3, TASK_PLAN_DATA_HEIGHT_MEASURE3)) {
		return false;
	}

	if (!prepareTransformForEnface(TASK_PLAN_DATA_WIDTH_ENFACE, TASK_PLAN_DATA_HEIGHT_ENFACE)) {
		return false;
	}
	return true;
}


bool SigChain::OclSystem2::prepareTransformForPreview(int dataWidth, int dataHeight)
{
	if (!isInitiated()) {
		return false;
	}

	// Check pre-existing task plan for preview.
	OclTaskPlan* plan = getContext().getTaskPlanForPreview(dataWidth, dataHeight);
	if (plan != nullptr) {
		return true;
	}

	// Request a plan with same data layout, which is supposed to be reinitailized with configurations. 
	plan = getContext().requestTaskPlanForPreview(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	// Apply transform configurations.
	plan->useSpectrumDataCallback(true, ChainSetup::useSpectrumDataCallback());
	plan->useBackgroundSubtraction(true, ChainSetup::useBackgroundSubtraction());
	plan->useWindowingFunction(true, ChainSetup::useWindowingSpectrum());
	plan->useResampleDataCallback(true, ChainSetup::useResampleDataCallback());
	plan->useDispersionCompensation(true, ChainSetup::useDispersionCompensation());
	plan->useZeroPadding(true, ChainSetup::useFFTZeroPadding(), ChainSetup::getMultipleOfZeroPaddingSize());
	plan->useFixedNoiseReduction(true, ChainSetup::useFixedNoiseReduction());
	plan->useIntensityDataCallback(true, ChainSetup::useIntensityDataCallback());
	plan->useAdaptiveGrayscale(true, ChainSetup::useAdaptiveGrayscale());
	plan->useKLinearResampling(true, ChainSetup::useKLinearResampling());

	// Allocate device memory buffers.
	bool ret = getContext().realizeTaskPlan(plan);
	return ret;
}


bool SigChain::OclSystem2::prepareTransformForEnface(int dataWidth, int dataHeight)
{
	if (!isInitiated()) {
		return false;
	}

	// Check pre-existing task plan for enface.
	OclTaskPlan* plan = getContext().getTaskPlanForEnface(dataWidth, dataHeight);
	if (plan != nullptr) {
		return true;
	}

	// Request a plan with same data layout, which is supposed to be reinitailized with configurations. 
	plan = getContext().requestTaskPlanForEnface(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	// Apply transform configurations.
	plan->useSpectrumDataCallback(true, false);
	plan->useBackgroundSubtraction(true, ChainSetup::useBackgroundSubtraction());
	plan->useWindowingFunction(true, ChainSetup::useWindowingSpectrum());
	plan->useResampleDataCallback(true, false);
	plan->useDispersionCompensation(true, ChainSetup::useDispersionCompensation());
	plan->useZeroPadding(true, false);
	plan->useFixedNoiseReduction(true, ChainSetup::useFixedNoiseReduction());
	plan->useIntensityDataCallback(true, false);
	plan->useAdaptiveGrayscale(true, false);
	plan->useKLinearResampling(true, ChainSetup::useKLinearResampling());

	// Allocate device memory buffers.
	bool ret = getContext().realizeTaskPlan(plan);
	return ret;
}


bool SigChain::OclSystem2::prepareTransformForMeasure(int dataWidth, int dataHeight)
{
	if (!isInitiated()) {
		return false;
	}

	// Check pre-existing task plan for measure.
	OclTaskPlan* plan = getContext().getTaskPlanForMeasure(dataWidth, dataHeight);
	if (plan != nullptr) {
		return true;
	}

	// Request a plan with same data layout, which is supposed to be reinitailized with configurations. 
	plan = getContext().requestTaskPlanForMeasure(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	// Apply transform configurations.
	plan->useSpectrumDataCallback(true, false);
	plan->useBackgroundSubtraction(true, ChainSetup::useBackgroundSubtraction());
	plan->useWindowingFunction(true, ChainSetup::useWindowingSpectrum());
	plan->useResampleDataCallback(true, false);
	plan->useDispersionCompensation(true, ChainSetup::useDispersionCompensation());
	plan->useZeroPadding(true, false);
	plan->useFixedNoiseReduction(true, ChainSetup::useFixedNoiseReduction());
	plan->useIntensityDataCallback(true, false);
	plan->useAdaptiveGrayscale(true, ChainSetup::useAdaptiveGrayscale());
	plan->useKLinearResampling(true, ChainSetup::useKLinearResampling());

	// Allocate device memory buffers.
	bool ret = getContext().realizeTaskPlan(plan);
	return ret;
}


bool SigChain::OclSystem2::prepareDefaultParameters(void)
{
	if (!isInitiated()) {
		return false;
	}

	if (!getContext().prepareContext()) {
		return false;
	}

	return true;
}


void SigChain::OclSystem2::checkBackgroundSpectrum(const unsigned short * data, int width, int height, float quality)
{
	if (ChainSetup::isBackgroundSpectrum() == false) {
		if (ChainSetup::resetBackgroundSpectrum(data, width, height, quality)) {
			getContext().prepareSubtractionParameters();
			LogD() << "Background spectrum has been reset";
		}
	}
	return;
}


bool SigChain::OclSystem2::executeTransformForPreview(const std::uint16_t * input, int dataWidth, int dataHeight, std::uint8_t * output, 
														int* imageWidth, int* imageHeight, float* imageQindex,
														float* snrRatio, int* referPoint, float* amplitudes)
{
	OclTaskPlan* plan = getContext().getTaskPlanForPreview(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	getContext().setCurrentTaskPlan(plan);

	bool result = executeTransform(input, dataWidth, dataHeight, output, imageWidth, imageHeight, imageQindex,
									snrRatio, referPoint, nullptr, amplitudes);

	return result;
}


bool SigChain::OclSystem2::executeTransformForEnface(const std::uint16_t * input, int dataWidth, int dataHeight, 
												std::uint8_t * output, int * imageWidth, int * imageHeight, 
												float* imageQindex, float* snrRatio, int* referPoint, std::uint8_t* laterals)
{
	OclTaskPlan* plan = getContext().getTaskPlanForEnface(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	getContext().setCurrentTaskPlan(plan);

	bool result = executeTransform(input, dataWidth, dataHeight, output, imageWidth, imageHeight,
									imageQindex, snrRatio, referPoint, laterals);

	return result;
}


bool SigChain::OclSystem2::executeTransformForMeasure(const std::uint16_t * input, int dataWidth, int dataHeight, 
												std::uint8_t * output, int * imageWidth, int * imageHeight, 
												float* imageQindex, float* snrRatio, int* referPoint, std::uint8_t* laterals, float* amplitudes)
{
	OclTaskPlan* plan = getContext().getTaskPlanForMeasure(dataWidth, dataHeight);
	if (plan == nullptr) {
		return false;
	}

	getContext().setCurrentTaskPlan(plan);

	bool result = executeTransform(input, dataWidth, dataHeight, output, imageWidth, imageHeight, 
									imageQindex, snrRatio, referPoint, laterals, amplitudes);

	return result;
}


bool SigChain::OclSystem2::executeTransform(const std::uint16_t * input, int dataWidth, int dataHeight, 
											std::uint8_t * output, int * imageWidth, int * imageHeight,
											float* imageQindex, float* snrRatio, int* referPoint,
											std::uint8_t* laterals, float* amplitudes)
{
	OclTaskPlan* plan = getContext().getCurrentTaskPlan();
	if (plan == nullptr) {
		return false;
	}

	bool result = getContext().executeContext(plan, input, output, laterals, amplitudes);

	if (result) {
		*imageWidth = (int)plan->grayScaleImageWidth();
		*imageHeight = (int)plan->grayScaleImageHeight();
		*imageQindex = plan->indexOfImageQuality();
		*referPoint = plan->referencePoint();
		*snrRatio = plan->ratioOfImageSignal();

		GlobalRecord::setOctImageBackgroundLevel(plan->imageBackgroundLevel());
		GlobalRecord::setOctImageSignalLevel(plan->imageSignalLevel());
		GlobalRecord::setOctImageNoiseLevel(plan->imageNoiseLevel());

		checkBackgroundSpectrum(input, dataWidth, dataHeight, *imageQindex);
	}

	return result;
}


OclContext & SigChain::OclSystem2::getContext(void)
{
	return getImpl().context;
}


OclSystem2::OclSystem2Impl & SigChain::OclSystem2::getImpl(void)
{
	return *d_ptr;
}