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
		auto size1 = (ipls[i] - nfls[i]) + 1;
		auto size2 = (opls[i] - ipls[i]) + 1;
		// auto offs1 = (int)(size1 * 0.50f);
		// auto offs2 = (int)(size2 * 0.35f);
		auto offs1 = (int)(size1 * 0.15f);
		auto offs2 = (int)(size2 * 0.15f);

		auto y1 = max(nfls[i], ipls[i] - offs1);
		auto y2 = min(opls[i], ipls[i] + offs2);
		upps[i] = y1;
		lows[i] = y2;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		const int moves = crta->getPathDiscRangeDeltaOPL();

		if (band->isNerveHeadDiscCupShaped()) {
			for (int x = disc_x1; x <= disc_x2; ++x) {
				upps[x] = min(ipls[x], nfls[x] + (int)((ipls[x] - nfls[x]) * 0.50f));
				lows[x] = min(opls[x], ipls[x] + (int)((opls[x] - ipls[x]) * 0.50f));
				delt[x] = moves;
			}
			delt[disc_x1] = moves * 10;
			delt[disc_x2] = moves * 10;
		}
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

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		const int moves = crta->getPathDiscRangeDeltaIPL();

		if (band->isNerveHeadDiscCupShaped()) {
			for (int x = disc_x1; x <= disc_x2; ++x) {
				delt[x] = moves;
			}
		}
	}
	this->upperYs() = upps;
	this->lowerYs() = lows;
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
		// matDst.setTo(gmin, (matDst < gmin));
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

	auto* band = segm->retinaBandExtractor();
	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sampleYs();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowIPL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowIPL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE);
			// path = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE1, DEGREE);
		}
		else {
			// path = smoothOptimalPathWithMultiSize(WINDOW_SIZE1, DEGREE, WINDOW_SIZE2, DEGREE, true);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}

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

	auto* band = segm->retinaBandExtractor();
	auto* bnfl = segm->boundaryNFL();
	auto inns = bnfl->sourceYs();
	auto* bopl = segm->boundaryOPL();
	auto outs = bopl->sourceYs();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowIPL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowIPL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			// path = smoothOptimalPathWithMultiSize(WINDOW_SIZE1, DEGREE, WINDOW_SIZE2, DEGREE, true);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
		else {
			// path = smoothOptimalPathWithMultiSize(WINDOW_SIZE1, DEGREE, WINDOW_SIZE2, DEGREE, true);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}

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