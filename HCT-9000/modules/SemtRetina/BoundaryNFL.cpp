#include "stdafx.h"
#include "BoundaryNFL.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryNFL::BoundaryNFLImpl
{
	BoundaryNFLImpl()
	{
	}
};


BoundaryNFL::BoundaryNFL(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryNFLImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryNFL::~BoundaryNFL() = default;
SemtRetina::BoundaryNFL::BoundaryNFL(BoundaryNFL&& rhs) = default;
BoundaryNFL& SemtRetina::BoundaryNFL::operator=(BoundaryNFL&& rhs) = default;


bool SemtRetina::BoundaryNFL::detectBoundary(void)
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
	if (!smoothBoundaryNFL()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryNFL::reconstructLayer(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto range = crta->getPathCostRangeDeltaNFL();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sourceYs();
	auto* bipl = segm->boundaryIPL();
	auto ipls = bipl->sourceYs();
	auto nfls = sourceYs();

	const int UPPER_OFFS = crta->getLayerUpperSpaceMinNFL();

	for (int i = 0; i < width; i++) {
		auto size1 = nfls[i] - ilms[i];
		auto size2 = ipls[i] - nfls[i];
		// auto offs1 = (int)(size1 * 0.50f);
		// auto offs2 = (int)(size2 * 0.35f);
		auto offs1 = (int)(size1 * 0.15f);
		auto offs2 = (int)(size2 * 0.15f);
		auto y1 = max((int)(nfls[i] - offs1), ilms[i]); 
		auto y2 = min((int)(nfls[i] + offs2), ipls[i]);
		upps[i] = y1;
		lows[i] = y2;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto moves = (band->isNerveHeadDiscCupShaped()) ? crta->getPathDiscRangeDeltaNFL() : crta->getPathCostRangeDeltaNFL();
		for (int x = disc_x1; x <= disc_x2; ++x) {
			upps[x] = min(nfls[x], ilms[x] + (int)((nfls[x] - ilms[x]) * 0.50f));
			lows[x] = min(ipls[x], nfls[x] + (int)((ipls[x] - nfls[x]) * 0.50f));
			delt[x] = moves;
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

	auto* edge = resa->sourceFallEdge();
	Mat fall = edge->getCvMatConst();
	Mat matCost;
	fall.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());

	if (!searchPathMinCost()) {
		return false;
	}
	if (!smoothRefinedNFL()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryNFL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	const auto imgMat = image->getCvMatConst();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	auto moves = crta->getPathCostRangeDeltaNFL();
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
		upps[i] = ilms[i] ;
		lows[i] = onls[i] ;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto disc_cx = (disc_x1 + disc_x2) / 2;

		auto est_upp1 = (int)((onls[disc_x1] - ilms[disc_x1]) * 0.15f);
		auto est_low1 = (int)((onls[disc_x1] - ilms[disc_x1]) * 0.75f);
		auto est_upp2 = (int)((onls[disc_x2] - ilms[disc_x2]) * 0.15f);
		auto est_low2 = (int)((onls[disc_x2] - ilms[disc_x2]) * 0.75f);
		auto moves = (band->isNerveHeadDiscCupShaped()) ? crta->getPathDiscRangeDeltaNFL() : crta->getPathCostRangeDeltaNFL();

		for (int x = disc_x1; x <= disc_cx; ++x) {
			lows[x] = min(max(onls[x], ilms[x] + est_low1), outs[x]);
			upps[x] = min(lows[x], ilms[x] + est_upp1);
			delt[x] = moves;
		}
		for (int x = disc_x2; x >= disc_cx; --x) {
			lows[x] = min(max(onls[x], ilms[x] + est_low2), outs[x]);
			upps[x] = min(lows[x], ilms[x] + est_upp2);
			delt[x] = moves;
		}
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
	return true;
}


bool SemtRetina::BoundaryNFL::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsNFL();
	const int KERNEL_COLS = crta->getGradientKernelColsNFL();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(+1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(-1.0f);

	// Normalized gradient map by the filtered B-scan image.
	{
		auto mean = image->imageMean();
		Mat matDst, matGrad, matOut;

		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}

	// Normalized gradient map by the probability map of NFL layer from the inference pipeline.
	{
		auto* pipe = segm->retinaInferPipeline();
		auto* p_rnfl = pipe->probMapRnfl();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* dst = matProb.ptr<float>(0);
		memcpy(dst, p_rnfl, N * sizeof(float));

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


bool SemtRetina::BoundaryNFL::preparePathCostMap(void)
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
				p_prob[idx] += p_edge[idx];
			}
		}
	}

	Mat matCost;
	matProb.copyTo(matCost);
	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryNFL::smoothBoundaryNFL(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* band = segm->retinaBandExtractor();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowNFL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowNFL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			path = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE1, DEGREE);
			path = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE1, DEGREE);
			filt = path;
		}
		else {
			path = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE);
			path = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE1, DEGREE);
			filt = path;
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


bool SemtRetina::BoundaryNFL::smoothRefinedNFL(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bilm = segm->boundaryILM();
	auto ilms = bilm->sourceYs();
	auto* bipl = segm->boundaryIPL();
	auto ipls = bipl->sourceYs();
	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sourceYs();

	auto path = this->optimalPath();
	// transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowNFL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowNFL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			// path = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE1, DEGREE, true);
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

	transform(cbegin(filt), cend(filt), cbegin(ilms), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(ioss), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elem) { return min(max(elem, 0), height - 1); });
	this->sourceYs() = filt;
	return true;
}


bool SemtRetina::BoundaryNFL::resizeToMatchSource(void)
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


bool SemtRetina::BoundaryNFL::enforceSourceOrder(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* binn = segm->boundaryILM();
	auto inns = binn->sourceYs();
	auto* bout = segm->boundaryIPL();
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


BoundaryNFL::BoundaryNFLImpl& SemtRetina::BoundaryNFL::impl(void) const
{
	return *d_ptr;
}