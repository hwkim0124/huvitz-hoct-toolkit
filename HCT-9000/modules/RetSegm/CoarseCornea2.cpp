#include "stdafx.h"
#include "CoarseCornea2.h"
#include "ImageMat.h"
#include "Boundary.h"
#include "DataFitt.h"
#include "Feature.h"
#include "SegmImage.h"
#include "SegmLayer.h"
#include "Logger.h"

#include "PathCornea2.h"
#include "Smooth2.h"
#include "OptimalPath.h"

#include "CppUtil2.h"

#include <algorithm>

using namespace CppUtil;
using namespace RetSegm;
using namespace cv;
using namespace std;


CoarseCornea2::CoarseCornea2()
{
}


CoarseCornea2::~CoarseCornea2()
{
}


bool RetSegm::CoarseCornea2::createGradientMaps(const SegmImage * imgSrc, SegmImage * imgAsc, 
	SegmImage * imgDes, SegmImage * imgOut, bool isMeye)
{
	const int KERNEL_ROWS_ASC = 7;
	const int KERNEL_COLS_ASC = 5;
	const int KERNEL_ROWS_DES = 7;
	const int KERNEL_COLS_DES = 3;

	Mat srcMat, mask;
	imgSrc->getCvMatConst().copyTo(srcMat);

	int grayMin = 0; // (int)imgSrc->getMean();
	int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*2.0f);

	mask = srcMat > grayMax;
	srcMat.setTo(grayMax, mask);

	// kernel: 7x7
	Mat kernel = Mat::ones(KERNEL_ROWS_ASC, KERNEL_COLS_ASC, CV_32F);
	for (int r = 0; r < kernel.rows; r++) {
		for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
			kernel.at<float>(r, c) *= 1.0f; // 2.0f;
		}
	}
	for (int r = 0; r < kernel.rows / 2; r++) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(r, c) *= -1.0f;
		}
	}
	if (kernel.rows % 2) {
		for (int c = 0; c < kernel.cols; c++) {
			kernel.at<float>(kernel.rows / 2, c) = 0.0f;
		}
	}

	Mat conv;
	filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

	// Leave only positive gradients, and normalize in range between [0, 255].
	// Note that the kernel weighted pixels would be out of normal gray level range. 
	Mat masc;
	Mat ascMat;

	mask = conv < 0;
	conv.copyTo(masc);
	masc.setTo(0.0f, mask);

	// Set limits to upper bound of normalization range. 
	// It prevents low constrast in poor snr region caused by excessive stretching
	// dominated by gradient peak among overall image. 
	float gmax = 255.0f * (float)sqrt(KERNEL_ROWS_ASC * KERNEL_COLS_ASC);
	mask = conv > gmax;
	masc.setTo(gmax, mask);

	cv::normalize(masc, ascMat, 0, 255, NORM_MINMAX, CV_8U);
	imgAsc->getCvMat() = ascMat;

	if (imgDes != nullptr) {
		imgSrc->getCvMatConst().copyTo(srcMat);

		float devRatio = (isMeye ? 2.0f : 0.0f);
		int grayMin = 0; // (int)imgSrc->getMean();
		int grayMax = (int)(imgSrc->getMean() + imgSrc->getStddev()*devRatio);

		mask = srcMat > grayMax;
		srcMat.setTo(grayMax, mask);

		// kernel: 5x5
		kernel = Mat::ones(KERNEL_ROWS_DES, KERNEL_COLS_DES, CV_32F);
		for (int r = 0; r < kernel.rows; r++) {
			for (int c = kernel.cols / 3; c < (kernel.cols * 2) / 3; c++) {
				kernel.at<float>(r, c) *= 1.0f; //  2.0f;
			}
		}
		for (int r = (kernel.rows / 2 + 1); r < kernel.rows; r++) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(r, c) *= -1.0f;
			}
		}
		if (kernel.rows % 2) {
			for (int c = 0; c < kernel.cols; c++) {
				kernel.at<float>(kernel.rows / 2, c) = 0.0f;
			}
		}

		filter2D(srcMat, conv, CV_32F, kernel, Point(-1, -1), 0, BORDER_REPLICATE);

		// Reverse negative gradients, and normalize in range between [0, 255].
		Mat mdes;
		Mat desMat;

		mask = conv < 0;
		conv.copyTo(mdes);
		mdes.setTo(0.0f, mask);
		gmax = 255.0f * (float)sqrt(KERNEL_ROWS_DES * 1.0f);
		mask = conv > gmax;
		mdes.setTo(gmax, mask);

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


bool RetSegm::CoarseCornea2::checkIfModelEyeImage(const SegmImage * imgSrc)
{
	Mat srcMat = imgSrc->getCvMatConst();
	int thresh = max((int)((imgSrc->getMean() + imgSrc->getStddev())), 10);

	int w = imgSrc->getWidth();
	int h = imgSrc->getHeight();
	
	int s1 = (int)(w * 0.25f);
	int e1 = (int)(w * 0.45f);
	int s2 = (int)(w * 0.55f);
	int e2 = (int)(w * 0.75f);

	auto sizes = std::vector<int>(w, 0);
	for (int i = s1; i <= e1; i++) {
		for (int j = 0; j < h; j++) {
			if (srcMat.at<unsigned char>(j, i) > thresh) {
				sizes[i] += 1;
			}
		}
	}

	for (int i = s2; i <= e2; i++) {
		for (int j = 0; j < h; j++) {
			if (srcMat.at<unsigned char>(j, i) > thresh) {
				sizes[i] += 1;
			}
		}
	}

	sort(sizes.begin(), sizes.end());

	const int MODEL_EYE_SIZE_MAX = 15;
	int index = (int)(w * 0.90f);
	int value = sizes[index];

	bool check = value <= MODEL_EYE_SIZE_MAX;
	return check;
}


bool RetSegm::CoarseCornea2::makeupCorneaBorderLines(const SegmImage * imgSrc, SegmImage * imgAsc, 
	SegmImage * imgDes, SegmLayer * layerInn, SegmLayer * layerOut, 
	int& centerX, int& centerY, float rangeX, bool isMeye)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	auto curve = std::vector<int>();

	if (!segmentInnerBoundary(imgSrc, imgAsc, layerInn, layerOut, rangeX, isMeye)) {
		return false;
	}

	if (!isMeye) {
		if (!estimateInnerCoreCurve(layerInn, curve, centerX, centerY, rangeX)) {
			return false;
		}

		layerOut->initialize(curve, width, height);

		if (!correctInnerPeripherals(layerInn, curve, centerX, centerY, rangeX)) {
			return false;
		}


		if (!estimateInnerCoreCurve(layerInn, curve, centerX, centerY, rangeX)) {
			return false;
		}
		layerInn->initialize(curve, width, height);

		if (!segmentInnerEnds(imgSrc, imgAsc, layerInn, centerX, centerY, rangeX)) {
			return false;
		}
	}
	else {
		centerX = width / 2;
		centerY = layerInn->getY(centerX);
	}

	if (!segmentOuterBoundary(imgSrc, imgDes, layerInn, layerOut, centerX, centerY, rangeX, isMeye)) {
		return false;
	}

	if (!isMeye) {
		if (!segmentOuterEnds(imgSrc, imgDes, layerInn, layerOut, centerX, centerY, rangeX)) {
			return false;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea2::correctCenterReflection(const SegmImage * imgSrc, std::vector<int>& layer, 
	int centerX, int centerY, float rangeX)
{
	int refl_x1, refl_x2;
	if (!locateCenterReflection(imgSrc, centerX, centerY, rangeX, refl_x1, refl_x2)) {
		return true;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	auto size = layer.size();

	if (size != width) {
		return false;
	}

	int sizePerMM = (int)(width / rangeX);
	int cent_x = (refl_x1 + refl_x2) / 2;
	int cent_x1 = max((int)(cent_x - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(cent_x + sizePerMM * 1.5f), width - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, 0);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cent_x1 && i <= cent_x2) {
				if (i < refl_x1 || i > refl_x2) {
					dataY[i] = layer[i];
				}
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= refl_x1 && i <= refl_x2) {
					layer[i] = fittY[i];
				}
			}
		}
		else {
			return false;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea2::correctCenterReflectionOfEND(const SegmImage * imgSrc, std::vector<int>& layer, int centerX, int centerY, float rangeX)
{
	int refl_x1, refl_x2;
	if (!locateCenterReflectionOfEND(imgSrc, centerX, centerY, rangeX, refl_x1, refl_x2)) {
		return true;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	auto size = layer.size();

	if (size != width) {
		return false;
	}

	int sizePerMM = (int)(width / rangeX);
	int cent_x = (refl_x1 + refl_x2) / 2;
	int cent_x1 = max((int)(cent_x - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(cent_x + sizePerMM * 1.5f), width - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, 0);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cent_x1 && i <= cent_x2) {
				if (i < refl_x1 || i > refl_x2) {
					dataY[i] = layer[i];
				}
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= cent_x1 && i <= cent_x2) {
					layer[i] = fittY[i];
				}
			}
		}
		else {
			return false;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea2::locateCenterReflection(const SegmImage * imgSrc, int centerX, int centerY, 
	float rangeX, int & reflX1, int & reflX2)
{
	Mat srcMat = imgSrc->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int sizePerMM = (int)(img_w / rangeX);
	int cent_x1 = max((int)(centerX - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(centerX + sizePerMM * 1.5f), img_w - 1);

	int threshold = max((int)(imgSrc->getMean() * 2.0f), 25);
	auto avgVals = imgSrc->getColumMeans();
	int maxVal = 0;
	int maxPos = 0;

	for (int i = cent_x1; i < cent_x2; i++) {
		if (avgVals[i] > maxVal) {
			maxVal = avgVals[i];
			maxPos = i;
		}
	}

	if (maxVal > threshold) {
		reflX1 = max((int)(maxPos - sizePerMM*0.36f), 0);
		reflX2 = min((int)(maxPos + sizePerMM*0.36f), img_w - 1);
		// LogD() << "Cornea center reflection: " << reflect_x1 << ", " << reflect_x2;
		return true;
	}
	else {
		reflX1 = -1;
		reflX2 = -1;
		return false;
	}
}

bool RetSegm::CoarseCornea2::locateCenterReflectionOfEND(const SegmImage * imgSrc, int centerX, int centerY, float rangeX, int & reflX1, int & reflX2)
{
	Mat srcMat = imgSrc->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int sizePerMM = (int)(img_w / rangeX);
	int cent_x1 = max((int)(centerX - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(centerX + sizePerMM * 1.5f), img_w - 1);

	int threshold = max((int)(imgSrc->getMean() * 2.0f), 25);
	auto avgVals = imgSrc->getColumMeans();
	int maxVal = 0;
	int maxPos = 0;

	for (int i = cent_x1; i < cent_x2; i++) {
		if (avgVals[i] > maxVal) {
			maxVal = avgVals[i];
			maxPos = i;
		}
	}

	if (maxVal > threshold) {
		reflX1 = max((int)(maxPos - sizePerMM*0.5f), 0);
		reflX2 = min((int)(maxPos + sizePerMM*0.5f), img_w - 1);
		return true;
	}
	else {
		reflX1 = -1;
		reflX2 = -1;
		return false;
	}
}

bool RetSegm::CoarseCornea2::correctSideRobeEnds(const SegmImage * imgSrc, const std::vector<int>& path, 
	std::vector<int>& layer, int centerX, int centerY, float rangeX)
{
	Mat srcMat = imgSrc->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int sizePerMM = (int)(img_w / rangeX);
	int sizeMin = max((int)(sizePerMM * 1.0f), (int)(img_w*0.1f));
	int sizeMax = (int)(sizePerMM * 2.0f);
	int robe_y = (int)(img_h * 0.91f);

	layer = path;

	{
		auto dataX = vector<int>(img_w, -1);
		auto dataY = vector<int>(img_w, 0);
		auto fittY = vector<int>(img_w, -1);
		int count = 0;

		int peri_x2 = (int)(img_w * 0.95f);
		int peri_x1 = max(centerX, peri_x2 - (int)(sizePerMM*2.0f));
		for (int i = 0; i < img_w; i++) {
			dataX[i] = i;
			if (i >= peri_x1 && i <= peri_x2) {
				if (path[i] >= 0) {
					if (path[i] < robe_y) {
						dataY[i] = path[i];
						++count;
					}
				}
			}
		}

		if (count >= sizeMin) {
			// 2th order polynomial fitting by periperal edge points around cornea center. 
			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				// Concave curve is allowed for model eye surface. 
				for (int i = img_w - 1; i >= peri_x1; i--) {
					if (fittY[i] > layer[i]) {
						layer[i] = fittY[i];
					}
					else {
						break;
					}
				}
			}
		}
	}

	{
		auto dataX = vector<int>(img_w, -1);
		auto dataY = vector<int>(img_w, 0);
		auto fittY = vector<int>(img_w, -1);
		int count = 0;

		int peri_x1 = (int)(img_w * 0.05f);
		int peri_x2 = min(centerX, peri_x1 + (int)(sizePerMM*2.0f));
		for (int i = 0; i < img_w; i++) {
			dataX[i] = i;
			if (i >= peri_x1 && i <= peri_x2) {
				if (path[i] >= 0) {
					if (path[i] < robe_y) {
						dataY[i] = path[i];
						++count;
					}
				}
			}
		}

		if (count >= sizeMin) {
			// 2th order polynomial fitting by periperal edge points around cornea center. 
			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				// Concave curve is allowed for model eye surface.
				for (int i = 0; i <= peri_x2; i++) {
					if (fittY[i] > layer[i]) {
						layer[i] = fittY[i];
					}
					else {
						break;
					}
				}
			}
		}
	}

	{
		auto dataX = vector<int>(img_w, -1);
		auto dataY = vector<int>(img_w, 0);
		auto fittY = vector<int>(img_w, -1);
		int count = 0;

		int cent_x1 = max((int)(centerX - sizePerMM*1.0f), 0);
		int cent_x2 = min((int)(centerX + sizePerMM*1.0f), img_w-1);
		int peri_x1 = max((int)(centerX - sizePerMM*3.0f), 0);
		int peri_x2 = min((int)(centerX + sizePerMM*3.0f), img_w-1);
		for (int i = 0; i < img_w; i++) {
			dataX[i] = i;
			if ((i >= peri_x1 && i <= cent_x1) || (i >= cent_x2 && i <= peri_x2)) {
				if (path[i] >= 0) {
					dataY[i] = path[i];
					++count;
				}
			}
		}

		if (count >= sizeMin) {
			// 2th order polynomial fitting by periperal edge points around cornea center. 
			vector<double> coeffs;
			if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
				// Concave curve is allowed for model eye surface.
				for (int i = 0; i < img_w; i++) {
					if (i < peri_x1 || i > peri_x2) {
						if (fittY[i] > layer[i]) {
							layer[i] = fittY[i];
						}
					}
				}
			}
		}
	}

	return true;
}


bool RetSegm::CoarseCornea2::correctSurfaceReflection(const SegmImage * imgSrc, SegmLayer* layerEPI, SegmLayer* layerBow, int centerX, int centerY, float rangeX)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	int sizePerMM = (int)(width / rangeX);
	int refl_x1 = max((int)(centerX - sizePerMM * 2.5f), 0);
	int refl_x2 = min((int)(centerX + sizePerMM * 2.5f), width - 1);

	auto epis = layerEPI->getYs();
	auto bows = layerBow->getYs();
	
	int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev()*2.5f);
	thresh = max(min(thresh, 225), 120);

	for (int c = refl_x1; c <= refl_x2; c++) {
		int r1 = epis[c];
		int r2 = bows[c];

		int rc = r1 + (r2 - r1) / 3;
		
		int wght = 0;
		int wsum = 0;
		int rsum = 0;
		int grey = 0;
		int max_g = 0;
		int max_r = 0;
		for (int r = r1; r <= rc; r++) {
			grey = srcMat.at<uchar>(r, c);
			if (grey >= thresh) {
				wght = (grey - thresh + 1);
				wsum += wght;
				rsum += (wght * r);
			}
		}

		if (rsum > 0) {
			epis[c] = rsum / wsum;
		}
	}

	if (!correctCenterReflection(imgSrc, epis, centerX, centerY, rangeX)) {
		return false;
	}

	vector<int> upper;
	vector<int> lower;
	if (!Smooth2::smoothLayerEPI(epis, upper)) {
		return false;
	}
	lower = bows;

	transform(begin(upper), end(upper), begin(lower), begin(upper), [](int elem1, int elem2) { return min(elem1+1, elem2); });
	transform(begin(lower), end(lower), begin(upper), begin(lower), [](int elem1, int elem2) { return max(elem1, elem2); });

	layerEPI->initialize(upper, width, height);
	layerBow->initialize(lower, width, height);	
	return true;
}


bool RetSegm::CoarseCornea2::segmentInnerBoundary(const SegmImage * imgSrc, const SegmImage* imgGrad,
	SegmLayer* layerInn, SegmLayer* layerOut, float rangeX, bool isMeye)
{
	if (imgSrc == nullptr || imgGrad == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfInner(imgSrc, imgGrad, upper, lower, delta, isMeye)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfInner(imgGrad, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth2::smoothBoundaryINN(path, layer)) {
		return false;
	}

	layerInn->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::segmentOuterBoundary(const SegmImage * imgSrc, const SegmImage * imgGrad, 
	SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX, bool isMeye)
{
	if (imgSrc == nullptr || imgGrad == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfOuter(imgSrc, imgGrad, layerInn, layerOut, 
		centerX, centerY, rangeX, upper, lower, delta, isMeye)) {
		return false;
	}

	if (!PathCornea2::createLayerCostMapOfOuter(imgGrad, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}

	auto inner = layerInn->getYs();
	if (!Smooth2::smoothBoundaryOUT(path, inner, layer)) {
		return false;
	}

	layerOut->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::segmentInnerEnds(const SegmImage * imgSrc, const SegmImage* imgGrad, 
	SegmLayer * layerInn, int centerX, int centerY, float rangeX)
{
	if (imgSrc == nullptr || imgGrad == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfInnerEnds(imgGrad, layerInn,
		centerX, centerY, rangeX, upper, lower, delta)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfInnerEnds(imgGrad, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!Smooth2::smoothBoundaryINN(path, layer)) {
		return false;
	}

	layerInn->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::segmentOuterEnds(const SegmImage* imgSrc, const SegmImage* imgGrad,
	SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX)
{
	if (imgSrc == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfOuterEnds(imgSrc, layerInn, layerOut,
		centerX, centerY, rangeX, upper, lower, delta)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfOuterEnds(imgSrc, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}

	auto inner = layerInn->getYs();
	if (!Smooth2::smoothBoundaryOUT(path, inner, layer)) {
		return false;
	}

	layerOut->initialize(path, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::elaborateEPI(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerEPI, 
	int centerX, int centerY, float rangeX)
{
	if (imgSrc == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerInn->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfEPI(imgSrc, layerInn, centerX, centerY, rangeX, upper, lower, delta)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfEPI(imgSrc, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}
	if (!CoarseCornea2::correctCenterReflection(imgSrc, path, centerX, centerY, rangeX)) {
		return false;
	}
	if (!Smooth2::smoothLayerEPI(path, layer)) {
		return false;
	}

	layerEPI->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::elaborateEND(const SegmImage * imgSrc, SegmLayer * layerEPI, SegmLayer * layerOut, 
	SegmLayer * layerEND, int centerX, int centerY, float rangeX)
{
	if (imgSrc == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerOut->resize(width, height);

	vector<int> upper, lower, delta;
	vector<int> path, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfEND(imgSrc, layerEPI, layerOut, centerX, centerY, rangeX, upper, lower, delta)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfEND(imgSrc, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}
	if (!Smooth2::smoothLayerEND(path, layerEPI, layer)) {
		return false;
	}

	path = layer;
	if (!correctCenterReflectionOfEND(imgSrc, path, centerX, centerY, rangeX)) {
		return false;
	}

	if (!correctSideRobeEnds(imgSrc, path, layer, centerX, centerY, rangeX)) {
		return false;
	}

	path = layer;
	if (!Smooth2::smoothLayerEND2(path, layerEPI, layer)) {
		return false;
	}

	layerEND->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::elaborateBOW(const SegmImage * imgSrc, SegmLayer * layerEPI, SegmLayer * layerEND, 
	SegmLayer * layerBOW, int centerX, int centerY, float rangeX)
{
	if (imgSrc == nullptr) {
		return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	vector<int> upper, lower, delta;
	vector<int> path, path2, layer;
	auto imgCost = SegmImage();

	if (!PathCornea2::designLayerConstraintsOfBOW(imgSrc, layerEPI, layerEND, centerX, centerY, rangeX, upper, lower, delta)) {
		return false;
	}
	if (!PathCornea2::createLayerCostMapOfBOW(imgSrc, &imgCost, upper, lower, delta)) {
		return false;
	}
	if (!OptimalPath::searchPathWithMinCost(&imgCost, upper, lower, delta, path)) {
		return false;
	}

	makeFittingCurveOfBOW(path, path2, centerX, centerY, rangeX);
	if (!Smooth2::smoothLayerBOW(path2, layerEPI, layerEND, layer)) {
		return false;
	}

	layerBOW->initialize(layer, width, height);
	return true;
}


bool RetSegm::CoarseCornea2::makeFittingCurveOfBOW(std::vector<int>& input, std::vector<int>& output, int centerX, int centerY, float rangeX)
{
	auto size = input.size();
	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);
	auto curve = vector<int>(size, -1);
	int sizePerMM = (int)(size / rangeX);

	// Position shift by kernel radius. 
	output = input;
	for (int i = 0; i < size; i++) {
		output[i] = (input[i] + 1);
	}
	curve = output;

	auto centerStart = max((int)(centerX - 1.5f * sizePerMM), 0);
	auto centerClose = min((int)(centerX + 1.5f * sizePerMM), (int)(size - 1));
	auto exceptStart = max((int)(centerX - 0.18f * sizePerMM), 0);
	auto exceptClose = min((int)(centerX + 0.18f * sizePerMM), (int)(size - 1));

	// Fitting pheripheral cornea. 
	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (centerStart <= i && i <= centerClose) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (output[i] < 0 ? 0 : output[i]);
		}
	}

	vector<double> coeffs;
	for (int i = 0, outliers = 0; i < 1; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		}
		break;
	}

	// Smoothing by concave pheripheral curve. 
	if (coeffs.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	else {
		for (int i = 0; i < size; i++) {
			if (i < centerStart || i > centerClose) {
				curve[i] = (int)((output[i] + fittY[i]) * 0.5f);
			}
		}
	}

	// Fitting center curve. 
	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i < centerStart || i > centerClose || (i >= exceptStart && i <= exceptClose)) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (output[i] < 0 ? 0 : output[i]);
		}
	}

	vector<double> coeffs2;
	for (int i = 0, outliers = 0; i < 1; i++) {
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs2)) {
		}
		break;
	}

	// Smoothing by concave center curve. 
	if (coeffs2.back() < COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA) {
		return false;
	}
	else {
		for (int i = 0; i < size; i++) {
			if ((i >= centerStart && i <= exceptStart) || (i >= exceptClose && i <= centerClose)) {
				curve[i] = (int)((output[i] + fittY[i]) * 0.5f);
			}
		}
	}

	output = curve;
	return true;
}


bool RetSegm::CoarseCornea2::estimateInnerCoreCurve(SegmLayer * layerInn, std::vector<int>& curve, 
	int& centerX, int& centerY, float rangeX)
{
	auto inner = layerInn->getYs();
	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);

	int center = size / 2;
	int periX1 = max((int)(center - sizePerMM * 1.75f), 0);
	int periX2 = min((int)(center + sizePerMM * 1.75f), size-1);
	int centX1 = max((int)(center - sizePerMM * 0.25f), 0);
	int centX2 = min((int)(center + sizePerMM * 0.25f), size-1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i <= periX1 || (i >= centX1 && i <= centX2) || (i >= periX2)) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
		}
	}

	// 2th order polynomial fitting by periperal edge points around cornea center. 
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		// Concave curve is allowed for model eye surface. 
		curve = fittY;

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
		return true;
	}
	return false;
}

bool RetSegm::CoarseCornea2::estimateInnerFullCurve(SegmLayer * layerInn, std::vector<int>& curve, int & centerX, int & centerY, float rangeX)
{
	auto inner = layerInn->getYs();
	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);

	int center = centerX / 2;
	int centX1 = max((int)(center - sizePerMM * 0.5f), 0);
	int centX2 = min((int)(center + sizePerMM * 0.5f), size - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i >= centX1 && i <= centX2) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (inner[i] < 0 ? 0 : inner[i]);
		}
	}

	// 2th order polynomial fitting by periperal edge points around cornea center. 
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		// Concave curve is allowed for model eye surface. 
		curve = fittY;

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
		return true;
	}
	return false;
}

bool RetSegm::CoarseCornea2::estimateOuterCoreCurve(SegmLayer * layerOut, std::vector<int>& curve, 
	int centerX, int centerY, float rangeX)
{
	auto outer = layerOut->getYs();
	int size = (int)outer.size();
	int sizePerMM = (int)(size / rangeX);

	int center = size / 2;
	int periX1 = max((int)(center - sizePerMM * 2.5f), 0);
	int periX2 = min((int)(center + sizePerMM * 2.5f), size - 1);
	int centX1 = max((int)(center - sizePerMM * 0.5f), 0);
	int centX2 = min((int)(center + sizePerMM * 0.5f), size - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, -1);
	auto fittY = vector<int>(size, -1);

	for (int i = 0; i < size; i++) {
		dataX[i] = i;
		if (i <= periX1 || (i >= centX1 && i <= centX2) || (i >= periX2)) {
			dataY[i] = 0;
		}
		else {
			dataY[i] = (outer[i] < 0 ? 0 : outer[i]);
		}
	}

	// 2th order polynomial fitting by periperal edge points around cornea center. 
	vector<double> coeffs;
	if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
		// Concave curve is allowed for model eye surface. 
		curve = fittY;
		return true;
	}
	return false;
}


bool RetSegm::CoarseCornea2::correctInnerPeripherals(SegmLayer * layerInn, const std::vector<int> curve,
	int centerX, int centerY, float rangeX)
{
	auto inner = layerInn->getYs();
	int size = (int)inner.size();
	int sizePerMM = (int)(size / rangeX);

	int center = centerX;
	int periX1 = max((int)(center - sizePerMM * 1.75f), 0);
	int periX2 = min((int)(center + sizePerMM * 1.75f), size - 1);
	int centX1 = max((int)(center - sizePerMM * 0.25f), 0);
	int centX2 = min((int)(center + sizePerMM * 0.25f), size - 1);

	const int UPPER_LIMIT = 21;
	int limit = 0;

	for (int i = center; i >= 0; i--) {
		limit = max((int)(UPPER_LIMIT - ((float)abs(i - center) / sizePerMM) * 10), 3);
		if (inner[i] < (curve[i] - limit)) {
			for (int j = i; j < center; j++) {
				if (inner[j] < curve[j]) {
					inner[j] = curve[j];
				}
				else {
					break;
				}
			}
		}
	}

	for (int i = center; i < size; i++) {
		limit = max((int)(UPPER_LIMIT - ((float)abs(i - center) / sizePerMM) * 10), 3);
		if (inner[i] < (curve[i] - limit)) {
			for (int j = i; j > center; j--) {
				if (inner[j] < curve[j]) {
					inner[j] = curve[j];
				}
				else {
					break;
				}
			}
		}
	}

	layerInn->setPoints(inner);
	return true;
}


bool RetSegm::CoarseCornea2::correctOuterPeripherals(SegmLayer * layerOut, const std::vector<int> curve, 
	int centerX, int centerY, float rangeX)
{
	auto outer = layerOut->getYs();
	int size = (int)outer.size();
	int sizePerMM = (int)(size / rangeX);

	int center = centerX;
	int periX1 = max((int)(center - sizePerMM * 1.75f), 0);
	int periX2 = min((int)(center + sizePerMM * 1.75f), size - 1);

	const int UPPER_LIMIT = 21;
	int limit = 0;

	for (int i = periX1; i >= 0; i--) {
		limit = max((int)(UPPER_LIMIT - ((float)abs(i - center) / sizePerMM) * 10), 3);
		if (outer[i] < (curve[i] - limit)) {
			for (int j = i; j < center; j++) {
				if (outer[j] < curve[j]) {
					outer[j] = curve[j];
				}
				else {
					break;
				}
			}
		}
	}

	for (int i = periX2; i < size; i++) {
		limit = max((int)(UPPER_LIMIT - ((float)abs(i - center) / sizePerMM) * 10), 3);
		if (outer[i] < (curve[i] - limit)) {
			for (int j = i; j > center; j--) {
				if (outer[j] < curve[j]) {
					outer[j] = curve[j];
				}
				else {
					break;
				}
			}
		}
	}

	layerOut->setPoints(outer);
	return true;
}


bool RetSegm::CoarseCornea2::correctCenterReflectionForTopo(const SegmImage * imgSrc, std::vector<int>& layer,
	int centerX, int centerY, float rangeX)
{
	int refl_x1, refl_x2;
	if (!locateCenterReflectionForTopo(imgSrc, centerX, centerY, rangeX, refl_x1, refl_x2)) {
		return true;
		//return false;
	}

	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	auto size = layer.size();

	if (size != width) {
		return false;
	}

	int sizePerMM = (int)(width / rangeX);
	int cent_x = (refl_x1 + refl_x2) / 2;
	int cent_x1 = max((int)(cent_x - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(cent_x + sizePerMM * 1.5f), width - 1);

	auto dataX = vector<int>(size, -1);
	auto dataY = vector<int>(size, 0);
	auto fittY = vector<int>(size, -1);

	// Interpolate the empty points of border line by the fitting curve from 
	// the center cornea. 
	{
		for (int i = 0; i < size; i++) {
			dataX[i] = i;
			if (i >= cent_x1 && i <= cent_x2) {
				if (i < refl_x1 || i > refl_x2) {
					dataY[i] = layer[i];
				}
			}
		}

		// 2th order polynomial fitting by periperal edge points around cornea center. 
		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			for (int i = 0; i < size; i++) {
				if (i >= refl_x1 && i <= refl_x2) {
					layer[i] = fittY[i];
				}
			}
		}
		else {
			return false;
		}
	}
	return true;
}


bool RetSegm::CoarseCornea2::correctSurfaceReflectionForTopo(const SegmImage * imgSrc, SegmLayer* layerEPI, SegmLayer* layerBow, int centerX, int centerY, float rangeX)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	Mat srcMat = imgSrc->getCvMatConst();

	int sizePerMM = (int)(width / rangeX);
	int refl_x1 = max((int)(centerX - sizePerMM * 2.5f), 0);
	int refl_x2 = min((int)(centerX + sizePerMM * 2.5f), width - 1);

	auto epis = layerEPI->getYs();
	auto bows = layerBow->getYs();

	/*int thresh = (int)(imgSrc->getMean() + imgSrc->getStddev()*3.0f);
	thresh = max(min(thresh, 225), 230);*/
	int thresh = 230;

	for (int c = refl_x1; c <= refl_x2; c++) {
		int r1 = epis[c];
		int r2 = bows[c];

		int grey = 0;
		int nextGrey = 0;
		int rc = r1 + (r2 - r1) / 4;

		bool isTear = false;
		int edgeR = 0;
		int temp = 0;

		for (int r = r1; r <= rc; r++) {
			if (static_cast<int>(srcMat.at<uchar>(r, c)) >= thresh) {
				isTear = true;
			}

			if (r != rc) {
				grey = static_cast<int>(srcMat.at<uchar>(r, c));
				nextGrey = static_cast<int>(srcMat.at<uchar>(r + 1, c));
				int diff = grey - nextGrey;
				if (abs(diff) >= temp) {
					temp = abs(diff);
					edgeR = r;
				}
			}
		}

		if (isTear) {
			for (int r = r1; r < rc; r++) {
				grey = static_cast<int>(srcMat.at<uchar>(r, c));
				nextGrey = static_cast<int>(srcMat.at<uchar>(r + 1, c));

				if (grey >= nextGrey) {
					epis[c] = r;
					break;
				}
				if (grey >= thresh) {
					epis[c] = r;
					break;
				}

				if (r == edgeR) {
					epis[c] = r + 1;
					break;
				}
			}
		}
		else {
			epis[c] += 1;
		}
	}

	if (!correctCenterReflectionForTopo(imgSrc, epis, centerX, centerY, rangeX)) {
		return false;
	}

	vector<int> upper;
	vector<int> lower;
	if (!Smooth2::smoothLayerEPI(epis, upper)) {
		return false;
	}
	lower = bows;

	transform(begin(upper), end(upper), begin(lower), begin(upper), [](int elem1, int elem2) { return min(elem1 + 1, elem2); });
	transform(begin(lower), end(lower), begin(upper), begin(lower), [](int elem1, int elem2) { return max(elem1, elem2); });

	layerEPI->initialize(upper, width, height);
	layerBow->initialize(lower, width, height);

	if (!locateCenterReflectionForTopo(imgSrc, centerX, centerY, rangeX, refl_x1, refl_x2)) {
		return false;
	}
	return true;
}

bool RetSegm::CoarseCornea2::locateCenterReflectionForTopo(const SegmImage * imgSrc, int centerX, int centerY,
	float rangeX, int & reflX1, int & reflX2)
{
	Mat srcMat = imgSrc->getCvMat();
	int img_w = srcMat.cols;
	int img_h = srcMat.rows;

	int sizePerMM = (int)(img_w / rangeX);
	int cent_x1 = max((int)(centerX - sizePerMM * 1.5f), 0);
	int cent_x2 = min((int)(centerX + sizePerMM * 1.5f), img_w - 1);

	int threshold = max((int)(imgSrc->getMean() * 2.0f), 25);
	auto avgVals = imgSrc->getColumMeans();
	int maxVal = 0;
	int maxPos = 0;

	for (int i = cent_x1; i < cent_x2; i++) {
		if (avgVals[i] > maxVal) {
			maxVal = avgVals[i];
			maxPos = i;
		}
	}

	if (maxVal > threshold) {
		reflX1 = max((int)(maxPos - sizePerMM*0.36f), 0);
		reflX2 = min((int)(maxPos + sizePerMM*0.36f), img_w - 1);
		// LogD() << "Cornea center reflection: " << reflect_x1 << ", " << reflect_x2;
		return true;
	}
	else {
		reflX1 = -1;
		reflX2 = -1;
		return false;
	}
}
