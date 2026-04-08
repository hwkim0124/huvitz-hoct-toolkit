#include "stdafx.h"
#include "LayerNFL.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerNFL::LayerNFLImpl
{

	LayerNFLImpl()
	{
	}
};



LayerNFL::LayerNFL(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerNFLImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerNFL::~LayerNFL() = default;
SemtSegm::LayerNFL::LayerNFL(LayerNFL && rhs) = default;
LayerNFL & SemtSegm::LayerNFL::operator=(LayerNFL && rhs) = default;


bool SemtSegm::LayerNFL::buildFlattenedPath()
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

bool SemtSegm::LayerNFL::buildBoundaryLayer()
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


bool SemtSegm::LayerNFL::designFlattenedConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pBorder = pSegm->getBodering();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->sample();
	
	const auto& inn_layer = pSegm->getLayerILM()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerIPL()->getOptimalPath();

	Mat matVIT = pClass->getScoreVIT()->getCvMatConst();
	Mat matNFL = pClass->getScoreNFL()->getCvMatConst();
	Mat matOPL = pClass->getScoreOPL()->getCvMatConst();
	Mat matONL = pClass->getScoreONL()->getCvMatConst();
	Mat matRPE = pClass->getScoreRPE()->getCvMatConst();
	Mat matCHO = pClass->getScoreCHO()->getCvMatConst();
	Mat matSrc = pImage->getCvMatConst();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const int MOVE_DELTA = 3;

	int width = pImage->getWidth();
	int height = pImage->getHeight();

	uppers = vector<int>(width, 0);
	lowers = vector<int>(width, 0);
	deltas = vector<int>(width, MOVE_DELTA);

	auto* pWeight = getWeightMap();
	Mat weight = Mat::zeros(height, width, CV_32F);

	int thresh_min = (int)(pSample->getMean() + pSample->getStddev()*1.5f);
	int thresh_max = (int)(pSample->getMean() + pSample->getStddev()*4.0f);
	thresh_max = min(thresh_max, 160);

	// LogD() << "mean: " << pSample->getMean() << ", stdev: " << pSample->getStddev() << " : " << thresh_min << ", " << thresh_max;

	bool is_disc = pBorder->isOpticDiscRegion();
	int disc_x1 = pBorder->getOpticDiscX1();
	int disc_x2 = pBorder->getOpticDiscX2();

	int r, c;
	for (c = 0; c < width; c++) {
		int y1 = inn_layer[c];
		int y2 = out_layer[c];
		int yt1 = -1;
		int yt2 = -1;
		int yb1 = -1;
		int yb2 = -1;

		{
			// NFL boundary point is located after intensity peak.
			// To exclude the strong reflection from the surface.  
			int y0 = min(y2, y1 + 0);

			int max_val = 0, max_pos = 0, gval = 0;
			for (r = y0; r <= y2; r++) {
				gval = matSrc.at<uchar>(r, c);
				if (gval >= max_val || gval >= thresh_max) {
					max_val = gval;
					max_pos = r;
				}
			}

			if (max_val >= thresh_max && !is_disc) {
				yt1 = max_pos;

				for (r = y2; r >= y1; r--) {
					gval = matSrc.at<uchar>(r, c);
					if (gval >= thresh_min) {
						yt2 = r;
						break;
					}
				}
			}
			else {
				yb1 = y1;
				yb2 = y2;

				// NFL boundary point is located after probability ratio peak.
				float max_val = 0.0f;
				int max_pos = -1;
				for (r = y1; r < y2; r++) {
					float p1 = matNFL.at<float>(r, c);
					float p2 = matOPL.at<float>(r, c);
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
				}
				yb1 = (max_pos >= 0 ? max_pos : yb1);
			}

			/*
			if (pSegm->getImageIndex() == 91 && c == 20) {
				LogD() << y1 << ", " << y2 << " : " << yt1 << ", " << yt2 << ", " << max_pos << ", " << max_val << " : " << yb1 << ", " << yb2;
			}
			*/
		}

		y1 = min(max(y1, 0), height - 1);
		y2 = min(max(y1, y2), height - 1);

		uppers[c] = y1;
		lowers[c] = y2;

		/*
		{
			// NFL boundary point is located after probability ratio peak.
			float max_val = 0.0f;
			for (r = y1; r < y2; r++) {
				float p1 = matNFL.at<float>(r, c);
				float p2 = matOPL.at<float>(r, c);
				float ratio = p1 / (p2 + 0.0001f);
				if (ratio > max_val && p1 > 0.5f) {
					max_val = ratio;
					yt = r;
				}
				if (p2 > 0.5f) {
					break;
				}
			}
		}
		*/

		for (r = y1; r < y2; r++) {
			float esum = 0.0f;
			float p1_max = 0.0f;
			float p2_max = 0.0f;
			for (int k = -1; k <= 1; k++) {
				int r2 = min(max(r + k, 0), height - 1);
				float p1 = matNFL.at<float>(r2, c);
				float p2 = matOPL.at<float>(r2, c);
				// p1 = max(p1, matVIT.at<float>(r2, c));
				// p1 = max(p1, matONL.at<float>(r2, c));
				p1 = max(p1, matRPE.at<float>(r2, c));
				p1 = max(p1, matCHO.at<float>(r2, c));
				esum += (p1 * log10(p1) + p2 * log10(p2));
				p1_max = max(p1_max, p1);
				p2_max = max(p2_max, p2);
			}
			esum *= -1.0f;

			p1_max = matNFL.at<float>(r, c);
			if (p1_max <= 0.01f) {
				esum = 0.01f;
			}

			if (yt1 >= 0 && yt2 >= 0) {
				if (r < yt1 || r > yt2) {
					esum = 0.01f;
				}
				else {
					esum = max(0.1f, esum);
				}
			}
			else {
				if (yb1 >= 0 && yb2 >= 0) {
					if (r < yb1 || r > yb2) {
						esum = 0.01f;
					}
					else {
						esum = max(0.01f, p1_max);
					}
				}
			}

			weight.at<float>(r, c) = esum;
			/*
			if (pSegm->getImageIndex() == 91 && c == 20) {
				LogD() << c << ", " << r << " : " << esum << " , " << matNFL.at<float>(r, c) << " , (" << p1_max << "), " << matOPL.at<float>(r, c) << " : " << (p1_max / (p2_max + 0.00001f));
			}
			*/
		}
	}
	pWeight->getCvMat() = weight;

	/*
	if (pSegm->getImageIndex() == 49) {
		c = 235;
		for (r = 36; r <= 128; r++) {
			LogD() << c << ", " << r << " : " << matNFL.at<float>(r, c) << " , " << matOPL.at<float>(r, c);
		}

	}
	*/
	return true;
}

bool SemtSegm::LayerNFL::designBoundaryConstraints()
{
	auto* pSegm = getSegmentator();
	auto* pClass = pSegm->getClassifier();
	auto* pImage = pSegm->getSampling()->source();

	const auto& inn_layer = pSegm->getLayerNFL()->getOptimalPath();
	const auto& out_layer = pSegm->getLayerIPL()->getOptimalPath();

	auto& uppers = getUppers();
	auto& lowers = getLowers();
	auto& deltas = getDeltas();

	const float UPPER_MARGIN = 0.0f;
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

bool SemtSegm::LayerNFL::createFlattenedCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getClassifier()->getInputDenoised();
	auto* pSample = pSegm->getSampling()->sample();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*0.0f);
	//int gray_max = (int)(pSample->getMean() * 1.0f + pSample->getStddev()*9.5f);

	//mask = srcMat > gray_max;
	//srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 9; //  7; // 5; // 9; // 7; // 5;
	const int KERNEL_COLS = 5; // 3; // 3; // 5; // 3;

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

	/*
	if (pSegm->getImageIndex() == 3)
	{
		int c = 5;
		for (int r = uppers[c]; r <= lowers[c]; r++) {
			LogD() << c << ", " << r << " : " << outMat.at<float>(r, c) << ", " << (int)srcMat.at<uchar>(r, c-1) << ", " << (int)srcMat.at<uchar>(r, c) << ", " << (int)srcMat.at<uchar>(r, c+1);
		}
	}
	*/

	outMat = outMat.mul(pWeight->getCvMatConst());

	/*
	if (pSegm->getImageIndex() == 3)
	{
		int c = 5;
		for (int r = uppers[c]; r <= lowers[c]; r++) {
			LogD() << c << ", " << r << " : " << outMat.at<float>(r, c);
		}
	}
	*/

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

			/*
			if (pSegm->getImageIndex() == 3) {
				if (c == 5) {
					LogD() << c << ", " << r << " : " << outMat.at<float>(r, c);
				}
			}
			*/
		}
	}

	getCostMap()->getCvMat() = outMat;
	return true;
}

bool SemtSegm::LayerNFL::createBoundaryCostMap()
{
	auto* pSegm = getSegmentator();
	auto* pImage = pSegm->getSampling()->source();
	auto* pWeight = getWeightMap();

	Mat srcMat, mask;
	pImage->getCvMat().copyTo(srcMat);

	int gray_min = (int)(pImage->getMean() * 1.0f + pImage->getStddev()*0.0f);
	//int gray_max = (int)(pImage->getMean() * 1.0f + pImage->getStddev()*2.0f);

	// mask = srcMat > gray_max;
	// srcMat.setTo(gray_max, mask);
	mask = srcMat < gray_min;
	srcMat.setTo(gray_min, mask);

	const int KERNEL_ROWS = 9; // 7;
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


bool SemtSegm::LayerNFL::smoothFlattenedPath()
{
	auto& path = getOptimalPath();
	auto* pImage = getSegmentator()->getClassifier()->getInputImage();
	const auto& inner = getSegmentator()->getLayerILM()->getOptimalPath();
	const auto& outer = getSegmentator()->getLayerIPL()->getOptimalPath();

	int rows = pImage->getHeight();

	const int FILTER_SIZE = 5; // 7; // 13;
	const int CURVE_DEGREE = 1;

	path = CppUtil::SgFilter::smoothInts(path, FILTER_SIZE, CURVE_DEGREE);

	std::transform(cbegin(path), cend(path), cbegin(inner), begin(path), [=](int elem1, int elem2) { return max(elem1, elem2); });
	std::transform(cbegin(path), cend(path), cbegin(outer), begin(path), [=](int elem1, int elem2) { return min(elem1, elem2); });
	transform(begin(path), end(path), begin(path), [=](int elem) { return min(max(elem, 0), rows - 1); });
	return true;
}

bool SemtSegm::LayerNFL::smoothBoundaryPath()
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

	const auto& inner = pSegm->getLayerILM()->getOptimalPath();

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


LayerNFL::LayerNFLImpl & SemtSegm::LayerNFL::getImpl(void) const
{
	return *d_ptr;
}
