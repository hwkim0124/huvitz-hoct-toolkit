#include "stdafx.h"
#include "BoundaryNFL.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryNFL::BoundaryNFLImpl
{
	BoundaryNFLImpl()
	{
	}
};


BoundaryNFL::BoundaryNFL(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryNFLImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryNFL::~BoundaryNFL() = default;
SemtRetina::BoundaryNFL::BoundaryNFL(BoundaryNFL&& rhs) = default;
BoundaryNFL& SemtRetina::BoundaryNFL::operator=(BoundaryNFL&& rhs) = default;


bool SemtRetina::BoundaryNFL::detectBoundary(void)
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
	if (!smoothBoundaryNFL()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryNFL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();
	const auto imgMat = image->getCvMatConst();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();

	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i] ;
		lows[i] = outs[i] ;
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryNFL::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 9;

	Mat matImg = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);

	/*
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	*/
	for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	/*
	{
		auto mean = image->imageMean();
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matImg;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}
	*/

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* prob = pipe->probMapRnfl();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* dst = matProb.ptr<float>(0);

		for (int i = 0; i < N; ++i) {
			float val = prob[i];
			dst[i] = val;
		}

		auto mean = cv::mean(matProb);
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matProb;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathProbMat());
	}
	return true;
}


bool SemtRetina::BoundaryNFL::preparePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();
	const Mat& matProb = this->pathProbMat();

	Mat matCost;
	// cv::add(matProb, matEdge, matCost);
	// cv::add(matProb, 1.0f, matCost); // matCost = matProb + 1.0f
	// cv::multiply(matCost, matEdge, matCost); // matCost *= matEdge
	matProb.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryNFL::smoothBoundaryNFL(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	const int WINDOW_SIZE = 21;
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}


bool SemtRetina::BoundaryNFL::resizeToMatchSource(void)
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


bool SemtRetina::BoundaryNFL::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryILM();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryIPL();
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


BoundaryNFL::BoundaryNFLImpl& SemtRetina::BoundaryNFL::impl(void) const
{
	return *d_ptr;
}