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
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto moves = crta->getPathCostRangeDeltaIOS();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, moves);

	auto* bopl = segm->boundaryOPL();
	auto opls = bopl->sourceYs();
	auto* brpe = segm->boundaryRPE();
	auto rpes = brpe->sourceYs();
	auto ioss = sourceYs();

	auto ret_x1 = band->retinaBeginX();
	auto ret_x2 = band->retinaEndX();
	for (int i = 0; i < ret_x1; i++) {
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		delt[i] = 1;
	}

	const int OFFSET = crta->getLayerRefiningRangeIOS();
	for (int i = 0; i < width; i++) {
		auto y1 = (int)(ioss[i] - OFFSET);
		auto y2 = (int)(ioss[i] + OFFSET);
		upps[i] = max(y1, opls[i]);
		lows[i] = min(y2, rpes[i]);
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		const int OFFSET_MIN = crta->getLayerDiscOffsetMinIOS();
		const int OFFSET_MAX = crta->getLayerDiscOffsetMaxIOS();
		const int moves = (band->isNerveHeadDiscCupShaped()) ? crta->getPathDiscRangeDeltaIOS() : crta->getPathCostRangeDeltaIOS();

		for (int x = disc_x1; x <= disc_x2; x++) {
			auto y1 = rpes[x] - OFFSET_MAX;
			auto y2 = rpes[x] - OFFSET_MIN;
			upps[x] = y1;
			lows[x] = y2;
			delt[x] = moves;
		}

		delt[disc_x1] = moves * 10;
		delt[disc_x2] = moves * 10;
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
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();
	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	auto range = crta->getPathCostRangeDeltaIOS();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	for (int i = 0; i < width; i++) {
		upps[i] = max(onls[i], nfls[i]);
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
		lows[i] = min(onls[i] + TOP_OFFSET, outs[i]);
		delt[i] = 1;
	}
	for (int i = ret_x2 + 1; i < width; i++) {
		lows[i] = min(onls[i] + TOP_OFFSET, outs[i]);
		delt[i] = 1;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto disc_cx = (disc_x1 + disc_x2) / 2;

		if (band->isNerveHeadDiscCupShaped()) {
			const int est_retin1 = onls[disc_x1] - ilms[disc_x1];
			const int est_retin2 = onls[disc_x2] - ilms[disc_x2];
			const int moves = crta->getPathDiscRangeDeltaIOS();

			for (int x = disc_x1; x <= disc_cx; ++x) {
				upps[x] = min(max(onls[x], ilms[x] + est_retin1), outs[x]);
				lows[x] = outs[x];
				delt[x] = moves;
			}
			for (int x = disc_x2; x >= disc_cx; --x) {
				upps[x] = min(max(onls[x], ilms[x] + est_retin2), outs[x]);
				lows[x] = outs[x];
				delt[x] = moves;
			}
		}
		else {
			const auto OFFSET_MIN = crta->getPathDiscUpperOffsetMinIOS();
			const auto OFFSET_MAX = crta->getPathDiscUpperOffsetMaxIOS();
			for (int x = disc_x1; x <= disc_x2; ++x) {
				upps[x] = min(max(onls[x], nfls[x]) + OFFSET_MIN, outs[x]);
				lows[x] = min(max(onls[x], nfls[x]) + OFFSET_MAX, outs[x]);
			}
		}
		delt[disc_x1] *= 10;
		delt[disc_x2] *= 10;
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
	auto* image = resa->imageSample();

	const int KERNEL_ROWS = crta->getGradientKernelRowsIOS();
	const int KERNEL_COLS = crta->getGradientKernelColsIOS();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(-1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(1.0f);

	{
		auto mean = image->imageMean();
		auto stdv = image->imageStdev();
		int gmin = (int)(mean + stdv * 1.0f);
		int gmax = (int)(mean + stdv * 2.0f);
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		// matDst.setTo(gmin, (matDst < gmin));
		// matDst.setTo(gmax, (matDst > gmax));
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
		float* dst = matProb.ptr<float>(0);
		memcpy(dst, p_rpes, N * sizeof(float));

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
	auto* crta = segm->retinaSegmCriteria();
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

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sampleYs();

	auto upps = this->upperYs();
	auto lows = this->lowerYs();

	const int TOP_MARGIN = crta->getPathTopOverMarginILM();
	for (int x = 0; x < width; x++) {
		if (ilms[x] <= TOP_MARGIN) {
			for (int y = upps[x]; y <= lows[x]; y++) {
				auto idx = y * width + x;
				p_prob[idx] = p_edge[idx];
			}
		}
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		for (int x = disc_x1; x <= disc_x2; x++) {
			for (int y = upps[x]; y <= lows[x]; y++) {
				auto idx = y * width + x;
				p_prob[idx] += p_edge[idx];
			}
		}
	}

	Mat matCost;
	// cv::add(matProb, matEdge, matCost); 
	// cv::multiply(matProb, matEdge, matCost); 
	matProb.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryIOS::smoothBoundaryIOS(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bnfl = segm->boundaryNFL();
	auto* bonl = segm->boundaryONL();
	auto inns = bnfl->sampleYs();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();
	auto path = this->optimalPath();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowIOS(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowIOS(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, true);
			// filt = CppUtil::SgFilter::smoothInts(filt, WINDOW_SIZE1, DEGREE);
		}
		else {
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
	auto opls = bopl->sourceYs();
	auto* bbrm = segm->boundaryBRM();
	auto brms = bbrm->sourceYs();
	auto path = this->optimalPath();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowIOS(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowIOS(false);
	const int DEGREE = 1;
	const int DISC_GAP = crta->getLayerDiscEdgeGapIOS();

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE1, DEGREE, WINDOW_SIZE1, DEGREE, false, DISC_GAP);
			// filt = CppUtil::SgFilter::smoothInts(filt, WINDOW_SIZE1, DEGREE);
		}
		else {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, false, 1);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE1, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE1, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(opls), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2+1); });
	transform(cbegin(filt), cend(filt), cbegin(brms), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
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