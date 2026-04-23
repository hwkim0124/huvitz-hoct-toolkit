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
	vector<int> indices;
	int map_width = 0;
	int map_height = 0;

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


bool SemtRetina::RetinaInferPipeline::inferProbabilityMaps(void)
{
	if (!RetinaSegmentModel::isInitialized()) {
		return false;
	}

	auto* segm = impl().segm;
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	int width = image->getWidth();
	int height = image->getHeight();

	int input_w = (resa->isAngio() ? RetinaSegmentModel::inputWidthS() : RetinaSegmentModel::inputWidthM());
	int input_h = (resa->isAngio() ? RetinaSegmentModel::inputHeightS() : RetinaSegmentModel::inputHeightM());
	int n_class = RetinaSegmentModel::numberOfClasses();

	CvImage input;
	cv::resize(image->getCvMatConst(), input.getCvMat(), Size(input_w, input_h), 0.0, 0.0, INTER_AREA);

	int map_size = (resa->isAngio() ? RetinaSegmentModel::outputProbMapSizeS() : RetinaSegmentModel::outputProbMapSizeM());
	auto probs = vector<float>(map_size);

	auto* p_data = input.getCvMatConst().data;
	float* p_prob = probs.data();
	
	if (!RetinaSegmentModel::runInference(p_data, input_w, input_h, p_prob)) {
		return false;
	}

	cv::Mat src(input_h, input_w, CV_MAKETYPE(CV_32F, n_class), const_cast<float*>(p_prob));
	cv::Mat dst;

	cv::resize(src, dst, cv::Size(width, height), 0.0, 0.0, INTER_LINEAR);
	std::vector<float> output;
	if (dst.isContinuous()) {
		output.assign((float*)dst.datastart, (float*)dst.dataend);
	}
	else {
		for (int i = 0; i < dst.rows; ++i) {
			const float* row_ptr = dst.ptr<float>(i);
			output.insert(output.end(), row_ptr, row_ptr + dst.cols * n_class);
		}
	}

	impl().map_width = width;
	impl().map_height = height;
	impl().probMaps = output;

	std::vector<int> indice = vector<int>(width * height, 0);
	auto* vits = probMapVitreous();
	auto* rnfl = probMapRnfl();
	auto* iopl = probMapIplOpl();
	auto* onls = probMapOnl();
	auto* rpes = probMapRpe();
	auto* chrs = probMapChoroid();
	auto* scls = probMapSclera();
	auto* head = probMapDiscHead();

	const int NUM_CLASSES = RetinaSegmentModel::numberOfClasses();
	for (int i = 0; i < (width * height); i++) {
		float pvals[8] = {vits[i], rnfl[i], iopl[i], onls[i], rpes[i], chrs[i], scls[i], head[i]};
		auto max_idx = 0;
		auto max_val = pvals[0];
		for (int j = 1; j < NUM_CLASSES; j++) {
			if (pvals[j] > max_val) {
				max_val = pvals[j];
				max_idx = j;
			}
		}
		indice[i] = max_idx;
	}
    impl().indices = indice;
	return true;
}

int SemtRetina::RetinaInferPipeline::probMapWidth(void) const
{
	auto width = impl().map_width;
	return width;
}

int SemtRetina::RetinaInferPipeline::probMapHeight(void) const
{
	auto height = impl().map_height;
	return height;
}

const float* SemtRetina::RetinaInferPipeline::probMapVitreous(void) const
{
	auto index = RetinaSegmentModel::classIndexVitreous();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapRnfl(void) const
{
	auto index = RetinaSegmentModel::classIndexRnfl();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapIplOpl(void) const
{
	auto index = RetinaSegmentModel::classIndexIplOpl();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapOnl(void) const
{
	auto index = RetinaSegmentModel::classIndexOnl();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapRpe(void) const
{
	auto index = RetinaSegmentModel::classIndexRpe();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapChoroid(void) const
{
	auto index = RetinaSegmentModel::classIndexChoroid();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapSclera(void) const
{
	auto index = RetinaSegmentModel::classIndexSclera();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const float* SemtRetina::RetinaInferPipeline::probMapDiscHead(void) const
{
	auto index = RetinaSegmentModel::classIndexDiscHead();
	auto width = probMapWidth();
	auto height = probMapHeight();
	auto* p = impl().probMaps.data() + index * width * height;
	return p;
}

const int* SemtRetina::RetinaInferPipeline::classIndices(void) const
{
	auto* p = impl().indices.data();
	return p;
}

RetinaInferPipeline::RetinaInferPipelineImpl& SemtRetina::RetinaInferPipeline::impl(void) const
{
	return *d_ptr;
}
