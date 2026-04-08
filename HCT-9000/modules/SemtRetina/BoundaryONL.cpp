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
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();
	const auto imgMat = image->getCvMatConst();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* brpe = segm->boundaryRPE();
	auto outs = brpe->sampleYs();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int x = 0; x < width; x++) {
		auto y1 = inns[x];
		auto y2 = outs[x];

		auto gmax = 0;
		auto ylim = y1;
		for (int y = y1; y <= y2; y++) {
			auto val = imgMat.at<uchar>(y, x);
			if (val >= gmax) {
				gmax = val;
				ylim = y;
			}
			else {
				break;
			}
		}

		auto offs = ylim - y1;
		auto size = y2 - y1 + 1;
		auto smax = (int)(size * 0.5f);
		auto smin = (int)(size * 0.0f); //(size * 0.2f);
		offs = min(max(offs, smin), smax);
		upps[x] = y1 + offs;
		lows[x] = y2;
	}

	const int WINDOW_SIZE = 7;
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
	auto* image = resa->imageSample();

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

	auto* pipe = segm->retinaInferPipeline();
	auto* onls = pipe->probMapOnl();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;
	
	Mat matProb = Mat::zeros(height, width, CV_32F);
	float* dst = matProb.ptr<float>(0);

	for (int i = 0; i < N; ++i) {
		float val = onls[i];
		dst[i] = val;
	}

	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	Mat matNegs;
	cv::subtract(1.0, matEdge, matNegs);
	cv::add(matProb, 1.0f, matCost);
	cv::multiply(matCost, matNegs, matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryONL::smoothBoundaryONL(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageCoarse();
	auto width = image->getWidth();
	auto height = image->getHeight();
	
	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* brpe = segm->boundaryRPE();
	auto outs = brpe->sampleYs();

	const int WINDOW_SIZE1 = 21;
	const int DEGREE = 1;

	auto path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true);

	const int WINDOW_SIZE2 = 31;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
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