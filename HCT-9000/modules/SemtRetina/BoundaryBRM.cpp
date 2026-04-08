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

bool SemtRetina::BoundaryBRM::refineBoundary(void)
{
	if (!refinePathConstraints()) {
		return false;
	}
	if (!refinePathCostMap()) {
		return false;
	}

	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedBRM()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryBRM::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* brpe = segm->boundaryRPE();
	auto inns = brpe->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

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

bool SemtRetina::BoundaryBRM::refinePathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();
	
	auto inns = sampleYs();
	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = inns[i];
	}

	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		for (int i = head_x1; i <= head_x2; i++) {
			lows[i] = outs[i];
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
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 5;

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

	{
		Mat matGray;
		matImg.convertTo(matGray, CV_32F);
		cv::normalize(matGray, matGray, 0.0, 1.0, cv::NORM_MINMAX);

		matGray.copyTo(this->pathEdgeMat());
		/*
		auto mean = image->imageMean();
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matImg;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
		*/
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


bool SemtRetina::BoundaryBRM::preparePathCostMap(void)
{
	const Mat& matProb = this->pathProbMat();

	Mat matCost;
	matProb.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryBRM::refinePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	matEdge.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryBRM::smoothBoundaryBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* brpe = segm->boundaryRPE();
	auto inns = brpe->sampleYs();
	auto outs = band->outerYsFull();

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

bool SemtRetina::BoundaryBRM::smoothRefinedBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto brm_path = sampleYs();
	auto inns = brm_path;
	auto outs = band->outerYsFull();

	const int WINDOW_SIZE1 = 21;
	const int DEGREE = 1;

	auto opt_path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, false);
	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		for (auto x = head_x1; x <= head_x2; x++) {
			brm_path[x] = opt_path[x];
		}
	}

	auto filt = brm_path;
	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
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