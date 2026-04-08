#include "stdafx.h"
#include "LayerBRM.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerBRM::LayerBRMImpl
{

	LayerBRMImpl()
	{
	}
};



LayerBRM::LayerBRM(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerBRMImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerBRM::~LayerBRM() = default;
SemtSegm::LayerBRM::LayerBRM(LayerBRM && rhs) = default;
LayerBRM & SemtSegm::LayerBRM::operator=(LayerBRM && rhs) = default;


bool SemtSegm::LayerBRM::buildFlattenedPath()
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

bool SemtSegm::LayerBRM::buildBoundaryLayer()
{
	auto* pSegm = getSegmentator();
	auto* pSample = pSegm->getSampling();

	int width = pSample->getSourceWidth();
	int height = pSample->getSourceHeight();
	float rangeX = pSegm->getImageRangeX();

	const int FILTER_SIZE = 13;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int size = (int)path.size();
	if (size <= 0) {
		return false;
	}

	const auto& inner = pSegm->getLayerRPE()->getOptimalPath();

	float scale = (1.0f / pSample->getSampleWidthRatio());
	int filter = (int)(scale * FILTER_SIZE);
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, CURVE_DEGREE);
	std::transform(cbegin(outs), cend(outs), cbegin(inner), begin(outs), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });

	path = outs;

	/*
	if (pSegm->getBodering()->isOpticDiscRegion()) {
		int discX1 = pSegm->getBodering()->getOpticDiscX1();
		int discX2 = pSegm->getBodering()->getOpticDiscX2();
		for (int i = discX1; i <= discX2; i++) {
			path[i] = 128;
		}
	}
	*/
	return true;
}

bool SemtSegm::LayerBRM::designFlattenedConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pClass->getInputImage();

	const auto& inn_layer = pSegm->getLayerIOS()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();

	Mat matVIT = pClass->getScoreVIT()->getCvMatConst();
	Mat matNFL = pClass->getScoreNFL()->getCvMatConst();
	Mat matOPL = pClass->getScoreOPL()->getCvMatConst();
	Mat matONL = pClass->getScoreONL()->getCvMatConst();
	Mat matRPE = pClass->getScoreRPE()->getCvMatConst();
	Mat matCHO = pClass->getScoreCHO()->getCvMatConst();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const int UPPER_SPAN = 5;
	const int LOWER_SPAN = 35;
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
		int y1 = inn_layer[c] + UPPER_SPAN;
		int y2 = inn_layer[c] + LOWER_SPAN;

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		int y0 = y1;
		float max_val = 0.0f;
		int max_pos = -1;
		for (r = y1; r < y2; r++) {
			float p1 = matRPE.at<float>(r, c);
			float p2 = matCHO.at<float>(r, c);
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
			float esum = 0.0f;
			float p1_max = 0.0f;
			float p2_max = 0.0f;
			for (int k = -1; k <= 1; k++) {
				int r2 = min(max(r + k, 0), height - 1);
				float p1 = matRPE.at<float>(r2, c);
				float p2 = matCHO.at<float>(r2, c);

				p1 = max(p1, matVIT.at<float>(r2, c));
				p1 = max(p1, matNFL.at<float>(r2, c));
				p1 = max(p1, matOPL.at<float>(r2, c));

				esum += (p1 * log10(p1) + p2 * log10(p2));
				p1_max = max(p1_max, p1);
				p2_max = max(p2_max, p2);
			}
			esum *= -1.0f;

			if (r < y0) {
				esum = 0.01f;
			}
			if (p1_max <= 0.01f) {
				esum = 0.01f;
			}

			weight.at<float>(r, c) = esum;
		}
	}
	pWeight->getCvMat() = weight;
	return true;
}

bool SemtSegm::LayerBRM::createFlattenedCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getClassifier()->getInputImage();
	auto* pSample = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*1.0f);
	//int gray_max = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*7.5f);

	//mask = srcMat > gray_max;
	//srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 9;
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

bool SemtSegm::LayerBRM::smoothFlattenedPath()
{
	auto& path = getOptimalPath();
	auto* pImage = getSegmentator()->getClassifier()->getInputImage();
	const auto& inner = getSegmentator()->getLayerIOS()->getOptimalPath();

	int rows = pImage->getHeight();

	const int FILTER_SIZE = 5; // 7; // 13;
	const int CURVE_DEGREE = 1;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);

	std::transform(cbegin(path), cend(path), cbegin(inner), begin(path), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

LayerBRM::LayerBRMImpl & SemtSegm::LayerBRM::getImpl(void) const
{
	return *d_ptr;
}
