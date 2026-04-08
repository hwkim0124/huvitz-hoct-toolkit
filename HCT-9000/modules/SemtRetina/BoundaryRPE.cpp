#include "stdafx.h"
#include "BoundaryRPE.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryRPE::BoundaryRPEImpl
{
	BoundaryRPEImpl()
	{
	}
};


BoundaryRPE::BoundaryRPE(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryRPEImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryRPE::~BoundaryRPE() = default;
SemtRetina::BoundaryRPE::BoundaryRPE(BoundaryRPE&& rhs) = default;
BoundaryRPE& SemtRetina::BoundaryRPE::operator=(BoundaryRPE&& rhs) = default;


bool SemtRetina::BoundaryRPE::detectBoundary(void)
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
	if (!smoothBoundaryRPE()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryRPE::refineBoundary(void)
{
	if (!refinePathConstraints()) {
		return false;
	}
	if (!refinePathCostMap()) {
		return false;
	}

	if (!searchPathMinCostInRange()) {
		return false;
	}
	if (!smoothRefinedRPE()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryRPE::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = outs[i];
	}

	if (band->isNerveHeadRangeValid()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		auto min_upp = *std::min_element(upps.begin(), upps.end());

		for (int x = x1; x <= x2; ++x) {
			upps[x] = min_upp;
		}
	}

	const int WINDOW_SIZE = 7;
	const int DEGREE = 1;
	auto upp2 = CppUtil::SgFilter::smoothInts(upps, WINDOW_SIZE, DEGREE);
	transform(begin(upp2), end(upp2), begin(upp2), [=](int elm) { return min(max(elm, 0), height - 1); });
	transform(begin(upp2), end(upp2), begin(upp2), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->upperYs() = upp2;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}

bool SemtRetina::BoundaryRPE::refinePathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto inns = bios->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto outs = bbrm->sampleYs();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	const int RANGE_MAX = 36;
	for (int i = 0; i < width; i++) {
		auto ylim = inns[i] + RANGE_MAX;
		auto ypos = outs[i];
		
		if (ypos < ylim) {
			auto y1 = (int)(inns[i] + ((outs[i] - inns[i]) * 0.35f));
			auto y2 = (int)(inns[i] + ((outs[i] - inns[i]) * 0.85f));
			upps[i] = y1;
			lows[i] = y2;
		}
		else {
			auto y1 = inns[i];
			auto y2 = ylim;
			upps[i] = y1;
			lows[i] = y2;
		}
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


bool SemtRetina::BoundaryRPE::prepareGradientMap(void)
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


bool SemtRetina::BoundaryRPE::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();
	auto* band = segm->retinaBandExtractor();

	auto* pipe = segm->retinaInferPipeline();
	auto* rpes = pipe->probMapRpe();
	auto* onls = pipe->probMapOnl();
	auto* nfls = pipe->probMapRnfl();
	auto* ipls = pipe->probMapIplOpl();
	auto* head = pipe->probMapDiscHead();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat matProb = Mat::zeros(height, width, CV_32F);
	float* p_prob = matProb.ptr<float>(0);
	for (int i = 0; i < N; ++i) {
		float val = rpes[i];
		val = val - max(nfls[i], ipls[i]);
		// val = max(val, 0.0f);
		p_prob[i] = val;
	}
	
	Mat& matEdge = this->pathEdgeMat();
	float* p_edge = matEdge.ptr<float>(0);
	
	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = head_x1; x <= head_x2; x++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1; y <= y2; y++) {
				auto idx = y * width + x;
				// matEdge.at<float>(y, x) = 0.0f;
				// matProb.at<float>(y, x) = 0.0f;
				if (head[idx] >= 0.5f) {
					// p_prob[idx] = 0.0f;
					p_edge[idx] = 0.0f;
				}
			}
		}
	}

	Mat matCost;
	// cv::add(matProb, 1.0f, matCost);
	// cv::multiply(matCost, matEdge, matCost);
	cv::add(matProb, matEdge, matCost);
	
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryRPE::refineGraidentMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 5;

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);

	/*
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c <= (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	*/
	for (int r = 0; r < (kernel.rows / 2); r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

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
	return true;
}

bool SemtRetina::BoundaryRPE::refinePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	matEdge.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryRPE::smoothBoundaryRPE(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto outs = band->outerYsFull();

	const int WINDOW_SIZE1 = 11; // 21;
	const int DEGREE = 1;

	auto path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true);

	const int WINDOW_SIZE2 = 21;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryRPE::smoothRefinedRPE(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto* bbrm = segm->boundaryBRM();
	auto inns = bios->sampleYs();
	auto outs = bbrm->sampleYs();

	const int WINDOW_SIZE1 = 11; // 21;
	const int DEGREE = 1;

	auto path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true);

	const int WINDOW_SIZE2 = 21;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}


bool SemtRetina::BoundaryRPE::resizeToMatchSource(void)
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


bool SemtRetina::BoundaryRPE::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryIOS();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryBRM();
	auto outs = bout->sourceYs();

	auto width = resa->sourceWidth();
	auto height = resa->sourceHeight();
	auto filt = this->sourceYs();

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sourceYs() = filt;
	return true;
}


BoundaryRPE::BoundaryRPEImpl& SemtRetina::BoundaryRPE::impl(void) const
{
	return *d_ptr;
}