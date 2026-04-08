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


bool SemtRetina::BoundaryOPL::designPathConstraints(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();
	const auto imgMat = image->getCvMatConst();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();

	auto* bonl = segm->boundaryONL();
	auto outs = bonl->sampleYs();

	auto* bout = segm->boundaryOUT();
	auto out2 = bout->sampleYs();

	auto width = image->getWidth();
	auto height = image->getHeight();

	auto upps = std::vector<int>(width, 0);
	auto lows = std::vector<int>(width, 0);
	auto delt = std::vector<int>(width, 7);

	const int UPPER_MARGIN = 12; // 24;

	for (int i = 0; i < width; i++) {
		auto y1 = inns[i];
		auto y2 = outs[i];
		auto lim1 = y1 + (int)((y2 - y1) * 0.5f);
		auto lim2 = y2;
		y1 = min(max(y1, lim1), lim2);
		upps[i] = y1;
		lows[i] = y2;
	}

	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		for (int x = head_x1; x <= head_x2; x++) {
			auto y1 = inns[x];
			auto y2 = outs[x];
			auto lim1 = y1 + (int)((y2 - y1) * 0.85f);
			auto lim2 = y2;
			y1 = min(max(y1, lim1), lim2);
			upps[x] = y1;
			lows[x] = y2;
		}
	}

	const int WINDOW_SIZE = 7;
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
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	const int KERNEL_ROWS = 15;
	const int KERNEL_COLS = 9;

	Mat matImg = image->getCvMatConst();
	Mat kernel = Mat::ones(KERNEL_ROWS, KERNEL_COLS, CV_32F);

	/*
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	*/
	for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

	{
		auto mean = image->imageMean();
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matImg;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathEdgeMat());
	}

	{
		auto* pipe = segm->retinaInferPipeline();
		auto* prob = pipe->probMapIplOpl();

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
		Mat matSrc, matDst, matGrad, matOut;

		matSrc = matProb;
		cv::copyMakeBorder(matSrc, matDst, 9, 9, 0, 0, cv::BORDER_CONSTANT, mean);
		cv::filter2D(matDst, matGrad, CV_32F, kernel, Point(-1, -1), 0.0, cv::BORDER_REFLECT);
		matOut = matGrad(cv::Rect(0, 9, matGrad.cols, matGrad.rows - 18));

		matOut.setTo(0.0f, (matOut < 0.0f));
		cv::normalize(matOut, matOut, 0.0, 1.0, cv::NORM_MINMAX);
		matOut.copyTo(this->pathProbMat());
		/*
		for (int x = 75; x < 76; ++x) {
			for (int y = 0; y < height; ++y) {
				auto idx = y * width + x;
				float val1 = prob[idx];
				float val2 = matOut.at<float>(y, x);
				if (resa->sampleIndex() == 55) {
					LogD() << "y = " << y << ", p = " << val1 << ", v = " << val2 << ", rnfl = " << rnfl[idx] << ", onl = " << onls[idx] << ", rpe = " << rpes[idx];
				}
			}
		}
		*/
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

	const Mat& matEdge = this->pathEdgeMat();
	const Mat& matProb = this->pathProbMat();

	const float* p_edge = matEdge.ptr<float>(0);
	const float* p_prob = matProb.ptr<float>(0);

	Mat matCost;
	// cv::add(matProb, matEdge, matCost);
	// cv::add(matProb, 1.0f, matCost); // matCost = matProb + 1.0f
	// cv::multiply(matCost, matEdge, matCost); // matCost *= matEdge
	matProb.copyTo(matCost);
	float* p_cost = matCost.ptr<float>(0);

	int head_x1, head_x2;
	if (band->getNerveHeadRangeX(head_x1, head_x2)) {
		auto upps = this->upperYs();
		auto lows = this->lowerYs();

		for (int x = head_x1; x <= head_x2; x++) {
			auto y1 = upps[x];
			auto y2 = lows[x];
			for (int y = y1; y <= y2; y++) {
				auto idx = y * width + x;
				// p_cost[idx] = (p_cost[idx] + 0.5f) * p_edge[idx];
				p_cost[idx] = p_edge[idx];
			}
		}
	}

	matCost *= -1.0f;
	matCost.copyTo(this->pathCostMat());
	return true;
}


bool SemtRetina::BoundaryOPL::smoothBoundaryOPL(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();
	auto* image = resa->imageSample();

	auto width = image->getWidth();
	auto height = image->getHeight();
	auto path = this->optimalPath();

	auto* bilm = segm->boundaryILM();
	auto inns = bilm->sampleYs();
	auto* bonl = segm->boundaryONL();
	auto outs = bonl->sampleYs();

	const int WINDOW_SIZE = 31;
	const int DEGREE = 1;
	auto filt = CppUtil::SgFilter::smoothInts(path, WINDOW_SIZE, DEGREE);

	transform(cbegin(filt), cend(filt), cbegin(inns), begin(filt), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(cbegin(filt), cend(filt), cbegin(outs), begin(filt), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(filt), end(filt), begin(filt), [=](int elm) { return min(max(elm, 0), height - 1); });
	this->sampleYs() = filt;
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