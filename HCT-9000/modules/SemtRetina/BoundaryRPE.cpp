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
	if (!searchPathMinCost()) {
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

bool SemtRetina::BoundaryRPE::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaRPE();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* bios = segm->boundaryIOS();
	auto* bbrm = segm->boundaryBRM();
	auto ioss = bios->sourceYs();
	auto brms = bbrm->sourceYs();
	auto rpes = sourceYs();

	const int OFFS_MIN = crta->getLayerLowerOffsetMinRPE();
	const int OFFS_MAX = crta->getLayerLowerOffsetMaxRPE();

	for (int i = 0; i < width; i++) {
		auto dist = (brms[i] - ioss[i]) + 1;
		auto offs = min((int)(dist * 0.5f), OFFS_MAX);
		auto y1 = max(brms[i] - offs, ioss[i]);
		auto y2 = max(brms[i] - OFFS_MIN, y1);
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

	auto* edge = resa->sourceRiseEdge();
	Mat fall = edge->getCvMatConst();

	Mat matCost;
	fall.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	
	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedRPE()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryRPE::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto brms = bbrm->sampleYs();

	auto range = crta->getPathCostRangeDeltaRPE();
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
		auto y1 = ioss[i];
		auto y2 = brms[i];
		auto size = y2 - y1;
		auto offs1 = (int)(size * 0.50f);
		auto offs2 = (int)(size * 0.95f);
		upps[i] = min(y1 + offs1, y2);
		lows[i] = min(y1 + offs2, y2);
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryRPE::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsRPE();
	const int KERNEL_COLS = crta->getGradientKernelColsRPE();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(-1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(1.0f);

	{
		auto mean = image->imageMean();
		auto stdv = image->imageStdev();
		int gmax = (int)(mean + stdv * 2.0f);
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		// matDst.setTo(gmax, (matDst > gmax));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}
	return true;
}


bool SemtRetina::BoundaryRPE::preparePathCostMap(void)
{
	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	Mat matCost;
	matEdge.copyTo(matCost);
	
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryRPE::smoothBoundaryRPE(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto inns = bios->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto outs = bbrm->sampleYs();
	auto path = this->optimalPath();

	auto filt = path;
	if (band->isNerveHeadRangeValid() && !band->isNerveHeadDiscCupShaped()) {
		const int WINDOW_SIZE1 = crta->getLayerSmoothWindowRPE(true);
		const int DEGREE = 1;
		path = smoothOptimalPath(WINDOW_SIZE1, DEGREE, true);

		const int WINDOW_SIZE2 = crta->getLayerSmoothWindowRPE(true);
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}
	else {
		const int WINDOW_SIZE = crta->getLayerSmoothWindowRPE(true);
		const int DEGREE = 1;
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryRPE::smoothRefinedRPE(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto* bbrm = segm->boundaryBRM();
	auto inns = bios->sourceYs();
	auto outs = bbrm->sourceYs();

	auto path = this->optimalPath();
	transform(begin(path), end(path), begin(path), [=](int elm) { return elm + 1; });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowRPE(false);
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sourceYs() = filt;
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