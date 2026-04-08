#include "stdafx.h"
#include "PathCornea.h"
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


PathCornea::PathCornea()
{
}


PathCornea::~PathCornea()
{
}


bool RetSegm::PathCornea::designLayerContraintsOfEPI(const SegmImage * imgSrc, SegmLayer * layerInn, float rangeX, int hingeY, int centerX, int centerY,
													std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerInn->getYs();
	Mat srcMat = imgSrc->getCvMatConst();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	/*
	int cent_x1, cent_x2;
	makeCenterRegionOfEPI(inner, cent_x1, cent_x2, rangeX);
	auto aroundCenter = [=](auto x) { return (x >= cent_x1 && x <= cent_x2); };
	*/

	float sizePerMM = (float)(size / rangeX);
	int cornea_x1 = max((int)(centerX - sizePerMM * 5.0f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 5.0f), size - 1);
	int offset_y1 = (int)(sizePerMM * +1.0f);
	int offset_y2 = (int)(sizePerMM * +2.0f);

	int reflect_x1 = max((int)(centerX - sizePerMM * 0.35f), 0);
	int reflect_x2 = min((int)(centerX + sizePerMM * 0.35f), size - 1);
	int edge_thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());
	int band1, band2, knot;
	int y1, y2, gsum, gcnt, gwgh;
	float dist;
	bool found;

	for (int i = 0; i < size; i++) {
		dist = (float)(fabs(i - centerX) / sizePerMM);

		if (i >= reflect_x1 && i <= reflect_x2) {
			y1 = max(inner[i] - 5, 0);
			y2 = min(inner[i] + 9, height - 1);
			gsum = 0;
			gcnt = 0;
			gwgh = 0;
			for (int r = y1; r <= y2; r++) {
				gwgh = srcMat.at<uchar>(r, i);
				gcnt += gwgh;
				gsum += (gwgh * r);
			}

			if (gcnt > 0) {
				knot = (gsum / gcnt);
				band1 = +0;
				band2 = +9;
			}
			else {
				knot = inner[i];
				band1 = +3;
				band2 = +9;
			}

			upper[i] = max(knot - band1, 0);
			lower[i] = min(knot + band2, height - 1);
			delta[i] = 1;
		}
		else if (i >= cornea_x1 && i <= cornea_x2) {
			band1 = (int)(0.15f * dist * dist + 5.0f);
			band2 = (int)(0.25f * dist * dist + 9.0f);

			y1 = max((int)(inner[i] - band1), 0);
			y2 = min((int)(inner[i] + band2), height-1);
			found = false;

			for (int r = y1, c = i; r <= y2; r++) {
				if (srcMat.at<uchar>(r, c) > edge_thresh) {
					found = true;
					break;
				}
			}

			if (!found) {
				band1 = +5;
				band2 = +5;
			}

			upper[i] = max(inner[i] - band1, 0);
			lower[i] = min(inner[i] + band2, height - 1);
			delta[i] = (int)(dist / 2.0f + 1.0f);
		}
		else {
			band1 = (int)(0.25f * dist * dist + 5.0f);
			band2 = (int)(0.45f * dist * dist + 9.0f);

			y1 = max((int)(inner[i] - band1), 0);
			y2 = min((int)(inner[i] + band2), height - 1);
			found = false;

			for (int r = y1, c = i; r <= y2; r++) {
				if (srcMat.at<uchar>(r, c) > edge_thresh) {
					found = true;
					break;
				}
			}

			if (!found) {
				band1 = +7;
				band2 = +7;
			}

			upper[i] = max(inner[i] - band1, 0);
			lower[i] = min(inner[i] + band2, height - 1);
			delta[i] = (int)(dist / 2.0f + 1.0f);
		}
	}
	return true;
}


bool RetSegm::PathCornea::designLayerContraintsOfEND(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut, 
													float rangeX, int hingeY, int centerX, int centerY, int times,
													std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();
	Mat srcMat = imgSrc->getCvMatConst();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	float sizePerMM = (float)(size / rangeX);
	int cornea_x1 = max((int)(centerX - sizePerMM * 5.0f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 5.0f), size - 1);
	int offset_y1 = (int)(sizePerMM * +1.0f);
	int offset_y2 = (int)(sizePerMM * +2.0f);

	int reflect_x1 = max((int)(centerX - sizePerMM * 0.35f), 0);
	int reflect_x2 = min((int)(centerX + sizePerMM * 0.35f), size - 1);
	int edge_thresh = (int)(imgSrc->getMean() + imgSrc->getStddev());
	int band1, band2;

	if (rangeX < 11.0f) {
		for (int i = 0; i < size; i++) {
			float dist = (float)(fabs(i - centerX) / sizePerMM);

			band1 = (int)(0.5f * dist * dist + 13.0f);
			band2 = (int)(2.0f * dist * dist + 27.0f);

			/*
			int y1 = max((int)(outer[i] - band1), 0);
			int y2 = min((int)(outer[i] + band2), height - 1);
			bool found = false;

			for (int r = y1; r <= y2; r++) {
				if (srcMat.at<uchar>(r, i) > edge_thresh) {
					found = true;
					break;
				}
			}
				
			if (!found) {
				band1 = (int)(0.15f * dist * dist + 5.0f);
				band2 = (int)(0.15f * dist * dist + 9.0f);
			}
			*/

			upper[i] = max(outer[i] - band1, 0);
			lower[i] = min(outer[i] + band2, height - 1);
			delta[i] = (int)(dist / 2.0f + 1.0f);
		}
	}
	else {
		for (int i = 0; i < size; i++) {
			float dist = (float)(fabs(i - centerX) / sizePerMM);
			if (i >= reflect_x1 && i <= reflect_x2) {
				int band1 = max(outer[i] - 13, 0);
				int band2 = min(outer[i] + 13, height - 1);

				upper[i] = max(max(band1, inner[i]), 0);
				lower[i] = min(band2, height - 1);
				delta[i] = 1;
			}
			else if (i >= cornea_x1 && i <= cornea_x2) {
				int band1 = max(outer[i] - 15, 0);
				int band2 = (int)(outer[i] + (dist * dist + 19.0f));

				upper[i] = max(max(band1, inner[i]), 0);
				lower[i] = min(band2, height - 1);
				delta[i] = (int)(dist / 2.0f + 1.0f);
			}
			else {
				int band1 = max(outer[i] - 15, 0);
				int band2 = (int)(outer[i] + (2.0f * dist * dist + 27.0f));

				upper[i] = max(max(band1, inner[i]), 0);
				lower[i] = min(band2, height - 1);
				delta[i] = (int)(dist / 2.0f + 1.0f);
			}
		}
	}
	return true;
}


bool RetSegm::PathCornea::designLayerContraintsOfBOW(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut,
										float rangeX, int hingeY, int centerX, int centerY,
										std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	int size = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto inner = layerInn->getYs();
	auto outer = layerOut->getYs();

	upper = vector<int>(size, -1);
	lower = vector<int>(size, -1);
	delta = vector<int>(size, -1);

	float sizePerMM = (float)(size / rangeX);
	int cornea_x1 = max((int)(centerX - sizePerMM * 5.0f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 5.0f), size - 1);
	int offset_y1 = (int)(sizePerMM * +1.0f);
	int offset_y2 = (int)(sizePerMM * +2.0f);

	int reflect_x1 = max((int)(centerX - sizePerMM * 0.35f), 0);
	int reflect_x2 = min((int)(centerX + sizePerMM * 0.35f), size - 1);

	if (rangeX < 11.0f) {
		for (int i = 0; i < size; i++) {
			float dist = (float)(fabs(i - centerX) / sizePerMM);
			if (i >= reflect_x1 && i <= reflect_x2) {
				int band1 = max(inner[i] + 9, 0);
				int band2 = min(inner[i] + 30, height - 1);

				upper[i] = min(min(band1, outer[i]), height - 1);
				lower[i] = min(min(band2, outer[i]), height - 1);
				delta[i] = 1;
			}
			else {
				int band1 = max(inner[i] + 9, 0);
				int band2 = (int)(inner[i] + (0.25f * dist * dist + 30.0f));

				upper[i] = min(min(band1, outer[i]), height - 1);
				lower[i] = min(min(band2, outer[i]), height - 1);
				delta[i] = (int)(dist / 2.0f + 1.0f);
			}
		}
	}
	else {
		for (int i = 0; i < size; i++) {
			float dist = (float)(fabs(i - centerX) / sizePerMM);
			if (i >= reflect_x1 && i <= reflect_x2) {
				int band1 = max(inner[i] + 3, 0);
				int band2 = min(inner[i] + 21, height - 1);

				upper[i] = min(min(band1, outer[i]), height - 1);
				lower[i] = min(min(band2, outer[i]), height - 1);
				delta[i] = 1;
			}
			else if (i >= cornea_x1 && i <= cornea_x2) {
				int band1 = max(inner[i] + 4, 0);
				int band2 = (int)(inner[i] + (0.25f * dist * dist + 21.0f));

				upper[i] = min(min(band1, outer[i]), height - 1);
				lower[i] = min(min(band2, outer[i]), height - 1);
				delta[i] = (int)(dist / 2.0f + 1.0f);
			}
			else {
				int band1 = max(inner[i] + 5, 0);
				int band2 = (int)(inner[i] + (0.25f * dist * dist + 21.0f));

				upper[i] = min(min(band1, outer[i]), height - 1);
				lower[i] = min(min(band2, outer[i]), height - 1);
				delta[i] = (int)(dist / 2.0f + 1.0f);
			}
		}
	}
	return true;
}


bool RetSegm::PathCornea::createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask ;
	imgSrc->getCvMatConst().copyTo(srcMat);

	/*
	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_EPI_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	*/

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_EPI_KERNEL_ROWS, LAYER_EPI_KERNEL_COLS, CV_32F);
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


bool RetSegm::PathCornea::createLayerCostMapOfEPI2(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_EPI_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_EPI_KERNEL_ROWS2, LAYER_EPI_KERNEL_COLS2, CV_32F);
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
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

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


bool RetSegm::PathCornea::createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	/*
	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_EPI_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	*/

	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_END_KERNEL_ROWS, LAYER_END_KERNEL_COLS, CV_32F);
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
	filter2D(srcMat, outMat, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

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


bool RetSegm::PathCornea::createLayerCostMapOfBOW(const SegmImage * imgSrc, SegmImage * imgCost, std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta)
{
	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*LAYER_EPI_STDDEV_TO_GRAY_MAX);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);
	
	// Narrow kernel with vertical strip causes ILM hovering over the true surface. 
	Mat kernel = Mat::zeros(LAYER_BOW_KERNEL_ROWS, LAYER_BOW_KERNEL_COLS, CV_32F);
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


bool RetSegm::PathCornea::makeFittingCurveOfEPI(std::vector<int>& input, std::vector<int>& output)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = (input[i] < 0 ? 0 : input[i]);
	}

	int diff;
	vector<double> coeffs;
	// for (int i = 0, outliers = 0; i < COARSE_OUTLIERS_RETRY_MAX_CORNEA; i++) {
	for (int i = 0, outliers = 0; i < 1; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				diff = abs(fittY[i] - dataY[i]);
				if (diff > COARSE_INNER_OUTLIERS_OFFSET_CORNEA) {
					//dataY[i] = -1;
					//outliers++;
				}
			}

			if (outliers > 0) {
				outliers = 0;
				// interpolateBoundaryByLinearFitting(dataY, dataY);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	// input = dataY;
	output = fittY;

	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	return true;
}


bool RetSegm::PathCornea::makeFittingCurveOfBOW(std::vector<int>& input, std::vector<int>& output, int& centX, int& centY)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	auto exceptStart = max((int)(centX - size * LAYER_BOW_TOP_RANGE_HALF), 0);
	auto exceptClose = min((int)(centX + size * LAYER_BOW_TOP_RANGE_HALF), (int)(size-1));
	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (exceptStart <= i && i <= exceptClose) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (input[i] < 0 ? 0 : input[i]);
		}
	}

	int diff;
	vector<double> coeffs;
	// for (int i = 0, outliers = 0; i < COARSE_OUTLIERS_RETRY_MAX_CORNEA; i++) {
	for (int i = 0, outliers = 0; i < 1; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				diff = abs(fittY[i] - dataY[i]);
				if (diff > COARSE_INNER_OUTLIERS_OFFSET_CORNEA) {
					//dataY[i] = -1;
					//outliers++;
				}
			}

			if (outliers > 0) {
				outliers = 0;
				// interpolateBoundaryByLinearFitting(dataY, dataY);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	// input = dataY;
	output = fittY;

	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	return true;
}


bool RetSegm::PathCornea::makeFittingCurveOfEND(std::vector<int>& input, std::vector<int>& output)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	auto start = (int)(size * 0.15f);
	auto close = (int)(size * 0.85f);
	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i < start || i > close) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (input[i] < 0 ? 0 : input[i]);
		}
	}

	int diff;
	vector<double> coeffs;
	// for (int i = 0, outliers = 0; i < COARSE_OUTLIERS_RETRY_MAX_CORNEA; i++) {
	for (int i = 0, outliers = 0; i < 1; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				diff = abs(fittY[i] - dataY[i]);
				if (diff > COARSE_INNER_OUTLIERS_OFFSET_CORNEA) {
					//dataY[i] = -1;
					//outliers++;
				}
			}

			if (outliers > 0) {
				outliers = 0;
				// interpolateBoundaryByLinearFitting(dataY, dataY);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	// input = dataY;
	output = fittY;

	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	return true;
}


bool RetSegm::PathCornea::correctReflectiveRegionOfEPI(const SegmImage * imgSrc, std::vector<int>& input, float rangeX, int hingeY, int centerX, int centerY)
{
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	auto size = (int)input.size();
	float sizePerMM = (float)(size / rangeX);
	int cornea_x1 = max((int)(centerX - sizePerMM * 1.8f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 1.8f), size - 1);

	int reflect_x1 = max((int)(centerX - sizePerMM * 0.35f), 0);
	int reflect_x2 = min((int)(centerX + sizePerMM * 0.35f), size - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cornea_x1 && i <= cornea_x2) {
				if (i < reflect_x1 || i > reflect_x2) {
					dataY[i] = input[i];
				}
				else {
					float dist = (float)(fabs(i - centerX) / sizePerMM);
					int y1 = max(input[i] - 3, 0);
					int y2 = min(input[i] + 9, height - 1);
					int gsum = 0;
					int gcnt = 0;
					int gwgh = 0;
					for (int r = y1; r <= y2; r++) {
						gwgh = srcMat.at<uchar>(r, i);
						gcnt += gwgh;
						gsum += (gwgh * r);
					}

					if (gcnt > 0) {
						dataY[i] = gsum / gcnt;
					}
				}
			}
			else {
				dataY[i] = 0;
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= reflect_x1 && i <= reflect_x2) {
					input[i] = fittY[i];
				}
			}
		}
		else {
			return false;
		}
	}
	return true;
}


void RetSegm::PathCornea::makeCenterRegionOfEPI(std::vector<int>& input, int & centX1, int & centX2, float rangeX)
{
	auto size = (int) input.size();
	int sizePerMM = (int)(size / rangeX);

	auto m_iter = min_element(cbegin(input), cend(input));
	auto cent_x = distance(cbegin(input), m_iter);
	auto cent_s = max((int)(cent_x - sizePerMM * 0.25f), 0);
	auto cent_e = min((int)(cent_x + sizePerMM * 0.25f), size - 1);

	centX1 = cent_s;
	centX2 = cent_e;
	return;
}


bool RetSegm::PathCornea::replaceCenterRegionOfEPI(std::vector<int>& input, std::vector<int>& output)
{
	vector<int> fitt;
	if (!makeFittingCurveOfEPI(input, fitt)) {
		return false;
	}

	int cent_s, cent_e;
	makeCenterRegionOfEPI(fitt, cent_s, cent_e);

	auto iter_s = fitt.begin() + cent_s;
	auto iter_e = fitt.begin() + cent_e;

	auto result = input;
	std::copy(iter_s, iter_e, result.begin() + cent_s);
	output = result;

	for (auto& elem : output) {
		elem -= 3;
		elem = max(elem, 0);
	}
	return true;
}
