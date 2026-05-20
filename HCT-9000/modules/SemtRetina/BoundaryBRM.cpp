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
	if (!searchPathMinCost()) {
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

bool SemtRetina::BoundaryBRM::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaBRM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* brpe = segm->boundaryRPE();
	auto rpes = brpe->sourceYs();
	auto brms = sourceYs();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sourceYs();

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		delt[i] = 1;
	}

	const int UPPER_OFFS = crta->getLayerRefiningUpperSpaceBRM();
	const int LOWER_OFFS = crta->getLayerRefiningLowerSpaceBRM();
	for (int i = 0; i < width; i++) {
		auto y1 = (int)(brms[i] - UPPER_OFFS);
		auto y2 = (int)(brms[i] + LOWER_OFFS);
		upps[i] = max(y1, rpes[i]);
		lows[i] = min(y2, outs[i]);
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		const int OFFSET_MIN = crta->getLayerDiscUpperOffsetMinBRM();
		const int OFFSET_MAX = crta->getLayerDiscUpperOffsetMaxBRM();
		const int moves = (band->isNerveHeadDiscCupShaped()) ? crta->getPathDiscRangeDeltaBRM() : crta->getPathCostRangeDeltaBRM();

		for (int x = disc_x1; x <= disc_x2; x++) {
			auto y1 = rpes[x] + OFFSET_MIN;
			auto y2 = rpes[x] + OFFSET_MAX;
			upps[x] = min(y1, outs[x]);
			lows[x] = min(y2, outs[x]);
			delt[x] = moves;
		}
		delt[disc_x1] *= 10;
		delt[disc_x2] *= 10;
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
	if (!smoothRefinedBRM()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryBRM::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();
	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sampleYs();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	auto range = crta->getPathCostRangeDeltaBRM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	const int OFFSET_MIN = crta->getPathUpperOffsetMinBRM();
	for (int i = 0; i < width; i++) {
		upps[i] = min(ioss[i] + OFFSET_MIN, outs[i]);
		lows[i] = outs[i];
	}

	const int TOP_MARGIN = crta->getPathTopOverMarginILM();
	const int TOP_OFFSET = crta->getPathTopOverOffsetIOS();
	for (int x = 0; x < width; x++) {
		if (ilms[x] <= TOP_MARGIN) {
			lows[x] = min(lows[x], upps[x] + TOP_OFFSET);
			delt[x] = 1;
		}
	}

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		lows[i] = min(ioss[i] + TOP_OFFSET, outs[i]);
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		lows[i] = min(ioss[i] + TOP_OFFSET, outs[i]);
		delt[i] = 1;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		const int MARGIN_MIN = crta->getPathDiscUpperOffsetMinBRM();
		const int MARGIN_MAX = crta->getPathDiscUpperOffsetMaxBRM();
		const int moves = (band->isNerveHeadDiscCupShaped()) ? crta->getPathDiscRangeDeltaBRM() : crta->getPathCostRangeDeltaBRM();

		for (int x = disc_x1; x <= disc_x2; ++x) {
			upps[x] = min(ioss[x] + MARGIN_MIN, height-1);
			lows[x] = min(ioss[x] + MARGIN_MAX, height-1);
			delt[x] = moves;
		}
		delt[disc_x1] *= 10;
		delt[disc_x2] *= 10;
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryBRM::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	const int KERNEL_ROWS = crta->getGradientKernelRowsBRM();
	const int KERNEL_COLS = crta->getGradientKernelColsBRM();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);
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

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* p_rpes = pipe->probMapRpe();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* p_prob = matProb.ptr<float>(0);
		memcpy(p_prob, p_rpes, N * sizeof(float));

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


bool SemtRetina::BoundaryBRM::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();
	auto* head = pipe->probMapDiscHead();
	auto* resa = segm->bscanResampler();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	float* p_edge = matEdge.ptr<float>(0);
	float* p_prob = matProb.ptr<float>(0);
	auto upps = this->upperYs();
	auto lows = this->lowerYs();

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto disc_cx = (disc_x1 + disc_x2) / 2;

		for (int x = disc_x1, k = 0; x <= disc_cx; x++, k++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1, d = 0; y <= y2; y++, d++) {
				auto idx = y * width + x;
				auto w = min(k, d) * 0.001f;
				p_prob[idx] += w;
			}
		}

		for (int x = disc_x2, k = 0; x >= disc_cx; x--, k++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1, d = 0; y <= y2; y++, d++) {
				auto idx = y * width + x;
				auto w = min(k, d) * 0.001f;
				p_prob[idx] += w;
			}
		}
	}

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();

	const int TOP_MARGIN = crta->getPathTopOverMarginILM();
	for (int x = 0; x < width; x++) {
		if (ilms[x] <= TOP_MARGIN) {
			for (int y = 0; y < height - 1; y++) {
				auto idx = y * width + x;
				p_prob[idx] = p_edge[idx];
			}
		}
	}

	Mat matCost;
	matProb.copyTo(matCost);
	// cv::multiply(matProb, matEdge, matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryBRM::smoothBoundaryBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bios = segm->boundaryIOS();
	auto inns = bios->sampleYs();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowBRM(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowBRM(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE);
			// filt = CppUtil::SgFilter::smoothInts(filt, WINDOW_SIZE1, DEGREE);
		}
		else {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, true);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}

	if (resa->sampleScaleRatioY() < 1.0f) {
		transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm + 1, 0), height - 1); });
	}

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryBRM::smoothRefinedBRM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sourceYs();
	auto path = this->optimalPath();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowBRM(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowBRM(false);
	const int DEGREE = 1;
	const int DISC_GAP = crta->getLayerDiscEdgeGapBRM();

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, false, DISC_GAP);
			// filt = CppUtil::SgFilter::smoothInts(filt, WINDOW_SIZE1, DEGREE);
		}
		else {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, false, 1);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(ioss), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2+1); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
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