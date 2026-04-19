#include "stdafx.h"
#include "BoundaryOPL.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryOPL::BoundaryOPLImpl
{
	BoundaryOPLImpl()
	{
	}
};


BoundaryOPL::BoundaryOPL(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryOPLImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryOPL::~BoundaryOPL() = default;
SemtRetina::BoundaryOPL::BoundaryOPL(BoundaryOPL&& rhs) = default;
BoundaryOPL& SemtRetina::BoundaryOPL::operator=(BoundaryOPL&& rhs) = default;


bool SemtRetina::BoundaryOPL::detectBoundary(void)
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
	if (!smoothBoundaryOPL()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryOPL::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaOPL();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* bipl = segm->boundaryNFL();
	auto ipls = bipl->sourceYs();
	auto* bios = segm->boundaryONL();
	auto ioss = bios->sourceYs();
	auto opls = sourceYs();

	const int OFFS_MIN = crta->getLayerOffsetMinOPL();
	const int OFFS_MAX = crta->getLayerOffsetMaxOPL();

	for (int i = 0; i < width; i++) {
		auto y1 = opls[i];
		auto y2 = ioss[i];

		auto size = y2 - y1 + 1;
		auto dlim = (int)(size * 0.25f);
		y1 = min(max(y1, ipls[i] + OFFS_MIN), y2);
		y2 = min(y2 - dlim, y1);
		upps[i] = min(y1, y2);
		lows[i] = min(y1, y2);
	}

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		for (int x = x1; x <= x2; ++x) {
			auto y1 = max(ipls[x] + OFFS_MIN, upps[x]);
			auto y2 = max(ipls[x] + OFFS_MAX, lows[x]);
		}
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;

	auto* edge = resa->sourceFallEdge();
	Mat fall = edge->getCvMatConst();
	Mat matCost;
	fall.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());

	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedOPL()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryOPL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	const auto imgMat = image->getCvMatConst();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	auto range = crta->getPathCostRangeDeltaOPL();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	for (int i = 0; i < width; i++) {
		auto y1 = nfls[i];
		auto y2 = onls[i];
		auto lim1 = y1 + (int)((y2 - y1) * 0.5f);  
		upps[i] = min(lim1, y2);
		lows[i] = y2;
	}

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		const int UPPER_MARGIN = crta->getPathDiscUpwardMarginOPL();
		range = crta->getPathDiscRangeDeltaOPL();
		for (int x = disc_x1; x <= disc_x2; ++x) {
			auto ulim = onls[x] - UPPER_MARGIN;
			auto y1 = max(ulim, nfls[x]);

			upps[x] = y1;
			lows[x] = outs[x];
			delt[x] = range;
		}
	}

	const int WINDOW_SIZE = crta->getPathSmoothWindowOPL();
	const int DEGREE = 1;
	auto upp2 = CppUtil::SgFilter::smoothInts(upps, WINDOW_SIZE, DEGREE);
	auto low2 = CppUtil::SgFilter::smoothInts(lows, WINDOW_SIZE, DEGREE);
	transform(begin(upp2), end(upp2), begin(upp2), [=](int elm) { return min(max(elm, 0), height - 1); });
	transform(begin(low2), end(low2), begin(low2), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->upperYs() = upp2;
	this->lowerYs() = low2;
	this->deltaYs() = delt;;
	return true;
}


bool SemtRetina::BoundaryOPL::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsOPL();
	const int KERNEL_COLS = crta->getGradientKernelColsOPL();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(+1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(-1.0f);

	{
		auto mean = image->imageMean();
		auto stdv = image->imageStdev();
		int gmax = (int)(mean + stdv * 2.0f);
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		matDst.setTo(gmax, (matDst > gmax));
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* p_opls = pipe->probMapIplOpl();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* dst = matProb.ptr<float>(0);
		for (int i = 0; i < N; ++i) {
			float val = p_opls[i];
			dst[i] = val;
		}

		Mat matDst, matGrad, matOut;
		auto mean = cv::mean(matProb);
		cv::copyMakeBorder(matProb, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathProbMat());
	}
	return true;
}


bool SemtRetina::BoundaryOPL::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	float* p_edge = matEdge.ptr<float>(0);
	float* p_prob = matProb.ptr<float>(0);

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = disc_x1; x <= disc_x2; x++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1; y <= y2; y++) {
				auto idx = y * width + x;
				p_prob[idx] = p_edge[idx];
				// p_prob[idx] = 0.5f;
			}
		}
	}

	Mat matCost;
	// cv::multiply(matProb, matEdge, matCost);
	matProb.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryOPL::smoothBoundaryOPL(void)
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
	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	const int WINDOW_SIZE = crta->getLayerSmoothWindowOPL();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryOPL::smoothRefinedOPL(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bipl = segm->boundaryIPL();
	auto inns = bipl->sourceYs();
	auto* bios = segm->boundaryIOS();
	auto outs = bios->sourceYs();

	auto path = this->optimalPath();
	// transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE = crta->getLayerSmoothWindowOPL();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
	return true;
}


bool SemtRetina::BoundaryOPL::resizeToMatchSource(void)
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

bool SemtRetina::BoundaryOPL::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryIPL();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryIOS();
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

BoundaryOPL::BoundaryOPLImpl& SemtRetina::BoundaryOPL::impl(void) const
{
	return *d_ptr;
}