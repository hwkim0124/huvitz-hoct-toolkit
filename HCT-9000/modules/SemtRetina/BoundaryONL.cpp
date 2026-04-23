#include "stdafx.h"
#include "BoundaryONL.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryONL::BoundaryONLImpl
{
	BoundaryONLImpl()
	{
	}
};


BoundaryONL::BoundaryONL(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryONLImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryONL::~BoundaryONL() = default;
SemtRetina::BoundaryONL::BoundaryONL(BoundaryONL&& rhs) = default;
BoundaryONL& SemtRetina::BoundaryONL::operator=(BoundaryONL&& rhs) = default;


bool SemtRetina::BoundaryONL::detectBoundary(void)
{
	if (!prepareGradientMap()) {
		return false;
	}
	if (!designPathConstraints()) {
		return false;
	}
	if (!preparePathCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothBoundaryONL()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryONL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();
	const auto imgMat = image->getCvMatConst();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	auto* pipe = segm->retinaInferPipeline();
	auto* indice = pipe->classIndices();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto moves = crta->getPathCostRangeDeltaONL();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, moves);

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		delt[i] = 1;
	}

	for (int x = 0; x < width; x++) {
		auto y1 = inns[x];
		auto y2 = outs[x];
		upps[x] = y1;
		lows[x] = y2;
	}
	for (int x = 0; x < width; x++) {
		if (inns[x] <= 0) {
			auto ty = 0;
			for (int y = 0; y < outs[x]; y++) {
				auto idx = y * width + x;
				if (indice[idx] < CLASS_CHOROID) {
					ty = y;
				}
			}
			lows[x] = min(ty, outs[x]);
		}
	}

	const int WINDOW_SIZE = crta->getPathSmoothWindowONL();
	const int DEGREE = 1;
	auto upp2 = CppUtil::SgFilter::smoothInts(upps, WINDOW_SIZE, DEGREE);
	auto low2 = CppUtil::SgFilter::smoothInts(lows, WINDOW_SIZE, DEGREE);
	transform(begin(upp2), end(upp2), begin(upp2), [=](int elm) { return min(max(elm, 0), height - 1); });
	transform(begin(low2), end(low2), begin(low2), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->upperYs() = upp2;
	this->lowerYs() = low2;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryONL::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	// auto* image = resa->imageSample();
	auto* image = resa->imageCoarse();

	Mat matImage = image->getCvMatConst();
	Mat matGray;
	matImage.convertTo(matGray, CV_32F);
	cv::normalize(matGray, matGray, 0.0, 1.0, cv::NORM_MINMAX);

	matGray.copyTo(this->pathEdgeMat());
	return true;
}


bool SemtRetina::BoundaryONL::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();
	auto* vits = pipe->probMapVitreous();
	auto* nfls = pipe->probMapRnfl();
	auto* ipls = pipe->probMapIplOpl();
	auto* onls = pipe->probMapOnl();
	auto* rpes = pipe->probMapRpe();
	auto* chrs = pipe->probMapChoroid();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;
	
	Mat matProb = Mat::zeros(height, width, CV_32F);
	float* p_prob = matProb.ptr<float>(0);
	memcpy(p_prob, onls, N * sizeof(float));
	/*
	for (int i = 0; i < N; ++i) {
		float val = onls[i];
		// float lim1 = vits[i] + nfls[i] + ipls[i];
		// float lim2 = rpes[i] + chrs[i];
		// p_prob[i] = max(val - lim1 * 1.0f - lim2 * 0.5f, 0.0f);
		p_prob[i] = val;
	}
	*/

	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	Mat matInvs;
	cv::subtract(1.0, matEdge, matInvs);
	cv::add(matProb, matInvs, matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryONL::smoothBoundaryONL(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageCoarse();
	auto width = image->getWidth();
	auto height = image->getHeight();
	
	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto outs = band->outerYsFull();
	auto path = this->optimalPath();

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		const int WINDOW_SIZE1 = crta->getLayerSmoothWindowONL(true);
		const int DEGREE = 1;
		auto path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true, inns);

		const int WINDOW_SIZE2 = crta->getLayerSmoothWindowONL(true);
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}
	else {
		const int WINDOW_SIZE = crta->getLayerSmoothWindowONL(false);
		const int DEGREE = 1;
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}


bool SemtRetina::BoundaryONL::resizeToMatchSource(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto src_w = resa->sampleWidth();
	auto src_h = resa->sampleHeight();
	auto dst_w = resa->sourceWidth();
	auto dst_h = resa->sourceHeight();

	auto path = this->sampleYs();
	auto outs = std::vector<int>(dst_w, -1);
	if (!resizeBoundaryPath(path, src_w, src_h, dst_w, dst_h, outs)) {
		return false;
	}

	this->sourceYs() = outs;
	return true;
}

BoundaryONL::BoundaryONLImpl& SemtRetina::BoundaryONL::impl(void) const
{
	return *d_ptr;
}