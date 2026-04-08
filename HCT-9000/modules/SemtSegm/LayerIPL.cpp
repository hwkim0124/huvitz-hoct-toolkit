#include "stdafx.h"
#include "LayerIPL.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerIPL::LayerIPLImpl
{

	LayerIPLImpl()
	{
	}
};



LayerIPL::LayerIPL(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerIPLImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerIPL::~LayerIPL() = default;
SemtSegm::LayerIPL::LayerIPL(LayerIPL && rhs) = default;
LayerIPL & SemtSegm::LayerIPL::operator=(LayerIPL && rhs) = default;

bool SemtSegm::LayerIPL::buildFlattenedPath()
{
	if (!designFlattenedConstraints()) {
		return false;
	}
	if (!createFlattenedCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	smoothFlattenedPath();
	return true;
}


bool SemtSegm::LayerIPL::alterFlattenedPath()
{
	if (!designFlattenedConstraints2()) {
		return false;
	}
	if (!createFlattenedCostMap2()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}

	smoothFlattenedPath();
	return true;
}


bool SemtSegm::LayerIPL::buildBoundaryLayer()
{
	if (!designBoundaryConstraints()) {
		return false;
	}
	if (!createBoundaryCostMap()) {
		return false;
	}
	if (!searchPathMinCost()) {
		return false;
	}
	smoothBoundaryPath();
	return true;
}


bool SemtSegm::LayerIPL::designFlattenedConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pClass->getInputImage();

	Mat matNFL = pClass->getScoreNFL()->getCvMatConst();
	Mat matOPL = pClass->getScoreOPL()->getCvMatConst();
	Mat matONL = pClass->getScoreONL()->getCvMatConst();
	Mat matCHO = pClass->getScoreCHO()->getCvMatConst();

	const auto& inn_layer = pSegm->getLayerILM()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOPL()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const int UPPER_SPAN = 20;
	const int LOWER_SPAN = 7; // 3;
	const int MOVE_DELTA = 3;

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, 0);
	deltas = vector<int>(width, MOVE_DELTA);

	auto* pWeight = getWeightMap();
	Mat weight = Mat::zeros(height, width, CV_32F);

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = inn_layer[c];
		int y2 = out_layer[c];
		
		y1 = y1 + (y2 - y1) / 3;
		y2 = max(y1, y2 - LOWER_SPAN);

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		int y0 = y1;
		float max_val = 0.0f;
		int max_pos = -1;
		for (r = y1; r < y2; r++) {
			float p1 = matOPL.at<float>(r, c);
			float p2 = matONL.at<float>(r, c);
			p1 = max(p1, matNFL.at<float>(r, c));
			p2 = max(p2, matCHO.at<float>(r, c));

			float ratio = p1 / (p2 + 0.0001f);
			if (p2 > 0.01f) {
				if (ratio > max_val && p1 > 0.01f) {
					max_val = ratio;
					max_pos = r;
				}
			}
			else {
				max_val = 0.0f;
				max_pos = -1;
			}
			y0 = (max_pos >= 0 ? max_pos : y0);
		}

		for (r = y1; r < y2; r++) {
			float esum = matOPL.at<float>(r, c);

			if (r < y0) {
				esum = 0.01f;
			}
			weight.at<float>(r, c) = esum;
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}


bool SemtSegm::LayerIPL::designFlattenedConstraints2()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pClass->getInputImage();

	const auto& inn_layer = pSegm->getLayerNFL()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOPL()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const float UPPER_MARGIN = 0.25f;
	const float LOWER_MARGIN = 0.33f;
	const int MOVE_DELTA = 3; // 5; // 3;

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, 0);
	deltas = vector<int>(width, MOVE_DELTA);

	auto* pWeight = getWeightMap();
	Mat weight = Mat::zeros(height, width, CV_32F);

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = inn_layer[c];
		int y2 = out_layer[c];

		int d1 = (int)((y2 - y1) * UPPER_MARGIN);
		int d2 = (int)((y2 - y1) * LOWER_MARGIN);
		y1 = y1 + d1;
		y2 = y2 - d2;

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		for (r = y1; r < y2; r++) {
			weight.at<float>(r, c) = 1.0f;
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}

bool SemtSegm::LayerIPL::designBoundaryConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pSegm->getSampling()->source();
	
	const auto& inn_layer = pSegm->getLayerNFL()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOPL()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const float UPPER_MARGIN = 0.25f;
	const float LOWER_MARGIN = 0.25f;
	const int MOVE_DELTA = 5; // 3;

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, 0);
	deltas = vector<int>(width, MOVE_DELTA);

	auto* pWeight = getWeightMap();
	Mat weight = Mat::zeros(height, width, CV_32F);

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = inn_layer[c];
		int y2 = out_layer[c];

		int d1 = (int)((y2 - y1) * UPPER_MARGIN);
		int d2 = (int)((y2 - y1) * LOWER_MARGIN);
		y1 = y1 + d1;
		y2 = y2 - d2;

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		for (r = y1; r < y2; r++) {
			weight.at<float>(r, c) = 1.0f;
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}


bool SemtSegm::LayerIPL::createFlattenedCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->average();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pSample->getMean() * 0.5f + pSample->getStddev()*0.0f);
	int gray_max = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*2.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 5; // 7;
	const int KERNEL_COLS = 3;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	
	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	outMat = outMat.mul(pWeight->getCvMatConst());
	
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

bool SemtSegm::LayerIPL::createFlattenedCostMap2()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->average();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pSample->getMean() * 0.5f + pSample->getStddev()*0.0f);
	int gray_max = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*2.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 5; // 7;
	const int KERNEL_COLS = 3; // 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	outMat = outMat.mul(pWeight->getCvMatConst());

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

bool SemtSegm::LayerIPL::createBoundaryCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->source();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pImage->getMean() * 0.5f + pImage->getStddev()*0.0f);
	// int gray_max = (int)(pImage->getMean() * 1.0f + pImage->getStddev()*2.0f);

	// mask = srcMat > gray_max;
	// srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 7;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1.0f;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	const auto& uppers = getUppers();
	const auto& lowers = getLowers();
	const auto& deltas = getDeltas();

	outMat = outMat.mul(pWeight->getCvMatConst());

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

bool SemtSegm::LayerIPL::smoothFlattenedPath()
{
	auto& path = getOptimalPath();
	auto* pImage = getSegmentator()->getClassifier()->getInputImage();
	const auto& inner = getSegmentator()->getLayerILM()->getOptimalPath();
	const auto& outer = getSegmentator()->getLayerOPL()->getOptimalPath();

	int rows = pImage->getHeight();

	const int FILTER_SIZE = 5; // 7; // 13;
	const int CURVE_DEGREE = 1;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);

	std::transform(cbegin(path), cend(path), cbegin(inner), begin(path), [=](int elem1, int elem2) { return max(elem1, elem2); });
	std::transform(cbegin(path), cend(path), cbegin(outer), begin(path), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerIPL::smoothBoundaryPath()
{
	auto* pSegm = getSegmentator();
	auto* pSample = pSegm->getSampling();

	int width = pSample->getSourceWidth();
	int height = pSample->getSourceHeight();
	float rangeX = pSegm->getImageRangeX();

	const int FILTER_SIZE = 11; // 17; // 9;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int size = (int)path.size();
	if (size <= 0) {
		return false;
	}

	const auto& inner = pSegm->getLayerNFL()->getOptimalPath();

	float scale = (1.0f / pSample->getSampleWidthRatio());
	int filter = (int)(scale * FILTER_SIZE);

	auto outs = path;
	transform(begin(outs), end(outs), begin(outs), [=](int elem) { return (elem + 1); });

	outs = CppUtil::SgFilter::smoothInts(outs, filter, CURVE_DEGREE);
	std::transform(cbegin(outs), cend(outs), cbegin(inner), begin(outs), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });

	path = outs;
	return true;
}


LayerIPL::LayerIPLImpl & SemtSegm::LayerIPL::getImpl(void) const
{
	return *d_ptr;
}
