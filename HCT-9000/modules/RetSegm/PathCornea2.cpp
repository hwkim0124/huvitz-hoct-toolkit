#include "stdafx.h"
#include "PathCornea2.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Coarse.h"

#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetSegm;
using namespace cv;

#include <iostream>
#include <iomanip>
#include <algorithm>
using namespace std;


PathCornea2::PathCornea2()
{
}


PathCornea2::~PathCornea2()
{
}


bool RetSegm::PathCornea2::designLayerConstraintsOfInner(const SegmImage * imgSrc, const SegmImage* imgGrad,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isMeye)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	Mat srcMat = imgSrc->getCvMat();
	Mat ascMat = imgGrad->getCvMat();

	const int UPPER_SPAN = 35;
	const int LOWER_SPAN = 25;
	int gmax = 0, gpos = 0;

	int x1 = (int)(width * 0.25f);
	int x2 = (int)(width * 0.75f);

	// Surface height by flat curve around cornea apex.
	for (int r = 0; r < height; r++) {
		int gsum = 0;
		for (int c = x1; c < x2; c++) {
			gsum += ascMat.at<uchar>(r, c);
		}
		if (gsum > gmax) {
			gmax = gsum;
			gpos = r;
		}
	}

	int bound1 = max(0, gpos - UPPER_SPAN);
	int bound2 = height - 1;

	upper = vector<int>(width, bound1);
	lower = vector<int>(width, bound2);
	delta = vector<int>(width, 5);

	if (isMeye) {
		/*
		for (int c = 0; c < width; c++) {
			for (int r = 0; r < height; r++) {
				float ratio = (float)(height - r) / height;
				srcMat.at<uchar>(r, c) = (unsigned char)(srcMat.at<uchar>(r, c) * ratio);
			}
			upper[c] = 0;
		}
		*/
		int thresh = max((int)(imgSrc->getMean() + imgSrc->getStddev() * 3.5f), 10);
		int cx1 = (int)(width * 0.45f);
		int cx2 = (int)(width * 0.55f);

		for (int c = 0; c < width; c++) {
			upper[c] = 0;
			lower[c] = height - 1;
			if (c < cx1 || c > cx2) {
				for (int r = 0; r < height; r++) {
					if (srcMat.at<uchar>(r, c) > thresh) {
						upper[c] = max(r - UPPER_SPAN, 0);
						lower[c] = min(r + LOWER_SPAN, height-1);
						break;
					}
				}
			}
		}
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfOuter(const SegmImage * imgSrc, const SegmImage* imgGrad, 
	SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isMeye)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerInn->getYs();
	if (inner.size() != width) {
		return false;
	}

	Mat srcMat = imgSrc->getCvMat();
	Mat desMat = imgGrad->getCvMat();
	int centX1 = (int)(width * 0.35f);
	int centX2 = (int)(width * 0.65f);

	const int INNER_SHIFT_MIN = 35;
	const int INNER_SHIFT_MAX = 150;

	// Shift surface line to the position of maximum correlation. 
	int gmax = 0, gpos = INNER_SHIFT_MIN;
	for (int r = INNER_SHIFT_MIN; r < INNER_SHIFT_MAX; r++) {
		int gsum = 0;
		for (int c = centX1; c <= centX2; c++) {
			int y = min(inner[c] + r, height-1);
			gsum += desMat.at<uchar>(y, c);
		}
		gsum /= (centX2 - centX1);
		if (gsum > gmax) {
			gmax = gsum;
			gpos = r;
		}
	}

	const int UPPER_SPAN = 35;
	const int LOWER_SPAN = 75;

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 3);

	// Model eye surface. 
	int span1, span2;
	if (gmax < 24 || isMeye) {
		span1 = 0; span2 = height; // LOWER_SPAN;

		for (int i = 0; i < width; i++) {
			upper[i] = inner[i] - span1;
			lower[i] = inner[i] + span2;
			upper[i] = max(min(upper[i], height - 1), 0);
			lower[i] = max(min(lower[i], height - 1), 0);
		}
	}
	else {
		float sizePerMM = (width / rangeX);
		int reflX1 = max((int)(centerX - 0.18f * sizePerMM), 0);
		int reflX2 = min((int)(centerX + 0.18f * sizePerMM), width - 1);

		int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());

		for (int i = 0; i < width; i++) {
			float xdist = (float)(abs(i - centerX) / sizePerMM);
			int shift = inner[i] - centerY;
			int y1 = max(0, min(inner[i], height-1));
			int y2 = max(0, min(inner[i] + gpos, height-1));
			int gsum = 0;
			int gcnt = 1;

			for (int j = y1; j <= y2; j++) {
				gsum += srcMat.at<uchar>(j, i);
				gcnt += 1;
			}

			float gavg = (float) gsum / gcnt;
			float ratio = min(gavg / thresh, 1.0f);
			float bias = xdist * xdist;
			int span1 = (int)(gpos - UPPER_SPAN * ratio + bias);
			int span2 = gpos + LOWER_SPAN;

			if (i >= reflX1 && i <= reflX2) {
				span2 = gpos + LOWER_SPAN / 4;
			}

			upper[i] = inner[i] + span1;
			lower[i] = inner[i] + span2;
			upper[i] = max(min(upper[i], height - 1), 0);
			lower[i] = max(min(lower[i], height - 1), 0);
		}
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfInnerEnds(const SegmImage * imgSrc, SegmLayer * layerInn, 
	int centerX, int centerY, float rangeX, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerInn->getYs();
	int size = (int)inner.size();
	if (size != width) {
		return false;
	}

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 5);

	const int UPPER_LIMIT = 15;
	const int LOWER_LIMIT = 120;
	float sizePerMM = (size / rangeX);

	int span1 = 0;
	int span2 = 0;
	float xdist = 0.0f;

	for (int i = 0; i < width; i++) {
		xdist = (float)(abs(i - centerX) / sizePerMM);
		span1 = max((int)(UPPER_LIMIT + xdist * xdist), 9);
		span2 = LOWER_LIMIT; // min((int)(xdist * 10.0f + 3), LOWER_LIMIT);
		upper[i] = inner[i] - span1;
		lower[i] = inner[i] + span2;
		upper[i] = max(min(upper[i], height - 1), 0);
		lower[i] = max(min(lower[i], height - 1), 0);
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfOuterEnds(const SegmImage * imgSrc, 
	SegmLayer* layerInn, SegmLayer * layerOut,
	int centerX, int centerY, float rangeX,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();
	if (inner.size() != width || outer.size() != width) {
		return false;
	}

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 3);

	const int UPPER_SPAN = 5;
	const int LOWER_SPAN = 15;

	float sizePerMM = (width / rangeX);
	int reflX1 = max((int)(centerX - 0.18f * sizePerMM), 0);
	int reflX2 = min((int)(centerX + 0.18f * sizePerMM), width - 1);

	int span1 = 0;
	int span2 = 0;
	float xdist = 0.0f;

	for (int i = 0; i < width; i++) {
		xdist = (float)(abs(i - centerX) / sizePerMM);
		span1 = UPPER_SPAN;
		span2 = (int)(xdist * xdist * 2.0f + LOWER_SPAN);

		if (i >= reflX1 && i <= reflX2) {
			span2 = UPPER_SPAN;
		}

		upper[i] = outer[i] - span1;
		lower[i] = outer[i] + span2;
		upper[i] = max(upper[i], inner[i]);
		lower[i] = max(lower[i], inner[i]);
		upper[i] = max(min(upper[i], height - 1), 0);
		lower[i] = max(min(lower[i], height - 1), 0);
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfEPI(const SegmImage * imgSrc, SegmLayer * layerInn, 
	int centerX, int centerY, float rangeX,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerInn->getYs();
	if (inner.size() != width) {
		return false;
	}

	const int UPPER_LIMIT = 25;
	const int LOWER_LIMIT = 160;
	float sizePerMM = (width / rangeX);

	int span1 = 0;
	int span2 = 0;
	float xdist = 0.0f;

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 2);

	for (int i = 0; i < width; i++) {
		xdist = (float)(abs(i - centerX) / sizePerMM);
		span1 = max((int)(UPPER_LIMIT - xdist * xdist * 2.0f), 35);
		span2 = min((int)(xdist * xdist * 9.0f + 15.0f), LOWER_LIMIT);
		upper[i] = inner[i] - span1;
		lower[i] = inner[i] + span2;
		upper[i] = max(min(upper[i], height - 1), 0);
		lower[i] = max(min(lower[i], height - 1), 0);
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfEND(const SegmImage * imgSrc, SegmLayer * layerEPI, SegmLayer * layerOut, 
	int centerX, int centerY, float rangeX,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerEPI->getYs();
	auto outer = layerOut->getYs();
	if (inner.size() != width || outer.size() != width) {
		return false;
	}

	const int UPPER_SPAN = 15;
	const int LOWER_SPAN = 25;
	float sizePerMM = (width / rangeX);

	int span1 = 0;
	int span2 = 0;
	float xdist = 0.0f;

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 2);

	for (int i = 0; i < width; i++) {
		xdist = (float)(abs(i - centerX) / sizePerMM);
		span1 = UPPER_SPAN;
		span2 = (int)(xdist * xdist * 4.0f + LOWER_SPAN);

		upper[i] = outer[i] - span1;
		lower[i] = outer[i] + span2;
		upper[i] = max(upper[i], inner[i]);
		lower[i] = max(lower[i], inner[i]);
		upper[i] = max(min(upper[i], height - 1), 0);
		lower[i] = max(min(lower[i], height - 1), 0);
	}
	return true;
}


bool RetSegm::PathCornea2::designLayerConstraintsOfBOW(const SegmImage * imgSrc, SegmLayer * layerEPI, SegmLayer * layerEND, 
	int centerX, int centerY, float rangeX, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (width <= 0 || height <= 0) {
		return false;
	}

	auto inner = layerEPI->getYs();
	auto outer = layerEND->getYs();
	if (inner.size() != width || outer.size() != width) {
		return false;
	}

	const int UPPER_SPAN = 10; // 5;
	const int LOWER_SPAN = 25;
	float sizePerMM = (width / rangeX);

	int span1 = 0;
	int span2 = 0;
	float xdist = 0.0f;

	upper = vector<int>(width, 0);
	lower = vector<int>(width, height - 1);
	delta = vector<int>(width, 2);

	for (int i = 0; i < width; i++) {
		xdist = (float)(abs(i - centerX) / sizePerMM);
		span1 = (int)(UPPER_SPAN + xdist * xdist * 0.25f); 
		span2 = (int)(LOWER_SPAN + xdist * xdist * 0.25f);
		upper[i] = inner[i] + span1;
		lower[i] = inner[i] + span2;
		upper[i] = max(min(upper[i], outer[i]), inner[i]);
		lower[i] = max(min(lower[i], outer[i]), inner[i]);
		upper[i] = max(min(upper[i], height - 1), 0);
		lower[i] = max(min(lower[i], height - 1), 0);
		delta[i] = min(max((int)(xdist), 1), 2);
	}
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfInner(const SegmImage * imgSrc, SegmImage * imgCost, 
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{

	Mat srcMat, outMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	srcMat.convertTo(outMat, CV_32F);
	outMat *= (-1.0f / 255.0f);

	/*
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*2.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	const int KERNEL_ROWS = 9;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	
	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfOuter(const SegmImage * imgSrc, SegmImage * imgCost,
	std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, outMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	srcMat.convertTo(outMat, CV_32F);
	outMat *= (-1.0f / 255.0f);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfInnerEnds(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, outMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	srcMat.convertTo(outMat, CV_32F);
	outMat *= (-1.0f / 255.0f);

	/*
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*1.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	const int KERNEL_ROWS = 5;
	const int KERNEL_COLS = 3;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);
	*/

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfOuterEnds(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	/*
	Mat srcMat, outMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	srcMat.convertTo(outMat, CV_32F);
	outMat *= (-1.0f / 255.0f);
	*/

	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean()*1.0f + imgSrc->getStddev()*0.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	const int KERNEL_ROWS = 7;
	const int KERNEL_COLS = 3;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	cv::filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*2.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	const int KERNEL_ROWS = 9;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	cv::filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*1.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	const int KERNEL_ROWS = 7;
	const int KERNEL_COLS = 5;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}


	Mat outMat;
	cv::filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}


bool RetSegm::PathCornea2::createLayerCostMapOfBOW(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);


	int grayMin = (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*2.0f);

	mask = srcMat < grayMin;
	srcMat.setTo(grayMin, mask);

	const int KERNEL_ROWS = 9; // 5;
	const int KERNEL_COLS = 3;

	Mat kernel = Mat::zeros(KERNEL_ROWS, KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = +1;
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, 0) = -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (2 * kernel.cols) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat outMat;
	cv::filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_CONSTANT);

	// Update from the second column with the cost accumulated along the path
	// taking minimum cost within allowed vertical distance from the nearest left column. 
	float minCost;

	for (int c = 1; c < outMat.cols; c++) {
		//string line;
		for (int r = upper[c]; r <= lower[c]; r++) {
			minCost = PATH_COST_MAX;
			for (int k = r - delta[c - 1]; k <= r + delta[c - 1]; k++) {
				if (k >= upper[c - 1] && k <= lower[c - 1]) {
					if (outMat.at<float>(k, c - 1) < minCost) {
						minCost = outMat.at<float>(k, c - 1);
					}
				}
			}
			outMat.at<float>(r, c) += minCost;
		}
	}

	imgCost->getCvMat() = outMat;
	return true;
}
