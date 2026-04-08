#include "stdafx.h"
#include "RetinaInferPipeline.h"
#include "RetinaSegmenter.h"
#include "RetinaSegmentModel.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

#include <algorithm>

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaInferPipeline::RetinaInferPipelineImpl
{
	const RetinaSegmenter* segm = nullptr;

	vector<float> probMaps;

	RetinaInferPipelineImpl()
	{
	}
};


RetinaInferPipeline::RetinaInferPipeline(RetinaSegmenter* segm) :
	d_ptr(make_unique<RetinaInferPipelineImpl>())
{
	impl().segm = segm;
}


SemtRetina::RetinaInferPipeline::~RetinaInferPipeline() = default;
SemtRetina::RetinaInferPipeline::RetinaInferPipeline(RetinaInferPipeline&& rhs) = default;
RetinaInferPipeline& SemtRetina::RetinaInferPipeline::operator=(RetinaInferPipeline&& rhs) = default;


bool SemtRetina::RetinaInferPipeline::inferLayerProbMaps(void)
{
	if (!RetinaSegmentModel::isInitialized()) {
		return false;
	}

	auto* segm = impl().segm;
	auto* resa = segm->bscanResampler();
	// auto* image = resa->imageCoarse;
	auto* image = resa->imageSample();

	auto* p_img = image->getCvMatConst().data;
	int width = image->getWidth();
	int height = image->getHeight();

	int mapSize = RetinaSegmentModel::outputProbMapSize();
	auto probs = vector<float>(mapSize);
	float* p_map = probs.data();
	
	if (!RetinaSegmentModel::runInference(p_img, width, height, p_map)) {
		return false;
	}

	impl().probMaps = probs;
	return true;
}

int SemtRetina::RetinaInferPipeline::probMapWidth(void) const
{
	auto width = RetinaSegmentModel::outputWidth();
	return width;
}

int SemtRetina::RetinaInferPipeline::probMapHeight(void) const
{
	auto height = RetinaSegmentModel::outputHeight();
	return height;
}


const float* SemtRetina::RetinaInferPipeline::probMapVitreous(void) const
{
	auto index = RetinaSegmentModel::classIndexVitreous();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapRnfl(void) const
{
	auto index = RetinaSegmentModel::classIndexRnfl();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapIplOpl(void) const
{
	auto index = RetinaSegmentModel::classIndexIplOpl();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapOnl(void) const
{
	auto index = RetinaSegmentModel::classIndexOnl();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapRpe(void) const
{
	auto index = RetinaSegmentModel::classIndexRpe();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapChoroid(void) const
{
	auto index = RetinaSegmentModel::classIndexChoroid();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapSclera(void) const
{
	auto index = RetinaSegmentModel::classIndexSclera();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapDiscHead(void) const
{
	auto index = RetinaSegmentModel::classIndexDiscHead();
	auto width = RetinaSegmentModel::outputWidth();
	auto height = RetinaSegmentModel::outputHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

RetinaInferPipeline::RetinaInferPipelineImpl& SemtRetina::RetinaInferPipeline::impl(void) const
{
	return *d_ptr;
}
