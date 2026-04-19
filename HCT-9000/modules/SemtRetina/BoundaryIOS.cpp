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

bool SemtRetina::BoundaryIOS::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaIOS();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* brpe = segm->boundaryRPE();
	auto rpes = brpe->sourceYs();
	auto ioss = sourceYs();

	for (int i = 0; i < width; i++) {
		auto dist = (rpes[i] - ioss[i]);
		upps[i] = ioss[i];
		lows[i] = min((int)(ioss[i] + dist * 0.5f), height - 1);
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;

	auto* edge = resa->sourceRiseEdge();
	Mat rise = edge->getCvMatConst();

	Mat matCost;
	rise.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());

	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedIOS()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryIOS::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();
	auto* bopl = segm->boundaryOPL();
	auto opls = bopl->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto brms = bbrm->sampleYs();

	auto range = crta->getPathCostRangeDeltaIOS();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	const int DOWARD_OFFSET = crta->getPathDownwardOffsetBRM();
	for (int i = 0; i < width; i++) {
		upps[i] = max(onls[i], opls[i]);
		lows[i] = max(brms[i] - DOWARD_OFFSET, upps[i]);
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryIOS::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsIOS();
	const int KERNEL_COLS = crta->getGradientKernelColsIOS();

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
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matProb, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
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
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();
	auto* head = pipe->probMapDiscHead();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	float* p_edge = matEdge.ptr<float>(0);
	float* p_prob = matProb.ptr<float>(0);

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = disc_x1; x <= disc_x2; x++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1, d = 0; y <= y2; y++, d++) {
				auto idx = y * width + x;
				p_prob[idx] = (d * 0.01f);
			}
		}
	}

	Mat matCost;
	// cv::add(matProb, matEdge, matCost); 
	cv::multiply(matProb, matEdge, matCost); 
	// matProb.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryIOS::smoothBoundaryIOS(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bopl = segm->boundaryOPL();
	auto inns = bopl->sampleYs();
	auto* bbrm = segm->boundaryBRM();
	auto outs = bbrm->sampleYs();

	auto path = this->optimalPath();
	transform(begin(path), end(path), begin(path), [=](int elm) { return elm + 1; });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowIOS(true);
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryIOS::smoothRefinedIOS(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bopl = segm->boundaryOPL();
	auto inns = bopl->sourceYs();
	auto* brpe = segm->boundaryRPE();
	auto outs = brpe->sourceYs();

	auto path = this->optimalPath();
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowIOS(true);
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
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