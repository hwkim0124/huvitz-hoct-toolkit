#include "stdafx.h"
#include "BoundaryBRM.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryBRM::BoundaryBRMImpl
{
	BoundaryBRMImpl()
	{
	}
};


BoundaryBRM::BoundaryBRM(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryBRMImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryBRM::~BoundaryBRM() = default;
SemtRetina::BoundaryBRM::BoundaryBRM(BoundaryBRM&& rhs) = default;
BoundaryBRM& SemtRetina::BoundaryBRM::operator=(BoundaryBRM&& rhs) = default;


bool SemtRetina::BoundaryBRM::detectBoundary(void)
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
	if (!searchPathMinCostInRange()) {
		return false;
	}
	if (!smoothBoundaryBRM()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryBRM::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaBRM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* brpe = segm->boundaryRPE();
	auto rpes = brpe->sourceYs();
	auto brms = sourceYs();

	const int OFFS_MAX = crta->getLayerLowerSpaceMaxBRM();
	for (int i = 0; i < width; i++) {
		auto y1 = brms[i];
		auto y2 = brms[i] + OFFS_MAX;
		upps[i] = y1;
		lows[i] = min(y2, height - 1);
	}

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		delt[i] = 1;
	}
	
	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;

	// auto* edge = resa->sourceFallEdgeHigh();
	auto* edge = resa->sourceFallEdge();
	Mat fall = edge->getCvMatConst();

	Mat matCost;
	fall.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());

	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedBRM()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryBRM::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto opls = bopl->sampleYs();
	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	auto range = crta->getPathCostRangeDeltaBRM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		delt[i] = 1;
	}

	for (int i = 0; i < width; i++) {
		upps[i] = max(onls[i], nfls[i]);
		lows[i] = outs[i];
	}

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		const int MARGIN_MIN = crta->getPathDiscUpperSpaceMinBRM();
		const int MARGIN_MAX = crta->getPathDiscUpperSpaceMaxBRM();
		for (int x = disc_x1; x <= disc_x2; ++x) {
			upps[x] = min(max(upps[x], nfls[x] + MARGIN_MIN), outs[x]);
			lows[x] = min(max(onls[x] + MARGIN_MAX, nfls[x] + MARGIN_MAX), outs[x]);
		}
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryBRM::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsBRM();
	const int KERNEL_COLS = crta->getGradientKernelColsBRM();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(+1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(-1.0f);

	{
		auto mean = image->imageMean();
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* p_rpes = pipe->probMapRpe();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* p_prob = matProb.ptr<float>(0);
		memcpy(p_prob, p_rpes, N * sizeof(float));
		/*
		for (int i = 0; i < N; ++i) {
			float val = prob[i];
			p_prob[i] = val;
		}
		*/

		auto mean = cv::mean(matProb);
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matProb, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathProbMat());
	}
	return true;
}


bool SemtRetina::BoundaryBRM::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();
	auto* head = pipe->probMapDiscHead();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	float* p_edge = matEdge.ptr<float>(0);
	float* p_prob = matProb.ptr<float>(0);

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto disc_cx = (disc_x1 + disc_x2) / 2;
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = disc_x1, k = 0; x <= disc_cx; x++, k++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1, d = 0; y <= y2; y++, d++) {
				auto idx = y * width + x;
				auto w = min(k, d) * 0.01f;
				p_prob[idx] += w;
			}
		}

		for (int x = disc_x2, k = 0; x >= disc_cx; x--, k++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1, d = 0; y <= y2; y++, d++) {
				auto idx = y * width + x;
				auto w = min(k, d) * 0.01f;
				p_prob[idx] += w;
			}
		}
	}

	Mat matCost;
	matProb.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryBRM::smoothBoundaryBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sampleYs();
	auto outs = band->outerYsFull();

	auto filt = path;
	if (band->isNerveHeadRangeValid() && !band->isNerveHeadDiscCupShaped()) {
		const int WINDOW_SIZE1 = crta->getLayerSmoothWindowBRM(true);
		const int DEGREE = 1;
		path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true);

		const int WINDOW_SIZE2 = crta->getLayerSmoothWindowBRM(true);
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}
	else {
		const int WINDOW_SIZE = crta->getLayerSmoothWindowBRM(true);
		const int DEGREE = 1;
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryBRM::smoothRefinedBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* brpe = segm->boundaryRPE();
	auto inns = brpe->sourceYs();

	auto path = this->optimalPath();
	// transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowBRM(false);
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
	return true;
}

bool SemtRetina::BoundaryBRM::resizeToMatchSource(void)
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

bool SemtRetina::BoundaryBRM::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryRPE();
	auto inns = binn->sourceYs();

	auto width = resa->sourceWidth();
	auto height = resa->sourceHeight();
	auto filt = this->sourceYs();

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sourceYs() = filt;
	return true;
}

BoundaryBRM::BoundaryBRMImpl& SemtRetina::BoundaryBRM::impl(void) const
{
	return *d_ptr;
}