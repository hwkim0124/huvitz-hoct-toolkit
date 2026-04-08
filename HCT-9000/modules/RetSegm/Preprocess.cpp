#include "stdafx.h"
#include "Preprocess.h"
#include "ImageMat.h"
#include "SegmImage.h"
#include "CppUtil2.h"

using namespace RetSegm;
using namespace CppUtil;

#include <opencv2/core/core.hpp>
using namespace cv;

#include <iostream>
#include <iomanip>
using namespace std;


Preprocess::Preprocess()
{
	// cv::setBreakOnError(true);
}


bool RetSegm::Preprocess::createSampleImage(const SegmImage * imgSrc, SegmImage * imgOut, int width, int height)
{
	if (width == imgSrc->getWidth() && height == imgSrc->getHeight()) {
		imgSrc->getCvMatConst().copyTo(imgOut->getCvMat());
	}
	else {
		//float wRatio = (float)width / (float)imgSrc->getWidth();
		//float hRatio = (float)height / (float)imgSrc->getHeight();
		// Size size((int)(width*wRatio), (int)(height*hRatio));

		Size size(width, height);
		resize(imgSrc->getCvMatConst(), imgOut->getCvMat(), size);
	}
	return true;
}



bool RetSegm::Preprocess::smoothBackgroundNoise(const SegmImage * imgSrc, SegmImage * imgOut)
{
	int wSize = NOISE_FILTER_WIDTH;
	int hSize = NOISE_FILTER_HEIGHT;
	Size ksize(wSize, hSize);

	// Kernel size for smoothing should be length-wise, e.g., 3 (x) x 15 (y)
	// since the most systematic noise in OCT scan image has the horizontal strip like pattern, 
	// laterally wide kernel makes it stronger. 

	// Gaussian filter size should be a positive odd, and sigma could be assumed by size.   
	// http://docs.opencv.org/3.0-beta/modules/imgproc/doc/filtering.html#void GaussianBlur(InputArray src, OutputArray dst, Size ksize, double sigmaX, double sigmaY, int borderType)
	GaussianBlur(imgSrc->getCvMat(), imgOut->getCvMat(), ksize, 5.0, 1.0);

	// It seems that median is better than gaussian in noisy image. 
	// medianBlur(imgSrc->getCvMat(), imgOut->getCvMat(), 5);

	/*
	// In grayscale, dialation is the maximum of all the pixels in the input pixel's neighborhood.
	// http://www-rohan.sdsu.edu/doc/matlab/toolbox/images/morph4.html
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
	morphologyEx(pInMat->getCvMat(), pOutMat->getCvMat(), CV_MOP_ERODE, kernel, Point(-1,-1), 1);
	*/
	return true;
}


bool RetSegm::Preprocess::collectImageStats(SegmImage * imgSrc)
{
	Mat srcMat = imgSrc->getCvMat();

	// Mat_<type> can be more convinient to access elements with shorter notation. 
	// http://docs.opencv.org/master/df/dfc/classcv_1_1Mat__.html#details

	// Vec is a template class representing short numerical vectors. 
	// http://docs.opencv.org/master/d6/dcf/classcv_1_1Vec.html#details

	// Scalar is a tuple of 4 doubles. 
	// http://docs.opencv.org/3.0-beta/modules/core/doc/basic_structures.html#Scalar_
	Scalar mean, stddev;
	meanStdDev(srcMat, mean, stddev);

	// These result from the first channel of image. 
	imgSrc->setMeanStddev((float)mean(0), (float)stddev(0));

	// DebugOut2() << "Image stats, mean: " << std::fixed << setprecision(2) << mean(0) << ", stddev: " << stddev(0);
	return true;
}


bool RetSegm::Preprocess::collectColumnStats(SegmImage * imgSrc)
{
	Mat srcMat = imgSrc->getCvMat();

	// Column-wise mean, maximum of all columns.
	// reduce(matSrc, pInMat->m_maxOfCols, 0, CV_REDUCE_MAX);

	Mat means;
	reduce(srcMat, means, 0, CV_REDUCE_AVG);

	auto meanVect = std::vector<int>(means.cols);
	for (int c = 0; c < means.cols; c++) {
		meanVect[c] = means.at<uchar>(0, c);
	}
	imgSrc->setColumnMean(meanVect);

	Mat maxVals = Mat(1, srcMat.cols, CV_32SC1);
	Mat maxIdxs = Mat(1, srcMat.cols, CV_32SC1);

	double maxVal;
	Point maxLoc;
	for (int c = 0; c < srcMat.cols; c++) {
		minMaxLoc(srcMat.col(c), NULL, &maxVal, NULL, &maxLoc);
		maxVals.at<int>(0, c) = (int)maxVal;
		maxIdxs.at<int>(0, c) = (int)maxLoc.y;
	}

	// Don't need deep copy of Mat, since internally reference counted by header copy.
	const int* pVals = maxVals.ptr<int>(0);
	const int* pIdxs = maxIdxs.ptr<int>(0);
	imgSrc->setColumnMax(std::vector<int>(pIdxs, pIdxs + maxIdxs.cols), std::vector<int>(pVals, pVals + maxVals.cols));
	
	/*
	for (int c = 0; c < srcMat.cols; c++) {
		DebugOut2() << "column: " << c << ", mean: " << meanVect[c] << ", max_val: " << maxVals.at<int>(0, c);
	}
	*/
	return true;
}


bool RetSegm::Preprocess::checkIfValidScanImage(SegmImage * imgSrc, int index)
{
	float mean = imgSrc->getMean();
	float stddev = imgSrc->getStddev();

	if (mean < SAMPLE_IMAGE_MEAN_MIN || mean > SAMPLE_IMAGE_MEAN_MAX) {
		LogD() << "Sample index: " << index << ", invalid mean: " << mean;
		return false;
	}

	/*
	// In the following method, the standard deviation should be measured only from 
	// the object pixels except background. 

	// In the domain of digital image, SNR is considered as the ratio between 
	// mean (of object) to standard deviation (of background), however, it needs 
	// prior knowledge about both seperated regions. 

	// Here regards the mean of image as overall intensity level mainly covering background, 
	// and the standard deviation of image as signifiace of object (retina region), 
	// so that the ratio should be over the threshold to confirm its existence distinguishable. 
	float ratio = stddev / mean;
	if (ratio < SCAN_OBJECT_TO_BACKGROUND_RATIO_MIN) {
		DebugOut2() << "Invalid stddev/mean ratio: " << ratio << ", stddev: " << stddev << ", mean: " << mean;
		return false;
	}

	int threshold = (int)(mean + stddev * COLUMN_PEAK_STDDEV_FACTOR);
	int under = 0;
	auto vals = imgSrc->getColumMaxVals();
	for (int c = 0; c < vals.size(); c++) {
		if (vals[c] <= threshold) {
			under++;
		}
	}

	ratio = (float)under / imgSrc->getWidth();
	if (ratio > EMPTY_PEAK_COLUMNS_RATIO) {
		DebugOut2() << "Empty peak columns ratio: " << ratio << ", thresh: " << under;
		return false;
	}
	*/

	auto peaks = imgSrc->getColumMaxVals();
	auto means = imgSrc->getColumMeans();
	int under = 0;
	int sidx = (int)(peaks.size() / 5);
	int eidx = (int)(peaks.size() * 4 / 5);
	int tcnt = (eidx - sidx + 1);
	for (int c = sidx; c < eidx; c++) {
		float ratio = peaks[c] / (means[c] + 1.0f);
		if (ratio < SAMPLE_PEAK_TO_MEAN_RATIO_MIN) {
			under++;
		}
	}

	float poor = (float)under / tcnt; // peaks.size();
	if (poor >= SAMPLE_POOR_PEAK_TO_MEAN_AREA_MIN) {
		LogD() << "Sample index: " << index << ", poor SNR ratio: " << poor;
		return false;
	}
	return true;
}


bool RetSegm::Preprocess::checkIfValidScanImageOfCornea(SegmImage * imgSrc, int index)
{
	float mean = imgSrc->getMean();
	float stddev = imgSrc->getStddev();

	if (mean < SAMPLE_IMAGE_MEAN_MIN || mean > SAMPLE_IMAGE_MEAN_MAX) {
		LogD() << "Sample index: " << index << ", invalid mean: " << mean;
		return false;
	}

	auto peaks = imgSrc->getColumMaxVals();
	auto means = imgSrc->getColumMeans();
	int sidx = (int)(peaks.size() / 3);
	int eidx = (int)(peaks.size() * 2 / 3);
	int tcnt = (eidx - sidx + 1);
	int under = 0;
	for (int c = sidx; c < eidx; c++) {
		float ratio = peaks[c] / (means[c] + 1.0f);
		if (ratio < SAMPLE_PEAK_TO_MEAN_RATIO_MIN_CORNEA) {
			under++;
		}
	}

	float poor = (float)under / peaks.size();
	if (poor >= SAMPLE_POOR_PEAK_TO_MEAN_AREA_MIN) {
		LogD() << "Sample index: " << index << ", poor SNR ratio: " << poor;
		return false;
	}
	return true;
}


bool RetSegm::Preprocess::createSampleImageByRatio(const SegmImage * imgSrc, SegmImage * imgOut, float widthRatio, float heightRatio)
{
	int width = (int)(imgSrc->getWidth()*widthRatio);
	int height = (int)(imgSrc->getHeight()*heightRatio);

	// Limit minimum size of sample image. 
	width = (width < SAMPLE_WIDTH_MIN ? SAMPLE_WIDTH_MIN : width);
	height = (height < SAMPLE_HEIGHT_MIN ? SAMPLE_HEIGHT_MIN : height);

	return createSampleImageBySize(imgSrc, imgOut, width, height);
}


bool RetSegm::Preprocess::createSampleImageBySize(const SegmImage * imgSrc, SegmImage * imgOut, int targetWidth, int targetHeight)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();

	if (targetWidth <= 0 || targetHeight <= 0 || width <= 0 || height <= 0) {
		return false;
	}

	// This is a sampling process, the target size isn't supposed to be close to source size. 
	float widthRatio = 1.0f, heightRatio = 1.0f;
	float offset = (float)(width - targetWidth) / targetWidth;
	if (offset >= SAMPLE_RATIO_MIN) {
		widthRatio = (float)targetWidth / width;
	}

	offset = (float)(height - targetHeight) / targetHeight;
	if (offset >= SAMPLE_RATIO_MIN) {
		heightRatio = (float)targetHeight / height;
	}


	Size size((int)(width*widthRatio), (int)(height*heightRatio));
	resize(imgSrc->getCvMatConst(), imgOut->getCvMat(), size);

	// Reset the final sampling ratio to make this image. 
	widthRatio *= imgSrc->getSampleRatioWidth();
	heightRatio *= imgSrc->getSampleRatioHeight();
	imgOut->setSampleRatio(widthRatio, heightRatio);
	return true;
}




bool RetSegm::Preprocess::suppressMoirePattern(SegmImage * imgSrc)
{
	Mat matSrc = imgSrc->getCvMat();
	Mat matRoi;
	Scalar mean, stddev;

	// Recover the empty pixels by canceling out on top of image. 
	// This makes a weak descending edge from the start. 
	matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

	int threshold = (int)imgSrc->getMean();
	for (int r = 0; r < matRoi.rows; r++) {
		meanStdDev(matRoi.row(r), mean, stddev);
		if (mean(0) < MOIRE_CANCELED_REGION_MEAN) {
			uchar* p = matRoi.ptr(r);
			for (int c = 0; c < matRoi.cols; c++) {
				if (p[c] < threshold) {
					p[c] = threshold;
				}
			}
		}
	}

	// Mean and stddev in ROI region. 
	meanStdDev(matRoi, mean, stddev);

	DebugOut2() << "ROI stats: mean = " << std::fixed << setprecision(2) << mean(0) << ", stddev = " << stddev(0);

	double diff = mean(0) - imgSrc->getMean();

	if (diff > 0) {
		// Reassign ROI region assumed to be induced by moire. 
		matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

		// Mat rowMean;
		// reduce(matRoi, rowMean, 1, CV_REDUCE_AVG);

		// This subtraction of average difference for each row could cause the prominent border line 
		// with the following region not suppressed, in particular when the retina locates at the top.
		// So this corrected image should not be used for segmentation of the sub layers.

		// To mitigate this, additionally subtract by stddev of each row (not ROI region). 
		// If part of the retina overlaps moires, corretion amount would be reduced by the high sttdev. 
		for (int r = 0; r < matRoi.rows; r++) {
			meanStdDev(matRoi.row(r), mean, stddev);


			// double amount = rowMean.at<uchar>(r, 0) - pInMat->getMean() - stddev(0);
			int amount = (int)(mean(0) - imgSrc->getMean() - stddev(0));

#ifdef _DEBUG 
			// DebugOut() << "Rows : " << r << " row_mean = " << mean(0) << ", stddev = " << stddev(0) << ", amount = " << amount;
#endif

			if (amount > 0) {
				uchar* p = matRoi.ptr(r);
				for (int c = 0; c < matRoi.cols; c++) {
					if (p[c] > amount) {
						p[c] = (uchar)(p[c] - amount);
					}
				}
			}
		}
	}
	else
	{
		/*
		// Canceled region on top of the image during raw scan image processing to zeros should be recovered,
		// or else it cause unexpected edges all the way to the border with actual background.
		if (stddev(0) < MOIRE_CANCELED_REGION_STDDEV) {
		// Reassign ROI region assumed to be induced by moire.
		matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

		int threshold = (int)srcImg.getMean();
		for (int r = 0; r < matRoi.rows; r++) {
		uchar* p = matRoi.ptr(r);
		for (int c = 0; c < matRoi.cols; c++) {
		if (p[c] < threshold) {
		p[c] = threshold;
		}
		}
		}
		}
		*/
	}
	return true;
}



bool RetSegm::Preprocess::checkValidityOfScanImage(SegmImage * srcImg)
{
	float mean = srcImg->getMean();
	float stddev = srcImg->getStddev();

	if (mean < IMAGE_MEAN_MIN || mean > IMAGE_MEAN_MAX) {
		DebugOut2() << "Invalid mean level: " << mean;
		return false;
	}

	// In the domain of digital image, SNR is considered as the ratio between 
	// mean (of object) to standard deviation (of background), however, it needs 
	// prior knowledge about both seperated regions. 

	// Here regards the mean of image as overall intensity level mainly covering background, 
	// and the standard deviation of image as signifiace of object (retina region), 
	// so that the ratio should be over the threshold to confirm its existence distinguishable. 
	float ratio = stddev / mean;
	if (ratio < IMAGE_OBJECT_TO_MEAN_RATIO_MIN) {
		DebugOut2() << "Invalid stddev/mean ratio: " << ratio << ", stddev: " << stddev << ", mean: " << mean;
		return false;
	}

	// Part of retina located too close to the top of image could be cut off by the border. 

	// This is not appropriate, in case the retina shape is nearly flat, and ILM surface could be distinct from the top 
	// border of image. => top region should be further reduced, also the number of peaks located at. 
	float border = srcImg->getHeight() * IMAGE_TOP_REGION_RATIO;
	int topped = 0;
	auto locs = srcImg->getColumMaxLocs();
	for (int c = 0; c < locs.size(); c++) {
		if (locs[c] <= border) {
			topped++;
		}
	}

	ratio = (float)topped / srcImg->getWidth();
	if (ratio > COLUMN_MAX_ON_TOP_REGION_RATIO) {
		DebugOut2() << "Invalid top region: topped: " << topped << ", ratio: " << ratio;
		return false;
	}

	int threshold = (int)(mean + stddev * COLUMN_PEAK_STDDEV_FACTOR);
	int under = 0;
	auto vals = srcImg->getColumMaxVals();
	for (int c = 0; c < vals.size(); c++) {
		if (vals[c] <= threshold) {
			under++;
		}
	}

	ratio = (float)under / srcImg->getWidth();
	if (ratio > COLUMN_PEAK_EMPTY_RATIO) {
		DebugOut2() << "Not enough column peaks: under:" << under << ", ratio = " << ratio;
		return false;
	}

	return false;
}




// Deprecated from here!!!!
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RetSegm::Preprocess::createSampleImage(const ImageMat & srcImage, ImageMat & outImage, float widthRatio, float heightRatio)
{
	int width = (int)(srcImage.getWidth()*widthRatio);
	int height = (int)(srcImage.getHeight()*heightRatio);
	
	width = (width < SAMPLE_WIDTH_MIN ? SAMPLE_WIDTH_MIN : width);
	height = (height < SAMPLE_HEIGHT_MIN ? SAMPLE_HEIGHT_MIN : height);

	return createSampleImage(srcImage, outImage, width, height);
}

bool RetSegm::Preprocess::createSampleImage(const ImageMat& srcImage, ImageMat& outImage, int targetWidth, int targetHeight)
{
	int width = srcImage.getWidth();
	int height = srcImage.getHeight();

	if (targetWidth <= 0 || targetHeight <= 0 || width <= 0 || height <= 0) {
		return false;
	}

	// This is a sampling process, the target size is supposed to be fairly smaller than the source. 
	float widthRatio = 1.0f, heightRatio = 1.0f;
	float offset = (float)(width - targetWidth) / targetWidth;
	if (offset >= SAMPLE_RATIO_MIN) {
		widthRatio = (float)targetWidth / width;
	}

	offset = (float)(height - targetHeight) / targetHeight;
	if (offset >= SAMPLE_RATIO_MIN) {
		heightRatio = (float)targetHeight / height;
	}


	Size size((int)(srcImage.getWidth()*widthRatio), (int)(srcImage.getHeight()*heightRatio));
	resize(srcImage.getCvMatConst(), outImage.getCvMat(), size);

	// Sampling ratio to make this image. 
	widthRatio *= srcImage.getSampleRatioWidth();
	heightRatio *= srcImage.getSampleRatioHeight();
	outImage.setSampleRatio(widthRatio, heightRatio);
	return true;
}

bool RetSegm::Preprocess::collectImageStats(ImageMat& srcImg)
{
	Mat srcMat = srcImg.getCvMat();

	// Mat_<type> can be more convinient to access elements with shorter notation. 
	// http://docs.opencv.org/master/df/dfc/classcv_1_1Mat__.html#details

	// Vec is a template class representing short numerical vectors. 
	// http://docs.opencv.org/master/d6/dcf/classcv_1_1Vec.html#details

	// Scalar is a tuple of 4 doubles. 
	// http://docs.opencv.org/3.0-beta/modules/core/doc/basic_structures.html#Scalar_
	Scalar mean, stddev;
	meanStdDev(srcMat, mean, stddev);

	// These result from the first channel of image. 
	srcImg.setMeanStddev((float)mean(0), (float)stddev(0));

#ifdef _DEBUG
	DebugOut() << "Image stats : mean = " << std::fixed << setprecision(2) << mean(0) << ", stddev = " << stddev(0) ;
#endif

	return true;
}

bool RetSegm::Preprocess::collectColumnStats(ImageMat& srcImg)
{
	Mat srcMat = srcImg.getCvMat();

	// Column-wise mean, maximum of all columns.
	// reduce(matSrc, pInMat->m_maxOfCols, 0, CV_REDUCE_MAX);

	Mat means;
	reduce(srcMat, means, 0, CV_REDUCE_AVG);
	srcImg.setMeanOfColumns(means);

	Mat maxVals = Mat(1, srcMat.cols, CV_32SC1);
	Mat maxIdxs = Mat(1, srcMat.cols, CV_32SC1);

	double maxVal;
	Point maxLoc;
	for (int c = 0; c < srcMat.cols; c++) {
		minMaxLoc(srcMat.col(c), NULL, &maxVal, NULL, &maxLoc);
		maxVals.at<int>(0, c) = (int)maxVal;
		maxIdxs.at<int>(0, c) = (int)maxLoc.y;
	}

	// Don't need deep copy of Mat, since internally reference counted by header copy.
	srcImg.setMaxValOfColumns(maxVals);
	srcImg.setMaxIdxOfColumns(maxIdxs);
	return false;
}


bool RetSegm::Preprocess::smoothBackgroundNoise(ImageMat& srcImg, ImageMat& outImg)
{
	int wSize = NOISE_FILTER_WIDTH;
	int hSize = NOISE_FILTER_HEIGHT;
	Size ksize(wSize, hSize);

	// Gaussian filter size should be a positive odd, and sigma could be assumed by size.   
	// http://docs.opencv.org/3.0-beta/modules/imgproc/doc/filtering.html#void GaussianBlur(InputArray src, OutputArray dst, Size ksize, double sigmaX, double sigmaY, int borderType)
	GaussianBlur(srcImg.getCvMat(), outImg.getCvMat(), ksize, 0.0);

	// It seems that median is better than gaussian in noisy image. 
	// medianBlur(srcImg.getCvMat(), outImg.getCvMat(), NOISE_FILTER_SIZE);

	/*
	// In grayscale, dialation is the maximum of all the pixels in the input pixel's neighborhood.
	// http://www-rohan.sdsu.edu/doc/matlab/toolbox/images/morph4.html
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(9, 9));
	morphologyEx(pInMat->getCvMat(), pOutMat->getCvMat(), CV_MOP_ERODE, kernel, Point(-1,-1), 1);
	*/
	return true;
}


bool RetSegm::Preprocess::suppressMoirePattern(ImageMat& srcImg)
{
	Mat matSrc = srcImg.getCvMat();
	Mat matRoi;
	Scalar mean, stddev;

	// Recover the empty pixels by canceling out on top of image. 
	// This makes a weak descending edge from the start. 
	matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

	int threshold = (int)srcImg.getMean();
	for (int r = 0; r < matRoi.rows; r++) {
		meanStdDev(matRoi.row(r), mean, stddev);
		if (mean(0) < MOIRE_CANCELED_REGION_MEAN) {
			uchar* p = matRoi.ptr(r);
			for (int c = 0; c < matRoi.cols; c++) {
				if (p[c] < threshold) {
					p[c] = threshold;
				}
			}
		}
	}

	// Mean and stddev in ROI region. 
	meanStdDev(matRoi, mean, stddev);

#ifdef _DEBUG
	DebugOut() << "ROI stats : mean = " << std::fixed << setprecision(2) << mean(0) << ", stddev = " << stddev(0);
#endif

	double diff = mean(0) - srcImg.getMean();

	if (diff > 0) {
		// Reassign ROI region assumed to be induced by moire. 
		matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

		// Mat rowMean;
		// reduce(matRoi, rowMean, 1, CV_REDUCE_AVG);

		// This subtraction of average difference for each row could cause the prominent border line 
		// with the following region not suppressed, in particular when the retina locates at the top.
		// So this corrected image should not be used for segmentation of the sub layers.

		// To mitigate this, additionally subtract by stddev of each row (not ROI region). 
		// If part of the retina overlaps moires, corretion amount would be reduced by the high sttdev. 
		for (int r = 0; r < matRoi.rows; r++) {
			meanStdDev(matRoi.row(r), mean, stddev);
			

			// double amount = rowMean.at<uchar>(r, 0) - pInMat->getMean() - stddev(0);
			int amount = (int)(mean(0) - srcImg.getMean() - stddev(0));
			
#ifdef _DEBUG 
			// DebugOut() << "Rows : " << r << " row_mean = " << mean(0) << ", stddev = " << stddev(0) << ", amount = " << amount;
#endif

			if (amount > 0) {
				uchar* p = matRoi.ptr(r);
				for (int c = 0; c < matRoi.cols; c++) {
					if (p[c] > amount) {
						p[c] = (uchar) (p[c] - amount);
					}
				}
			}
		}
	}
	else
	{
		/*
		// Canceled region on top of the image during raw scan image processing to zeros should be recovered, 
		// or else it cause unexpected edges all the way to the border with actual background.
		if (stddev(0) < MOIRE_CANCELED_REGION_STDDEV) {
			// Reassign ROI region assumed to be induced by moire. 
			matRoi = matSrc(Rect(0, 0, matSrc.cols, (int)(matSrc.rows*MOIRE_MARGIN_RATIO)));

			int threshold = (int)srcImg.getMean();
			for (int r = 0; r < matRoi.rows; r++) {
				uchar* p = matRoi.ptr(r);
				for (int c = 0; c < matRoi.cols; c++) {
					if (p[c] < threshold) {
						p[c] = threshold;
					}
				}
			}
		}
		*/
	}
	return true;
}


bool RetSegm::Preprocess::checkValidityOfScanImage(ImageMat& srcImg)
{
	float mean = srcImg.getMean();
	float stddev = srcImg.getStddev();

	if (mean < IMAGE_MEAN_MIN || mean > IMAGE_MEAN_MAX) {
#ifdef _DEBUG
		DebugOut() << "Error: Image mean level is out of range! = " << mean;
#endif
		return false;
	}

	// In the domain of digital image, SNR is considered as the ratio between 
	// mean (of object) to standard deviation (of background), however, it needs 
	// prior knowledge about both seperated regions. 

	// Here regards the mean of image as overall intensity level mainly covering background, 
	// and the standard deviation of image as signifiace of object (retina region), 
	// so that the ratio should be over the threshold to confirm its existence distinguishable. 
	float ratio = stddev / mean;
	if (ratio < IMAGE_OBJECT_TO_MEAN_RATIO_MIN) {
#ifdef _DEBUG
		DebugOut() << "Error: Image stddev/mean ratio is too low! = " << ratio;
#endif
		return false;
	}

	// Part of retina located too close to the top of image could be cut off by the border. 

	// This is not appropriate, in case the retina shape is nearly flat, and ILM surface could be distinct from the top 
	// border of image. => top region should be further reduced, also the number of peaks located at. 
	float border = srcImg.getHeight() * IMAGE_TOP_REGION_RATIO;
	int topped = 0;
	for (int c = 0; c < srcImg.m_maxIdxCols.size(); c++) {
		if (srcImg.m_maxIdxCols[c] <= border) {
			topped++;
		}
	}

	ratio = (float)topped / srcImg.getWidth();
	if (ratio > COLUMN_MAX_ON_TOP_REGION_RATIO) {
#ifdef _DEBUG
		DebugOut() << "Error: Retina region is located on the top of image!, topped = " << topped << ", ratio = " << ratio;
#endif
		return false;
	}

	int threshold = (int)(mean + stddev * COLUMN_PEAK_STDDEV_FACTOR);
	int under = 0;
	for (int c = 0; c < srcImg.m_maxValCols.size(); c++) {
		if (srcImg.m_maxValCols[c] <= threshold) {
			under++;
		}
	}

	ratio = (float)under / srcImg.getWidth();
	if (ratio > COLUMN_PEAK_EMPTY_RATIO) {
#ifdef _DEBUG
		DebugOut() << "Error: Number of peaks from retina is too low, under = " << under << ", ratio = " << ratio;
#endif
		return false;
	}

	return false;
}