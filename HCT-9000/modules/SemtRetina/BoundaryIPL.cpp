#include "stdafx.h"
#include "BoundaryIPL.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryIPL::BoundaryIPLImpl
{
	BoundaryIPLImpl()
	{
	}
};


BoundaryIPL::BoundaryIPL(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryIPLImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryIPL::~BoundaryIPL() = default;
SemtRetina::BoundaryIPL::BoundaryIPL(BoundaryIPL&& rhs) = default;
BoundaryIPL& SemtRetina::BoundaryIPL::operator=(BoundaryIPL&& rhs) = default;


bool SemtRetina::BoundaryIPL::detectBoundary(void)
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
	if (!smoothBoundaryIPL()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryIPL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sampleYs();

	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	for (int i = 0; i < width; i++) {
		auto y1 = inns[i];
		auto y2 = outs[i];

		auto size = y2 - y1 + 1;
		auto mag1 = (int)(size * 0.35f);
		auto mag2 = (int)(size * 0.35f);
		y1 = min(y1 + mag1, y2);
		y2 = max(y2 - mag2, y1);
		upps[i] = y1;
		lows[i] = y2;
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


bool SemtRetina::BoundaryIPL::prepareGradientMap(void)
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

	return true;
}


bool SemtRetina::BoundaryIPL::preparePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	matEdge.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryIPL::smoothBoundaryIPL(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	const int WINDOW_SIZE = 31;
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryIPL::resizeToMatchSource(void)
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


bool SemtRetina::BoundaryIPL::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryNFL();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryOPL();
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


BoundaryIPL::BoundaryIPLImpl& SemtRetina::BoundaryIPL::impl(void) const
{
	return *d_ptr;
}