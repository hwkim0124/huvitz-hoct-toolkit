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
	if (!prepareGradientMap()) {
		return false;
	}
	if (!designPathConstraints()) {
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

bool SemtRetina::BoundaryILM::refineBoundary(void)
{
	if (!refinePathConstraints()) {
		return false;
	}
	if (!refinePathCostMap()) {
		return false;
	}

	if (!searchPathMinCostInRange()) {
		return false;
	}
	if (!smoothRefinedILM()) {
		return false;
	}
	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}

bool SemtRetina::BoundaryILM::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageCoarse();
	const auto imgMat = image->getCvMatConst();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* band = segm->retinaBandExtractor();
	auto inns = band->innerYsFull();
	auto outs = band->outerYsFull();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 9);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = outs[i];
	}

	const float THRESH_MIN = 45.0f;
	const int LOWER_MARGIN = 12;

	auto col_means = image->columnMeans();
	auto img_stdev = image->imageStdev();

	auto* pipe = segm->retinaInferPipeline();
	auto* nfls = pipe->probMapRnfl();

	for (int x = 0; x < width; x++) {
		auto y1 = upps[x];
		auto y2 = lows[x];
		auto thresh = col_means[x] + img_stdev;
		thresh = max(thresh, THRESH_MIN);

		int lim_y1 = -1;
		int lim_y2 = -1;
		for (int y = y1; y <= y2; y++) {
			auto val = imgMat.at<uchar>(y, x);
			if (val >= thresh) {
				lim_y1 = min(y2, y + LOWER_MARGIN);
				break;
			}
		}

		for (int y = y1; y <= y2; y++) {
			auto idx = y * width + x;
			auto val = nfls[idx];
			if (val > 0.5f) {
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

	if (band->isNerveHeadRangeValid()) {
		auto x1 = band->opticDiscMinX();
		auto x2 = band->opticDiscMaxX();
		auto min_upp = *std::min_element(upps.begin(), upps.end());

		for (int x = x1; x <= x2; ++x) {
			upps[x] = min_upp;
			delt[x] = 15;
		}
	}

	const int WINDOW_SIZE = 7;
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

bool SemtRetina::BoundaryILM::refinePathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto* image = resa->imageCoarse();
	const auto imgMat = image->getCvMatConst();
	auto width = image->getWidth();
	auto height = image->getHeight();

	auto* band = segm->retinaBandExtractor();
	auto inns = sampleYs();
	auto* bnfl = segm->boundaryNFL();
	auto outs = bnfl->sampleYs();
	auto* bout = segm->boundaryOUT();
	auto out2 = bout->sampleYs();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 9);

	for (int i = 0; i < width; i++) {
		upps[i] = inns[i];
		lows[i] = outs[i];
	}

	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		for (int x = head_x1; x <= head_x2; x++) {
			lows[x] = out2[x];
			delt[x] = 15;
		}
	}

	const int WINDOW_SIZE = 7;
	const int DEGREE = 1;
	auto low2 = CppUtil::SgFilter::smoothInts(lows, WINDOW_SIZE, DEGREE);
	auto del2 = CppUtil::SgFilter::smoothInts(delt, WINDOW_SIZE, DEGREE);
	transform(begin(low2), end(low2), begin(low2), [=](int elm) { return min(max(elm, 0), height - 1); });

	this->upperYs() = upps;
	this->lowerYs() = low2;
	this->deltaYs() = del2;
	return true;
}

bool SemtRetina::BoundaryILM::prepareGradientMap(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 5;

	Mat matSrc = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);

	/*
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c <= (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	*/
	for (int r = 0; r < (kernel.rows / 2); r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

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
			dst[i] = std::max(val1, val2);
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
	return true;
}


bool SemtRetina::BoundaryILM::preparePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();
	const Mat& matProb = this->pathProbMat();

	Mat matCost;
	matProb.copyTo(matCost);
	// cv::add(matProb, 1.0f, matCost); // matCost = matProb + 1.0f
	// cv::multiply(matCost, matEdge, matCost); // matCost *= matEdge
	// cv::add(matProb, matEdge, matCost);

	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* band = segm->retinaBandExtractor();

	auto* image = resa->imageCoarse();
	auto width = image->getWidth();
	auto height = image->getHeight();

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
				auto cost = ((val2 + 0.5f) * val1);
				// matCost.at<float>(y, x) = cost;
				dst[idx] = cost;
			}
		}
	}

	/*
	double minEdge, maxEdge;
	cv::minMaxLoc(matEdge, &minEdge, &maxEdge);
	double minProb, maxProb;
	cv::minMaxLoc(matProb, &minProb, &maxProb);
	double minCost, maxCost;
	cv::minMaxLoc(matCost, &minCost, &maxCost);
	*/

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	
	/*
	const auto& uppers = upperYs();
	const auto& lowers = lowerYs();
	const auto& deltas = deltaYs();
	
	const float COST_MAX = 999999.0f;
	Mat matAccm;
	matCost.copyTo(matAccm);
	
	for (int c = 1; c < matCost.cols; c++) {
		auto y1 = uppers[c];
		auto y2 = lowers[c];
		for (int r = y1; r <= y2; r++) {
			auto minCost = COST_MAX;
			auto r1 = r - deltas[c - 1];
			auto r2 = r + deltas[c - 1];
			for (int k = r1; k <= r2; k++) {
				auto b1 = uppers[c - 1];
				auto b2 = lowers[c - 1];
				if (k >= b1 && k <= b2) {
					auto val = matAccm.at<float>(k, c - 1);
					if (val < minCost) {
						minCost = val;
					}
				}
			}
			auto sum = matAccm.at<float>(r, c);
			matAccm.at<float>(r, c) += minCost;
			if (segm->bscanResampler()->sampleIndex() == 54) {
			//	LogD() << "x = " << c << ", y = " << r << ", val = " << minCost << ", sum = " << sum << ", acc = " << matAccm.at<float>(r, c);
			}
		}
		if (segm->bscanResampler()->sampleIndex() == 54) {
			// LogD() << "\n";
			if (c == 231) {
				c = 231;
			}
		}
	}
	
	if (segm->bscanResampler()->sampleIndex() == 54) {
		for (int x = 0; x < width; ++x) {
			auto y1 = this->upperYs()[x];
			auto y2 = this->lowerYs()[x];
			for (int y = y1; y <= y2; ++y) {
				auto val = matCost.at<float>(y, x);
				auto val2 = matAccm.at<float>(y, x);
				LogD() << "x = " << x << ", y = " << y << ", val = " << val << ", acc = " << val2;
			}
			LogD() << "\n";
			if (x == 231) {
				x = 231;
			}
		}
	}
	*/
	return true;
}

bool SemtRetina::BoundaryILM::refinePathCostMap(void)
{
	const Mat& matEdge = this->pathEdgeMat();

	Mat matCost;
	matEdge.copyTo(matCost);

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}

bool SemtRetina::BoundaryILM::smoothBoundaryILM(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	const int WINDOW_SIZE = 5;
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
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageCoarse();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bnfl = segm->boundaryNFL();
	auto outs = bnfl->sampleYs();

	const int WINDOW_SIZE = 5;
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