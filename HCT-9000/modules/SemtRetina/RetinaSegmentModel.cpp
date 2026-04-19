#include "stdafx.h"
#include "RetinaSegmentModel.h"
#include "retina_layer_model_api.h"

#include <atomic>
#include <functional>
#include <mutex>

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaSegmentModel::RetinaSegmentModelImpl
{
    bool initialized = false;
    std::mutex mutexLock;

    RetinaSegmentModelImpl()
    {
        initializeRetinaSegmentModelImpl();
    }

    void initializeRetinaSegmentModelImpl()
    {
        initialized = false;
    }
};


// Direct initialization of static smart pointer.
std::unique_ptr<RetinaSegmentModel::RetinaSegmentModelImpl> RetinaSegmentModel::d_ptr(new RetinaSegmentModelImpl());

RetinaSegmentModel::RetinaSegmentModel()
{
}


RetinaSegmentModel::~RetinaSegmentModel()
{
}


bool RetinaSegmentModel::initializeNetwork()
{
    if (isInitialized()) {
        return true;
    }

    impl().initialized = false;

    RlmStatus status = RlmInitializeNetwork();
    if (status != RLM_STATUS_OK) {
        LogD() << "Retina layer model initialization failed";
        return false;
    }
    
    impl().initialized = true;
    LogD() << "Retina layer model initialized";
    return true;
}


bool RetinaSegmentModel::isInitialized()
{
    return impl().initialized;
}

bool SemtRetina::RetinaSegmentModel::runInference(const unsigned char* bscan, int width, int height, float* probs)
{
    unique_lock<mutex> lock(impl().mutexLock);
    // RlmStatus status = RlmInferRetinaBscan(static_cast<const void*>(bscan), width, height, 1, probs, nullptr);
    
    RlmStatus status;
    if (width <= inputWidthS()) {
		status = RlmInferRetinaBscanS(static_cast<const void*>(bscan), width, height, 1, probs, nullptr);
    }
    else {
		status = RlmInferRetinaBscanM(static_cast<const void*>(bscan), width, height, 1, probs, nullptr);
    }

    if (status != RLM_STATUS_OK) {
        return false;
    }
    return true;
}

int SemtRetina::RetinaSegmentModel::inputWidthS(void)
{
    return RLM_INPUT_WIDTH_S;
}

int SemtRetina::RetinaSegmentModel::inputWidthM(void)
{
    return RLM_INPUT_WIDTH_M;
}

int SemtRetina::RetinaSegmentModel::inputHeightS(void)
{
    return RLM_INPUT_HEIGHT_S;
}

int SemtRetina::RetinaSegmentModel::inputHeightM(void)
{
    return RLM_INPUT_HEIGHT_M;
}

int SemtRetina::RetinaSegmentModel::outputWidthS(void)
{
    return RLM_OUTPUT_WIDTH_S;
}

int SemtRetina::RetinaSegmentModel::outputWidthM(void)
{
    return RLM_OUTPUT_WIDTH_M;
}

int SemtRetina::RetinaSegmentModel::outputHeightS(void)
{
    return RLM_OUTPUT_HEIGHT_S;
}

int SemtRetina::RetinaSegmentModel::outputHeightM(void)
{
    return RLM_OUTPUT_HEIGHT_M;
}

int SemtRetina::RetinaSegmentModel::numberOfClasses(void)
{
    return RLM_NUM_CLASSES;
}

int SemtRetina::RetinaSegmentModel::outputProbMapSizeS(void)
{
    return RLM_OUTPUT_PROBS_SIZE_S;
}

int SemtRetina::RetinaSegmentModel::outputProbMapSizeM(void)
{
    return RLM_OUTPUT_PROBS_SIZE_M;
}

int SemtRetina::RetinaSegmentModel::classIndexVitreous(void)
{
    return RLM_CLASS_VITREOUS;
}

int SemtRetina::RetinaSegmentModel::classIndexRnfl(void)
{
    return RLM_CLASS_RNFL;
}

int SemtRetina::RetinaSegmentModel::classIndexIplOpl(void)
{
    return RLM_CLASS_IPL_OPL;
}

int SemtRetina::RetinaSegmentModel::classIndexOnl(void)
{
    return RLM_CLASS_ONL;
}

int SemtRetina::RetinaSegmentModel::classIndexRpe(void)
{
    return RLM_CLASS_RPE;
}

int SemtRetina::RetinaSegmentModel::classIndexChoroid(void)
{
    return RLM_CLASS_CHOROID;
}

int SemtRetina::RetinaSegmentModel::classIndexSclera(void)
{
    return RLM_CLASS_SCLERA;
}

int SemtRetina::RetinaSegmentModel::classIndexDiscHead(void)
{
    return RLM_CLASS_DISC_HEAD;
}

RetinaSegmentModel::RetinaSegmentModelImpl& RetinaSegmentModel::impl()
{
    return *d_ptr;
}