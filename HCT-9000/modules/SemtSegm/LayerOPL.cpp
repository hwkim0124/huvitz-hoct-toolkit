#include "stdafx.h"
#include "LayerOPL.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerOPL::LayerOPLImpl
{

	LayerOPLImpl()
	{
	}
};



LayerOPL::LayerOPL(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerOPLImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerOPL::~LayerOPL() = default;
SemtSegm::LayerOPL::LayerOPL(LayerOPL && rhs) = default;
LayerOPL & SemtSegm::LayerOPL::operator=(LayerOPL && rhs) = default;


bool SemtSegm::LayerOPL::buildFlattenedPath()
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

bool SemtSegm::LayerOPL::buildBoundaryLayer()
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

bool SemtSegm::LayerOPL::designFlattenedConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pBorder = pSegm->getBodering();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->sample();

	auto* pWeight = getWeightMap();

	const auto& inn_layer = pSegm->getLayerILM()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerOuter()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const int INN_OUT_GAP_MAX = 25;
	const int INN_OUT_GAP_MIN = 9;
	const int UPPER_SPAN = 9;
	const int LOWER_SPAN = 15;
	const int MOVE_DELTA = 3;

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, 0);
	deltas = vector<int>(width, MOVE_DELTA);

	Mat weight = Mat::zeros(height, width, CV_32F);
	Mat matNFL = pClass->getScoreNFL()->getCvMatConst();
	Mat matOPL = pClass->getScoreOPL()->getCvMatConst();
	Mat matONL = pClass->getScoreONL()->getCvMatConst();
	Mat matRPE = pClass->getScoreRPE()->getCvMatConst();
	Mat matCHO = pClass->getScoreCHO()->getCvMatConst();
	Mat matSrc = pImage->getCvMatConst();

	bool is_disc = pBorder->isOpticDiscRegion();
	int disc_x1 = pBorder->getOpticDiscX1();
	int disc_x2 = pBorder->getOpticDiscX2();
	int align_top = pClass->getAlignTopPosition();
	/*
	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = min(inn_layer[c], out_layer[c]);
		int y2 = max(inn_layer[c], out_layer[c]);

		y2 = max(y1, y2 - UPPER_SPAN);

		int thresh = (int)(pSample->getMean() + pSample->getStddev());
		int y0 = y2;
		for (r = y2; r >= y1; r--) {
			if (matSrc.at<uchar>(r, c) > thresh) {
				y0 = r;
				break;
			}
		}

		y1 = max(y1, y0 - UPPER_SPAN);
		y2 = min(y2, y0 + LOWER_SPAN);

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;
	}

	weight = Mat::ones(height, width, CV_32F);
	*/

	int thresh_min = (int)(pSample->getMean() + pSample->getStddev()*1.0f);
	thresh_min = max(min(thresh_min, 55), 35);

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = min(inn_layer[c], out_layer[c]);
		int y2 = max(inn_layer[c], out_layer[c]);
		int yt1 = -1;
		int yt2 = -1;

		y1 = max(y1, align_top);

		int dy = min((int)((y2 - y1) * 0.65f), 55);
		y1 = y1 + dy;
		y2 = max(y1, y2 - INN_OUT_GAP_MIN);

		if (is_disc) {
			if (c >= disc_x1 && c <= disc_x2) {
				// y1 = inn_layer[c];
				y2 = max(inn_layer[c], out_layer[c]);
				// deltas[c] = 1;
			}
		}

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		if (c < disc_x1 || c > disc_x2)
		{
			// OPL boundary point is located after intensity peak.
			int gval = 0;
			int yt = y2 - (y2 - y1) / 5;
			for (r = y2; r >= y1; r--) {
				gval = matSrc.at<uchar>(r, c);
				if (gval < thresh_min || r <= yt) {
					for (; r >= y1; r--) {
						gval = matSrc.at<uchar>(r, c);
						if (gval >= thresh_min) {
							yt2 = min(y2, r + LOWER_SPAN);
							yt1 = max(y1, r - UPPER_SPAN);
							break;
						}
					}
					break;
				}
			}
		}


		/*
		int y0 = y1;
		float max_val = 0.0f;
		for (r = y1; r < y2; r++) {
			float p1 = matOPL.at<float>(r, c);
			float p2 = matONL.at<float>(r, c);
			float ratio = p1 / (p2 + 0.0001f);
			if (ratio > max_val && p1 > 0.5f) {
				y0 = r;
			}
		}
		*/

		for (r = y1; r <= y2; r++) {
			float esum = 0.0f;
			float p1_max = 0.0f;
			float p2_max = 0.0f;
			for (int k = -1; k <= 1; k++) {
				int r2 = min(max(r + k, 0), height - 1);
				float p1 = matOPL.at<float>(r2, c);
				float p2 = matONL.at<float>(r2, c);
				p1 = max(p1, matNFL.at<float>(r2, c));
				p2 = max(p2, matRPE.at<float>(r2, c));
				p2 = max(p2, matCHO.at<float>(r2, c));
				esum += (p1 * log10(p1) + p2 * log10(p2));
				p1_max = max(p1_max, p1);
				p2_max = max(p2_max, p2);
			}
			esum *= -1.0f;

			// p1_max = matOPL.at<float>(r, c);
			if (p1_max <= 0.01f) {
				esum = 0.01f;
			}

			if (yt1 >= 0 && yt2 >= 0) {
				if (r < yt1 || r > yt2) {
					esum = 0.1f;
				}
				else {
					esum = max(0.5f, esum);
				}
			}
			weight.at<float>(r, c) = esum;
		}
	}

	pWeight->getCvMat() = weight;
	return true;
}

bool SemtSegm::LayerOPL::designBoundaryConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pSegm->getSampling()->source();

	const auto& inn_layer = pSegm->getLayerIPL()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerIOS()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const float UPPER_MARGIN = 0.25f;
	const float LOWER_MARGIN = 0.50f;
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

bool SemtSegm::LayerOPL::createFlattenedCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	// int gray_min = (int)(pSample->getMean()*0.0f + pSample->getStddev()*0.0f);
	int gray_max = (int)(pSample->getMean()*1.0f + pSample->getStddev()*1.0f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	// mask = srcMat < gray_min;
	// srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 11; // 13;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1;
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

bool SemtSegm::LayerOPL::createBoundaryCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->source();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pImage->getMean() * 0.0f + pImage->getStddev()*0.0f);
	int gray_max = (int)(pImage->getMean() * 1.0f + pImage->getStddev()*0.5f);

	mask = srcMat > gray_max;
	srcMat.setTo(gray_max, mask);
	// mask = srcMat < gray_min;
	// srcMat.setTo(gray_min, mask);

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

bool SemtSegm::LayerOPL::smoothFlattenedPath()
{
	auto& path = getOptimalPath();
	auto* pImage = getSegmentator()->getClassifier()->getInputImage();
	const auto& inner = getSegmentator()->getLayerILM()->getOptimalPath();

	int rows = pImage->getHeight();

	const int FILTER_SIZE = 5; // 7; // 13;
	const int CURVE_DEGREE = 1;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);

	std::transform(cbegin(path), cend(path), cbegin(inner), begin(path), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerOPL::smoothBoundaryPath()
{
	auto* pSegm = getSegmentator();
	auto* pSample = pSegm->getSampling();

	int width = pSample->getSourceWidth();
	int height = pSample->getSourceHeight();
	float rangeX = pSegm->getImageRangeX();

	const int FILTER_SIZE = 11; // 17; // 25;// 13;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int size = (int)path.size();
	if (size <= 0) {
		return false;
	}

	const auto& inner = pSegm->getLayerIPL()->getOptimalPath();

	float scale = (1.0f / pSample->getSampleWidthRatio());
	int filter = (int)(scale * FILTER_SIZE);
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, CURVE_DEGREE);
	std::transform(cbegin(outs), cend(outs), cbegin(inner), begin(outs), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });

	path = outs;
	return true;
}

LayerOPL::LayerOPLImpl & SemtSegm::LayerOPL::getImpl(void) const
{
	return *d_ptr;
}
