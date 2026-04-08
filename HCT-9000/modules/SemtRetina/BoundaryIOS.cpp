#include "stdafx.h"
#include "BoundaryIOS.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryIOS::BoundaryIOSImpl
{
	BoundaryIOSImpl()
	{
	}
};


BoundaryIOS::BoundaryIOS(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryIOSImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryIOS::~BoundaryIOS() = default;
SemtRetina::BoundaryIOS::BoundaryIOS(BoundaryIOS&& rhs) = default;
BoundaryIOS& SemtRetina::BoundaryIOS::operator=(BoundaryIOS&& rhs) = default;


bool SemtRetina::BoundaryIOS::detectBoundary(void)
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
	if (!smoothBoundaryIOS()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryIOS::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto* bonl = segm->boundaryONL();
	auto inns = bonl->sampleYs();

	auto* brpe = segm->boundaryRPE();
	auto outs = brpe->sampleYs();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = outs[i];
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryIOS::prepareGradientMap(void)
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
	for (int r = 0; r < (kernel.rows / 2); r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

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

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* prob = pipe->probMapRpe();

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

bool SemtRetina::BoundaryIOS::preparePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();
	const Mat& matProb = this->pathProbMat();

	Mat matCost;
	cv::add(matProb, matEdge, matCost); 
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryIOS::smoothBoundaryIOS(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bonl = segm->boundaryONL();
	auto inns = bonl->sampleYs();
	auto* brpe = segm->boundaryRPE();
	auto outs = brpe->sampleYs();

	const int WINDOW_SIZE1 = 21;
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

bool SemtRetina::BoundaryIOS::resizeToMatchSource(void)
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

bool SemtRetina::BoundaryIOS::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryOPL();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryRPE();
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

BoundaryIOS::BoundaryIOSImpl& SemtRetina::BoundaryIOS::impl(void) const
{
	return *d_ptr;
}