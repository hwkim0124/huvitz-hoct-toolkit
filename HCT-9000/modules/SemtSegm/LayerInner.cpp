#include "stdafx.h"
#include "LayerInner.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerInner::LayerInnerImpl
{

	LayerInnerImpl()
	{
	}
};



LayerInner::LayerInner(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerInnerImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerInner::~LayerInner() = default;
SemtSegm::LayerInner::LayerInner(LayerInner && rhs) = default;
LayerInner & SemtSegm::LayerInner::operator=(LayerInner && rhs) = default;

bool SemtSegm::LayerInner::buildOutlinePath()
{
	if (!designOutlineConstraints()) {
		return false;
	}
	if (!createOutlineCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	smoothBoundaryPath();
	return true;
}

bool SemtSegm::LayerInner::buildBoundaryPath()
{
	if (!designBorderConstraints()) {
		return false;
	}
	if (!createBorderCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	smoothBoundaryPath();
	return true;
}

bool SemtSegm::LayerInner::smoothBoundaryPath()
{
	auto& path = getOptimalPath();
	auto* pImage = getSegmentator()->getSampling()->sample();

	int rows = pImage->getHeight();

	const int FILTER_SIZE = 7;
	const int CURVE_DEGREE = 1;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);

	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerInner::designOutlineConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();
	auto* pBiases = getBiasesMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_edges = pSegm->getBodering()->getOuterEdges();
	const auto& inn_bound = pSegm->getBodering()->getInnerBound();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int UPPER_SPAN = 35;
	const int LOWER_SPAN = 15;
	const int MOVE_DELTA = 7; // 5;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height-1);
	deltas = vector<int>(width, MOVE_DELTA);

	Mat weight = Mat::ones(height, width, CV_32F);
	Mat biases = Mat::zeros(height, width, CV_32F);

	int y_min = *min_element(inn_bound.cbegin(), inn_bound.cend());

	int r;
	for (int c = 0; c < width; c++) {
		uppers[c] = inn_bound[c];
		lowers[c] = out_bound[c];

		if (inn_edges[c] >= 0) {
			if (out_edges[c] >= 0) {
				// Limit lower bound with outer point. 
				lowers[c] = out_edges[c];

				auto dist = max(out_edges[c] - inn_edges[c], 1);
				for (r = inn_edges[c] + 1; r <= out_edges[c]; r++) {
					auto ratio = (float)(r - inn_edges[c]) / dist;
					weight.at<float>(r, c) = 1.0f - ratio;
				}
				/*
				// Below outer points. 
				for (; r <= out_bound[c]; r++) {
					weight.at<float>(r, c) = 0.01f;
				}
				*/
			}
			else {
				// Cost weight is decreased as further from inner point. 
				auto dist = max((out_bound[c] - inn_edges[c]), 1);
				for (r = inn_edges[c] + 1; r <= out_bound[c]; r++) {
					auto ratio = (float)(r - inn_edges[c]) / dist;
					weight.at<float>(r, c) = 1.0f - ratio;
				}
			}

			// Inner retina passed out at top of image. 
			if (inn_edges[c] == 0) {
				// Encourage the optimal path to be adhered to the top.  
				weight.at<float>(0, c) = 1.0f;
				biases.at<float>(0, c) = -9999.0f;
				for (r = 1; r <= out_bound[c]; r++) {
					weight.at<float>(r, c) = 0.0001f;
				}
			}
		}
		else {
			/*
			auto dist = max((out_bound[c] - inn_bound[c]), 1);
			for (r = inn_bound[c] + 1; r <= out_bound[c]; r++) {
				auto ratio = (float)(r - inn_bound[c]) / dist;
				weight.at<float>(r, c) = 1.0f - ratio;
			}
			*/
		}
	}

	pWeight->getCvMat() = weight;
	pBiases->getCvMat() = biases;
	return true;
}

bool SemtSegm::LayerInner::designBorderConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->average();
	auto* pWeight = getWeightMap();

	const auto& inn_edges = pSegm->getBodering()->getInnerEdges();
	const auto& out_bound = pSegm->getBodering()->getOuterBound();

	const auto& inn_layer = getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	const int DELTA_MIN = 9;
	const int DELTA_MAX = 27;
	const int LOWER_OFFSET = 1;
	const int UPPER_SPAN = 36;// 25;
	const int LOWER_SPAN = 24;

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, height - 1);
	deltas = vector<int>(width, DELTA_MIN);

	Mat weight = Mat::ones(height, width, CV_32F);
	Mat srcMat = pImage->getCvMatConst();

	int thresh = (int)(pImage->getMean() * 0.5f + pImage->getStddev() * 0.5f);
	// int thresh = (int)max(pImage->getMean(), pImage->getStddev());

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = max(inn_layer[c] - UPPER_SPAN, 0);
		int y2 = out_bound[c];
		int yt = -1;
		int dist = 0;

		if (inn_layer[c] == 0) {
			// Inner layer line starting to overlapped at top of image. 
			y2 = y1 + 1;
			yt = y1;
		}
		else {
			// It makes the outer limit reach to the bottom of disc cup.
			for (r = y1; r <= y2; r++) {
				if (srcMat.at<uchar>(r, c) >= thresh) {
					yt = r;
					break;
				}
			}
		}

		yt = (yt < inn_layer[c] ? inn_layer[c] : yt);
		for (r = (yt + 1), dist = 1; r <= y2; r++, dist++) {
			weight.at<float>(r, c) = max(0.001f, 1.0f - dist * 0.05f);
			// weight.at<float>(r, c) = 0.001f;
		}

		// Estimate the proper delta from the gap between the consequtive points in fitting curve.  
		int delta = DELTA_MIN;
		if (yt <= inn_layer[c]) {
			if (c > 1) {
				int multi = abs(inn_layer[c] - inn_layer[c - 1]);
				delta *= multi;
			}
		}
		else {
			int dive = yt - inn_layer[c];
			delta = dive;
		}

		delta = min(max(delta, DELTA_MIN), DELTA_MAX);

		uppers[c] = y1;
		lowers[c] = max(y2, inn_layer[c]);
		// deltas[c] = min(max(dist, DELTA_MIN), DELTA_MAX);
		deltas[c] = delta;
	}

	pWeight->getCvMat() = weight;
	return true;
}

bool SemtSegm::LayerInner::createOutlineCostMap()
{
	const auto* pSegm = getSegmentator();
	const auto* pImage = pSegm->getSampling()->sample();
	const auto* pWeight = getWeightMap();
	const auto* pBiases = getBiasesMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = 0; // pImage->getMean() + pImage->getStddev()*0.0f;
	int gray_max = (int)(pImage->getMean() + pImage->getStddev()*1.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	// mask = srcMat < gray_min;
	// srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 9; // 15; // 7;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	outMat = outMat.mul(pWeight->getCvMatConst());
	outMat = outMat + pBiases->getCvMatConst();

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		for (int r = uppers[c]; r <= lowers[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - deltas[c - 1]; k <= r + deltas[c - 1]; k++) {
				if (k >= uppers[c - 1] && k <= lowers[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	getCostMap()->getCvMat() = outMat;
	return true;
}


bool SemtSegm::LayerInner::createBorderCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = 0; // pImage->getMean() + pImage->getStddev()*0.0f;
	int gray_max = (int)(pImage->getMean() + pImage->getStddev()*1.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	// mask = srcMat < gray_min;
	// srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 15; // 9; // 15; // 7;
	const int KERNEL_COLS = 5; // 3;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	outMat = outMat.mul(pWeight->getCvMatConst());

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		for (int r = uppers[c]; r <= lowers[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - deltas[c - 1]; k <= r + deltas[c - 1]; k++) {
				if (k >= uppers[c - 1] && k <= lowers[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	getCostMap()->getCvMat() = outMat;
	return true;
}


LayerInner::LayerInnerImpl & SemtSegm::LayerInner::getImpl(void) const
{
	return *d_ptr;
}
