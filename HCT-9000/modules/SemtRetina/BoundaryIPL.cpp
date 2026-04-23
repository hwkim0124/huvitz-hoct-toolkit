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

bool SemtRetina::BoundaryIPL::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sourceYs();
	auto* bopl = segm->boundaryOPL();
	auto opls = bopl->sourceYs();
	auto ipls = sourceYs();

	auto moves = crta->getPathCostRangeDeltaIPL();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, moves);

	for (int i = 0; i < width; i++) {
		auto size = (opls[i] - nfls[i]) + 1;
		auto offs1 = (int)(size * 0.35f);
		auto offs2 = (int)(size * 0.75f);
		auto y1 = (int)(nfls[i] + offs1);
		auto y2 = (int)(nfls[i] + offs2);
		upps[i] = y1;
		lows[i] = y2;
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
	if (!smoothRefinedIPL()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryIPL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto opls = bopl->sampleYs();

	auto moves = crta->getPathCostRangeDeltaIPL();
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

	for (int i = 0; i < width; i++) {
		auto y1 = nfls[i];
		auto y2 = opls[i];

		auto size = y2 - y1 + 1;
		auto mag1 = (int)(size * 0.25f);
		auto mag2 = (int)(size * 0.25f);
		y1 = min(y1 + mag1, y2);
		y2 = max(y2 - mag2, y1);
		upps[i] = y1;
		lows[i] = y2;
	}

	const int WINDOW_SIZE = crta->getPathSmoothWindowIPL();
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
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	const int KERNEL_ROWS = crta->getGradientKernelRowsIPL();
	const int KERNEL_COLS = crta->getGradientKernelColsIPL();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(+1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(-1.0f);

	{
		auto mean = image->imageMean();
		auto stdv = image->imageStdev();
		int gmin = (int)(mean + stdv * 1.0f);
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		matDst.setTo(gmin, (matDst < gmin));
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
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	const int WINDOW_SIZE = crta->getLayerSmoothWindowIPL();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}


bool SemtRetina::BoundaryIPL::smoothRefinedIPL(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sourceYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sourceYs();

	const int WINDOW_SIZE = crta->getLayerSmoothWindowIPL();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
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