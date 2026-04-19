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

	auto* edge1 = resa->sourceRiseEdge();
	auto* edge2 = resa->sourceFallEdge();
	Mat rise = edge1->getCvMatConst();
	Mat fall = edge2->getCvMatConst();
	Mat matCost;
	rise.copyTo(matCost);

	const int DIST_MIN = crta->getLayerDistanceMinRPE();
	const int DIST_MAX = crta->getLayerDistanceMaxRPE();

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		for (int x = 0; x < width; x++) {
			auto y1 = ioss[x];
			auto y2 = brms[x];
			auto dist = y2 - y1;
			if (x >= disc_x1 && x <= disc_x2) {
				upps[x] = min(y2, y1 + DIST_MIN);
				lows[x] = min(y2, y1 + DIST_MAX/2);
			}
			else {
				if (dist > DIST_MAX) {
					auto t1 = max(y1 - DIST_MAX, 0);
					auto t2 = min(y2 + DIST_MAX, height - 1);
					for (int y = t1; y <= t2; y++) {
						auto idx = y * width + x;
						matCost.at<float>(idx) = fall.at<float>(idx);
					}
					y2 = min(y1 + DIST_MAX, height - 1);
					y1 = min(y1 + DIST_MIN, y2);
				}
				else {
					y1 = rpes[x];
				}
				upps[x] = y1;
				lows[x] = y2;
			}
			/*
			if (resa->sampleIndex() == 242) {
				LogD() << "x: " << x << ", ios: " << ioss[x] << ", brm: " << brms[x] << ", rpe: " << rpes[x] << ", dist: " << dist << ", y: " << y1 << ", y2: " << y2 << ", disc: " << (x >= disc_x1 && x <= disc_x2);
			}
			*/
		}
	}
	else {
		for (int x = 0; x < width; x++) {
			auto y1 = ioss[x];
			auto y2 = brms[x];
			auto dist = y2 - y1;
			if (dist > DIST_MAX) {
				auto t1 = max(y1 - DIST_MAX, 0);
				auto t2 = min(y2 + DIST_MAX, height - 1);
				for (int y = t1; y <= t2; y++) {
					auto idx = y * width + x;
					matCost.at<float>(idx) = fall.at<float>(idx);
				}
				y2 = min(y1 + DIST_MAX, height - 1);
				y1 = min(y1 + DIST_MIN, y2);
			}
			else {
				y1 = rpes[x];
			}
			upps[x] = y1;
			lows[x] = y2;
		}
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;

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

	for (int i = 0; i < width; i++) {
		auto y1 = ioss[i];
		auto y2 = brms[i];
		auto size = y2 - y1;
		upps[i] = min(y1 + (int)(size * 0.45f), y2);
		lows[i] = min(y1 + (int)(size * 0.95f), y2);
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
	// auto* image = resa->imageSample();
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
	// cv::add(matProb, 1.0f, matCost);
	// cv::multiply(matCost, matEdge, matCost);
	// cv::add(matProb, matEdge, matCost);
	matEdge.copyTo(matCost);
	
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryRPE::smoothBoundaryRPE(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto inns = bios->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto outs = bbrm->sampleYs();

	auto path = this->optimalPath();
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowRPE(true);
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

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

	const int WINDOW_SIZE = crta->getLayerSmoothWindowRPE(true);
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