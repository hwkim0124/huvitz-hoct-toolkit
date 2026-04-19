#include "stdafx.h"
#include "BoundaryILM.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryILM::BoundaryILMImpl
{
	BoundaryILMImpl()
	{
	}
};


BoundaryILM::BoundaryILM(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryILMImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryILM::~BoundaryILM() = default;
SemtRetina::BoundaryILM::BoundaryILM(BoundaryILM&& rhs) = default;
BoundaryILM& SemtRetina::BoundaryILM::operator=(BoundaryILM&& rhs) = default;


bool SemtRetina::BoundaryILM::detectBoundary(void)
{
	if (!designPathConstraints()) {
		return false;
	}
	if (!prepareGradientMap()) {
		return false;
	}
	if (!preparePathCostMap()) {
		return false;
	}

	if (!searchPathMinCostInRange()) {
		return false;
	}
	if (!smoothBoundaryILM()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryILM::reconstructLayer(void)
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
	auto ilms = sourceYs();

	auto range = crta->getPathCostRangeDeltaILM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	for (int i = 0; i < width; i++) {
		upps[i] = max(ilms[i]-24, 0);
		lows[i] = ilms[i];
	}

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		range = crta->getPathDiscRangeDeltaILM();
		for (int x = x1; x <= x2; ++x) {
			delt[x] = range;
		}
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
	if (!smoothRefinedILM()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryILM::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSample();
	const auto imgMat = image->getCvMatConst();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* band = segm->retinaBandExtractor();
	auto inns = band->innerYsFull();
	auto outs = band->outerYsFull();

	auto range = crta->getPathCostRangeDeltaILM();
	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, range);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = outs[i];
	}

	const float THRESH_MIN = 45.0f;
	const float PROB_THRESH = 0.5f;
	const int LOWER_MARGIN = crta->getPathDownwardMarginILM();

	auto col_means = image->columnMeans();
	auto img_stdev = image->imageStdev();

	auto* pipe = segm->retinaInferPipeline();
	auto* nfls = pipe->probMapRnfl();
	auto* ipls = pipe->probMapIplOpl();

	for (int x = 0; x < width; x++) {
		auto y1 = upps[x];
		auto y2 = lows[x];
		auto thresh = col_means[x] + img_stdev * 2.0f;
		thresh = max(thresh, THRESH_MIN);

		int lim_y1 = -1;
		int lim_y2 = -1;

		// Limit the downward search range by the intensity. 
		for (int y = y1; y <= y2; y++) {
			auto val = imgMat.at<uchar>(y, x);
			if (val >= thresh) {
				lim_y1 = min(y2, y + LOWER_MARGIN);
				break;
			}
		}

		// Also, check the superficial probability.
		for (int y = y1; y <= y2; y++) {
			auto idx = y * width + x;
			auto val = max(nfls[idx], ipls[idx]);
			if (val > PROB_THRESH) {
				lim_y2 = min(y2, y + LOWER_MARGIN);
				break;
			}
		}

		if (lim_y1 >= 0 && lim_y2 >= 0) {
			lows[x] = max(lim_y1, lim_y2);
		}
		else if (lim_y1 >= 0) {
			lows[x] = lim_y1;
		}
		else if (lim_y2 >= 0) {
			lows[x] = lim_y2;
		}
	}

	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		auto min_upp = *std::min_element(upps.begin(), upps.end());

		range = crta->getPathDiscRangeDeltaILM();
		for (int x = x1; x <= x2; ++x) {
			upps[x] = min_upp;
			delt[x] = range;
		}
	}

	const int WINDOW_SIZE = crta->getPathSmoothWindowILM();
	const int DEGREE = 1;
	auto upp2 = CppUtil::SgFilter::smoothInts(upps, WINDOW_SIZE, DEGREE);
	auto low2 = CppUtil::SgFilter::smoothInts(lows, WINDOW_SIZE, DEGREE);
	auto del2 = CppUtil::SgFilter::smoothInts(delt, WINDOW_SIZE, DEGREE);
	transform(begin(upp2), end(upp2), begin(upp2), [=](int elm) { return min(max(elm, 0), height - 1); });
	transform(begin(low2), end(low2), begin(low2), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->upperYs() = upp2;
	this->lowerYs() = low2;
	this->deltaYs() = del2;
	return true;
}


bool SemtRetina::BoundaryILM::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = crta->getGradientKernelRowsILM();
	const int KERNEL_COLS = crta->getGradientKernelColsILM();

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	kernel.rowRange(0, KERNEL_ROWS / 2).setTo(-1.0f);
	kernel.rowRange(KERNEL_ROWS / 2 + 1, KERNEL_ROWS).setTo(1.0f);

	// Normalized gradient map by the intensity image.
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
		auto* prob_nfl = pipe->probMapRnfl();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* dst = matProb.ptr<float>(0);

		for (int i = 0; i < N; ++i) {
			float val1 = prob_nfl[i];
			dst[i] = val1;
		}
		matProb.copyTo(this->pathProbMat());
	}

	/*
	// Normalized gradient map by the RNFL & IPL probability maps.
	{
		auto* pipe = segm->retinaInferPipeline();
		auto* prob_nfl = pipe->probMapRnfl();
		auto* prob_ipl = pipe->probMapIplOpl();

		auto width = pipe->probMapWidth();
		auto height = pipe->probMapHeight();
		auto N = width * height;

		Mat matProb = Mat::zeros(height, width, CV_32F);
		float* dst = matProb.ptr<float>(0);

		for (int i = 0; i < N; ++i) {
			float val1 = prob_nfl[i];
			float val2 = prob_ipl[i];
			// dst[i] = std::max(val1, val2);
			dst[i] = val1 + val2;
		}

		auto mean = cv::mean(matProb);
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matProb;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathProbMat());
	}
	*/
	return true;
}


bool SemtRetina::BoundaryILM::preparePathCostMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto* band = segm->retinaBandExtractor();
	auto* pipe = segm->retinaInferPipeline();
	auto* head = pipe->probMapDiscHead();

	auto width = pipe->probMapWidth();
	auto height = pipe->probMapHeight();
	auto N = width * height;

	Mat& matEdge = this->pathEdgeMat();
	Mat& matProb = this->pathProbMat();

	float* p_prob = matProb.ptr<float>(0);
	float* p_edge = matEdge.ptr<float>(0);

	// Within optic disc region, the gradient derived from the intensity image
	// should be added to the path cost. 
	if (band->isNerveHeadRangeValid() && band->isNerveHeadDiscCupShaped()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = x1; x <= x2; ++x) {
			for (int y = upps[x]; y < lows[x]; ++y) {
				auto idx = y * width + x;
				// p_prob[idx] = max(head[idx], p_prob[idx]);
				p_prob[idx] = 0.5f;
			}
		}
	}

	Mat matCost;
	cv::multiply(matProb, matEdge, matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
	/*
	// As default, the path cost map is constructed using the gradient 
	// derived from the model's probability map.
	Mat matCost;
	matProb.copyTo(matCost);

	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageCoarse();
	auto width = image->getWidth();
	auto height = image->getHeight();
	*/
	/*
	// Within optic disc region, the gradient derived from the intensity image
	// should be added to the path cost. 
	if (band->isNerveHeadRangeValid()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		float* dst = matCost.ptr<float>(0);
		for (int x = x1; x <= x2; ++x) {
			for (int y = upps[x]; y < lows[x]; ++y) {
				auto idx = y * width + x;
				auto val1 = matEdge.at<float>(y, x);
				auto val2 = matProb.at<float>(y, x);
				auto cost = val1 + val2;
				// auto cost = ((val2 + 0.5f) * val1);
				dst[idx] = cost;
				// dst[idx] = matEdge.at<float>(y, x);
			}
		}
	}

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	*/
	return true;
}

bool SemtRetina::BoundaryILM::smoothBoundaryILM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	const int WINDOW_SIZE = crta->getLayerSmoothWindowILM();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);
	
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryILM::smoothRefinedILM(void)
{
	auto* segm = retinaSegmenter();
	auto* crta = segm->retinaSegmCriteria();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageSource();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* bnfl = segm->boundaryNFL();
	auto outs = bnfl->sourceYs();
	auto path = this->optimalPath();

	const int WINDOW_SIZE = crta->getLayerSmoothWindowILM();
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->sampleYs() = filt;
	return true;
}

bool SemtRetina::BoundaryILM::resizeToMatchSource(void)
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

BoundaryILM::BoundaryILMImpl& SemtRetina::BoundaryILM::impl(void) const
{
	return *d_ptr;
}