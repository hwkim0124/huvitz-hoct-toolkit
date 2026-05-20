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

	auto* bipl = segm->boundaryIPL();
	auto ipls = bipl->sourceYs();
	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sourceYs();
	auto opls = sourceYs();

	const int UPPER_OFFS = crta->getLayerUpperSpaceMinOPL();
	const int LOWER_OFFS = crta->getLayerLowerSpaceMaxOPL();
	for (int i = 0; i < width; i++) {
		auto size1 = (opls[i] - ipls[i]) + 1;
		auto size2 = (ioss[i] - opls[i]) + 1;
		// auto offs1 = (int)(size1 * 0.50f);
		// auto offs2 = (int)(size2 * 0.35f);
		auto offs1 = (int)(size1 * 0.15f);
		auto offs2 = (int)(size2 * 0.15f);

		auto y1 = max(ipls[i], opls[i] - offs1);
		auto y2 = min(ioss[i], opls[i] + offs2);
		upps[i] = y1;
		lows[i] = y2;
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();

		if (band->isNerveHeadDiscCupShaped()) {
			const int moves = crta->getPathDiscRangeDeltaOPL();
			for (int x = disc_x1; x <= disc_x2; ++x) {
				upps[x] = min(ioss[x], ipls[x] + (int)((ioss[x] - ipls[x]) * 0.50f));
				lows[x] = min(ioss[x], opls[x] + (int)((ioss[x] - opls[x]) * 0.50f));
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
	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto onls = bonl->sampleYs();

	auto* band = segm->retinaBandExtractor();
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	auto moves = crta->getPathCostRangeDeltaOPL();
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
		/*
		auto dist = (ioss[i] - nfls[i]) + 1;
		auto off1 = (int)(dist * 0.15f);
		auto off2 = (int)(dist * 0.15f);
		upps[i] = nfls[i] + off1;
		lows[i] = ioss[i] - off2;
		*/
		upps[i] = nfls[i];
		lows[i] = ioss[i];
	}

	if (band->isNerveHeadRangeValid()) {
		auto disc_x1 = band->opticDiscMinX();
		auto disc_x2 = band->opticDiscMaxX();
		auto disc_cx = (disc_x1 + disc_x2) / 2;

		if (band->isNerveHeadDiscCupShaped()) {
			auto est_reti1 = (int)((ioss[disc_x1] - nfls[disc_x1]) * 0.5f);
			auto est_reti2 = (int)((ioss[disc_x2] - nfls[disc_x2]) * 0.5f);
			const int moves = crta->getPathDiscRangeDeltaOPL();

			for (int x = disc_x1; x <= disc_cx; ++x) {
				auto offs2 = (int)((ioss[x] - nfls[x]) * 0.30f);
				lows[x] = ioss[x] - offs2;
				upps[x] = min(nfls[x] + est_reti1, lows[x]);
				delt[x] = moves;
			}
			for (int x = disc_x2; x >= disc_cx; --x) {
				auto offs2 = (int)((ioss[x] - nfls[x]) * 0.30f);
				lows[x] = ioss[x] - offs2;
				upps[x] = min(nfls[x] + est_reti2, lows[x]);
				delt[x] = moves;
			}
		}
		else {
			for (int x = disc_x1; x <= disc_x2; ++x) {
				auto offs1 = (int)((ioss[x] - nfls[x]) * 0.35f);
				auto offs2 = (int)((ioss[x] - nfls[x]) * 0.35f);
				upps[x] = min(nfls[x] + offs1, ioss[x]);
				lows[x] = max(ioss[x] - offs2, upps[x]);
			}
		}
		delt[disc_x1] = moves * 10;
		delt[disc_x2] = moves * 10;
	}

	this->upperYs() = upps;
	this->lowerYs() = lows;
	this->deltaYs() = delt;
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
		// matDst.setTo(gmax, (matDst > gmax));
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
		memcpy(dst, p_opls, N * sizeof(float));

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

	if (band->isNerveHeadRangeValid()) {
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
	// cv::multiply(matProb, matEdge, matCost);

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
	auto* bout = segm->boundaryOUT();
	auto outs = bout->sampleYs();

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowOPL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowOPL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE);
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

	if (resa->sampleScaleRatioY() < 1.0f) {
		transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm + 1, 0), height - 1); });
	}

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

	auto* bnfl = segm->boundaryNFL();
	auto nfls = bnfl->sourceYs();
	auto* bios = segm->boundaryIOS();
	auto ioss = bios->sourceYs();

	auto path = this->optimalPath();
	// transform(begin(path), end(path), begin(path), [=](int elem) { return min(elem + 1, height - 1); });

	const int WINDOW_SIZE1 = crta->getLayerSmoothWindowOPL(true);
	const int WINDOW_SIZE2 = crta->getLayerSmoothWindowOPL(false);
	const int DEGREE = 1;

	auto filt = path;
	if (band->isNerveHeadRangeValid()) {
		if (band->isNerveHeadDiscCupShaped()) {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE2, DEGREE, WINDOW_SIZE2, DEGREE, false, 3);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
		else {
			// filt = smoothOptimalPathWithMultiSize(WINDOW_SIZE1, DEGREE, WINDOW_SIZE2, DEGREE, false, 1);
			filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
		}
	}
	else {
		filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE2, DEGREE);
	}

	transform(cbegin(filt), cend(filt), cbegin(nfls), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2+1); });
	transform(cbegin(filt), cend(filt), cbegin(ioss), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
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