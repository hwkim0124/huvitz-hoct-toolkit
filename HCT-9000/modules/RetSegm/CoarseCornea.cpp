#include "stdafx.h"
#include "CoarseCornea.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "DataFitt.h"
#include "Feature.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Logger.h"

#include "CppUtil2.h"

#include <algorithm>

using namespace CppUtil;
using namespace RetSegm;
using namespace cv;
using namespace std;

CoarseCornea::CoarseCornea()
{
}


CoarseCornea::~CoarseCornea()
{
}


bool RetSegm::CoarseCornea::createGradientMapOfCornea(const SegmImage * imgSrc, SegmImage * imgAsc, SegmImage * imgDes, SegmImage * imgOut)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS_CORNEA, COARSE_GRADIENT_KERNEL_COLS_CORNEA, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	/*
	if (kernel.rows % 2) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(kernel.rows / 2, c) = 0.0f;
		}
	}
	*/

	Mat conv;
	Mat mask;
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Leave only positive gradients, and normalize in range between [0, 255].
	mask = conv < 0;
	Mat masc;
	Mat ascMat;

	conv.copyTo(masc);
	masc.setTo(0.0f, mask);
	cv::normalize(masc, ascMat, 0, 255, NORM_MINMAX, CV_8U);
	imgAsc->getCvMat() = ascMat;

	if (imgDes != nullptr) {
		kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS_CORNEA2, COARSE_GRADIENT_KERNEL_COLS_CORNEA2, CV_32F);
		for (int r = 0; r < kernel.rows; r++) {
			for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
				kernel.at<float>(r, c) *= 2.0f;
			}
		}
		for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(r, c) *= -1.0f;
			}
		}
		/*
		if (kernel.rows % 2) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(kernel.rows / 2, c) = 0.0f;
			}
		}
		*/

		/*
		for (int c = 0; c < kernel.cols; c++) {
			for (int r = kernel.rows/3; r < (kernel.rows*2)/3; r++) {
				kernel.at<float>(r, c) *= 2.0f;
			}
		}

		for (int c = 0; c < kernel.cols / 2; c++) {
			for (int r = 0; r < kernel.rows; r++) {
				kernel.at<float>(r, c) *= -1.0f;
			}
		}
		*/

		filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

		// Reverse negative gradients, and normalize in range between [0, 255].
		mask = conv < 0;
		Mat mdes;
		Mat desMat;

		conv.copyTo(mdes);
		mdes.setTo(0.0f, mask);
		// mdes = mdes.mul(Scalar(-1.0f));
		cv::normalize(mdes, desMat, 0, 255, NORM_MINMAX, CV_8U);
		imgDes->getCvMat() = desMat;
	}

	// Gradient map to display.
	if (imgOut != nullptr) {
		imgOut->getCvMat() = ascMat; // desMat;
	}
	return true;
}


bool RetSegm::CoarseCornea::createGradientMapOfChamber(const SegmImage * imgSrc, SegmImage * imgAsc, SegmImage * imgDes, SegmImage * imgOut)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS_CORNEA, COARSE_GRADIENT_KERNEL_COLS_CORNEA, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

	Mat conv;
	Mat mask;
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Leave only positive gradients, and normalize in range between [0, 255].
	mask = conv < 0;
	Mat masc;
	Mat ascMat;

	conv.copyTo(masc);
	masc.setTo(0.0f, mask);
	cv::normalize(masc, ascMat, 0, 255, NORM_MINMAX, CV_8U);
	imgAsc->getCvMat() = ascMat;

	kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS_CORNEA2, COARSE_GRADIENT_KERNEL_COLS_CORNEA2, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Reverse negative gradients, and normalize in range between [0, 255].
	mask = conv < 0;
	Mat mdes;
	Mat desMat;

	conv.copyTo(mdes);
	mdes.setTo(0.0f, mask);
	// mdes = mdes.mul(Scalar(-1.0f));
	cv::normalize(mdes, desMat, 0, 255, NORM_MINMAX, CV_8U);
	cv::flip(desMat, desMat, 0);
	imgDes->getCvMat() = desMat;

	// Gradient map to display.
	if (imgOut != nullptr) {
		imgOut->getCvMat() = ascMat; // desMat;
	}
	return true;
}


bool RetSegm::CoarseCornea::makeupCorneaBorderLines(const SegmImage* srcImg, const SegmImage * ascImg, const SegmImage * desImg, SegmLayer * layerInn, 
													SegmLayer * layerOut, float rangeX, int & corneaTopX, int & corneaTopY, int& wideHingeY, 
													bool meye)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(ascImg->getWidth(), -1);
	outer.resize(ascImg->getWidth(), -1);
	fitts.resize(ascImg->getWidth(), -1);

	int centerX = -1, centerY = -1;
	int hingeY = -1;
	int reflectX1 = -1;
	int reflectX2 = -1;

	if (!CoarseCornea::locateCenterReflection(srcImg, rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!locateCenterEdgesOfCornea(srcImg, ascImg, inner, outer, rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!removeFractionsInInnerEdgesLine(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!removeOutliersInCenterEdges(ascImg, inner, outer, rangeX)) {
		return false;
	}

	if (!composeInnerCurveByCenterEdges(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!detectHingesInWideCornea(ascImg, inner, rangeX, centerX, centerY, hingeY)) {
		return false;
	}

	if (!locateInnerBorderLine(srcImg, inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
		return false;
	}

	if (!removeFractionsInInnerBorderLine(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!removeOutliersInInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!interpolateInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!estimateOuterCurveByInnerCurve(desImg, inner, outer, rangeX, centerX, centerY, meye)) {
		return false;
	}

	if (!locateOuterBorderLine(desImg, inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
		return false;
	}

	if (!removeFractionsInInnerBorderLine(outer, rangeX, centerX, centerY)) {
		return false;
	}

	if (!removeOutliersInInnerBorderLine(outer, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!interpolateOuterBorderLine(inner, outer, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	corneaTopX = centerX;
	corneaTopY = centerY;
	wideHingeY = hingeY;

	layerInn->initialize(inner, ascImg->getWidth(), ascImg->getHeight());
	layerOut->initialize(outer, ascImg->getWidth(), ascImg->getHeight());
	return true;
}


bool RetSegm::CoarseCornea::locateCenterReflection(const SegmImage * srcImg, float rangeX, int & reflect_x1, int & reflect_x2)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;
	
	int sizePerMM = (int)(img_w / rangeX);

	bool isWideRange = rangeX >= 11.0f;

	int threshold = max((int)(srcImg->getMean() * 2.5f), 45);
	auto avgVals = srcImg->getColumMeans();
	int maxVal = 0;
	int maxPos = 0;

	for (int i = 0; i < img_w; i++) {
		if (avgVals[i] > maxVal) {
			maxVal = avgVals[i];
			maxPos = i;
		}
	}

	if (maxVal > threshold) {
		reflect_x1 = max((int)(maxPos - sizePerMM*0.25f), 0);
		reflect_x2 = min((int)(maxPos + sizePerMM*0.25f), img_w - 1);
		// LogD() << "Cornea center reflection: " << reflect_x1 << ", " << reflect_x2;
	}
	else {
		reflect_x1 = -1;
		reflect_x2 = -1;
	}

	return true;
}


bool RetSegm::CoarseCornea::locateCenterEdgesOfCornea(const SegmImage * srcImg, const SegmImage* ascImg, 
												std::vector<int>& inner, std::vector<int>& outer, 
												float rangeX, int reflect_x1, int reflect_x2)
{
	Mat srcMat = srcImg->getCvMat();
	Mat ascMat = ascImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);

	auto maxLocs = ascImg->getColumMaxLocs();
	auto maxVals = ascImg->getColumMaxVals();

	int edge_thresh; 
	bool isWideRange = rangeX >= 11.0f;

	if (isWideRange) {
		edge_thresh = (int)(ascImg->getMean() * 1.0f + ascImg->getStddev() * 1.0f);
	}
	else {
		edge_thresh = (int)(ascImg->getMean() * 2.0f + ascImg->getStddev() * 1.0f);
	}

	// Central region occupying about 0.6 width of image. 
	float halfSize = 1.8f * (rangeX / 6.0f);
	halfSize = (isWideRange ? halfSize : max(halfSize, 3.0f));
	float minSize = halfSize * 0.5f;

	int sizePerMM = (int)(img_w / rangeX);
	int covered = (int)(halfSize * sizePerMM);
	int cent_x1 = max((int)(img_w / 2 - covered), 0);
	int cent_x2 = min((int)(img_w / 2 + covered), img_w-1);
	int count = 0;

	// Gather gradient peaks around center region. 
	for (int c = cent_x1; c < cent_x2; c++) {
		int max_loc = maxLocs[c];
		int max_val = maxVals[c];
		if (max_val < edge_thresh) {
			continue;
		}

		if (c >= reflect_x1 && c <= reflect_x2) {
			continue;
		}

		for (int r = 0; r < img_h; r++) {
			int grad = ascMat.at<uchar>(r, c);
			if (grad >= edge_thresh) {
				if (r > 0) {
					inner[c] = r;
					count++;
				}
				break;
			}
		}
	}

	if (count <= (sizePerMM * minSize)) {
		return false;
	}

	// This assumes that the points closest to the reflection are the highest. 
	if (reflect_x1 >= 0 && reflect_x2 >= 0) {
		int top_y1 = -1;
		for (int c = reflect_x1; c >= cent_x1; c--) {
			if (inner[c] > 0) {
				top_y1 = inner[c];
				break;
			}
		}
		if (top_y1 >= 0) {
			for (int c = reflect_x1; c >= cent_x1; c--) {
				if (inner[c] < top_y1) {
					inner[c] = -1;
				}
			}
		}
		int top_y2 = -1;
		for (int c = cent_x2; c <= reflect_x2; c++) {
			if (inner[c] > 0) {
				top_y2 = inner[c];
				break;
			}
		}
		if (top_y2 >= 0) {
			for (int c = cent_x2; c <= reflect_x2; c++) {
				if (inner[c] < top_y2) {
					inner[c] = -1;
				}
			}
		}
	}

	auto x_peaks = vector<int>();
	auto y_peaks = vector<int>();
	for (int c = cent_x1; c <= cent_x2; c++) {
		if (inner[c] > 0) {
			x_peaks.push_back(c);
			y_peaks.push_back(inner[c]);
		}
	}

	// Only edge points which located within the range are valid. 
	std::sort(x_peaks.begin(), x_peaks.end());
	std::sort(y_peaks.begin(), y_peaks.end());
	int cutoff = (int)(x_peaks.size() * 0.35f); // 0.25f); // 0.35f);
	int cutt_x = x_peaks[cutoff];
	int cutt_y = y_peaks[cutoff];
	int band_y1 = max(cutt_y - 24, 0);
	int band_y2 = cutt_y + (int)(sizePerMM * (rangeX / 6.0f) * 2.5f);

	// Remove peripheral outliers and center pixels around optical pivot. 
	for (int c = cent_x1; c < cent_x2; c++) {
		if (inner[c] < band_y1) {
			inner[c] = -1;
		}
		else if (inner[c] > band_y2) {
		 	inner[c] = -1;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::locateCenterEdgesOfCorneaReversed(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX, int & centX, int & centY)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);

	auto maxLocs = srcImg->getColumMaxLocs();
	auto maxVals = srcImg->getColumMaxVals();

	int edge_thresh;
	if (rangeX < 11.0f) {
		edge_thresh = (int)(srcImg->getMean() * 2.0f + srcImg->getStddev() * 1.0f);
	}
	else {
		edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 0.5f);
	}

	int sizePerMM = (int)(img_w / rangeX);
	int covered = (int)(1.8f * (rangeX / 16.0f) * sizePerMM);
	int cent_x1 = (int)(img_w / 2 - covered);
	int cent_x2 = (int)(img_w / 2 + covered);

	auto x_peaks = vector<int>();
	auto y_peaks = vector<int>();

	// Gather gradient peaks around center region. 
	for (int c = cent_x1; c < cent_x2; c++) {
		int max_loc = maxLocs[c];
		int max_val = maxVals[c];
		if (max_val < edge_thresh) {
			continue;
		}

		for (int r = 0; r < img_h; r++) {
			int grad = srcMat.at<uchar>(r, c);
			if (grad >= edge_thresh) {
				x_peaks.push_back(c);
				y_peaks.push_back(r);
				inner[c] = r;
				break;
			}
		}
	}

	if (x_peaks.size() < (sizePerMM * 1.0f)) {
		return false;
	}

	std::sort(x_peaks.begin(), x_peaks.end());
	std::sort(y_peaks.begin(), y_peaks.end());
	int cutoff = (int)(x_peaks.size() * 0.35f);
	int cutt_x = x_peaks[cutoff];
	int cutt_y = y_peaks[cutoff];
	int band_y1 = max(cutt_y - 5, 0);
	int band_y2 = cutt_y + (int)(sizePerMM * (rangeX / 16.0f) * 1.0f);

	// Remove peripheral outliers and center pixels around optical pivot. 
	for (int c = cent_x1; c < cent_x2; c++) {
		if (inner[c] < band_y1) {
			inner[c] = -1;
		}
		else if (inner[c] > band_y2) {
			inner[c] = -1;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::removeOutliersInCenterEdges(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX)
{
	auto size = inner.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;
	int sizePerMM = (int)(img_w / rangeX);
	float minRange = rangeX * 0.25f;

	for (int k = 0; k < 7; k++) {
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
		}

		int count = 0;
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			int top_pos = 999;
			int top_idx = 0;
			for (int i = 0; i < size; i++) {
				if (fittY[i] < top_pos) {
					top_pos = fittY[i];
					top_idx = i;
				}
			}

			for (int i = 0; i < size; i++) {
				float dist = (float)abs(i - top_idx) / sizePerMM;
				int offs1 = (int)(dist*dist*0.5f + 11.0f);
				int offs2 = (int)(dist*dist*0.5f + 19.0f);

				int y1 = max((int)(fittY[i] - offs1), 0);
				int y2 = min((int)(fittY[i] + offs2), img_h - 1);

				if (inner[i] >= 0) {
					if (inner[i] < y1 || inner[i] > y2) {
						inner[i] = -1;
						count++;
					}
				}
			}

			if (count == 0) {
				count = 0;
				for (int j = 0; j < size; j++) {
					if (inner[j] >= 0) {
						count++;
					}
				}
				if (count >= (minRange*sizePerMM)) {
					return true;
				}
				return false;
			}
		}
	}

	return false;
}


bool RetSegm::CoarseCornea::removeOutliersInCenterEdgesReversed(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX)
{
	auto size = inner.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;
	int sizePerMM = (int)(img_w / rangeX);
	float minRange = 1.0f; // rangeX * 0.25f;

	for (int k = 0; k < 7; k++) {
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
		}

		int count = 0;
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			int top_pos = 999;
			int top_idx = 0;
			for (int i = 0; i < size; i++) {
				if (fittY[i] < top_pos) {
					top_pos = fittY[i];
					top_idx = i;
				}
			}

			for (int i = 0; i < size; i++) {
				float dist = (float)abs(i - top_idx) / sizePerMM;
				int offs1 = (int)(dist*dist*0.5f + 15.0f);
				int offs2 = (int)(dist*dist*0.5f + 25.0f);

				int y1 = max((int)(fittY[i] - offs1), 0);
				int y2 = min((int)(fittY[i] + offs2), img_h - 1);

				if (inner[i] >= 0) {
					if (inner[i] < y1 || inner[i] > y2) {
						inner[i] = -1;
						count++;
					}
				}
			}

			if (count == 0) {
				count = 0;
				for (int j = 0; j < size; j++) {
					if (inner[j] >= 0) {
						count++;
					}
				}
				if (count >= (minRange*sizePerMM)) {
					return true;
				}
				return false;
			}
		}
	}

	return false;
}


bool RetSegm::CoarseCornea::composeInnerCurveByCenterEdges(std::vector<int>& inner, float rangeX, int& centerX, int& centerY)
{
	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);
	float minRange = rangeX * 0.25f;

	int count = 0;
	for (int i = 0; i < size; i++) {
		if (inner[i] >= 0) {
			count++;
		}
	}
	if (count < (sizePerMM * minRange)) {
		return false;
	}

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
	}

	// 2th order polynomial fitting by periperal edge points around cornea center. 
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		int min_x = -1;
		int min_y = 9999;
		int min_size = 0;
		for (int i = 0; i < size; i++) {
			if (fittY[i] < min_y) {
				min_y = fittY[i];
				min_x = i;
				min_size = 1;
			}
			else if (fittY[i] == min_y) {
				min_size += 1;
			}
		}

		centerX = min_x + min_size / 2;
		centerY = min_y;
		inner = fittY;
		return true;
	}
	return false;
}


bool RetSegm::CoarseCornea::composeInnerCurveByCenterEdgesReversed(std::vector<int>& inner, float rangeX, int & centerX, int & centerY)
{
	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);
	float minRange = 1.0f; // rangeX * 0.25f;

	int count = 0;
	for (int i = 0; i < size; i++) {
		if (inner[i] >= 0) {
			count++;
		}
	}
	if (count < (sizePerMM * minRange)) {
		return false;
	}

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
	}

	// 2th order polynomial fitting by periperal edge points around cornea center. 
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		int min_x = -1;
		int min_y = 9999;
		int min_size = 0;
		for (int i = 0; i < size; i++) {
			if (fittY[i] < min_y) {
				min_y = fittY[i];
				min_x = i;
				min_size = 1;
			}
			else if (fittY[i] == min_y) {
				min_size += 1;
			}
		}

		centerX = min_x + min_size / 2;
		centerY = min_y;
		inner = fittY;
		return true;
	}
	return false;
}


bool RetSegm::CoarseCornea::locateInnerBorderLine(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, 
													float rangeX, int hingeY, int reflectX1, int reflectX2, 
													int centerX, int centerY, bool anteriorLens)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	float sizePerMM = (float) size / rangeX;

	int refl_x1, refl_x2;
	refl_x1 = reflectX1;
	refl_x2 = reflectX2;

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 0.5f);
	if (anteriorLens) {
		edge_thresh = (int)(srcImg->getMean() * 2.0f + srcImg->getStddev() * 1.0f);
	}

	int offs1, offs2, y1, y2, edge_y;
	float dist;

	for (int c = 0; c < img_w; c++) {
		if (c >= refl_x1 && c <= refl_x2) {
			inner[c] = -1;
		}
		else {
			dist = abs(c - centerX) / sizePerMM;
			offs1 = (int)(dist * dist * 0.5f + 5.0f);
			offs2 = (int)(dist * dist * 0.5f + 11.0f);

			y1 = max((int)(inner[c] - offs1), 0);
			y2 = min((int)(inner[c] + offs2), img_h - 1);
			
			edge_y = -1;
			for (int r = y1; r < y2; r++) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge_y = r;
					break;
				}
			}
			inner[c] = edge_y;
		}
	}

	if (hingeY >= 0) {
		// Clean up edge points near the hinge.
		for (int c = 0; c < img_w; c++) {
			if (inner[c] >= 0) {
				int dist = (inner[c] - hingeY);
				if (dist <= 3 && dist >= -9) {
					inner[c] = -1;
				}
			}
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::locateInnerBorderLineBySides(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, 
						float rangeX, int hingeY, int reflectX1, int reflectX2, 
						int centerX, int centerY, bool anteriorLens)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	int refl_x1, refl_x2;
	refl_x1 = reflectX1;
	refl_x2 = reflectX2;

	int cent_x1, cent_x2;
	cent_x1 = (int)(centerX - sizePerMM * 1.5f);
	cent_x2 = (int)(centerX + sizePerMM * 1.5f);

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 0.5f);
	if (anteriorLens) {
		edge_thresh = (int)(srcImg->getMean() * 2.0f + srcImg->getStddev() * 1.0f);
	}

	int offs1, offs2, y1, y2, edge_y;
	float dist;

	for (int c = 0; c < img_w; c++) {
		// Excludes points around center region.
		if (c >= cent_x1 && c <= cent_x2) {
			inner[c] = -1;
		}
		else {
			dist = abs(c - centerX) / sizePerMM;
			offs1 = (int)(dist * dist * 0.5f + 5.0f);
			offs2 = (int)(dist * dist * 0.5f + 11.0f);

			y1 = max((int)(inner[c] - offs1), 0);
			y2 = min((int)(inner[c] + offs2), img_h - 1);

			// Ignore lower bound by estimaged curve. 
			y2 = img_h;

			edge_y = -1;
			for (int r = y1; r < y2; r++) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge_y = r;
					break;
				}
			}
			inner[c] = edge_y;
		}
	}

	if (hingeY >= 0) {
		for (int c = 0; c < img_w; c++) {
			if (inner[c] >= 0) {
				int dist = (inner[c] - hingeY);
				if (dist <= 3 && dist >= -9) {
					inner[c] = -1;
				}
			}
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::locateOuterBorderLine(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, 
											float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	int refl_x1, refl_x2;
	/*
	if (!estimateCenterReflection(inner, rangeX, centerX, centerY, refl_x1, refl_x2)) {
		return false;
	}
	*/
	refl_x1 = reflectX1;
	refl_x2 = reflectX2;

	int covered = (int)(sizePerMM * (rangeX / 6.0f) * 3.0f);
	int cornea_x1 = max((int)(centerX - covered), 0);
	int cornea_x2 = min((int)(centerX + covered), size - 1);
	int cornea_y1 = inner[min(max(cornea_x1, 0), img_h-1)];
	int cornea_y2 = inner[min(max(cornea_x2, 0), img_h - 1)];

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 0.5f);

	int band1, band2, y1, y2, edge_y;
	float dist;

	for (int c = 0; c < img_w; c++) {
		if (c >= refl_x1 && c <= refl_x2) {
			outer[c] = -1;
		}
		else if (c >= cornea_x1 && c <= cornea_x2) {
			dist = abs(c - centerX) / sizePerMM;
			band1 = outer[c]; // -(int)((outer[c] - inner[c]) * 0.15f);
			band2 = outer[c] + (int)(dist * dist + 25.0f);
			// band2 = (c < centerX ? min(band2, cornea_y1) : min(band2, cornea_y2));

			y1 = max(band1, 0);
			y2 = min(band2, img_h - 1);

			edge_y = -1;
			for (int r = y2; r >= y1; r--) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge_y = r;
					break;
				}
			}

			if (edge_y > 0) {
				outer[c] = edge_y;
			}
		}
		else {
			outer[c] = -1;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::locateOuterBorderLineForWide(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	int refl_x1, refl_x2;
	refl_x1 = reflectX1;
	refl_x2 = reflectX2;

	int covered = (int)(sizePerMM * (rangeX / 6.0f) * 3.0f);
	int cornea_x1 = max((int)(centerX - covered), 0);
	int cornea_x2 = min((int)(centerX + covered), size - 1);
	int cornea_y1 = inner[min(max(cornea_x1, 0), img_h - 1)];
	int cornea_y2 = inner[min(max(cornea_x2, 0), img_h - 1)];

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 1.0f);

	int band1, band2, y1, y2, edge_y;
	float dist;

	auto curve = vector<int>(size, -1);

	for (int c = 0; c < img_w; c++) {
		if (c >= refl_x1 && c <= refl_x2) {
			curve[c] = -1;
		}
		else {
			dist = abs(c - centerX) / sizePerMM;
			band1 = outer[c] - 10; // -(int)((outer[c] - inner[c]) * 0.15f);
			band2 = outer[c] + (int)(dist * dist + 25.0f);

			y1 = max(band1, 0);
			y2 = min(band2, img_h - 1);

			edge_y = -1;
			for (int r = y2; r >= y1; r--) {
				int val = srcMat.at<uchar>(r, c);
				if (val > edge_thresh) {
					edge_y = r;
					break;
				}
			}

			if (edge_y > 0) {
				curve[c] = edge_y;
			}
		}
	}

	outer = curve;
	return true;
}


bool RetSegm::CoarseCornea::removeOutliersInInnerBorderLine(std::vector<int>& inner, float rangeX, int hingeY, int centerX, int centerY)
{
	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	int cornea_x1 = max((int)(centerX - sizePerMM * 5.0f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 5.0f), size - 1);
	int count = 0;

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	// Build a fitting curve by edge points on the center cornea.
	for (int t = 0; t < 5; t++) {
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cornea_x1 && i <= cornea_x2) {
				dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
			}
			else {
				dataY[i] = 0;
			}
		}

		count = 0;

		// Remove outlier points too far from the curve except the points located 
		// on the outside of cornea, they might be from the sclera and not fitted 
		// by the center curve well.  
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= cornea_x1 && i <= cornea_x2) {
					float dist = abs(i - centerX) / sizePerMM;
					if (inner[i] >= 0) {
						int yerr = inner[i] - fittY[i];
						int offs1 = (int)(0.25f * dist * dist + 3) * -1;
						int offs2 = (int)(0.50f * dist * dist + 5);
						if (yerr < offs1 || yerr > offs2) {
							inner[i] = -1;
							count++;
						}
					}
				}
			}
			if (count == 0) {
				break;
			}
		}
		else {
			return false;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::removeFractionsInInnerBorderLine(std::vector<int>& inner, float rangeX, int centerX, int centerY)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	auto input = inner;
	auto output = inner;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE_CORNEA);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE_CORNEA);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE_CORNEA);
	const int innRange = COARSE_INNER_REMOVE_Y_RANGE;
	const int innBound = COARSE_INNER_REMOVE_Y_BOUND_INN_CORNEA;

	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	if (centerX < 0 && centerY < 0) {
		centerX = size / 2;
	}

	// To remove random noise on inner boundary line. 
	for (int c = 0, none = 0; c < input.size(); c++) {
		float dist = (float)(c - centerX) / sizePerMM;
		int offset1 = (int)(0.1f * dist * dist + 5) * -1;
		int offset2 = (int)(0.1f * dist * dist + 5);

		if (input[c] >= 0) {
			// An inner line gets started. 
			if (innSize == 0) {
				innX1 = c; innY1 = input[c];
				innX2 = c; innY2 = input[c];
				innSize = 1;
				preDiff = 0;
			}
			else {
				// A subsequent point should be within the span limit. 
				// If the difference between the previous segment's end and the current's start has the same 
				// sign (direction in change) with the 
				curDiff = innY2 - input[c];
				curDiff *= (c <= centerX ? +1 : -1);

				// if (abs(curDiff - preDiff) <= innRange) {
				if (offset1 <= curDiff && curDiff <= offset2) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								curDiff *= (c <= centerX ? +1 : -1);
								if (offset1 <= curDiff && curDiff <= offset2) {
									// innX2 = j;
									innY2 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						// Look backward connectable points.
						for (int j = innX1 - 1, look = 0; j >= 0; j--) {
							if (input[j] >= 0) {
								curDiff = innY1 - input[j];
								curDiff *= (c <= centerX ? +1 : -1);
								if (offset1 <= curDiff && curDiff <= offset2) {
									// innX1 = j;
									innY1 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						if (innSize < innSliceSize) {
							// The looked ahead points should not be removed.
							for (int i = innX1; i <= innX2; i++) {
								output[i] = -1;
							}
						}
					}
					innX1 = c; innY1 = input[c];
					innX2 = c; innY2 = input[c];
					innSize = 1;
				}
				preDiff = curDiff;
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (innSize > 0) {
				if (++none >= innEmptySize) {
					if (innSize < innSliceSize) {
						for (int i = innX1; i <= innX2; i++) {
							output[i] = -1;
						}
					}
					innSize = 0;
					none = 0;
				}
			}
		}
	}

	if (innSize > 0 && innSize < innSliceSize) {
		// Look backward connectable points.
		for (int j = innX1 - 1, look = 0; j >= 0; j--) {
			float dist = (float)(j - centerX) / sizePerMM;
			int offset1 = (int)(0.25f * dist * dist + 5) * -1;
			int offset2 = (int)(0.25f * dist * dist + 5);

			if (input[j] >= 0) {
				curDiff = innY1 - input[j];
				curDiff *= (j <= centerX ? +1 : -1);
				if (offset1 <= curDiff && curDiff <= offset2) {
					// innX1 = j;
					innY1 = input[j];
					innSize++;
				}
			}
			if (++look >= innGuessSize) {
				break;
			}
		}

		if (innSize < innSliceSize) {
			// The looked ahead points should not be removed.
			for (int i = innX1; i <= innX2; i++) {
				output[i] = -1;
			}
		}
	}

	inner = output;
	return true;
}


bool RetSegm::CoarseCornea::removeFractionsInInnerEdgesLine(std::vector<int>& inner, float rangeX, int centerX, int centerY)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	auto input = inner;
	auto output = inner;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE_CORNEA);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE_CORNEA);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE_CORNEA*(1.5f));
	const int innRange = COARSE_INNER_REMOVE_Y_RANGE;
	const int innBound = COARSE_INNER_REMOVE_Y_BOUND_INN_CORNEA;

	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	if (centerX < 0 && centerY < 0) {
		centerX = size / 2;
	}

	// To remove random noise on inner boundary line. 
	for (int c = 0, none = 0; c < input.size(); c++) {
		float dist = (float)(c - centerX) / sizePerMM;
		int offset1 = (int)(0.1f * dist * dist + 7) * -1;
		int offset2 = (int)(0.1f * dist * dist + 7);

		if (input[c] >= 0) {
			// An inner line gets started. 
			if (innSize == 0) {
				innX1 = c; innY1 = input[c];
				innX2 = c; innY2 = input[c];
				innSize = 1;
				preDiff = 0;
			}
			else {
				// A subsequent point should be within the span limit. 
				// If the difference between the previous segment's end and the current's start has the same 
				// sign (direction in change) with the 
				curDiff = innY2 - input[c];
				curDiff *= (c <= centerX ? +1 : -1);

				// if (abs(curDiff - preDiff) <= innRange) {
				if (offset1 <= curDiff && curDiff <= offset2) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								curDiff *= (c <= centerX ? +1 : -1);
								if (offset1 <= curDiff && curDiff <= offset2) {
									// innX2 = j;
									innY2 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						// Look backward connectable points.
						for (int j = innX1 - 1, look = 0; j >= 0; j--) {
							if (input[j] >= 0) {
								curDiff = innY1 - input[j];
								curDiff *= (c <= centerX ? +1 : -1);
								if (offset1 <= curDiff && curDiff <= offset2) {
									// innX1 = j;
									innY1 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						if (innSize < innSliceSize) {
							// The looked ahead points should not be removed.
							for (int i = innX1; i <= innX2; i++) {
								output[i] = -1;
							}
						}
					}
					innX1 = c; innY1 = input[c];
					innX2 = c; innY2 = input[c];
					innSize = 1;
				}
				preDiff = curDiff;
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (innSize > 0) {
				if (++none >= innEmptySize) {
					if (innSize < innSliceSize) {
						for (int i = innX1; i <= innX2; i++) {
							output[i] = -1;
						}
					}
					innSize = 0;
					none = 0;
				}
			}
		}
	}

	if (innSize > 0 && innSize < innSliceSize) {
		// Look backward connectable points.
		for (int j = innX1 - 1, look = 0; j >= 0; j--) {
			float dist = (float)(j - centerX) / sizePerMM;
			int offset1 = (int)(0.25f * dist * dist + 5) * -1;
			int offset2 = (int)(0.25f * dist * dist + 5);

			if (input[j] >= 0) {
				curDiff = innY1 - input[j];
				curDiff *= (j <= centerX ? +1 : -1);
				if (offset1 <= curDiff && curDiff <= offset2) {
					// innX1 = j;
					innY1 = input[j];
					innSize++;
				}
			}
			if (++look >= innGuessSize) {
				break;
			}
		}

		if (innSize < innSliceSize) {
			// The looked ahead points should not be removed.
			for (int i = innX1; i <= innX2; i++) {
				output[i] = -1;
			}
		}
	}

	inner = output;
	return true;
}



bool RetSegm::CoarseCornea::interpolateInnerBorderLine(std::vector<int>& inner, float rangeX, int hingeY, int & centerX, int & centerY)
{
	int size = (int)inner.size();
	float sizePerMM = (float)size / rangeX;

	int covered = (int)(sizePerMM * (rangeX / 6.0f) * 1.8f);
	int cornea_x1 = max((int)(centerX - covered), 0);
	int cornea_x2 = min((int)(centerX + covered), size - 1);

	if (rangeX < 11.0f) {
		cornea_x1 = 0;
		cornea_x2 = size - 1;
	}
	
	int count = 0;
	for (int i = 0; i < size; i++) {
		if (inner[i] > 0) {
			count++;
		}
	}
	if (count < (size / 5)) {
		return false;
	}

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cornea_x1 && i <= cornea_x2) {
				dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
			}
			else {
				dataY[i] = 0;
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= cornea_x1 && i <= cornea_x2) {
					if (inner[i] < 0) {
						inner[i] = fittY[i];
					}
				}
			}

			int min_x = -1;
			int min_y = 9999;
			int min_size = 0;
			for (int i = 0; i < size; i++) {
				if (fittY[i] < min_y) {
					min_y = fittY[i];
					min_x = i;
					min_size = 1;
				}
				else if (fittY[i] == min_y) {
					min_size += 1;
				}
			}

			centerX = min_x + min_size / 2;
			centerY = min_y;
		}
		else {
			return false;
		}
	}

	// The exterior points are interpolated by simple linear fitting. 
	interpolateBoundaryByLinearFitting(inner, inner);

	// Smoothed inner border line
	int fsize = (int)(5);
	int forder = 1;
	auto outs = CppUtil::SgFilter::smoothInts(inner, fsize, forder);
	inner = outs;

	return true;
}


bool RetSegm::CoarseCornea::interpolateOuterBorderLine(std::vector<int>& inner, std::vector<int>& outer, float rangeX, int hingeY, int & centerX, int & centerY, float dragWeight)
{
	int size = (int)outer.size();
	float sizePerMM = (float)size / rangeX;

	int cornea_x1 = max((int)(centerX - sizePerMM * 4.5f), 0);
	int cornea_x2 = min((int)(centerX + sizePerMM * 4.5f), size - 1);
	int count = 0;

	for (int i = 0; i < size; i++) {
		if (outer[i] > 0) {
			count++;
		}
	}
	if (count < (int)(sizePerMM)) {
		return false;
	}

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cornea_x1 && i <= cornea_x2) {
				dataY[i] = (outer[i] < 0 ? 0 : outer[i]);
			}
			else {
				dataY[i] = 0;
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				// if (outer[i] < 0) {
				outer[i] = fittY[i];
				// }
			}
		}
		else {
			return false;
		}
	}

	if (dragWeight > 0.0f) {
		for (int i = 0; i < size; i++) {
			float dist = (float)abs(i - centerX) / sizePerMM * 2.0f;
			float corr = (dist * dist * dragWeight);
			if (outer[i] >= 0) {
				outer[i] = outer[i] + (int)corr;
			}
		}

		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			dataY[i] = (outer[i] < 0 ? 0 : outer[i]);
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				// if (outer[i] < 0) {
				outer[i] = fittY[i];
				// }
			}
		}
		else {
			return false;
		}
	}

	// The exterior points are interpolated by simple linear fitting. 
	// interpolateBoundaryByLinearFitting(outer, outer);

	for (int i = 0; i < size; i++) {
		if (outer[i] >= 0 && inner[i] >= 0) {
			outer[i] = max(inner[i], outer[i]);
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::estimateCenterReflection(std::vector<int>& inner, float rangeX, int centerX, int centerY, int & zone_x1, int& zone_x2)
{
	auto size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);

	if (size == 0) {
		return false;
	}

	zone_x1 = max((int)(centerX - sizePerMM * 0.35f), 0);
	zone_x2 = min((int)(centerX + sizePerMM * 0.35f), size - 1);
	return true;
}


bool RetSegm::CoarseCornea::estimateOuterCurveByInnerCurve(const SegmImage * srcImg, 
	std::vector<int>& inner, std::vector<int>& outer, float rangeX, int centerX, int centerY, bool meye)
{
	Mat srcMat = srcImg->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	float sizePerMM = size / rangeX;

	int covered = (int)(sizePerMM * (rangeX / 6.0f) * 1.5f);
	int cent_x1 = max((int)(centerX - covered), 0);
	int cent_x2 = min((int)(centerX + covered), img_w-1);
	int refl_x1 = max((int)(centerX - 0.18f * sizePerMM), 0);
	int refl_x2 = min((int)(centerX + 0.18f * sizePerMM), img_w-1);
	int upp_band = 25; // (int)(sizePerMM * 1.0f); // 25;
	int low_band = 140; // (int)(sizePerMM * 5.0f); // 120;

	if (meye) {
		low_band = img_h - 1;
	}

	int edge_thresh = (int)(srcImg->getMean() * 1.0f + srcImg->getStddev() * 1.0f);

	int max_pos = 0; 
	int max_val = 0;
	int max_cnt = 0;
	
	for (int k = low_band; k >= upp_band; k--) {
		int sum = 0;
		int cnt = 0;
		int val = 0;
		for (int i = cent_x1; i <= cent_x2; i++) {
			// if (i < refl_x1 || i > refl_x2) {
				int r = inner[i] + k;
				int c = i;
				if (r >= 0 && r < img_h && c >= 0 && c <= img_w) {
					val = srcMat.at<uchar>(r, c);
					sum += val;
					if (val >= edge_thresh) {
						cnt += 1;
					}
				}
			// }
		}
		if (sum > max_val) {
			max_val = sum;
			max_pos = k;
			max_cnt = cnt;
			if (cnt > sizePerMM) {
				break;
			}
		}
	}

	if (max_cnt < (sizePerMM / 2)) {
		int upp_band = 0;// (int)(sizePerMM * 0.0f); // 0;
		int low_band = 25; // (int)(sizePerMM * 1.0f); // 25;
		for (int k = upp_band; k < low_band; k++) {
			int sum = 0;
			int cnt = 0;
			int val = 0;
			for (int i = cent_x1; i <= cent_x2; i++) {
				if (i < refl_x1 || i > refl_x2) {
					int r = inner[i] + k;
					int c = i;
					if (r >= 0 && r < img_h && c >= 0 && c <= img_w) {
						val = srcMat.at<uchar>(r, c);
						sum += val;
						if (val >= edge_thresh) {
							cnt += 1;
						}
					}
				}
			}
			if (sum > max_val) {
				max_val = sum;
				max_pos = k;
				max_cnt = cnt;
			}
		}
	}

	for (int i = 0; i < size; i++) {
		float dist = (float) abs(i - centerX) / sizePerMM;
		float corr = (dist * dist);
		outer[i] = inner[i] + max_pos + (int)corr;
	}

	for (int i = 0; i < size; i++) {
		outer[i] = inner[i] + max_pos;
	}
	return true;
}


bool RetSegm::CoarseCornea::detectHingesInWideCornea(const SegmImage * srcImg, std::vector<int>& inner, float rangeX, 
							int centerX, int centerY, int & hingeY)
{
	hingeY = -1;
	if (rangeX < 11.0f) {
		return true;
	}

	Mat srcMat = srcImg->getCvMatConst();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);
	int hlen = (int)(sizePerMM * 1.5f);

	int offset = sizePerMM * 4;
	int x1 = centerX - offset;
	int x2 = centerX + offset;

	int max_val = 0;
	int max_pos = -1;
	
	// Determine the height of hignes where upper and lower cornea images stitched. 
	// In non-wide anterior imaging, it should not be defined. 
	while (true) {
		if (x1 < 0 && x2 >= img_w) {
			break;
		}

		int sum = 0;
		int cnt = 0;
		int r, c;

		if (x1 >= 0) {
			r = inner[x1];
			c = x1;

			if (r >= 0 && r < img_h) {
				for (; c <= (x1 + hlen); c++) {
					sum += srcMat.at<uchar>(r, c);
					cnt += 1;
				}
			}
		}
		if (x2 < img_w) {
			r = inner[x2];
			c = x2;

			if (r >= 0 && r < img_h) {
				for (; c >= (x2 - hlen); c--) {
					sum += srcMat.at<uchar>(r, c);
					cnt += 1;
				}
			}
		}

		if (cnt > sizePerMM) {
			int res = sum / cnt;
			if (res > max_val) {
				max_val = res;
				max_pos = r;
			}
		}
		else {
			break;
		}

		x1--;
		x2++;
	} 

	hingeY = max_pos;
	return true;
}


bool RetSegm::CoarseCornea::removeInnerFractions(std::vector<int>& input, std::vector<int>& output, int& centX, int& centY)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE_CORNEA);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE_CORNEA);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE_CORNEA);
	const int innRange = COARSE_INNER_REMOVE_Y_RANGE;
	const int innBound = COARSE_INNER_REMOVE_Y_BOUND_INN_CORNEA;
	const int outBound = COARSE_INNER_REMOVE_Y_BOUND_OUT_CORNEA;

	output = input;

	// To remove random noise on inner boundary line. 
	for (int c = 0, none = 0; c < input.size(); c++) {
		if (input[c] >= 0) {
			// An inner line gets started. 
			if (innSize == 0) {
				innX1 = c; innY1 = input[c];
				innX2 = c; innY2 = input[c];
				innSize = 1;
				preDiff = 0;
			}
			else {
				// A subsequent point should be within the span limit. 
				// If the difference between the previous segment's end and the current's start has the same 
				// sign (direction in change) with the 
				curDiff = innY2 - input[c];
				curDiff *= (c <= centX ? +1 : -1);

				// if (abs(curDiff - preDiff) <= innRange) {
				if (outBound <= curDiff && curDiff <= innBound) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								curDiff *= (c <= centX ? +1 : -1);
								if (outBound <= curDiff && curDiff <= innBound) {
									// innX2 = j;
									innY2 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						// Look backward connectable points.
						for (int j = innX1 - 1, look = 0; j >= 0; j--) {
							if (input[j] >= 0) {
								curDiff = innY1 - input[j];
								curDiff *= (c <= centX ? +1 : -1);
								if (outBound <= curDiff && curDiff <= innBound) {
									// innX1 = j;
									innY1 = input[j];
									innSize++;
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						if (innSize < innSliceSize) {
							// The looked ahead points should not be removed.
							for (int i = innX1; i <= innX2; i++) {
								output[i] = -1;
							}
						}
					}
					innX1 = c; innY1 = input[c];
					innX2 = c; innY2 = input[c];
					innSize = 1;
				}
				preDiff = curDiff;
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (innSize > 0) {
				if (++none >= innEmptySize) {
					if (innSize < innSliceSize) {
						for (int i = innX1; i <= innX2; i++) {
							output[i] = -1;
						}
					}
					innSize = 0;
					none = 0;
				}
			}
		}
	}

	if (innSize > 0 && innSize < innSliceSize) {
		// Look backward connectable points.
		for (int j = innX1 - 1, look = 0; j >= 0; j--) {
			if (input[j] >= 0) {
				curDiff = innY1 - input[j];
				curDiff *= (j <= centX ? +1 : -1);
				if (outBound <= curDiff && curDiff <= innBound) {
					// innX1 = j;
					innY1 = input[j];
					innSize++;
				}
			}
			if (++look >= innGuessSize) {
				break;
			}
		}

		if (innSize < innSliceSize) {
			// The looked ahead points should not be removed.
			for (int i = innX1; i <= innX2; i++) {
				output[i] = -1;
			}
		}
	}
	return true;
}


bool RetSegm::CoarseCornea::removeInnerOutliers(std::vector<int>& input, std::vector<int>& output)
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
				interpolateBoundaryByLinearFitting(dataY, dataY);
			}
			else {
				break;
			}
		}
		else {
			break;
		}
	}

	/*
	for (int i = 0; i < coeffs.size(); i++) {
	DebugOut() << "coeff: " << i << ", " << coeffs[i];
	}
	*/

	// input = dataY;
	output = fittY;

	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	return true;
}


bool RetSegm::CoarseCornea::interpolateBoundaryByLinearFitting(std::vector<int>& input, std::vector<int>& output)
{
	const int fittSize = (int)(COARSE_LINEAR_FITT_DATA_SIZE * input.size());

	float slope = 0.0f;
	int count = 0;
	int size = (int)input.size();
	output = input;

	int x1 = -1, x2 = -1;
	int y1 = -1, y2 = -1;

	if (input[0] < 0) {
		for (int i = 0; i < size; i++) {
			if (input[i] >= 0) {
					x1 = x2 = i;
					y1 = y2 = input[i];
					count = 1;

				for (int j = i + 1; j < size; j++) {
					if (input[j] >= 0) {
						x2 = j;
						y2 = input[j];
						if (++count >= fittSize) {
							break;
						}
						// break;
					}
				}

				if (x2 > x1) {
					slope = (float)(y2 - y1) / (float)(x2 - x1);
					for (int k = (x1 - 1), dist = -1; k >= 0; k--, dist--) {
						output[k] = (int)(y1 + dist * slope);
						output[k] = max(0, output[k]);
					}
				}
				else {
					output[i] = input[i];
				}

				/*
				for (int j = 0; j < i; j++) {
					output[j] = input[i];
				}
				*/
				break;
			}
		}
	}

	if (input[size - 1] < 0) {
		for (int i = size - 1; i >= 0; i--) {
			if (input[i] >= 0) {
				x1 = x2 = i;
				y1 = y2 = input[i];
				count = 1;

				for (int j = i - 1; j >= 0; j--) {
					if (input[j] >= 0) {
						x1 = j;
						y1 = input[j];
						if (++count >= fittSize) {
							break;
						}
						// break;
					}
				}

				if (x2 > x1) {
					slope = (float)(y2 - y1) / (float)(x2 - x1);
					for (int k = (x2 + 1), dist = 1; k < size; k++, dist++) {
						output[k] = (int)(y2 + dist * slope);
						output[k] = max(0, output[k]);
					}
				}
				else {
					output[i] = input[i];
				}
				/*
				for (int j = i + 1; j < size; j++) {
					output[j] = input[i];
				}
				*/
				break;
			}
		}
	}

	if (output[0] < 0 || output[size - 1] < 0) {
		return false;
	}

	for (int i = 0; i < size; i++)
	{
		if (output[i] >= 0) {
			x1 = i;
			y1 = output[i];
		}
		else
		{
			for (int j = i + 1; j < size; j++) {
				if (output[j] >= 0) {
					x2 = j;
					y2 = output[j];

					slope = (float)(y2 - y1) / (float)(x2 - x1);
					for (int k = (x1 + 1), dist = 1; k < x2; k++, dist++) {
						output[k] = (int)(y1 + dist * slope);
					}
					i = x2;
					break;
				}
			}
		}
	}
	return true;
}
