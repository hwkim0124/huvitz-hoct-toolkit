#include "stdafx.h"
#include "Coarse.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "DataFitt.h"
#include "Feature.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Logger.h"
#include <numeric>
#include <random>
#include <set>

#include "CppUtil2.h"


using namespace CppUtil;
using namespace RetSegm;
using namespace cv;
using namespace std;


Coarse::Coarse()
{
}


bool RetSegm::Coarse::createGradientMapOfRetina(const SegmImage * imgSrc, 
												SegmImage* imgAsc, SegmImage* imgDes, SegmImage * imgOut, bool isDisk)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS, COARSE_GRADIENT_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	/*
	for (int c = 0; c < kernel.cols; c++) {
		kernel.at<float>(kernel.rows / 2, c) = 0.0f;
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

	kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS2, COARSE_GRADIENT_KERNEL_COLS2, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Leave only positive gradients, and normalize in range between [0, 255].
	mask = conv < 0;
	Mat mdes;
	Mat desMat;

	conv.copyTo(mdes);
	mdes.setTo(0.0f, mask);
	cv::normalize(mdes, desMat, 0, 255, NORM_MINMAX, CV_8U);
	imgDes->getCvMat() = desMat;

	/*
	kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS/2+1, COARSE_GRADIENT_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	*/
	/*
	for (int c = 0; c < kernel.cols; c++) {
		kernel.at<float>(kernel.rows / 2, c) = 0.0f;
	}
	*/

	/*
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);


	kernel = Mat::zeros(7, 7, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f / 49.0f;
		}
	}
	for (int r = kernel.rows / 2; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = +1.0f / 49.0f;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 1.0;
		}
	}

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);


	// Reverse negative gradients, and normalize in range between [0, 255].
	// mask = conv > 0;
	mask = conv < 0;
	Mat mdes;
	Mat desMat;

	conv.copyTo(mdes);
	mdes.setTo(0.0f, mask);
	// mdes = mdes.mul(Scalar(-1.0f));
	cv::normalize(mdes, desMat, 0, 255, NORM_MINMAX, CV_8U);
	imgDes->getCvMat() = desMat;
	*/

	// Coarse::createAverageMap(imgSrc, imgDes, 5, 5);
	// Coarse::createGraidentMap(imgSrc, imgDes, 5, 5, true);

	// Gradient map to display.
	imgOut->getCvMat() = ascMat; // desMat;
	return true;
}


bool RetSegm::Coarse::createGradientMapOfCornea(const SegmImage * imgSrc, SegmImage * imgAsc, SegmImage * imgDes, SegmImage * imgOut, bool isDisk)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS, COARSE_GRADIENT_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	/*
	for (int c = 0; c < kernel.cols; c++) {
	kernel.at<float>(kernel.rows / 2, c) = 0.0f;
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

	kernel = Mat::ones(COARSE_GRADIENT_KERNEL_ROWS / 2 + 1, COARSE_GRADIENT_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Reverse negative gradients, and normalize in range between [0, 255].
	mask = conv > 0;
	Mat mdes;
	Mat desMat;

	conv.copyTo(mdes);
	mdes.setTo(0.0f, mask);
	mdes = mdes.mul(Scalar(-1.0f));
	cv::normalize(mdes, desMat, 0, 255, NORM_MINMAX, CV_8U);
	imgDes->getCvMat() = desMat;

	// Coarse::createAverageMap(imgSrc, imgDes, 5, 5);
	// Coarse::createGraidentMap(imgSrc, imgDes, 5, 5, true);

	// Gradient map to display.
	imgOut->getCvMat() = ascMat; // desMat;
	return true;
}


bool RetSegm::Coarse::createGraidentMap(const SegmImage * imgSrc, SegmImage * imgOut, int rows, int cols, bool descent)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::zeros(rows, cols, CV_32F);
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = (descent ? -1.0f : +1.0f);
		}
	}
	for (int r = kernel.rows / 2 + 1; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = (descent ? +1.0f : -1.0f);
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}

	Mat conv;
	Mat mask;
	Mat outMat;

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	if (descent) {
		// Reverse negative gradients, and normalize in range between [0, 255].
		mask = conv > 0;
		conv.setTo(0.0f, mask);
		conv = conv.mul(Scalar(-1.0f));
		cv::normalize(conv, outMat, 0, 255, NORM_MINMAX, CV_8U);
		imgOut->getCvMat() = outMat;
	}
	else {
		mask = conv < 0;
		conv.setTo(0.0f, mask);
		cv::normalize(conv, outMat, 0, 255, NORM_MINMAX, CV_8U);
		imgOut->getCvMat() = outMat;
	}

	return true;
}


bool RetSegm::Coarse::createAverageMap(const SegmImage * imgSrc, SegmImage * imgOut, int rows, int cols)
{
	Mat srcMat = imgSrc->getCvMat();

	Mat kernel = Mat::zeros(rows, cols, CV_32F);
	float size = (float)(rows * cols);

	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = 1.0f / size;
		}
	}
	for (int r = kernel.rows / 2; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) = 1.0f / size;
		}
	}
	/*
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 2.0f;
		}
	}
	*/

	Mat conv;
	Mat outMat;

	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);
	cv::normalize(conv, outMat, 0, 255, NORM_MINMAX, CV_8U);
	imgOut->getCvMat() = outMat;

	return true;
}


bool RetSegm::Coarse::makeBoundaryLinesOfRetina(const SegmImage * imgSrc, const SegmImage* imgAsc, SegmLayer * layerInn, SegmLayer * layerOut, bool isDisk)
{
	vector<int> inner;
	vector<int> outer;

	locateBoundaryEdgesOfRetina(imgSrc, imgAsc, inner, outer);


	removeInnerBoundaryOutliers(inner, outer);
	removeOuterBoundaryOutliers(inner, outer);

	replaceFalseInnerPoints(imgSrc, inner, outer);

	removeInnerLineFractions(inner, inner, isDisk);
	removeInnerLineOutliers(inner, outer, inner);
	removeOuterLineFractions(outer, outer, isDisk);
	removeOuterLineOutliers(inner, outer, isDisk);

	if (!isDisk) {
		Coarse::interpolateBoundaryByLinearFitting(inner, inner, true);
		Coarse::interpolateBoundaryByLinearFitting(outer, outer, true);
	}
	else {
		Coarse::interpolateBoundaryByLinearFitting(inner, inner, true);
	}

	layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());

	/*
	correctFalseOuterPoints(inner, outer);
	removeInnerFractions(inner, inner, isDisk);

	interpolateBoundaryByLinearFitting(inner, inner);
	removeInnerOutliers(inner, inner);

	removeOuterFractions(outer, outer, isDisk);

	if (!isDisk) {
		Coarse::interpolateBoundaryByLinearFitting(outer, outer);
	}

	layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	*/


	return true;
}


bool RetSegm::Coarse::makeInnerBoundaryLineOfCornea(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(imgSrc->getWidth(), -1);
	outer.resize(imgSrc->getWidth(), -1);
	fitts.resize(imgSrc->getWidth(), -1);

	locateInnerBoundaryOfCornea(imgSrc, inner, outer);
	removeInnerFractionsOfCornea(inner, inner);

	if (!checkIfPointsAvailable(inner, COARSE_INNER_POINTS_MIN_CORNEA)) {
		return false;
	}

	interpolateBoundaryByLinearFitting(inner, inner);
	if (!removeInnerOutliersOfCornea(inner, fitts)) {
		return false;
	}

	if (!checkIfPointsAvailable(inner, COARSE_INNER_POINTS_MIN_CORNEA)) {
		return false;
	}

	inner = fitts;

	layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	return true;
}


bool RetSegm::Coarse::makeOuterBoundaryLineOfCornea(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(imgSrc->getWidth(), -1);
	outer.resize(imgSrc->getWidth(), -1);
	fitts.resize(imgSrc->getWidth(), -1);

	inner = layerInn->getYsResized(imgSrc->getWidth(), imgSrc->getHeight());
	locateOuterBoundaryOfCornea(imgSrc, inner, outer);

	if (!checkIfPointsAvailable(outer, COARSE_OUTER_POINTS_MIN_CORNEA)) {
		return false;
	}

	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	return true;
}


bool RetSegm::Coarse::locateBoundaryEdgesOfRetina(const SegmImage * srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat srcMat = srcImg->getCvMat();
	Mat ascMat = ascImg->getCvMat();

	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);

	auto maxLocsInt = srcImg->getColumMaxLocs();
	auto maxValsInt = srcImg->getColumMaxVals();
	auto maxLocsAsc = ascImg->getColumMaxLocs();
	auto maxValsAsc = ascImg->getColumMaxVals();

	int minThresh = (int)(ascImg->getMean() + ascImg->getStddev() * COARSE_BOUNDARY_EDGE_STDDEV_FACTOR);
	minThresh = max(minThresh, COARSE_BOUNDARY_EDGE_MIN);
	int maxThresh = (int)(ascImg->getMean() + ascImg->getStddev() * COARSE_BOUNDARY_PEAK_STDDEV_FACTOR);
	maxThresh = max(maxThresh, COARSE_BOUNDARY_PEAK_MIN);
	int threshInt = (int)(srcImg->getMean() + srcImg->getStddev() * COARSE_BOUNDARY_EDGE_STDDEV_FACTOR);

	int r, c;
	for (c = 0; c < srcMat.cols; c++) {
		int maxIdx = maxLocsAsc[c];
		int maxVal = maxValsAsc[c];
		int curVal = 0;
		int innIdx = -1;
		int outIdx = -1;
		int thresh = 0;

		// Peak threshold.
		if (maxVal < maxThresh) {
			continue;
		}

		thresh = (int)(ascImg->getStddev());

		int uppVal = 0;
		int uppIdx = -1;
		for (r = maxIdx; r >= 0; r--) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal < thresh) {
				uppVal = curVal;
				uppIdx = r;
				break;
			}
		}

		for (; r >= 0; r--) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal >= uppVal) {
				uppVal = curVal;
				uppIdx = r;
			}
		}

		int lowVal = 0;
		int lowIdx = -1;
		for (r = maxIdx;  r < srcMat.rows - 1; r++) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal < thresh) {
				lowVal = curVal;
				lowIdx = r;
				break;
			}
		}

		for (; r < srcMat.rows - 1; r++) {
			curVal = ascMat.at<uchar>(r, c);
			if (curVal >= lowVal) {
				lowVal = curVal;
				lowIdx = r;
			}
		}

		int secIdx = (lowVal > uppVal ? lowIdx : uppIdx);
		int secVal = (lowVal > uppVal ? lowVal : uppVal);

		if (secVal > minThresh) {
			innIdx = min(maxIdx, secIdx);
			outIdx = max(maxIdx, secIdx);
		}
		else {
			innIdx = maxIdx;
		}

		/*
		if (maxLocsInt[c] < 50) {
			maxIdx = maxLocsInt[c];
			for (int r = 0; r < maxIdx; r++) {
				if (srcMat.at<uchar>(r, c) > threshInt) {
					innIdx = r;
					outIdx = maxIdx;
					break;
				}
			}
		}
		*/

		inner[c] = innIdx;
		outer[c] = outIdx;
	}

	return true;
}


bool RetSegm::Coarse::locateInnerBoundaryOfCornea(const SegmImage * imgSrc, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat srcMat = imgSrc->getCvMat();

	auto maxLocs = imgSrc->getColumMaxLocs();
	auto maxVals = imgSrc->getColumMaxVals();

	int minThresh = (int)(imgSrc->getMean() + imgSrc->getStddev() * COARSE_BOUNDARY_EDGE_STDDEV_FACTOR);
	minThresh = max(minThresh, COARSE_BOUNDARY_PEAK_MIN_CORNEA);

	int exceptStart = (int)(srcMat.cols * COARSE_BOUNDARY_EXCEPT_START);
	int exceptClose = (int)(srcMat.cols * COARSE_BOUNDARY_EXCEPT_CLOSE);

	int c;
	for (c = 0; c < srcMat.cols; c++) {
		int maxIdx = maxLocs[c];
		int maxVal = maxVals[c];

		if (c >= exceptStart && c <= exceptClose) {
			continue;
		}

		// Peak threshold.
		if (maxVal < minThresh) {
			continue;
		}

		inner[c] = maxIdx;
	}
	return true;
}



bool RetSegm::Coarse::locateOuterBoundaryOfCornea(const SegmImage * imgSrc, std::vector<int>& inner, std::vector<int>& outer)
{
	Mat srcMat = imgSrc->getCvMat();

	int offsetMin = COARSE_INNER_OUTER_OFFSET_MIN_CORNEA;
	int offsetMax = COARSE_INNER_OUTER_OFFSET_MAX_CORNEA;
	int overlap = (int)(srcMat.cols * COARSE_INNER_OUTER_OVERLAP_MIN_CORNEA);

	int gmaxVal = 0, gmaxPos = 0;
	int gsum, gcnt;

	int r, c;
	for (int offset = offsetMin; offset < offsetMax; offset++) {
		gsum = gcnt = 0;
		for (c = 0; c < srcMat.cols; c++) {
			if (inner[c] >= 0) {
				r = inner[c] + offset;
				if (r < srcMat.rows) {
					gsum += srcMat.at<uchar>(r, c);
					gcnt++;
				}
			}
		}
		if (gcnt >= overlap) {
			if (gsum > gmaxVal) {
				gmaxVal = gsum;
				gmaxPos = offset;
			}
		}
		else {
			break;
		}
	}


	for (c = 0; c < srcMat.cols; c++) {
		outer[c] = inner[c] + gmaxPos;
	}
	return true;
}



void RetSegm::Coarse::removeInnerBoundaryOutliers(std::vector<int>& inner, std::vector<int>& outer)
{
	int size = (int)inner.size();
	int minSegm = (int)(size * COARSE_BOUNDARY_SEGMENT_SIZE_MIN);
	int maxEmpt = (int)(size * COARSE_BOUNDARY_POINTS_EMPTY_MAX);
	int yRange = (int)(COARSE_BOUNDARY_POINTS_Y_RANGE);

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0, slen = 0, none = 0;
	for (int c = 0; c < size; c++) {
		if (inner[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = inner[c];
				slen = 1;
			}
			else {
				if ((abs(inner[c] - y2) > yRange)) {
					for (int k = c + 1; k < c + maxEmpt; k++) {
						if ((inner[k] >= 0 && abs(inner[k] - y2) <= yRange) ||
							(outer[k] >= 0 && abs(outer[k] - y2) <= yRange)) {
							slen++;
						}
					}
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							inner[k] = -1;
						}
					}
					x1 = x2 = c;
					y1 = y2 = inner[c];
					slen = 1;
				}
				else {
					x2 = c;
					y2 = inner[c];
					slen++;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							inner[k] = -1;
						}
						slen = 0;
					}
				}
			}
		}

		if (c >= (size / 2)) {
			if (slen == 0 || slen >= minSegm) {
				break;
			}
		}
	}

	x1 = 0, x2 = 0, y1 = 0, y2 = 0, slen = 0, none = 0;
	for (int c = size-1; c >= 0; c--) {
		if (inner[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = inner[c];
				slen = 1;
			}
			else {
				if ((abs(inner[c] - y1) > yRange)) {
					for (int k = c - 1; k > c - maxEmpt; k--) {
						if ((inner[k] >= 0 && abs(inner[k] - y1) <= yRange) || 
							(outer[k] >= 0 && abs(outer[k] - y1) <= yRange)) {
							slen++;
						}
					}
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							inner[k] = -1;
						}
					}
					x1 = x2 = c;
					y1 = y2 = inner[c];
					slen = 1;
				}
				else {
					x1 = c;
					y1 = inner[c];
					slen++;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							inner[k] = -1;
						}
						slen = 0;
					}
				}
			}
		}

		if (c <= (size / 2)) {
			if (slen == 0 || slen >= minSegm) {
				break;
			}
		}
	}
	return ;
}


void RetSegm::Coarse::removeOuterBoundaryOutliers(std::vector<int>& inner, std::vector<int>& outer)
{
	int size = (int)outer.size();
	int minSegm = (int)(size * COARSE_BOUNDARY_SEGMENT_SIZE_MIN);
	int maxEmpt = (int)(size * COARSE_BOUNDARY_POINTS_EMPTY_MAX);
	int yRange = (int)(COARSE_BOUNDARY_POINTS_Y_RANGE);

	int x1 = 0, x2 = 0, y1 = 0, y2 = 0, slen = 0, none = 0;
	for (int c = 0; c < size; c++) {
		if (outer[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = outer[c];
				slen = 1;
			}
			else {
				if ((abs(outer[c] - y2) > yRange)) {
					for (int k = c + 1; k < c + maxEmpt; k++) {
						if ((inner[k] >= 0 && abs(inner[k] - y2) <= yRange) ||
							(outer[k] >= 0 && abs(outer[k] - y2) <= yRange)) {
							slen++;
						}
					}
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							outer[k] = -1;
						}
					}
					x1 = x2 = c;
					y1 = y2 = outer[c];
					slen = 1;
				}
				else {
					x2 = c;
					y2 = outer[c];
					slen++;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							outer[k] = -1;
						}
						slen = 0;
					}
				}
			}
		}

		if (c >= (size / 2)) {
			if (slen == 0 || slen >= minSegm) {
				break;
			}
		}
	}

	x1 = 0, x2 = 0, y1 = 0, y2 = 0, slen = 0, none = 0;
	for (int c = size - 1; c >= 0; c--) {
		if (outer[c] >= 0) {
			none = 0;
			if (slen <= 0) {
				x1 = x2 = c;
				y1 = y2 = outer[c];
				slen = 1;
			}
			else {
				if ((abs(outer[c] - y1) > yRange)) {
					for (int k = c - 1; k > c - maxEmpt; k--) {
						if ((inner[k] >= 0 && abs(inner[k] - y1) <= yRange) ||
							(outer[k] >= 0 && abs(outer[k] - y1) <= yRange)) {
							slen++;
						}
					}
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							outer[k] = -1;
						}
					}
					x1 = x2 = c;
					y1 = y2 = outer[c];
					slen = 1;
				}
				else {
					x1 = c;
					y1 = outer[c];
					slen++;
				}
			}
		}
		else {
			if (slen > 0) {
				if (++none > maxEmpt) {
					if (slen < minSegm) {
						for (int k = x1; k <= x2; k++) {
							outer[k] = -1;
						}
						slen = 0;
					}
				}
			}
		}

		if (c <= (size / 2)) {
			if (slen == 0 || slen >= minSegm) {
				break;
			}
		}
	}

	return;
}


bool RetSegm::Coarse::replaceFalseInnerPoints(const SegmImage* imgSrc, std::vector<int>& inner, std::vector<int>& outer)
{
	int size = (int)inner.size();
	auto inn_data = inner;
	auto out_data = outer;
	auto maxLocs = imgSrc->getColumMaxLocs();

	int edgeSize = (int)(COARSE_INNER_LINE_EDGE_SIZE * size);

	int xpos1 = -1, ypos1 = -1, xpos2 = -1, ypos2 = -1;
	int count = 0;
	for (int j = 0; j < (size / 2); j++) {
		if (inn_data[j] >= 0 || out_data[j] >= 0) {
			xpos1 = (xpos1 < 0 ? j : xpos1);
			if (out_data[j] >= 0) {
				ypos1 = out_data[j];
				break;
			}
		}
	}

	for (int j = size - 1; j > (size / 2); j--) {
		if (inn_data[j] >= 0 || out_data[j] >= 0) {
			xpos2 = (xpos2 < 0 ? j : xpos2);
			if (out_data[j] >= 0) {
				ypos2 = out_data[j];
				break;
			}
		}
	}

	int ymax = 0, limit1 = 0, limit2 = 0;
	for (int j = xpos1; j <= xpos2; j++) {
		ymax = max(max(inn_data[j], out_data[j]), ymax);
	}
	limit1 = (int)(ymax - max((ymax - ypos1) * 0.70f, 45.0f));
	limit2 = (int)(ymax - max((ymax - ypos2) * 0.70f, 45.0f));

	for (int j = xpos1; j <= (xpos1 + edgeSize); j++) {
		if (inn_data[j] >= 0 && out_data[j] >= 0 && inn_data[j] < limit1) {
			if (abs(inn_data[j] - maxLocs[j]) < abs(out_data[j] - maxLocs[j])) {
				out_data[j] = -1;
			}
		}
	}

	for (int j = xpos2; j >= (xpos2 - edgeSize); j--) {
		if (inn_data[j] >= 0 && out_data[j] >= 0 && inn_data[j] < limit2) {
			if (abs(inn_data[j] - maxLocs[j]) < abs(out_data[j] - maxLocs[j])) {
				out_data[j] = -1;
			}
		}
	}

	// inner = inn_data;
	// outer = out_data;
	// return true;

	for (int retry = 0; retry < 5; retry++) {
		auto inn_curv = std::vector<int>(size, -1);
		auto out_curv = std::vector<int>(size, -1);
		auto mid_curv = std::vector<int>(size, -1);

		inn_curv = inn_data;
		out_curv = out_data;

		interpolateBoundaryByLinearFitting(inn_curv, inn_curv, true);
		interpolateBoundaryByLinearFitting(out_curv, out_curv, true);

		for (int i = 0; i < size; i++) {
			mid_curv[i] = min((inn_curv[i] + out_curv[i]) / 2, out_curv[i]);
		}

		smoothBoundaryLine(inn_curv, inn_curv, COARSE_INNER_LINE_FILTER_SIZE);
		smoothBoundaryLine(out_curv, out_curv, COARSE_OUTER_LINE_FILTER_SIZE);
		smoothBoundaryLine(mid_curv, mid_curv, COARSE_OUTER_LINE_FILTER_SIZE);

		for (int j = xpos1; j <= (xpos1 + edgeSize); j++) {
			if (inn_data[j] >= 0 && inn_data[j] < limit1) {
				int diff1 = abs(inn_data[j] - inn_curv[j]);
				int diff2 = abs(inn_data[j] - out_curv[j]);
				if (inn_data[j] >= (out_curv[j] - 5) || diff1 > diff2) {
					out_data[j] = inn_data[j];
					inn_data[j] = -1;
					count++;
				}
			}
		}

		for (int j = xpos2; j >= (xpos2 - edgeSize); j--) {
			int diff1 = abs(inn_data[j] - inn_curv[j]);
			int diff2 = abs(inn_data[j] - out_curv[j]);
			if (inn_data[j] >= 0 && inn_data[j] < limit2) {
				if (inn_data[j] >= (out_curv[j] - 5) || diff1 > diff2) {
					out_data[j] = inn_data[j];
					inn_data[j] = -1;
					count++;
				}
			}
		}

		/*
		for (int j = xpos1 + 1; j < xpos2; j++) {
			if (out_data[j] < 0) {
				if (inn_data[j] > (mid_curv[j] + 15)) {
					out_data[j] = inn_data[j];
					inn_data[j] = -1;
					count++;
				}
			}
		}
		*/

		//inn_data = inn_curv;
		//out_data = out_curv;
		//break;
		if (count <= 0) {
			break;
		}
	}

	for (int j = xpos1; j <= (xpos1 + edgeSize/2); j++) {
		if (inn_data[j] >= 0 && out_data[j] < 0) {
			inn_data[j] = -1;
		}
	}

	for (int j = xpos2; j >= (xpos2 - edgeSize/2); j--) {
		if (inn_data[j] >= 0 && out_data[j] < 0) {
			inn_data[j] = -1;
		}
	}

	inner = inn_data;
	outer = out_data;
	return true;
}


bool RetSegm::Coarse::smoothBoundaryLine(std::vector<int>& input, std::vector<int>& output, float filtSize)
{
	Mat msrc(1, (int)input.size(), CV_32SC1, input.data());
	Mat mdst(1, (int)output.size(), CV_32SC1, output.data());

	Size fsize = Size((int)(input.size() * filtSize), 1);
	blur(msrc, mdst, fsize);
	return true;
}


bool RetSegm::Coarse::removeInnerLineFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE);
	const int innRange = (isDisk ? COARSE_INNER_REMOVE_Y_RANGE_DISK : COARSE_INNER_REMOVE_Y_RANGE);

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
				// if (abs(curDiff - preDiff) <= innRange) {
				if (abs(curDiff) <= innRange) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								if (abs(curDiff) <= innRange) {
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
								if (abs(curDiff) <= innRange) {
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
				if (abs(curDiff) <= innRange) {
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




bool RetSegm::Coarse::removeOuterLineFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_EMPTY_SIZE_DISK : COARSE_OUTER_REMOVE_EMPTY_SIZE));
	const int innGuessSize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_GUESS_SIZE_DISK : COARSE_OUTER_REMOVE_GUESS_SIZE));
	const int innSliceSize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_SLICE_SIZE_DISK : COARSE_OUTER_REMOVE_SLICE_SIZE));
	const int innRange = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK : COARSE_OUTER_REMOVE_Y_RANGE);
	const int innRange1 = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK1 : COARSE_OUTER_REMOVE_Y_RANGE1);
	const int innRange2 = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK2 : COARSE_OUTER_REMOVE_Y_RANGE2);

	output = input;

	// To remove random noise on outer boundary line. 
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
				curDiff = input[c] - innY2;
				// if (abs(curDiff - preDiff) <= innRange) {
				// if (abs(curDiff) <= innRange) {
				if (curDiff >= innRange1 && curDiff <= innRange2) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = input[j] - innY2;
								if (curDiff >= innRange1 && curDiff <= innRange2) {
									// innX2 = j;
									innY2 = input[j];
									innSize++;
									look = 0;
									if (innSize >= innSliceSize) {
										break;
									}
								}
								else {
									if (isDisk) {
										break;
									}
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						if (innSize < innSliceSize) {
							// Look backward connectable points.
							for (int j = innX1 - 1, look = 0; j >= 0; j--) {
								if (input[j] >= 0) {
									curDiff = input[j] - innY1;
									if (curDiff >= innRange1 && curDiff <= innRange2) {
										// innX1 = j;
										innY1 = input[j];
										innSize++;
										look = 0;
										if (innSize >= innSliceSize) {
											break;
										}
									}
									else {
										if (isDisk) {
											break;
										}
									}
								}
								if (++look >= innGuessSize) {
									break;
								}
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
				curDiff = input[j] - innY1;
				if (curDiff >= innRange1 && curDiff <= innRange2) {
					// innX1 = j;
					innY1 = input[j];
					innSize++;
					look = 0;
					if (innSize >= innSliceSize) {
						break;
					}
				}
				else {
					if (isDisk) {
						break;
					}
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


bool RetSegm::Coarse::removeInnerLineOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& output)
{
	auto inns = inner;
	auto outs = outer;

	interpolateBoundaryByLinearFitting(inns, inns, false);
	interpolateBoundaryByLinearFitting(outs, outs, false);

	auto inns_curv = inns;
	auto outs_curv = outs;
	smoothBoundaryLine(inns, inns_curv, COARSE_INNER_SMOOTH_FILTER_SIZE);
	smoothBoundaryLine(outs, outs_curv, COARSE_INNER_SMOOTH_FILTER_SIZE);

	int rever_x1 = -1, rever_x2 = -1;
	int perip_x1 = -1, perip_x2 = -1;
	int size = (int)inns_curv.size();
	for (int i = 0; i < size; i++) {
		if (inns_curv[i] > outs_curv[i]) {
			rever_x1 = (rever_x1 >= 0 ? rever_x1 : i);
			rever_x2 = i;
		}
	}

	bool found = (rever_x2 - rever_x1) > 0;
	if (found) {
		int width = rever_x2 - rever_x1 + 1;
		int side = width / 4;
		perip_x1 = max(rever_x1 - side, 0);
		perip_x2 = min(rever_x2 + side, size - 1);

		auto minmax = minmax_element(inns.begin() + perip_x1, inns.begin() + perip_x2);
		int top_y = *minmax.first;
		int bot_y = *minmax.second;
		int perip_limit = max((bot_y - top_y) / 3, COARSE_INNER_SMOOTH_OUTLIER_OFFSET);

		for (int i = 0, outliers = 0; i < COARSE_INNER_SMOOTH_COUNT_MAX; i++) {
			for (int j = 0; j < inns_curv.size(); j++) {
				int offset = inns_curv[j] - inns[j];
				if (j >= perip_x1 && j <= perip_x2) {
					if (offset >= perip_limit) {
						inns[j] = -1;
						outliers++;
					}
				}
				else {
					if (offset >= COARSE_INNER_SMOOTH_OUTLIER_OFFSET) {
						inns[j] = -1;
						outliers++;
					}
				}
			}
			if (outliers > 0) {
				interpolateBoundaryByLinearFitting(inns, inns, false);
				smoothBoundaryLine(inns, inns_curv, COARSE_INNER_SMOOTH_FILTER_SIZE);
				outliers = 0;
			}
			else {
				break;
			}
		}

		output = inns_curv;
		for (int i = perip_x1; i <= perip_x2; i++) {
			if (inns_curv[i] > inns[i]) {
				output[i] = inns[i];
			}
		}
	}
	else {
		for (int i = 0, outliers = 0; i < COARSE_INNER_SMOOTH_COUNT_MAX; i++) {
			for (int j = 0; j < inns_curv.size(); j++) {
				if ((inns_curv[j] - inns[j]) >= COARSE_INNER_SMOOTH_OUTLIER_OFFSET) {
					inns[j] = -1; 
					outliers++;
				}
			}
			if (outliers > 0) {
				interpolateBoundaryByLinearFitting(inns, inns, false);
				smoothBoundaryLine(inns, inns_curv, COARSE_INNER_SMOOTH_FILTER_SIZE);
				outliers = 0;
			}
			else {
				break;
			}
		}

		output = inns_curv;
	}

	output = inns;

	/*
	output = input;
	auto smooth = input;

	interpolateBoundaryByLinearFitting(input, output, true);

	for (int i = 0, outliers = 0; i < COARSE_INNER_SMOOTH_COUNT_MAX; i++) {
		smoothBoundaryLine(output, smooth, COARSE_INNER_SMOOTH_FILTER_SIZE);

		for (int j = 0; j < smooth.size(); j++) {
			if ((smooth[j] - output[j]) >= COARSE_INNER_SMOOTH_OUTLIER_OFFSET) {
				output[j] = -1; // smooth[j];
				outliers++;
			}
		}
		if (outliers > 0) {
			interpolateBoundaryByLinearFitting(output, output, true);
			outliers = 0;
		}
		else {
			break;
		}
	}
	// output = smooth;
	*/
	return true;
}


bool RetSegm::Coarse::removeOuterLineOutliers(std::vector<int>& inner, std::vector<int>& outer, bool isDisk)
{
	auto inns = inner;
	auto outs = outer;

	if (!isDisk) {
		return true;
	}

	interpolateBoundaryByLinearFitting(inns, inns, true);
	smoothBoundaryLine(inns, inns, COARSE_INNER_SMOOTH_FILTER_SIZE);

	interpolateBoundaryByLinearFitting(outs, outs, true);
	smoothBoundaryLine(outs, outs, COARSE_OUTER_SMOOTH_FILTER_SIZE * 10.0f);

	for (int c = 0; c < inns.size(); c++) {
		if (inns[c] > (outs[c] + 3)) {
			outer[c] = -1;
		}
	}
	return true;
}


bool RetSegm::Coarse::correctFalseOuterPoints(std::vector<int>& inner, std::vector<int>& outer)
{
	int outX1 = -1, outY1 = -1;
	int outX2 = -1, outY2 = -1;
	int outSize = 0;
	int none = 0;
	bool broken = false;
	bool tangled = false;

	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;

	int conX1 = -1, conY1 = -1;
	int conX2 = -1, conY2 = -1;
	int conSize1 = 0, conSize2 = 0;

	const int innEmptySize = (int)(inner.size()*COARSE_INNER_POINTS_EMPTY_MAX);
	const int outEmptySize = (int)(outer.size()*COARSE_OUTER_POINTS_EMPTY_MAX);
	const int innSliceSize = (int)(inner.size()*COARSE_INNER_POINTS_SLICE_SIZE);
	const int outSliceSize = (int)(outer.size()*COARSE_OUTER_POINTS_SLICE_SIZE);
	const int innRange = COARSE_INNER_POINTS_Y_RANGE;
	const int outRange = COARSE_OUTER_POINTS_Y_RANGE;

	for (int c = 0, none = 0; c < outer.size(); c++) {
		
		if (outer[c] >= 0) {
			// Starting of an outer segment.
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// The current point is connected with the segment. 
				if (abs(outY2 - outer[c]) <= outRange) {
					outX2 = c; outY2 = outer[c];
					outSize++;
				}
				else {
					// If the current segment is large enough, it's passed then the next segment is started. 
					if (outSize >= outSliceSize) {
						outX1 = c; outY1 = outer[c];
						outX2 = c; outY2 = outer[c];
						outSize = 1;
					}
					else {
						// else it's broken. 
						broken = true;
					}
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none >= outEmptySize) {
					none = 0;
					if (outSize < outSliceSize) {
						broken = true;
					}
					else {
						outSize = 0;
					}
				}
			}
		}

		// The end of outer boundary reached. 
		if (c == outer.size() - 1) {
			if (outSize > 0 && outSize < outSliceSize) {
				broken = true;
			}
		}

		if (broken) {
			broken = false;
			tangled = false;

			// Size of inner segment corresponding the broken outer segment. 
			innX1 = outX1; innY1 = inner[outX1];
			innX2 = outX2; innY2 = inner[outX2];
			innSize = innX2 - innX1 + 1;

			for (int i = innX1 - 1, none = 0; i >= 0; i--) {
				if (inner[i] >= 0) {
					none = 0;
					if (abs(innY1 - inner[i]) <= innRange) {
						innX1 = i; innY1 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= innEmptySize) {
						break;
					}
				}
			}

			for (int i = innX2 + 1, none = 0; i < inner.size(); i++) {
				if (inner[i] >= 0) {
					none = 0;
					if (abs(innY2 - inner[i]) <= innRange) {
						innX2 = i; innY2 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= innEmptySize) {
						break;
					}
				}
			}

			//DebugOut() << "Broken outer segment: " << outX1 << ", " << outX2 << ", corresponding inner: " << innX1 << ", " << innX2;

			// If inner segment over the current outer is also not large enough, 
			if (innSize < innSliceSize) {
				conSize1 = conSize2 = 0;
				conX1 = outX1;
				conX2 = outX2;

				//	Size of inner boundary interwinded on the left.
				for (int i = outX1 - 1, none = 0; i >= 0; i--) {
					if (inner[i] >= 0) {
						if (tangled == false) {
							if (abs(outY1 - inner[i]) <= innRange) {
								conX1 = i; conY1 = inner[i];
								conX2 = i; conY2 = inner[i];
								conSize1 = 1;
								tangled = true;
								none = 0;
							}
							else {
								if (++none >= innEmptySize) {
									break;
								}
								// break;
							}
						}
						else {
							if (abs(conY1 - inner[i]) <= innRange) {
								conX1 = i; conY1 = inner[i];
								conSize1++;
								none = 0;
							}
							else {
								if (++none >= innEmptySize) {
									break;
								}
								// break;
							}
						}
					}
					else {
						if (++none >= innEmptySize) {
							break;
						}
					}
				}

				//	Size of inner boundary interwinded on the right.
				for (int i = outX2 + 1, none = 0; i < inner.size(); i++) {
					if (inner[i] >= 0) {
						if (tangled == false) {
							if (abs(outY2 - inner[i]) <= innRange) {
								conX1 = i; conY1 = inner[i];
								conX2 = i; conY2 = inner[i];
								conSize2 = 1;
								tangled = true;
								none = 0;
							}
							else {
								if (++none >= innEmptySize) {
									break;
								}
								// break;
							}
						}
						else {
							if (abs(conY2 - inner[i]) <= innRange) {
								conX2 = i; conY2 = inner[i];
								conSize2++;
								none = 0;
							}
							else {
								if (++none >= innEmptySize) {
									break;
								}
								// break;
							}
						}
					}
					else {
						if (++none >= innEmptySize) {
							break;
						}
					}
				}

				//DebugOut() << "Interwinded with inner boundary by both sides, size1: " << conSize1 << ", size2: " << conSize2;

				if (tangled) {
					/*
					if ((conSize1 > outSize && conSize1 > INNER_SECT_SIZE) ||
						(conSize2 > outSize && conSize2 > INNER_SECT_SIZE)) {
						for (int i = outX1; i <= outX2; i++) {
							// continue;
							if (outer[i] >= 0) {
								inner[i] = outer[i];
								outer[i] = -1;
							}
						}
						//DebugOut() << "Outer segment replaced in " << outX1 << " to " << outX2;
					}
					*/
					if ((conX2 - conX1 + 1) > innSliceSize) {
						for (int i = outX1; i <= outX2; i++) {
							if (outer[i] >= 0) {
								inner[i] = outer[i];
								outer[i] = -1;
							}
						}
					}
				}
			}

			if (outer[c] >= 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				outSize = 0;
			}
		}
	}

	return true;
}


bool RetSegm::Coarse::removeInnerFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE);
	const int innRange = (isDisk ? COARSE_INNER_REMOVE_Y_RANGE_DISK : COARSE_INNER_REMOVE_Y_RANGE);

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
				// if (abs(curDiff - preDiff) <= innRange) {
				if (abs(curDiff) <= innRange) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								if (abs(curDiff) <= innRange) {
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
								if (abs(curDiff) <= innRange) {
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
				if (abs(curDiff) <= innRange) {
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




bool RetSegm::Coarse::removeOuterFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_EMPTY_SIZE_DISK : COARSE_OUTER_REMOVE_EMPTY_SIZE));
	const int innGuessSize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_GUESS_SIZE_DISK : COARSE_OUTER_REMOVE_GUESS_SIZE));
	const int innSliceSize = (int)(input.size()*(isDisk ? COARSE_OUTER_REMOVE_SLICE_SIZE_DISK : COARSE_OUTER_REMOVE_SLICE_SIZE));
	const int innRange = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK : COARSE_OUTER_REMOVE_Y_RANGE);
	const int innRange1 = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK1 : COARSE_OUTER_REMOVE_Y_RANGE1);
	const int innRange2 = (isDisk ? COARSE_OUTER_REMOVE_Y_RANGE_DISK2 : COARSE_OUTER_REMOVE_Y_RANGE2);

	output = input;

	// To remove random noise on outer boundary line. 
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
				curDiff = input[c] - innY2;
				// if (abs(curDiff - preDiff) <= innRange) {
				// if (abs(curDiff) <= innRange) {
				if (curDiff >= innRange1 && curDiff <= innRange2) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = input[j] - innY2;
								if (curDiff >= innRange1 && curDiff <= innRange2) {
									// innX2 = j;
									innY2 = input[j];
									innSize++;
									look = 0;
									if (innSize >= innSliceSize) {
										break;
									}
								}
								else {
									if (isDisk) {
										break;
									}
								}
							}
							if (++look >= innGuessSize) {
								break;
							}
						}

						if (innSize < innSliceSize) {
							// Look backward connectable points.
							for (int j = innX1 - 1, look = 0; j >= 0; j--) {
								if (input[j] >= 0) {
									curDiff = input[j] - innY1;
									if (curDiff >= innRange1 && curDiff <= innRange2) {
										// innX1 = j;
										innY1 = input[j];
										innSize++;
										look = 0;
										if (innSize >= innSliceSize) {
											break;
										}
									}
									else {
										if (isDisk) {
											break;
										}
									}
								}
								if (++look >= innGuessSize) {
									break;
								}
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
				curDiff = input[j] - innY1;
				if (curDiff >= innRange1 && curDiff <= innRange2) {
					// innX1 = j;
					innY1 = input[j];
					innSize++;
					look = 0;
					if (innSize >= innSliceSize) {
						break;
					}
				}
				else {
					if (isDisk) {
						break;
					}
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


bool RetSegm::Coarse::removeInnerOutliers(std::vector<int>& input, std::vector<int>& output)
{
	output = input;
	auto smooth = input;

	for (int i = 0, outliers = 0; i < COARSE_INNER_SMOOTH_COUNT_MAX; i++) {
		smoothBoundaryOfRetina(output, smooth, COARSE_INNER_SMOOTH_FILTER_SIZE);

		for (int j = 0; j < smooth.size(); j++) {
			if ((smooth[j] - output[j]) >= COARSE_INNER_SMOOTH_OUTLIER_OFFSET) {
				output[j] = -1; // smooth[j];
				outliers++;
			}
		}
		if (outliers > 0) {
			interpolateBoundaryByLinearFitting(output, output);
			outliers = 0;
		}
		else {
			break;
		}
	}
	output = smooth;
	return true;
}


bool RetSegm::Coarse::removeInnerFractionsOfCornea(std::vector<int>& input, std::vector<int>& output)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;

	const int innEmptySize = (int)(input.size()*COARSE_INNER_REMOVE_EMPTY_SIZE_CORNEA);
	const int innGuessSize = (int)(input.size()*COARSE_INNER_REMOVE_GUESS_SIZE_CORNEA);
	const int innSliceSize = (int)(input.size()*COARSE_INNER_REMOVE_SLICE_SIZE_CORNEA);
	const int innRange = COARSE_INNER_REMOVE_Y_RANGE;

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
				// if (abs(curDiff - preDiff) <= innRange) {
				if (abs(curDiff) <= innRange) {
					innX2 = c; innY2 = input[c];
					innSize++;
				}
				else {
					if (innSize < innSliceSize) {
						// Look forward connectable points.
						for (int j = c + 1, look = 0; j < input.size(); j++) {
							if (input[j] >= 0) {
								curDiff = innY2 - input[j];
								if (abs(curDiff) <= innRange) {
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
								if (abs(curDiff) <= innRange) {
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
				if (abs(curDiff) <= innRange) {
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


bool RetSegm::Coarse::removeInnerOutliersOfCornea(std::vector<int>& input, std::vector<int>& output)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = input[i];
	}

	int diff;
	vector<double> coeffs;
	for (int i = 0, outliers = 0; i < COARSE_OUTLIERS_RETRY_MAX_CORNEA; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				diff = abs(fittY[i] - dataY[i]);
				if (diff > COARSE_INNER_OUTLIERS_OFFSET_CORNEA) {
					dataY[i] = -1;
					outliers++;
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

	input = dataY;
	output = fittY;

	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	return true;
}


bool RetSegm::Coarse::makeFittingCurveOfCornea(std::vector<int>& input, std::vector<int>& output)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		dataY[i] = input[i];
	}

	int diff;
	vector<double> coeffs;
	for (int i = 0, outliers = 0; i < COARSE_OUTLIERS_RETRY_MAX_CORNEA; i++, outliers = 0) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				diff = fittY[i] - dataY[i];
				if (diff > COARSE_INNER_OUTLIERS_OFFSET_CORNEA) {
					dataY[i] = fittY[i]; // -1;
					outliers++;
				}
			}

			if (outliers <= 0) {
				break;
			}
		}
		else {
			break;
		}
	}

	for (int i = 0; i < coeffs.size(); i++) {
		DebugOut() << "coeff: " << i << ", " << coeffs[i];
	}
	// input = dataY;
	output = fittY;
	// output = dataY;

	/*
	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	*/
	return true;
}


bool RetSegm::Coarse::makeCurveRadiusOfCornea(float resolutionX, float resolutionY, float rangeX, std::vector<int>& input, std::vector<float>& output, bool isAxial)
{
	/*
	auto size = (int)input.size();
	float sizePerMM = (float)(size / rangeX);
	auto curvR = vector<float>(size, -1);

	int segm = size / 16;
	int half = segm / 2;
	for (int i = 0; i < size; i++) {
		int k1 = max(i - half, 0);
		int k2 = min(i + half, size - 1);

		k2 += ((k1 < half) ? half - k1 : 0);
		k1 -= ((k2 > (size - half)) ? half - (size - k2) : 0);
		k1 = max(k1, 0);
		k2 = min(k2, size - 1);

		int n = (k2 - k1 + 1);
		int t = i - k1;

		auto dataX = vector<double>(n, -1);
		auto dataY = vector<double>(n, -1);
		auto fittY = vector<double>(n, -1);

		for (int j = k1, d = 0; j <= k2; j++, d++) {
			dataX[d] = j * resolutionX * 0.001;
			dataY[d] = input[j] * resolutionY * 0.001;
		}

		vector<double> coeffs;
		CppUtil::Regressor::polyFit(dataX, dataY, 2, coeffs);

		double fx1, fx2, num;
		fx1 = coeffs[2] * 2.0 * dataX[t] + coeffs[1];
		fx2 = 2.0 * coeffs[2];
		num = fx1 * fx1 + 1.0;
		curvR[i] = (float)((num * sqrt(num)) / fx2);

		LogD() << "Index: " << i << ", r: " << curvR[i];
	}
	*/

	auto size = (int)input.size();
	float sizePerMM = (float)(size / rangeX);
	auto curvR = vector<float>(size, -1);
	auto dataX = vector<double>(size, -1);
	auto dataY = vector<double>(size, -1);
	auto fittY = vector<double>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i * resolutionX * 0.001;
		dataY[i] = input[i] * resolutionY * 0.001;
	}

	vector<double> coeffs;
	CppUtil::Regressor::polyFit(dataX, dataY, 2, coeffs);

	/*
	if (!CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		return false;
	}
	*/
	
	double fx1, fx2, num;
	for (int i = 0; i < size; i++) {
		fx1 = coeffs[2] * 2.0 * dataX[i] + coeffs[1];
		fx2 = 2.0 * coeffs[2];
		num = fx1 * fx1 + 1.0;
		curvR[i] = (float)((num * sqrt(num)) / fx2);

		// LogD() << "Index: " << i << ", r: " << curvR[i];
	}

	output = curvR;
	return true;
}



bool RetSegm::Coarse::makeBowmanLayerOfCornea(const SegmImage * imgSrc, const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& output)
{
	int minPosInn = 9999;
	int minIdxInn = 0;

	for (int i = 0; i < inner.size(); i++) {
		if (inner[i] >= 0 && inner[i] < minPosInn) {
			minPosInn = inner[i];
			minIdxInn = i;
		}
	}

	int minPosOut = outer[minIdxInn];
	int distInnOut = minPosOut - minPosInn;

	int startPos = 10;
	int closePos = distInnOut / 3;
	int startIdx = (int)(inner.size() * 0.25f);
	int closeIdx = (int)(inner.size() * 0.75f);

	Mat srcMat = imgSrc->getCvMatConst();

	int maxSum = 0, maxPos = 0;
	int gsum, grad, gpos;

	for (int r = startPos; r <= closePos; r++) {
		gsum = 0;
		for (int c = startIdx; c <= closeIdx; c++) {
			gpos = r + inner[c];
			if (gpos > 1 && gpos < (srcMat.rows-2)) {
				grad = srcMat.at<uchar>(gpos + 1, c) + srcMat.at<uchar>(gpos + 2, c)
					   - srcMat.at<uchar>(gpos - 1, c) - srcMat.at<uchar>(gpos - 2, c);
				gsum += grad;
			}
		}

		if (maxSum < gsum) {
			maxSum = gsum;
			maxPos = r;
		}
	}

	for (int c = 0; c < inner.size(); c++) {
		output[c] = inner[c] + maxPos;
	}

	return true;
}


bool RetSegm::Coarse::checkIfPointsAvailable(std::vector<int>& input, float sizeMin)
{
	int count = 0;
	for (int i = 0; i < input.size(); i++) {
		if (input[i] >= 0) {
			count++;
		}
	}
	if (count < (int)(input.size() * sizeMin)) {
		return false;
	}
	return true;
}


bool RetSegm::Coarse::interpolateBoundaryByLinearFitting(const std::vector<int>& input, std::vector<int>& output, bool sideFitt)
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
				if (!sideFitt) {
					for (int j = 0; j < i; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i; j < size; j++) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					for (j = j+1; j < size; j++) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						for (int k = (x1 - 1), dist = -1; k >= 0; k--, dist--) {
							output[k] = (int)(y1 + dist * slope);
							output[k] = max(0, output[k]);
						}
					}
				}
				break;
			}
		}
	}

	if (input[size - 1] < 0) {
		for (int i = size - 1; i >= 0; i--) {
			if (input[i] >= 0) {
				if (!sideFitt) {
					for (int j = i + 1; j < size; j++) {
						output[j] = input[i];
					}
				}
				else {
					int xs1 = 0, xs2 = 0;
					int ys1 = 0, ys2 = 0;
					int cnt1 = 0, cnt2 = 0;
					int j;

					for (j = i - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs2 += j;
							ys2 += input[j];
							if (++cnt2 >= fittSize) {
								x2 = xs2 / cnt2;
								y2 = ys2 / cnt2;
								break;
							}
						}
					}

					for (j = j - 1; j >= 0; j--) {
						if (input[j] >= 0) {
							xs1 += j;
							ys1 += input[j];
							if (++cnt1 >= fittSize) {
								x1 = xs1 / cnt1;
								y1 = ys1 / cnt1;
								break;
							}
						}
					}

					if (x1 >= 0 && x2 >= 0 && x2 > x1) {
						slope = (float)(y2 - y1) / (float)(x2 - x1);
						for (int k = (x2 + 1), dist = 1; k < size; k++, dist++) {
							output[k] = (int)(y2 + dist * slope);
							output[k] = max(0, output[k]);
						}
					}
				}
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


bool RetSegm::Coarse::interpolateBoundaryByCompare(std::vector<int>& input, std::vector<int>& output, bool upper)
{
	int size = (int)input.size();
	output = input;

	for (int c = 0; c < size; c++) {
		if (input[c] < 0) {
			int dat1 = -1, dat2 = -1;
			for (int k = c - 1; k >= 0; k--) {
				if (input[k] >= 0) {
					dat1 = input[k];
					break;
				}
			}
			for (int k = c + 1; k < size; k++) {
				if (input[k] >= 0) {
					dat2 = input[k];
					break;
				}
			}
			if (dat1 < 0 || dat2 < 0) {
				output[c] = max(dat1, dat2);
			}
			else {
				if (upper) {
					output[c] = min(dat1, dat2);
				}
				else {
					output[c] = max(dat1, dat2);
				}
			}
		}
	}
	return true;
}


bool RetSegm::Coarse::smoothBoundaryOfRetina(std::vector<int>& input, std::vector<int>& output, float filtSize)
{
	Mat msrc(1, (int)input.size(), CV_32SC1, input.data());
	Mat mdst(1, (int)output.size(), CV_32SC1, output.data());

	Size fsize = Size((int)(input.size() * filtSize), 1);
	blur(msrc, mdst, fsize);
	return true;
}


bool RetSegm::Coarse::createGradientMap(const SegmImage * imgSrc, SegmImage * imgOut)
{
	Mat srcMat = imgSrc->getCvMat();

	/*
	Mat sobelx, sobely;
	Sobel(matSrc, sobelx, CV_32F, 1, 0, 7);
	Sobel(matSrc, sobely, CV_32F, 0, 1, 7);

	Mat conv = abs(sobelx) + sobely;
	*/


	// If kernel height is too large(32x12), the close inner and outer retian peaks tends 
	// to be merged into a single peak, hard to identify each of them.
	// Nevertheless, the large kernel is necessary to extract meaningful edge information 
	// avoiding the background noise, also source image smoothing step could be skipped.

	// Kernel for gradient along y-direction.
	Mat kernel = Mat::ones(COARSE_Y_KERNEL_ROWS, COARSE_Y_KERNEL_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) = 2;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) /= (COARSE_Y_KERNEL_SIZE / 2);
		}
	}

	// Perform convolutions for each kernel.
	Mat conv;
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	// Considering x directional gradient results in widely scattered edges rather than building 
	// boundary estimation, particularly around the center of optic disk nerve head with steady change of curve. 
	/*
	// Kernel for gradient along x-direction.
	Mat kernel2 = Mat::ones(COARSE_GRADIENT_X_ROWS, COARSE_GRADIENT_X_COLS, CV_32F);
	for (int r = kernel2.rows/3; r < (kernel2.rows*2)/3; r++) {
	for (int c = 0; c < kernel2.cols; c++) {
	kernel2.at<float>(r, c) = 2;
	}
	}
	for (int r = 0; r < kernel2.rows; r++) {
	for (int c = 0; c < kernel2.cols/2; c++) {
	kernel2.at<float>(r, c) *= -1;
	}
	}
	for (int r = 0; r < kernel2.rows; r++) {
	for (int c = 0; c < kernel2.cols; c++) {
	kernel2.at<float>(r, c) /= (COARSE_GRADIENT_X_SIZE /2);
	}
	}

	Mat conv2;
	filter2D(srcMat, conv2, CV_32F, kernel2, Point(-1, -1), 0, BORDER_REFLECT);
	conv += abs(conv2);
	*/

	// Leave only the positive edge information (upper side edges). 
	Mat mask = conv < 0;
	conv.setTo(0, mask);

	// Normalize the convoluted values.
	double minVal, maxVal;
	minMaxLoc(conv, &minVal, &maxVal);

	Mat outMat;
	conv.convertTo(outMat, CV_8U, 255.0 / (maxVal - minVal), -minVal*255.0 / (maxVal - minVal));
	imgOut->getCvMat() = outMat;
	return true;
}


bool RetSegm::Coarse::buildBoundaryPairOfRetina(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> extra;

	locateBoundaryPeaksOfRetina(imgSrc, inner, outer, extra);
	correctFalseOuterPoints(inner, outer, extra);
	replaceIntrudedInnerPoints(inner, outer, extra);
	replaceIntrudedOuterPoints(inner, outer, extra);

	removeOuterFractions(outer);
	removeInnerFractions(inner);

	layerInn->initialize(inner, imgSrc->getWidth(), imgSrc->getHeight());
	layerOut->initialize(outer, imgSrc->getWidth(), imgSrc->getHeight());
	return true;
}


bool RetSegm::Coarse::locateBoundaryPeaksOfRetina(const SegmImage * srcImg, std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra)
{
	Mat srcMat = srcImg->getCvMat();
	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);
	extra.resize(srcMat.cols, -1);

	auto maxLocs = srcImg->getColumMaxLocs();
	auto maxVals = srcImg->getColumMaxVals();

	int r, c;
	for (c = 0; c < srcMat.cols; c++) {
		int maxIdx = maxLocs[c]; // pInMat->m_maxIdxOfCols.at<int>(0, c);
		int maxVal = maxVals[c]; // pInMat->m_maxValOfCols.at<int>(0, c);

		int preVal = maxVal;
		int curVal = 0;
		int uppIdx = -1;
		int uppVal = -1;
		int lowIdx = -1;
		int lowVal = -1;

		int rBeg, rEnd;
		int thVal;
		int ascCnt = 0;

		if (maxVal < COARSE_PEAK_LEVEL_MIN) {
			continue;
		}

		// Passing through the upper sided slope of maximum peak on the column, 
		// identify the next peak over threshold as a ILM. 
		rBeg = maxIdx - COARSE_DISTANCE_TO_PEAK_MAX;
		rBeg = (rBeg < 0 ? 0 : rBeg);
		thVal = (int)(maxVal * COARSE_PEAK_BASE_RATIO);

		// thVal = (thVal < (COARSE_PEAK_LEVEL_MIN / 2) ? (COARSE_PEAK_LEVEL_MIN / 2) : thVal);

		// This routine at first proceeds until the level becomes lower than threshold, 
		// then exits with the subsequent peak with larger than the given size (=3), 
		// but of which range is hard to be properly confined. 
		// It's better find the maximum level position over the threshold within searching range 
		// without considering the peak size. => it's not, since the found position might be a fall from the maximum peak.
		// the ascending of the next peak must be recognized. 

		// Proceed to the ascending side. 
		for (r = maxIdx - 1; r >= rBeg; r--) {
			// Search the upper end of the peak sliding down by threshold level.
			curVal = srcMat.at<uchar>(r, c);
			if (curVal > preVal) {
				if (++ascCnt >= COARSE_PEAK_HILL_SIZE) {
					for (; r >= rBeg; r--) {
						curVal = srcMat.at<uchar>(r, c);
						if (curVal >= COARSE_PEAK_ILM_LEVEL_MIN  && curVal >= uppVal) {
							uppVal = srcMat.at<uchar>(r, c);
							uppIdx = r;
						}
					}
					break;
				}
			}
			else {
				if (curVal < preVal) {
					ascCnt = 0;
				}
			}
			preVal = curVal;
		}

		// Passing through the lower downward slope of maximum peak on the column, 
		// indentify the next peak over threshold as a RPE. 
		preVal = maxVal;
		rEnd = maxIdx + COARSE_DISTANCE_TO_PEAK_MAX;
		rEnd = (rEnd >= srcMat.rows ? srcMat.rows - 1 : rEnd);

		// Proceed to the ascending side. 
		for (r = maxIdx + 1; r <= rEnd; r++) {
			// Search the lower end of the peak sliding down by threshold level.
			curVal = srcMat.at<uchar>(r, c);
			if (curVal > preVal) {
				if (++ascCnt >= COARSE_PEAK_HILL_SIZE) {
					for (; r <= rEnd; r++) {
						curVal = srcMat.at<uchar>(r, c);
						if (curVal >= COARSE_PEAK_RPE_LEVEL_MIN && curVal >= lowVal) {
							lowVal = srcMat.at<uchar>(r, c);
							lowIdx = r;
						}
					}
					break;
				}
			}
			else {
				if (curVal < preVal) {
					ascCnt = 0;
				}
			}
			preVal = curVal;
		}

		int innerPos = -1;
		int outerPos = -1;
		int extraPos = -1;

		// If the secondary peak found is trivial, use maximum peak as initial point of inner boundary.
		if (uppIdx >= 0 && lowIdx >= 0) {
			if (uppVal > lowVal) {
				innerPos = uppIdx;
				outerPos = maxIdx;
				extraPos = lowIdx;	// Extra peak is reserved for the correction of false identification.
			}
			else {
				innerPos = maxIdx;
				outerPos = lowIdx;
			}
		}
		else if (uppIdx >= 0) {
			innerPos = uppIdx;		// In case maximum peak belongs to the outer boundary.
			outerPos = maxIdx;
		}
		else if (lowIdx >= 0) {
			innerPos = maxIdx;		// In case maximum peak belongs to the inner boundary.
			outerPos = lowIdx;
		}
		else {
			innerPos = maxIdx;
		}

		/*
		// Relocate inner boundary point to the ascending position of the peak,
		// which makes the inner boundary points appear more continous along the ILM surface.
		for (r = innerPos; r > 0; r--) {
		if (srcMat.at<uchar>(r, c) <= COARSE_PEAK_ILM_LEVEL_MIN) {
		innerPos = r;
		break;
		}
		}
		*/

		inner[c] = innerPos;
		outer[c] = outerPos;
		extra[c] = extraPos;
	}
	return true;
}


bool RetSegm::Coarse::correctFalseOuterPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra)
{
	int outX1 = -1, outY1 = -1;
	int outX2 = -1, outY2 = -1;
	int outSize = 0;
	int none = 0;
	bool broken = false;
	bool tangled = false;

	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;

	int conX1 = -1, conY1 = -1;
	int conX2 = -1, conY2 = -1;
	int conSize1 = 0, conSize2 = 0;

	const int INNER_SEGM_SIZE = (int)(inner.size()*COARSE_INNER_SEGMENT_SIZE);
	const int OUTER_SEGM_SIZE = (int)(outer.size()*COARSE_OUTER_SEGMENT_SIZE);
	const int INNER_SECT_SIZE = (int)(inner.size()*COARSE_INNER_SECTION_SIZE);
	const int OUTER_SECT_SIZE = (int)(outer.size()*COARSE_OUTER_SECTION_SIZE);

	for (int c = 0, none = 0; c < outer.size(); c++) {
		if (outer[c] >= 0) {
			// Starting of an outer segment.
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// The current point is connected with the segment. 
				if (abs(outY2 - outer[c]) <= COARSE_OUTER_Y_SPAN) {
					outX2 = c; outY2 = outer[c];
					outSize++;
				}
				else {
					// If the current segment is large enough, it's passed then the next segment is started. 
					if (outSize >= OUTER_SECT_SIZE) {
						outX1 = c; outY1 = outer[c];
						outX2 = c; outY2 = outer[c];
						outSize = 1;
					}
					else {
						// else it's broken. 
						broken = true;
					}
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none >= OUTER_SEGM_SIZE) {
					none = 0;
					if (outSize < OUTER_SECT_SIZE) {
						broken = true;
					}
					else {
						outSize = 0;
					}
				}
			}
		}

		// The end of outer boundary reached. 
		if (c == outer.size() - 1) {
			if (outSize > 0 && outSize < OUTER_SECT_SIZE) {
				broken = true;
			}
		}

		if (broken) {
			broken = false;
			tangled = false;

			// Size of inner segment corresponding the broken outer segment. 
			innX1 = outX1; innY1 = inner[outX1];
			innX2 = outX2; innY2 = inner[outX2];
			innSize = innX2 - innX1 + 1;

			for (int i = innX1 - 1, none = 0; i >= 0; i--) {
				if (inner[i] >= 0) {
					none = 0;
					if (abs(innY1 - inner[i]) <= COARSE_INNER_Y_SPAN) {
						innX1 = i; innY1 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= INNER_SEGM_SIZE) {
						break;
					}
				}
			}

			for (int i = innX2 + 1, none = 0; i < inner.size(); i++) {
				if (inner[i] >= 0) {
					none = 0;
					if (abs(innY2 - inner[i]) <= COARSE_INNER_Y_SPAN) {
						innX2 = i; innY2 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= INNER_SEGM_SIZE) {
						break;
					}
				}
			}

			//DebugOut() << "Broken outer segment: " << outX1 << ", " << outX2 << ", corresponding inner: " << innX1 << ", " << innX2;

			// If inner segment over the current outer is also not large enough, 
			if (innSize < INNER_SECT_SIZE) {
				conSize1 = conSize2 = 0;

				//	Size of inner boundary interwinded on the left.
				for (int i = outX1 - 1, none = 0; i >= 0; i--) {
					if (inner[i] >= 0) {
						none = 0;
						if (tangled == false) {
							if (abs(outY1 - inner[i]) <= COARSE_OUTER_Y_SPAN) {
								conX1 = i; conY1 = inner[i];
								conX2 = i; conY2 = inner[i];
								conSize1 = 1;
								tangled = true;
							}
							else {
								break;
							}
						}
						else {
							if (abs(conY1 - inner[i]) <= COARSE_INNER_Y_SPAN) {
								conX1 = i; conY1 = inner[i];
								conSize1++;
							}
							else {
								break;
							}
						}
					}
					else {
						if (++none >= INNER_SEGM_SIZE) {
							break;
						}
					}
				}

				//	Size of inner boundary interwinded on the right.
				for (int i = outX2 + 1, none = 0; i < inner.size(); i++) {
					if (inner[i] >= 0) {
						none = 0;
						if (tangled == false) {
							if (abs(outY2 - inner[i]) <= COARSE_OUTER_Y_SPAN) {
								conX1 = i; conY1 = inner[i];
								conX2 = i; conY2 = inner[i];
								conSize2 = 1;
								tangled = true;
							}
							else {
								break;
							}
						}
						else {
							if (abs(conY2 - inner[i]) <= COARSE_INNER_Y_SPAN) {
								conX2 = i; conY2 = inner[i];
								conSize2++;
							}
							else {
								break;
							}
						}
					}
					else {
						if (++none >= INNER_SEGM_SIZE) {
							break;
						}
					}
				}

				//DebugOut() << "Interwinded with inner boundary by both sides, size1: " << conSize1 << ", size2: " << conSize2;

				if (tangled) {
					if ((conSize1 > outSize && conSize1 > INNER_SECT_SIZE) ||
						(conSize2 > outSize && conSize2 > INNER_SECT_SIZE)) {
						for (int i = outX1; i <= outX2; i++) {
							// continue;
							if (outer[i] >= 0) {
								inner[i] = outer[i];
								outer[i] = -1;
							}
							if (extra[i] >= 0) {
								outer[i] = extra[i];
							}
						}
						//DebugOut() << "Outer segment replaced in " << outX1 << " to " << outX2;
					}
				}
			}

			if (outer[c] >= 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				outSize = 0;
			}
		}
	}

	return true;
}


bool RetSegm::Coarse::replaceIntrudedInnerPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra)
{
	int outX1 = -1, outY1 = -1;
	int outX2 = -1, outY2 = -1;
	int outSize = 0;
	int none = 0;
	bool broken = false;
	bool replaced = false;

	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int conOutSpan = 1;
	int conInnSpan = 1;

	const int INNER_SEGM_SIZE = (int)(inner.size()*COARSE_INNER_SEGMENT_SIZE);
	const int OUTER_SEGM_SIZE = (int)(outer.size()*COARSE_OUTER_SEGMENT_SIZE);
	const int INNER_SECT_SIZE = (int)(inner.size()*COARSE_INNER_SECTION_SIZE);
	const int OUTER_SECT_SIZE = (int)(outer.size()*COARSE_OUTER_SECTION_SIZE);

	for (int c = 0, none = 0; c < outer.size(); c++) {
		if (outer[c] >= 0) {
			// An outer line gets started. 
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// A subsequent point should be within the span limit. 
				if (abs(outY2 - outer[c]) <= COARSE_OUTER_Y_SPAN) {
					outX2 = c; outY2 = outer[c];
					outSize++;
				}
				else {
					// If not, check whether a series of subsequent inner points are connected instead, 
					// and they might be actually outer points.
					if (inner[c] >= 0 && abs(outY2 - inner[c]) <= conOutSpan) {
						broken = true;
					}
					else {
						outX1 = c; outY1 = outer[c];
						outX2 = c; outY2 = outer[c];
						outSize = 1;
					}
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none <= OUTER_SEGM_SIZE) {
					if (inner[c] >= 0 && abs(outY2 - inner[c]) <= conOutSpan) {
						broken = true;
					}
				}
				else {
					outSize = 0;
					none = 0;
				}
			}
		}

		if (broken) {
			broken = false;

			innX1 = c; innY1 = inner[c];
			innX2 = c; innY2 = inner[c];
			innSize = 1;

			// Count the subsequent inner points interconnected with. 
			for (int i = innX2 + 1, none = 0; i < inner.size(); i++) {
				if (inner[i] >= 0) {
					none = 0;
					if (outer[i] >= 0 && abs(innY2 - outer[i]) < abs(innY2 - inner[i])) {
						break;
					}
					if (abs(innY2 - inner[i]) <= conInnSpan) {
						innX2 = i; innY2 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= OUTER_SEGM_SIZE) {
						break;
					}
				}
			}

			//DebugOut() << "Inner points intruded at " << c << ", x1: " << innX1 << ", x2: " << innX2 << ", size: " << innSize << ", outSize: " << outSize;

			if (innSize < INNER_SECT_SIZE && outSize >= OUTER_SECT_SIZE && innSize < outSize) {
				for (int i = innX1; i <= innX2; i++) {
					if (inner[i] >= 0) {
						outer[i] = inner[i];
					}
				}
				//DebugOut() << "Replaced innSize: " << innSize << ", outSize: " << outSize;
				outX2 = innX2; outY2 = innY2;
				outSize += innSize;
			}
			else {
				if (outer[c] >= 0) {
					outX1 = c; outY1 = outer[c];
					outX2 = c; outY2 = outer[c];
					outSize = 1;
				}
				else {
					outSize = 0;
				}
			}
		}
	}

	outSize = 0;
	for (int c = (int)outer.size() - 1, none = 0; c >= 0; c--) {
		if (outer[c] >= 0) {
			// An outer line gets started. 
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// A subsequent point should be within the span limit. 
				if (abs(outY1 - outer[c]) <= COARSE_OUTER_Y_SPAN) {
					outX1 = c; outY1 = outer[c];
					outSize++;
				}
				else {
					// If not, check whether a series of subsequent inner points are connected instead, 
					// and they might be actually outer points.
					if (inner[c] >= 0 && abs(outY1 - inner[c]) <= conOutSpan) {
						broken = true;
					}
					else {
						outX1 = c; outY1 = outer[c];
						outX2 = c; outY2 = outer[c];
						outSize = 1;
					}
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none <= OUTER_SEGM_SIZE) {
					if (inner[c] >= 0 && abs(outY1 - inner[c]) <= conOutSpan) {
						broken = true;
					}
				}
				else {
					outSize = 0;
					none = 0;
				}
			}
		}

		if (broken) {
			broken = false;

			innX1 = c; innY1 = inner[c];
			innX2 = c; innY2 = inner[c];
			innSize = 1;

			// Count the subsequent inner points interconnected with. 
			for (int i = innX1 - 1, none = 0; i >= 0; i--) {
				if (inner[i] >= 0) {
					none = 0;
					if (outer[i] >= 0 && abs(innY1 - outer[i]) < abs(innY1 - inner[i])) {
						break;
					}
					if (abs(innY1 - inner[i]) <= conInnSpan) {
						innX1 = i; innY1 = inner[i];
						innSize++;
					}
					else {
						break;
					}
				}
				else {
					if (++none >= OUTER_SEGM_SIZE) {
						break;
					}
				}
			}

			//DebugOut() << "Inner points intruded at " << c << ", x1: " << innX1 << ", x2: " << innX2 << ", size: " << innSize << ", outSize: " << outSize;

			if (innSize < INNER_SECT_SIZE && outSize >= OUTER_SECT_SIZE && innSize < outSize) {
				for (int i = innX1; i <= innX2; i++) {
					if (inner[i] >= 0) {
						outer[i] = inner[i];
					}
				}
				//DebugOut() << "Replaced innSize: " << innSize << ", outSize: " << outSize;
				outX1 = innX1; outY1 = innY1;
				outSize += innSize;
			}
			else {
				if (outer[c] >= 0) {
					outX1 = c; outY1 = outer[c];
					outX2 = c; outY2 = outer[c];
					outSize = 1;
				}
				else {
					outSize = 0;
				}
			}
		}
	}

	return true;
}


bool RetSegm::Coarse::replaceIntrudedOuterPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra)
{
	int outX1 = -1, outY1 = -1;
	int outX2 = -1, outY2 = -1;
	int outSize = 0;
	int none = 0;
	bool broken = false;
	bool tangled = false;

	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize1 = 0, innSize2 = 0, innSize = 0;
	int conOutSpan = 1;
	int conInnSpan = 1;

	const int INNER_SEGM_SIZE = (int)(inner.size()*COARSE_INNER_SEGMENT_SIZE);
	const int OUTER_SEGM_SIZE = (int)(outer.size()*COARSE_OUTER_SEGMENT_SIZE);
	const int INNER_SECT_SIZE = (int)(inner.size()*COARSE_INNER_SECTION_SIZE);
	const int OUTER_SECT_SIZE = (int)(outer.size()*COARSE_OUTER_SECTION_SIZE);

	for (int c = 0, none = 0; c < outer.size(); c++) {
		if (outer[c] >= 0) {
			// An outer line gets started. 
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// A subsequent point should be within the span limit. 
				if (abs(outY2 - outer[c]) <= COARSE_OUTER_Y_SPAN) {
					outX2 = c; outY2 = outer[c];
					outSize++;
				}
				else {
					if (outSize < OUTER_SEGM_SIZE) {
						broken = true;
					}
					else {
						outX1 = c; outY1 = outer[c];
						outX2 = c; outY2 = outer[c];
						outSize = 1;
					}
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none >= OUTER_SEGM_SIZE) {
					if (outSize < OUTER_SEGM_SIZE) {
						broken = true;
					}
					else {
						outSize = 0;
						none = 0;
					}
				}
			}
		}

		if (broken) {
			broken = false;
			tangled = false;

			innSize1 = innSize2 = 0;

			//	Size of inner boundary interwinded on the left.
			for (int i = outX1 - 1, none = 0; i >= 0; i--) {
				if (inner[i] >= 0) {
					none = 0;
					if (tangled == false) {
						if (abs(outY1 - inner[i]) <= conInnSpan) {
							innX1 = i; innY1 = inner[i];
							innX2 = i; innY2 = inner[i];
							innSize1 = 1;
							tangled = true;
						}
						else {
							break;
						}
					}
					else {
						if (abs(innY1 - inner[i]) <= conInnSpan) {
							innX1 = i; innY1 = inner[i];
							innSize1++;
						}
						else {
							break;
						}
					}
				}
				else {
					if (++none >= INNER_SEGM_SIZE) {
						break;
					}
				}
			}

			//	Size of inner boundary interwinded on the right.
			for (int i = outX2 + 1, none = 0; i < inner.size(); i++) {
				if (inner[i] >= 0) {
					none = 0;
					if (tangled == false) {
						if (abs(outY2 - inner[i]) <= conInnSpan) {
							innX1 = i; innY1 = inner[i];
							innX2 = i; innY2 = inner[i];
							innSize2 = 1;
							tangled = true;
						}
						else {
							break;
						}
					}
					else {
						if (abs(innY2 - inner[i]) <= conInnSpan) {
							innX2 = i; innY2 = inner[i];
							innSize2++;
						}
						else {
							break;
						}
					}
				}
				else {
					if (++none >= INNER_SEGM_SIZE) {
						break;
					}
				}
			}

			//DebugOut() << "Outer points intruded at " << c << ", x1: " << outX1 << ", x2: " << outX2 << ", size: " << outSize ;

			if (innSize1 >= INNER_SECT_SIZE || innSize2 >= INNER_SECT_SIZE) {
				for (int i = outX1; i <= outX2; i++) {
					if (outer[i] >= 0) {
						inner[i] = outer[i];
						outer[i] = -1;
					}
					if (extra[i] >= 0) {
						outer[i] = extra[i];
					}
				}
				//DebugOut() << "Replaced outSize: " << outSize << ", innSize1: " << innSize1 << ", innSize2: " << innSize2;
			}
			outSize = 0;
		}
	}
	return true;
}


bool RetSegm::Coarse::removeInnerFractions(std::vector<int>& inner)
{
	int innX1 = -1, innY1 = -1;
	int innX2 = -1, innY2 = -1;
	int innSize = 0;
	int preDiff = 0, curDiff = 0;
	int none = 0;

	const int INNER_SEGM_SIZE = (int)(inner.size()*COARSE_INNER_SEGMENT_SIZE);
	const int INNER_SECT_SIZE = (int)(inner.size()*COARSE_INNER_SECTION_SIZE);

	int emptyMax = INNER_SEGM_SIZE / 2;

	for (int c = 0, none = 0; c < inner.size(); c++) {
		if (inner[c] >= 0) {
			// An inner line gets started. 
			if (innSize == 0) {
				innX1 = c; innY1 = inner[c];
				innX2 = c; innY2 = inner[c];
				innSize = 1;
				preDiff = 0;
			}
			else {
				// A subsequent point should be within the span limit. 
				// If the difference between the previous segment's end and the current's start has the same 
				// sign (direction in change) with the 
				curDiff = innY2 - inner[c];
				if (abs(curDiff - preDiff) <= COARSE_INNER_Y_SPAN) {
					innX2 = c; innY2 = inner[c];
					innSize++;
				}
				else {
					if (innSize < INNER_SEGM_SIZE) {
						for (int i = innX1; i <= innX2; i++) {
							inner[i] = -1;
						}
					}
					innX1 = c; innY1 = inner[c];
					innX2 = c; innY2 = inner[c];
					innSize = 1;
				}
				preDiff = curDiff;
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (innSize > 0) {
				if (++none >= emptyMax) {
					if (innSize < INNER_SEGM_SIZE) {
						for (int i = innX1; i <= innX2; i++) {
							inner[i] = -1;
						}
					}
					innSize = 0;
					none = 0;
				}
			}
		}
	}

	if (innSize > 0 && innSize < INNER_SEGM_SIZE) {
		for (int i = innX1; i <= innX2; i++) {
			inner[i] = -1;
		}
	}
	return false;
}


bool RetSegm::Coarse::removeOuterFractions(std::vector<int>& outer)
{
	int outX1 = -1, outY1 = -1;
	int outX2 = -1, outY2 = -1;
	int outSize = 0;
	int none = 0;

	const int OUTER_SEGM_SIZE = (int)(outer.size()*COARSE_OUTER_SEGMENT_SIZE);
	const int OUTER_SECT_SIZE = (int)(outer.size()*COARSE_OUTER_SECTION_SIZE);

	int emptyMax = OUTER_SEGM_SIZE / 2;

	for (int c = 0, none = 0; c < outer.size(); c++) {
		if (outer[c] >= 0) {
			// An outer line gets started. 
			if (outSize == 0) {
				outX1 = c; outY1 = outer[c];
				outX2 = c; outY2 = outer[c];
				outSize = 1;
			}
			else {
				// A subsequent point should be within the span limit. 
				if (abs(outY2 - outer[c]) <= COARSE_OUTER_Y_SPAN) {
					outX2 = c; outY2 = outer[c];
					outSize++;
				}
				else {
					if (outSize < OUTER_SEGM_SIZE) {
						for (int i = outX1; i <= outX2; i++) {
							outer[i] = -1;
						}
					}
					outX1 = c; outY1 = outer[c];
					outX2 = c; outY2 = outer[c];
					outSize = 1;
				}
			}
			none = 0;
		}
		else {
			// Attempt to connect to a subsequent outer point to the size limit.
			if (outSize > 0) {
				if (++none >= emptyMax) {
					if (outSize < OUTER_SEGM_SIZE) {
						for (int i = outX1; i <= outX2; i++) {
							outer[i] = -1;
						}
					}
					outSize = 0;
					none = 0;
				}
			}
		}
	}

	if (outSize > 0 && outSize < OUTER_SEGM_SIZE) {
		for (int i = outX1; i <= outX2; i++) {
			outer[i] = -1;
		}
	}
	return false;
}


bool RetSegm::Coarse::createGradientMap(ImageMat& srcImg, ImageMat& outImg)
{
	Mat srcMat = srcImg.getCvMat();

	/*
	Mat sobelx, sobely;
	Sobel(matSrc, sobelx, CV_32F, 1, 0, 7);
	Sobel(matSrc, sobely, CV_32F, 0, 1, 7);

	Mat conv = abs(sobelx) + sobely;
	*/


	// If kernel height is too large(32x12), the close inner and outer retian peaks tends 
	// to be merged into a single peak, hard to identify each of them.
	// Nevertheless, the large kernel is necessary to extract meaningful edge information 
	// avoiding the background noise, also source image smoothing step could be skipped.

	// Kernel for gradient along y-direction.
	Mat kernel = Mat::ones(COARSE_GRADIENT_Y_ROWS, COARSE_GRADIENT_Y_COLS, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols/3; c < (kernel.cols*2)/3; c++) {
			kernel.at<float>(r, c) = 2;
		}
	}
	for (int r = 0; r < kernel.rows/2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1;
		}
	}
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) /= (COARSE_GRADIENT_Y_SIZE/2);
		}
	}

	// Perform convolutions for each kernel.
	Mat conv;
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REFLECT);

	// Considering x directional gradient results in widely scattered edges rather than building 
	// boundary estimation, particularly around the center of optic disk nerve head with steady change of curve. 
	/*
	// Kernel for gradient along x-direction.
	Mat kernel2 = Mat::ones(COARSE_GRADIENT_X_ROWS, COARSE_GRADIENT_X_COLS, CV_32F);
	for (int r = kernel2.rows/3; r < (kernel2.rows*2)/3; r++) {
		for (int c = 0; c < kernel2.cols; c++) {
			kernel2.at<float>(r, c) = 2;
		}
	}
	for (int r = 0; r < kernel2.rows; r++) {
		for (int c = 0; c < kernel2.cols/2; c++) {
			kernel2.at<float>(r, c) *= -1;
		}
	}
	for (int r = 0; r < kernel2.rows; r++) {
		for (int c = 0; c < kernel2.cols; c++) {
			kernel2.at<float>(r, c) /= (COARSE_GRADIENT_X_SIZE /2);
		}
	}

	Mat conv2;
	filter2D(srcMat, conv2, CV_32F, kernel2, Point(-1, -1), 0, BORDER_REFLECT);
	conv += abs(conv2);
	*/

	// Leave only the positive edge information (upper side edges). 
	Mat mask = conv < 0;
	conv.setTo(0, mask);

	// Normalize the convoluted values.
	double minVal, maxVal;
	minMaxLoc(conv, &minVal, &maxVal);

	Mat outMat;
	conv.convertTo(outMat, CV_8U, 255.0 / (maxVal - minVal), -minVal*255.0 / (maxVal - minVal));
	outImg.getCvMat() = outMat;
	return true;
}


bool RetSegm::Coarse::buildBoundaryPairOfRetina(ImageMat& srcImg, Boundary& innerBound, Boundary& outerBound, Boundary& boundRPE)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> extra;
	
	locateBoundaryPointsOfRetina(srcImg, inner, outer, extra);

	correctFalseOuterPoints(inner, outer, extra);
	replaceIntrudedInnerPoints(inner, outer, extra);
	replaceIntrudedOuterPoints(inner, outer, extra);

	removeOuterFractions(outer);
	removeInnerFractions(inner);

	innerBound.createPoints(inner, srcImg.getWidth(), srcImg.getHeight());
	outerBound.createPoints(outer, srcImg.getWidth(), srcImg.getHeight());
	return true;
}




bool RetSegm::Coarse::locateBoundaryPointsOfRetina(ImageMat & srcImg, std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra)
{
	Mat srcMat = srcImg.getCvMat();
	inner.resize(srcMat.cols, -1);
	outer.resize(srcMat.cols, -1);
	extra.resize(srcMat.cols, -1);

	int r, c;
	for (c = 0; c < srcMat.cols; c++) {
		int maxIdx = srcImg.m_maxIdxCols[c]; // pInMat->m_maxIdxOfCols.at<int>(0, c);
		int maxVal = srcImg.m_maxValCols[c]; // pInMat->m_maxValOfCols.at<int>(0, c);

		int preVal = maxVal;
		int curVal = 0;
		int uppIdx = -1;
		int uppVal = -1;
		int lowIdx = -1;
		int lowVal = -1;

		int rBeg, rEnd;
		int thVal;
		int ascCnt = 0;

		if (maxVal < COARSE_PEAK_LEVEL_MIN) {
			continue;
		}

		// Passing through the upper downward slope of maximum peak on the column, 
		// identify the next peak over threshold as a ILM. 
		rBeg = maxIdx - COARSE_DISTANCE_TO_PEAK_MAX;
		rBeg = (rBeg < 0 ? 0 : rBeg);
		thVal = (int)(maxVal * COARSE_PEAK_BASE_RATIO);

		// thVal = (thVal < (COARSE_PEAK_LEVEL_MIN / 2) ? (COARSE_PEAK_LEVEL_MIN / 2) : thVal);

		// This routine at first proceeds until the level becomes lower than threshold, 
		// then exits with the subsequent peak with larger than the given size (=3), 
		// but of which range is hard to be properly confined. 
		// It's better find the maximum level position over the threshold within searching range 
		// without considering the peak size. => it's not, since the found position might be a fall from the maximum peak.
		// the ascending of the next peak must be recognized. 

		// Proceed to the ascending side. 
		for (r = maxIdx - 1; r >= rBeg; r--) {
			// Search the upper end of the peak sliding down by threshold level.
			curVal = srcMat.at<uchar>(r, c);
			if (curVal > preVal) {
				if (++ascCnt >= COARSE_PEAK_HILL_SIZE) {
					for (; r >= rBeg; r--) {
						curVal = srcMat.at<uchar>(r, c);
						if (curVal >= COARSE_PEAK_ILM_LEVEL_MIN  && curVal >= uppVal) {
							uppVal = srcMat.at<uchar>(r, c);
							uppIdx = r;
						}
					}
					break;
				}
			}
			else {
				if (curVal < preVal) {
					ascCnt = 0;
				}
			}
			preVal = curVal;
		}

		// Passing through the lower downward slope of maximum peak on the column, 
		// indentify the next peak over threshold as a RPE. 
		preVal = maxVal;
		rEnd = maxIdx + COARSE_DISTANCE_TO_PEAK_MAX;
		rEnd = (rEnd >= srcMat.rows ? srcMat.rows - 1 : rEnd);

		// Proceed to the ascending side. 
		for (r = maxIdx + 1; r <= rEnd; r++) {
			// Search the lower end of the peak sliding down by threshold level.
			curVal = srcMat.at<uchar>(r, c);
			if (curVal > preVal) {
				if (++ascCnt >= COARSE_PEAK_HILL_SIZE) {
					for (; r <= rEnd; r++) {
						curVal = srcMat.at<uchar>(r, c);
						if (curVal >= COARSE_PEAK_RPE_LEVEL_MIN && curVal >= lowVal) {
							lowVal = srcMat.at<uchar>(r, c);
							lowIdx = r;
						}
					}
					break;
				}
			}
			else {
				if (curVal < preVal) {
					ascCnt = 0;
				}
			}
			preVal = curVal;
		}

		int innerPos = -1;
		int outerPos = -1;
		int extraPos = -1;

		// If the secondary peak found is trivial, use maximum peak as initial point of inner boundary.
		if (uppIdx >= 0 && lowIdx >= 0) {
			if (uppVal > lowVal) {
				innerPos = uppIdx;
				outerPos = maxIdx;
				extraPos = lowIdx;	// Extra peak is reserved for the correction of false identification.
			}
			else {
				innerPos = maxIdx;
				outerPos = lowIdx;
			}
		}
		else if (uppIdx >= 0) {
			innerPos = uppIdx;		// In case maximum peak belongs to the outer boundary.
			outerPos = maxIdx;
		}
		else if (lowIdx >= 0) {
			innerPos = maxIdx;		// In case maximum peak belongs to the inner boundary.
			outerPos = lowIdx;
		}
		else {
			innerPos = maxIdx;
		}

		/*
		// Relocate inner boundary point to the ascending position of the peak, 
		// which makes the inner boundary points appear more continous along the ILM surface. 
		for (r = innerPos; r > 0; r--) {
			if (srcMat.at<uchar>(r, c) <= COARSE_PEAK_ILM_LEVEL_MIN) {
				innerPos = r;
				break;
			}
		}
		*/

		inner[c] = innerPos;
		outer[c] = outerPos;
		extra[c] = extraPos;
	}
	return true;
}


bool RetSegm::Coarse::interpolateBoundaryByLinearFitting(Boundary & inBound, Boundary & outBound)
{
	outBound = inBound;
	vector<EdgePoint> edges = inBound.getPoints();

	int lastX = -1, lastY = -1;
	int nextX = -1, nextY = -1;
	
	for (int i = 0; i < edges.size(); i++) {
		if (edges[i].isValid()) {
			lastX = i;
			lastY = edges[i].getY();
		}
		else {
			if (lastX < 0) {
				// Interpolate the empty boundary on the image left margin. 
				for (int j = i + 1; j < edges.size(); j++) {
					if (edges[j].isValid()) {
						lastX = j;
						lastY = edges[j].getY();
						for (int k = 0; k < j; k++) {
							outBound.setPoint(k, k, lastY);
						}
						i = lastX;
						break;
					}
				}
			}
			else {
				nextX = nextY = -1;
				for (int j = i + 1; j < edges.size(); j++) {
					if (edges[j].isValid()) {
						nextX = j;
						nextY = edges[j].getY();
						break;
					}
				}

				if (nextX < 0) {
					// Interpolate the empty boundary on the image right margin. 
					for (int k = (lastX+1); k < edges.size(); k++) {
						outBound.setPoint(k, k, lastY);
					}
					break;	// Exit loop.
				}
				else {
					float slope = (float)(nextY - lastY) / (float)(nextX - lastX);
					int devX = 1;

					// Linear interpolation at the empty hole among boundary.
					for (int k = i; k < nextX; k++, devX++) {
						nextY = (int)(lastY + devX * slope);
						outBound.setPoint(k, k, nextY);
					}
					i = nextX;
				}
			}
		}
	}
	return true;
}

bool RetSegm::Coarse::calculateCurvatureOfCornea(float resolutionX, float resolutionY, std::vector<int>& layerEPI, std::vector<int>& layerBOW, std::vector<int>& layerEND, std::vector<int>& layerEpiForEpi, std::vector<int>& layerBowForEpi,
	std::vector<float>& axialAnteriorRadius, std::vector<float>& axialAnteriorRadiusSimK, std::vector<float>& axialPosteriorRadius, std::vector<float>& tangentialAnteriorRadius, std::vector<float>& tangentialPosteriorRadius,
	std::vector<float>& refractivePowerKerato, std::vector<float>& refractivePowerAnterior, std::vector<float>& refractivePowerPosterior, std::vector<float>& refractivePowerTotal,
	std::vector<float>& netMap, std::vector<float>& axialTrueNet, std::vector<float>& equivalentKeratometer, std::vector<float>& elevationAnterior,
	std::vector<float>& elevationPosterior, std::vector<float>& height, std::vector<float>& pachymetry, std::vector<float>& epithelium,
	std::vector<double>& incidenceAngle, int bscanIndex, bool modelEye)
{
	float blank = -999.f;
	auto size = (int)layerEPI.size();
	auto dataX = vector<double>(size, blank);
	auto dataEPI = vector<double>(size, blank);
	auto dataBOW = vector<double>(size, blank);
	auto dataEND = vector<double>(size, blank);
	auto offsets = vector<double>(size, blank);
	auto dataEpiForEpi = vector<double>(size, blank);
	auto dataBowForEpi = vector<double>(size, blank);

	for (int i = 0; i < size; i++) {
		dataX[i] = i * resolutionX * 0.001;
		dataEPI[i] = layerEPI[i] * resolutionY * 0.001;
		dataBOW[i] = layerBOW[i] * resolutionY * 0.001;
		dataEND[i] = layerEND[i] * resolutionY * 0.001;
		if (!modelEye) {
			dataEpiForEpi[i] = layerEpiForEpi[i] * resolutionY * 0.001;
			dataBowForEpi[i] = layerBowForEpi[i] * resolutionY * 0.001;
		}
	}

	// Read Offset data
	offsets = GlobalSettings::topoCalibrationData(bscanIndex);

	vector<double> coeffs1;
	vector<double> coeffs2;
	vector<double> coeffs3;
	vector<double> coeffs4;
	vector<double> coeffs5;

	CppUtil::Regressor::polyFit(dataX, dataEPI, 6, coeffs1);
	CppUtil::Regressor::polyFit(dataX, dataBOW, 6, coeffs2);
	CppUtil::Regressor::polyFit(dataX, dataEND, 6, coeffs3);
	if (!modelEye) {
		CppUtil::Regressor::polyFit(dataX, dataEpiForEpi, 6, coeffs4);
		CppUtil::Regressor::polyFit(dataX, dataBowForEpi, 6, coeffs5);
	}

	for (int i = 0; i < size; i++) {
		dataEPI[i] = coeffs1[6] * pow(dataX[i], 6) + coeffs1[5] * pow(dataX[i], 5) + coeffs1[4] * pow(dataX[i], 4) + coeffs1[3] * pow(dataX[i], 3) + coeffs1[2] * pow(dataX[i], 2) + coeffs1[1] * dataX[i] + coeffs1[0] - offsets[i];
		dataBOW[i] = coeffs2[6] * pow(dataX[i], 6) + coeffs2[5] * pow(dataX[i], 5) + coeffs2[4] * pow(dataX[i], 4) + coeffs2[3] * pow(dataX[i], 3) + coeffs2[2] * pow(dataX[i], 2) + coeffs2[1] * dataX[i] + coeffs2[0] - offsets[i];
		dataEND[i] = coeffs3[6] * pow(dataX[i], 6) + coeffs3[5] * pow(dataX[i], 5) + coeffs3[4] * pow(dataX[i], 4) + coeffs3[3] * pow(dataX[i], 3) + coeffs3[2] * pow(dataX[i], 2) + coeffs3[1] * dataX[i] + coeffs3[0] - offsets[i];
		if (!modelEye) {
			dataEpiForEpi[i] = coeffs4[6] * pow(dataX[i], 6) + coeffs4[5] * pow(dataX[i], 5) + coeffs4[4] * pow(dataX[i], 4) + coeffs4[3] * pow(dataX[i], 3) + coeffs4[2] * pow(dataX[i], 2) + coeffs4[1] * dataX[i] + coeffs4[0] - offsets[i];
			dataBowForEpi[i] = coeffs5[6] * pow(dataX[i], 6) + coeffs5[5] * pow(dataX[i], 5) + coeffs5[4] * pow(dataX[i], 4) + coeffs5[3] * pow(dataX[i], 3) + coeffs5[2] * pow(dataX[i], 2) + coeffs5[1] * dataX[i] + coeffs5[0] - offsets[i];
		}
	}

	int centerIndex = 0;
	getCurvatureSimK(dataX, dataEPI, axialAnteriorRadiusSimK);

	getCurvature(dataX, dataEPI, tangentialAnteriorRadius, axialAnteriorRadius, incidenceAngle, centerIndex, 256, true);

	if (centerIndex < 0 || centerIndex > size - 1)
		return false;

	auto dewarpX = vector<double>(size, -1);
	dewarpingCornea(dataX, dewarpX, dataEPI, dataEND, incidenceAngle);
	getCurvature(dewarpX, dataEND, tangentialPosteriorRadius, axialPosteriorRadius, incidenceAngle, centerIndex, 32, false);

	calculateThickness(dataX, dewarpX, dataEPI, dataEND, incidenceAngle, pachymetry);
	calculateElevation(dataX, dataEPI, elevationAnterior);
	calculateElevation(dewarpX, dataEND, elevationPosterior);

	for (int i = 0; i < size; i++) {
		auto distance = dataX[centerIndex] - dataX[i];
		double theta2 = asin(sin(incidenceAngle[i]) / CORNEA_REFLECTIVE_INDEX);
		double phi = (incidenceAngle[i] - theta2);
		double theta3 = asin(CORNEA_REFLECTIVE_INDEX * sin(theta2) / CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX);
		double theta4 = CV_PI / 2.f - phi - theta2 + theta3;

		// refractive Power
		auto anteriorFocalLength = distance / cos(CV_PI / 2.f - phi) * cos(phi);//cos(CV_PI / 2.f - theta) * axialAnt[i];
		refractivePowerAnterior[i] = CORNEA_REFLECTIVE_INDEX / anteriorFocalLength * 1000;
		refractivePowerKerato[i] = CORNEA_KERATO_REFLECTIVE_INDEX / anteriorFocalLength * 1000;

		auto focalR = distance / cos(theta4);
		auto focal1 = abs(dataEPI[i] - dataEND[i]);
		auto focal2 = abs(focalR * cos(CV_PI / 2.f - theta4));
		auto totalFocalLength = focal1 + focal2;
		refractivePowerPosterior[i] = CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX / focal2 * 1000;
		refractivePowerTotal[i] = CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX / totalFocalLength * 1000; //CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX / focalLength * 1000;
		
		// Net Power
		if (axialAnteriorRadius[i] != blank &&  axialPosteriorRadius[i] != blank) {
			auto kAnt = (CORNEA_REFLECTIVE_INDEX - 1) / axialAnteriorRadius[i];
			auto kPost = (CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX - CORNEA_REFLECTIVE_INDEX) / axialPosteriorRadius[i];
			netMap[i] = (kAnt + kPost) * 1000;
			axialTrueNet[i] = (kAnt + kPost - ((pachymetry[i] / 1000.f) * kAnt * kPost)) * 1000;

			// original EKR fomula.
			// EKR = (1.376 - 1) / anteriorRadius + (1.3375 - 1) * (1 - 1 / R2) * R1 / posteriorRadius.
			// R1 : The ratio of posterior and anterior curvature from an untreated cornea = 0.822.
			// R2 : The ratio of simulated keratometry and anterior corneal power = 0.8976.
			equivalentKeratometer[i] = 376.0f / axialAnteriorRadius[i] - 31.65f / axialPosteriorRadius[i];
		}
		
		// min = highest point of the cornea.
		auto min = *min_element(dataEPI.begin(), dataEPI.end());
		height[i] = abs(dataEPI[i] - min) * 1000;
	}

	correctionPeripheral(refractivePowerAnterior, 30.f, 67.5f, 192);
	correctionPeripheral(refractivePowerKerato, 30.f, 67.5f, 192);
	correctionPeripheral(refractivePowerPosterior, 30.f, 67.5f, 192);
	correctionPeripheral(refractivePowerTotal, 30.f, 67.5f, 192);

	correctionCenter(refractivePowerAnterior, centerIndex, 64);
	correctionCenter(refractivePowerKerato, centerIndex, 64);
	correctionCenter(refractivePowerPosterior, centerIndex, 64);
	correctionCenter(refractivePowerTotal, centerIndex, 64);


	
	if (!modelEye) {
		dewarpingCornea(dataX, dewarpX, dataEpiForEpi, dataBowForEpi, incidenceAngle);
		calculateThickness(dataX, dewarpX, dataEpiForEpi, dataBowForEpi, incidenceAngle, epithelium);
	}
	else {
		dewarpingCornea(dataX, dewarpX, dataEPI, dataBOW, incidenceAngle);
		calculateThickness(dataX, dewarpX, dataEPI, dataBOW, incidenceAngle, epithelium);
	}

	correctionPeripheral(pachymetry, 150.f, 775.f, 192);
	correctionPeripheral(epithelium, 30.f, 70.f, 192);

	return true;
}

void  RetSegm::Coarse::getCurvature(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& tangentialRadius, std::vector<float>& axialRadius, std::vector<double>& incidenceAngle, int& axialX, int step, bool isAnterior)
{
	int size = dataX.size();
	int centerIndex = 0;
	int margin = 192;
	float blank = -999.f;
	float minR = 4.f;
	float maxR = 13.f;

	std::vector<float> tangentialR(size, blank);
	std::vector<float> axialR(size, blank);
	std::vector<float> vecA(size, blank);
	std::vector<float> vecB(size, blank);
	std::vector<cv::Point2f>vecInput(size, cv::Point2f(0, 0));

	std::vector<double> lightAngle(size, 1);
	std::vector<float> absoluteAngle(size, 1);

	cv::Point2f pplus, pminus;
	cv::Point2f f1stDerivative, f2ndDerivative;

	for (int i = 0; i < size; i++) {
		vecInput[i].x = dataX[i];
		vecInput[i].y = dataY[i];
	}

	if (vecInput.size() < step)
		return;

	auto frontToBack = vecInput.front() - vecInput.back();
	bool isClosed = ((int)std::max(std::abs(frontToBack.x), std::abs(frontToBack.y))) <= 1;

	// tangential Radius
	for (int i = 1; i < size - 1; i++)
	{
		const cv::Point2f& pos = vecInput[i];

		int maxStep = step;
		if (!isClosed)
		{
			maxStep = std::min(std::min(step, i), (int)vecInput.size() - 1 - i);
			if (maxStep == 0)
			{
				tangentialR[i] = blank;
				continue;
			}
		}

		int iminus = i - maxStep;
		int iplus = i + maxStep;
		pminus = vecInput[iminus < 0 ? iminus + vecInput.size() : iminus];
		pplus = vecInput[iplus > vecInput.size() ? iplus - vecInput.size() : iplus];

		f1stDerivative.x = (pplus.x - pminus.x) / (iplus - iminus);
		f1stDerivative.y = (pplus.y - pminus.y) / (iplus - iminus);
		f2ndDerivative.x = (pplus.x - 2 * pos.x + pminus.x) / ((iplus - iminus) / 2 * (iplus - iminus) / 2);
		f2ndDerivative.y = (pplus.y - 2 * pos.y + pminus.y) / ((iplus - iminus) / 2 * (iplus - iminus) / 2);

		double curvature2D;
		double divisor = f1stDerivative.x * f1stDerivative.x + f1stDerivative.y * f1stDerivative.y;
		if (std::abs(divisor) > 10e-8)
		{
			curvature2D = pow(divisor, 3.0 / 2.0) /
				std::abs(f2ndDerivative.y * f1stDerivative.x - f2ndDerivative.x * f1stDerivative.y);
		}
		else
		{
			curvature2D = blank;
		}

		if (isnan(curvature2D))
			curvature2D = blank;

		tangentialR[i] = curvature2D;

		auto d1 = (pos.x - pminus.x) / (pos.y - pminus.y);
		auto d2 = (pplus.x - pos.x) / (pplus.y - pos.y);

		auto cx = ((pplus.y - pminus.y) + (pos.x + pplus.x) * d2 - (pminus.x + pos.x) * d1) / (2 * (d2 - d1));
		auto cy = -d1 * (cx - (pminus.x + pos.x) / 2) + (pminus.y + pos.y) / 2;

		vecA[i] = (cy - pos.y) / (cx - pos.x);
		vecB[i] = pos.y - vecA[i] * pos.x;

		lightAngle[i] = CV_PI / 2.f - atan2f((cy - pos.y), (cx - pos.x));
		absoluteAngle[i] = fabs(lightAngle[i]);

		// finding center
		/*if (i > 448 && i< 576){
			if (cx < pos.x && centerIndex == 0) {
				centerIndex = i;
			}
		}*/
	}

	int minAngelIndex = min_element(absoluteAngle.begin(), absoluteAngle.end()) - absoluteAngle.begin();
	if (minAngelIndex > 448 && minAngelIndex < 576) {
		centerIndex = minAngelIndex;
	}

	// axial Radius
	for (int i = 0; i < size; i++)
	{
		auto axialY = vecA[i]*dataX[centerIndex] + vecB[i];
		auto axialRadius = sqrt(pow(dataX[centerIndex] - dataX[i], 2) + pow(axialY - dataY[i], 2));
		if (isnan(axialRadius))
			axialRadius = blank;
		axialR[i] = axialRadius;
	}

	// shifting center
	if (centerIndex > size / 2) {
		int padding = centerIndex - size / 2;
		tangentialR.erase(tangentialR.begin(), tangentialR.begin()+padding);
		axialR.erase(axialR.begin(), axialR.begin() + padding);

		for (int i = 0; i < padding; i++) {
			tangentialR.push_back(blank);
			axialR.push_back(blank);
		}
	}
	else if (centerIndex < size / 2) {
		int padding = size / 2 - centerIndex;

		for (int i = 0; i < padding; i++) {
			tangentialR.insert(tangentialR.begin(), blank);
			axialR.insert(axialR.begin(), blank);

			tangentialR.pop_back();
			axialR.pop_back();
		}
	}

	correctionCenter(axialR, centerIndex, 64);

	if (isAnterior) {
		axialX = centerIndex;
		incidenceAngle = lightAngle;
	}

	correctionPeripheral(axialR, minR, maxR, margin);
	correctionPeripheral(tangentialR, minR, maxR, margin);

	tangentialRadius = tangentialR;
	axialRadius = axialR;

	return;
}

void  RetSegm::Coarse::getCurvatureSimK(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& axialRadius)
{
	int step = 192;
	int size = dataX.size();
	int centerIndex = 0;
	int margin = 192;
	float blank = -999.f;
	float minR = 4.f;
	float maxR = 13.f;

	std::vector<float> axialR(size, blank);
	std::vector<float> vecA(size, blank);
	std::vector<float> vecB(size, blank);
	std::vector<cv::Point2f>vecInput(size, cv::Point2f(0, 0));

	std::vector<double> lightAngle(size, 1);
	std::vector<float> absoluteAngle(size, 1);

	cv::Point2f pplus, pminus;
	cv::Point2f f1stDerivative, f2ndDerivative;

	for (int i = 0; i < size; i++) {
		vecInput[i].x = dataX[i];
		vecInput[i].y = dataY[i];
	}

	if (vecInput.size() < step)
		return;

	auto frontToBack = vecInput.front() - vecInput.back();
	bool isClosed = ((int)std::max(std::abs(frontToBack.x), std::abs(frontToBack.y))) <= 1;

	// tangential Radius
	for (int i = 1; i < size - 1; i++)
	{
		const cv::Point2f& pos = vecInput[i];

		int maxStep = step;
		if (!isClosed)
		{
			maxStep = std::min(std::min(step, i), (int)vecInput.size() - 1 - i);
			if (maxStep == 0)
			{
				continue;
			}
		}

		int iminus = i - maxStep;
		int iplus = i + maxStep;
		pminus = vecInput[iminus < 0 ? iminus + vecInput.size() : iminus];
		pplus = vecInput[iplus > vecInput.size() ? iplus - vecInput.size() : iplus];

		auto d1 = (pos.x - pminus.x) / (pos.y - pminus.y);
		auto d2 = (pplus.x - pos.x) / (pplus.y - pos.y);

		auto cx = ((pplus.y - pminus.y) + (pos.x + pplus.x) * d2 - (pminus.x + pos.x) * d1) / (2 * (d2 - d1));
		auto cy = -d1 * (cx - (pminus.x + pos.x) / 2) + (pminus.y + pos.y) / 2;

		vecA[i] = (cy - pos.y) / (cx - pos.x);
		vecB[i] = pos.y - vecA[i] * pos.x;

		lightAngle[i] = CV_PI / 2.f - atan2f((cy - pos.y), (cx - pos.x));
		absoluteAngle[i] = fabs(lightAngle[i]);

		// finding center
		/*if (i > 448 && i< 576){
			if (cx < pos.x && centerIndex == 0) {
				centerIndex = i;
			}
		}*/
	}

	int minAngelIndex = min_element(absoluteAngle.begin(), absoluteAngle.end()) - absoluteAngle.begin();
	if (minAngelIndex > 448 && minAngelIndex < 576) {
		centerIndex = minAngelIndex;
	}
	else {
		centerIndex = size / 2;
	}

	// axial Radius
	for (int i = 0; i < size; i++)
	{
		auto axialY = vecA[i] * dataX[centerIndex] + vecB[i];
		auto axialRadius = sqrt(pow(dataX[centerIndex] - dataX[i], 2) + pow(axialY - dataY[i], 2));
		if (isnan(axialRadius))
			axialRadius = blank;
		axialR[i] = axialRadius;
	}

	// shifting center
	/*if (centerIndex > size / 2) {
		int padding = centerIndex - size / 2;
		axialR.erase(axialR.begin(), axialR.begin() + padding);

		for (int i = 0; i < padding; i++) {
			axialR.push_back(blank);
		}
	}
	else if (centerIndex < size / 2) {
		int padding = size / 2 - centerIndex;

		for (int i = 0; i < padding; i++) {
			axialR.insert(axialR.begin(), blank);
			axialR.pop_back();
		}
	}
	axialR[0] = centerIndex;*/
	axialRadius = axialR;

	return;


}

void RetSegm::Coarse::correctionPeripheral(std::vector<float>&curveR, float minR, float maxR, int ofsset)
{
	int size = curveR.size();

	float blank = -999.f;
	for (int i = 0; i < ofsset; i++) {
		if (curveR[i] < minR || curveR[i] > maxR) {
			curveR[i] = blank;
		}

	}
	for (int i = size - ofsset; i < size; i++) {
		if (curveR[i] < minR || curveR[i] > maxR) {
			curveR[i] = blank;
		}
	}

	for (int i = ofsset; i < size - ofsset; i++) {
		if (curveR[i] < minR) {
			curveR[i] = minR;
		}
		if (curveR[i] > maxR) {
			curveR[i] = maxR;
		}
	}
	return;
}


void RetSegm::Coarse::correctionCenter(std::vector<float>&curveR, int centerIndex, int ofsset)
{
	auto correctionX = vector<double>(3, -1);
	auto correctionY = vector<double>(3, -1);

	int point1 = centerIndex - ofsset;
	int point2 = centerIndex;
	int point3 = centerIndex + ofsset;

	if (point1 < 0 || point3>curveR.size())
		return;

	correctionX[0] = point1;
	correctionX[1] = point2;
	correctionX[2] = point3;

	correctionY[0] = curveR[point1];
	correctionY[2] = curveR[point3];
	correctionY[1] = (correctionY[0] + correctionY[2]) / 2.f;

	vector<double> coeffs;
	CppUtil::Regressor::polyFit(correctionX, correctionY, 2, coeffs);

	for (int i = point1; i < point3; i++) {
		curveR[i] = coeffs[2] * pow(i, 2) + coeffs[1] * i + coeffs[0];
	}

	return;
}

void RetSegm::Coarse::correctCurveCenter(std::vector<float>&dataY, double sigma, bool isOpen)
{
	int M = round((10.0*sigma + 1.0) / 2.0) * 2 - 1;
	assert(M % 2 == 1); //M is an odd number
	vector<double> g, dg, d2g, smoothy;
	getGaussianDerivs(sigma, M, g, dg, d2g);

	vector<double> curveY(dataY.begin(), dataY.end());

	vector<double> Y, YY;
	getdXcurve(curveY, sigma, smoothy, Y, YY, g, dg, d2g, isOpen);

	for (int i = 0; i < dataY.size(); i++) {
		dataY[i] = smoothy[i];
	}
}


void RetSegm::Coarse::getGaussianDerivs(double sigma, int M, vector<double>& gaussian, vector<double>& dg, vector<double>& d2g) {
	int L = (M - 1) / 2;
	double sigma_sq = sigma * sigma;
	double sigma_quad = sigma_sq*sigma_sq;
	dg.resize(M); d2g.resize(M); gaussian.resize(M);

	Mat_<double> g = getGaussianKernel(M, sigma, CV_64F);
	for (double i = -L; i < L + 1.0; i += 1.0) {
		int idx = (int)(i + L);
		gaussian[idx] = g(idx);
		dg[idx] = (-i / sigma_sq) * g(idx);
		d2g[idx] = (-sigma_sq + i*i) / sigma_quad * g(idx);
	}
}


bool RetSegm::Coarse::getAxialCenter(std::vector<double>& dataX, std::vector<double>& dataY, double& axialX, double& axialY)
{
	int size = dataX.size();
	int sampleNum = 16;
	int iter = (int)(log(1 - 0.999) / log(1 - pow(0.8, sampleNum)));
	int count_max = 0;
	int effective_sample = 0;

	float offset = 0.01f;
	double a, b;

	for (int i = 0; i < iter; ++i) {
		Mat A = Mat(sampleNum, 3, CV_64FC1);
		Mat X = Mat(3, 1, CV_64FC1);
		Mat B = Mat(sampleNum, 1, CV_64FC1);

		// Sampling from discrete distribution without replacement 
		auto current = std::chrono::system_clock::now();
		auto duration = current.time_since_epoch();
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

		std::mt19937_64 generator(millis);
		std::uniform_int_distribution<__int64> distribution(0, size - 1);

		// Make 6 random numbers.
		std::set<__int64> sample;
		while (sampleNum > sample.size())
		{
			sample.insert(distribution(generator));
		}

		int j = 0;
		for (auto iter : sample) {
			auto x = dataX[iter];
			auto y = dataY[iter];

			A.at<double>(j, 0) = -2 * x;
			A.at<double>(j, 1) = -2 * y;
			A.at<double>(j, 2) = 1;

			B.at<double>(j, 0) = -pow(x, 2) - pow(y, 2);
			j++;
		}

		Mat Apinv;
		cv::invert(A, Apinv, DECOMP_SVD);

		X = Apinv * B;

		auto tempA = X.at<double>(0, 0);
		auto tempB = X.at<double>(1, 0);
		auto tempC = X.at<double>(2, 0);
		auto tempR = sqrt(pow(tempA, 2) + pow(tempB, 2) - tempC);

		int count = 0;
		for (int k = 0; k < size; k++) {
			auto x = dataX[k];
			auto y = dataY[k];

			double cost = fabs(sqrt(pow((x - tempA), 2) + pow((y - tempB), 2)) - tempR);
			if (cost < offset)
			{
				count++;
			}
		}
		if (count > count_max) {
			count_max = count;
			a = tempA;
			b = tempB;
		}
	}

	axialX = a;
	axialY = b;

	return true;
}

bool RetSegm::Coarse::calculateAnteriorTangential(std::vector<double>& dataX, std::vector<double>& dataEPI, std::vector<float>& tangentailAnt, std::vector<double>& incidenceAngle, int splitRange)
{
	int size = dataX.size();
	int splitHalf = splitRange / 2;

	for (int i = 0; i < size; i++)
	{
		int start = i - splitHalf;
		int end = i + splitHalf;

		int offset = 32;
		if (start < offset)
			start = offset;
		if (end > size - 1 - offset)
			end = size - 1 - offset;

		auto range = end - start;

		Mat A = Mat(range, 3, CV_64FC1);
		Mat X = Mat(3, 1, CV_64FC1);
		Mat B = Mat(range, 1, CV_64FC1);

		for (int j = start; j < start + range; j++)
		{
			double x = dataX[j];
			double y = dataEPI[j];

			A.at<double>(j - start, 0) = -2 * x;
			A.at<double>(j - start, 1) = -2 * y;
			A.at<double>(j - start, 2) = 1;
			B.at<double>(j - start, 0) = -pow(x, 2) - pow(y, 2);

		}

		Mat Apinv;
		invert(A, Apinv, DECOMP_SVD);

		X = Apinv * B;

		auto a = X.at<double>(0, 0);
		auto b = X.at<double>(1, 0);
		auto c = X.at<double>(2, 0);

		auto r = sqrt(pow(a, 2) + pow(b, 2) - c);
		auto slope = -(dataX[i] - a) / (dataEPI[i] - b);

		incidenceAngle[i] = atan(slope);
		tangentailAnt[i] = r;
	}

	return true;
}

bool RetSegm::Coarse::calculateAnteriorCurvature(std::vector<double>& dataX, std::vector<double>& dataEPI, std::vector<float>& tangentialAnt, std::vector<float>& axialAnt, std::vector<float>& refractiveAnt,
	std::vector<float>& refractiveKerato, std::vector<double>& incidenceAngle, double centerX)
{
	int size = dataX.size();

	for (int i = 0; i < size; i++)
	{
		auto distance = dataX[i] - centerX;
		double theta = (CV_PI / 2.f - incidenceAngle[i]);
		double theta2 = asin(sin(incidenceAngle[i]) / CORNEA_REFLECTIVE_INDEX);
		double phi = (incidenceAngle[i] - theta2);

		//axialAnt[i] = distance / cos(theta);
		if (abs(incidenceAngle[i]) < 0.01f || abs(distance) < 0.1f) {
			axialAnt[i] = tangentialAnt[i];
		}
		else {
			axialAnt[i] = distance / cos(theta);
		}
		
		// refractive
		if (abs(distance) > 0.1f) {
			auto focalR = distance / cos(CV_PI / 2.f - phi);
			auto focalLength = focalR * cos(phi);//cos(CV_PI / 2.f - theta) * axialAnt[i];
			refractiveAnt[i] = CORNEA_REFLECTIVE_INDEX / focalLength * 1000;
			refractiveKerato[i] = CORNEA_KERATO_REFLECTIVE_INDEX / focalLength * 1000;
		}
		else {
			refractiveAnt[i] = refractiveAnt[i - 5];
			refractiveKerato[i] = refractiveKerato[i - 5];
		}
	}

	return true;
}


bool RetSegm::Coarse::calculatePosteriorTangential(std::vector<double>& dataX, std::vector<double>& dataEND, std::vector<float>& tangentailPost, std::vector<double>& incidenceAngle, int splitRange)
{
	int size = dataX.size();
	int splitHalf = splitRange / 2;

	for (int i = 0; i < size; i++)
	{
		int start = i - splitHalf;
		int end = i + splitHalf;

		int offset = 64;
		if (start < offset)
			start = offset;
		if (end > size - 1 - offset)
			end = size - 1 - offset;

		auto range = end - start;

		Mat A = Mat(range, 3, CV_64FC1);
		Mat X = Mat(3, 1, CV_64FC1);
		Mat B = Mat(range, 1, CV_64FC1);

		for (int j = start; j < start + range; j++)
		{
			double x = dataX[j];
			double y = dataEND[j];

			A.at<double>(j - start, 0) = -2 * x;
			A.at<double>(j - start, 1) = -2 * y;
			A.at<double>(j - start, 2) = 1;

			B.at<double>(j - start, 0) = -pow(x, 2) - pow(y, 2);

		}

		Mat Apinv;
		invert(A, Apinv, DECOMP_SVD);

		X = Apinv * B;

		auto a = X.at<double>(0, 0);
		auto b = X.at<double>(1, 0);
		auto c = X.at<double>(2, 0);

		auto r = sqrt(pow(a, 2) + pow(b, 2) - c);
		auto slope = -(dataX[i] - a) / (dataEND[i] - b);

		incidenceAngle[i] = atan(slope);
		tangentailPost[i] = r;
	}

	return true;
}


bool RetSegm::Coarse::calculatePosteriorCurvature(std::vector<double>& dataX, std::vector<double>& dataEND, std::vector<float>& tangentailPost, std::vector<float>& axialPost,
	std::vector<float>& refractivePost, std::vector<double>& incidenceAngle, double centerX)
{
	int size = dataX.size();

	for (int i = 0; i < size; i++)
	{
		auto distance = dataX[i] - centerX;
		double theta = (CV_PI / 2.f - incidenceAngle[i]);
		double theta2 = asin(sin(incidenceAngle[i]) / CORNEA_REFLECTIVE_INDEX);
		double phi = (incidenceAngle[i] - theta2);

		//axialPost[i] = distance / cos(theta);
		if (abs(incidenceAngle[i]) < 0.01f || abs(distance) < 0.1f) {
			axialPost[i] = tangentailPost[i];
		}
		else {
			axialPost[i] = distance / cos(theta);
		}

		/*if (abs(distance) > 0.1f) {
			auto focalR = distance / cos(CV_PI / 2.f - phi);
			auto focalLength = focalR * cos(phi);
			refractivePost[i] = CORNEA_REFLECTIVE_INDEX / focalLength * 1000;
		}
		else {
			refractivePost[i] = refractivePost[i - 5];
		}*/
	}

	return true;
}


bool RetSegm::Coarse::dewarpingCornea(std::vector<double>&dataX, std::vector<double>& dewarpX, std::vector<double>& layerUp, std::vector<double>& layerDown, std::vector<double>& incidenceAngle)
{	
	int size = dataX.size();

	for (int i = 0; i < size; i++) {
		double theta2 = asin(sin(incidenceAngle[i]) / CORNEA_REFLECTIVE_INDEX);
		double phi = (incidenceAngle[i] - theta2);
		double OPL = abs(layerUp[i] - layerDown[i]) / CORNEA_REFLECTIVE_INDEX;
		
		dewarpX[i] = dataX[i] + OPL * sin(phi);
		if (abs(incidenceAngle[i]) < 0.7f)
			layerDown[i] = layerUp[i] + OPL * cos(phi);
		else {
			layerDown[i] = -999.f;
		}
		if (dewarpX[i] < dataX[0])
			dewarpX[i] = dataX[0];
		if (dewarpX[i] > dataX[size-1])
			dewarpX[i] = dataX[size - 1];

		if (layerDown[i] < 0)
			layerDown[i] = -999.f;
	}

	return true;
}


bool RetSegm::Coarse::calculateThickness(std::vector<double>& dataX, std::vector<double>& dewarpX, std::vector<double>& layerUp, std::vector<double>& LayerDown, std::vector<double>& theta, std::vector<float>& thickness)
{
	auto size = dataX.size();

	for (int i = 0; i < size; i++) {
		double a = tan(-1*theta[i] - CV_PI / 2.f);
		double b = layerUp[i] - (a*dataX[i]);

		auto distance = std::vector<double>(size, -1);

		for (int j = 0; j < size; j++) {
			distance[j] = abs(a*dewarpX[j] - 1 * LayerDown[j] + b) / sqrt(a*a + 1);
		}

		auto minIndex = min_element(distance.begin(), distance.end()) - distance.begin();

		thickness[i] = sqrt(pow((dataX[i] - dewarpX[minIndex]), 2) + pow((layerUp[i] - LayerDown[minIndex]), 2)) * 1000;
	}

	return true;
}


bool RetSegm::Coarse::calculateElevation(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& elevation)
{
	int size = dataX.size();
	int start = 0;
	int end = size;
	int margin = 0;
	int offset = 64;
	float blank = -999.f;

	if (start < offset)
		start = offset;
	if (end > size - 1 - offset)
		end = size - 1 - offset;

	for (int i = start; i < size / 2; i++) {
		if (dataY[i] == blank)
			margin++;
	}

	start += margin;
	margin = 0;

	for (int i = end; i > size / 2; i--) {
		if (dataY[i] == blank)
			margin++;
	}

	end -= margin;

	auto range = end - start;

	if(range <= 0)
		return true;

	Mat A = Mat(range, 3, CV_64FC1);
	Mat X = Mat(3, 1, CV_64FC1);
	Mat B = Mat(range, 1, CV_64FC1);

	for (int i = start; i < end; i++)
	{
		A.at<double>(i - start, 0) = -2 * dataX[i];
		A.at<double>(i - start, 1) = -2 * dataY[i];
		A.at<double>(i - start, 2) = 1;

		B.at<double>(i - start, 0) = -pow(dataX[i], 2) - pow(dataY[i], 2);
	}

	Mat Apinv;
	invert(A, Apinv, DECOMP_SVD);
	X = Apinv * B;

	auto a = X.at<double>(0, 0);
	auto b = X.at<double>(1, 0);
	auto c = X.at<double>(2, 0);
	auto r = sqrt(pow(a, 2) + pow(b, 2) - c);

	for (int i = 0; i < size; i++)
	{
		if (dataY[i] != blank) {
			auto circleY = -sqrt(pow(r, 2) - pow(dataX[i] - a, 2)) + b;
			elevation[i] = (circleY - dataY[i]) * 1000;
		}
		else
			elevation[i] = blank;
	}
	return true;
}

void RetSegm::Coarse::getdX(vector<double> x, int n, double sigma, double& gx, double& dgx, double& d2gx, vector<double> g, vector<double> dg, vector<double> d2g, bool isOpen)
{
	int L = (g.size() - 1) / 2;
	gx = dgx = d2gx = 0.0;

	for (int k = -L; k < L + 1; k++) {
		double x_n_k;
		if (n - k < 0) {
			if (isOpen) {
				//open curve - mirror values on border
				x_n_k = x[-(n - k)];
			}
			else {
				//closed curve - take values from end of curve
				x_n_k = x[x.size() + (n - k)];
			}
		}
		else if (n - k > x.size() - 1) {
			if (isOpen) {
				//mirror value on border
				x_n_k = x[n + k];
			}
			else {
				x_n_k = x[(n - k) - (x.size())];
			}
		}
		else {
			x_n_k = x[n - k];
		}
		gx += x_n_k * g[k + L]; //gaussians go [0 -> M-1]
		dgx += x_n_k * dg[k + L];
		d2gx += x_n_k * d2g[k + L];
	}
}

void RetSegm::Coarse::getdXcurve(vector<double> x, double sigma, vector<double>& gx, vector<double>& dx, vector<double>& d2x, vector<double> g, vector<double> dg, vector<double> d2g, bool isOpen)
{
	gx.resize(x.size());
	dx.resize(x.size());
	d2x.resize(x.size());
	for (int i = 0; i<x.size(); i++) {
		double gausx, dgx, d2gx; getdX(x, i, sigma, gausx, dgx, d2gx, g, dg, d2g, isOpen);
		gx[i] = gausx;
		dx[i] = dgx;
		d2x[i] = d2gx;
	}
}
