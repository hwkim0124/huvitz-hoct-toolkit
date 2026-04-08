#include "stdafx.h"
#include "ImageMat.h"
#include "CppUtil2.h"

using namespace cv;

#include <iostream>
using namespace std;
using namespace RetSegm;

ImageMat::ImageMat()
{
	m_pCvMat = make_unique<cv::Mat>();
	m_sampleRatioWidth = 1.0f;
	m_sampleRatioHeight = 1.0f;
}

RetSegm::ImageMat::~ImageMat()
{
}


bool ImageMat::fromFile(std::wstring path)
{
	// Convert a path string in UTF-16 to UTF-8.
	string str = "";
	str.assign(path.begin(), path.end());

#ifdef _DEBUG
	DebugOut() << "Image file loaded : " << str;
#endif

	// c++ interface of opencv, IplImage and cvLoadImage has been deprecated. 
	Mat& cvMat = getCvMat();
	cvMat = imread(str.c_str(), IMREAD_GRAYSCALE);
	if (cvMat.empty()) {
#ifdef _DEBUG
		DebugOut() << "Can't open the file";
#endif
		return false;
	}


	/*
	// resize(m_imageMat, m_imageMat, cv::Size(), 1.0, 0.5, INTER_AREA);
	// GaussianBlur(m_imageMat, m_imageMat, Size(), 5.0);
	Mat matResult;
	m_imageMat.copyTo(matResult);
	bilateralFilter(m_imageMat, matResult, 15, 60.0, 20.0);
	m_imageMat = matResult;
	*/
	return true;
}

bool ImageMat::fromBitmap(int width, int height, const unsigned char* pBits, int paddingSize)
{
	// initialize matrix header that points to the bits data. no data is copied. 
	// if padding size is zero, no padding bytes at the end of each row is assumed. 
	// size_t is defined as unsigned long long in 64bit. 
	Mat& cvMat = getCvMat();
	cvMat = Mat(width, height, CV_8UC1, (void*)pBits, (size_t)(paddingSize == 0 ? Mat::AUTO_STEP : paddingSize));
	return true;
}

const unsigned char* ImageMat::getBitsData(void) const
{
	return m_pCvMat->data;
}

int RetSegm::ImageMat::getWidth(void) const
{
	return getCols();
}

int RetSegm::ImageMat::getHeight(void) const
{
	return getRows();
}

int RetSegm::ImageMat::getCols(void) const
{
	return m_pCvMat->cols;
}

int RetSegm::ImageMat::getRows(void) const
{
	return m_pCvMat->rows;
}

int ImageMat::getWidthStep(void) const
{
	return (int)m_pCvMat->step;
}

int RetSegm::ImageMat::getByteSize(void) const
{
	return getWidthStep()*getHeight();
}

void ImageMat::convertToRGB(ImageMat* pRGBMat) const
{
	cvtColor(*(m_pCvMat), pRGBMat->getCvMat(), CV_GRAY2RGBA);

	// byteSize = (int)(matRGB.total()*matRGB.elemSize());

	// Passed by a reference to pointer, it's C++ style!
	// refer: http://stackoverflow.com/questions/8494909/c-why-double-pointer-for-out-return-function-parameter
	// pBuff = new BYTE[byteSize];
	// memcpy(pBuff, matRGB.data, byteSize);
	return;
}

Gdiplus::Bitmap * RetSegm::ImageMat::createBitmap(void) const
{
	Mat matRgb;
	cvtColor(*(m_pCvMat), matRgb, CV_GRAY2RGBA);
	return CGdiPlus::CopyMatToBmp(matRgb);
}

cv::Mat& RetSegm::ImageMat::getCvMat(void)
{
	return *(m_pCvMat);
}

const cv::Mat & RetSegm::ImageMat::getCvMatConst(void) const
{
	// TODO: insert return statement here
	return *(m_pCvMat);
}

void RetSegm::ImageMat::setMeanStddev(float mean, float stddev)
{
	m_mean = mean;
	m_stddev = stddev;
	return;
}

void RetSegm::ImageMat::setMeanOfColumns(const cv::Mat & mean)
{
	// const int* p = mean.ptr<uchar>(0);
	// m_meanCols = std::vector<int>(p, p + mean.cols);
	m_meanCols = std::vector<int>(mean.cols);
	for (int c = 0; c < mean.cols; c++) {
		m_meanCols[c] = mean.at<uchar>(0, c);
	}
	return;
}

void RetSegm::ImageMat::setMaxValOfColumns(const cv::Mat & vals)
{
	const int* p = vals.ptr<int>(0);
	m_maxValCols = std::vector<int>(p, p + vals.cols);
	return;
}

void RetSegm::ImageMat::setMaxIdxOfColumns(const cv::Mat & idxs)
{
	const int* p = idxs.ptr<int>(0);
	m_maxIdxCols = std::vector<int>(p, p + idxs.cols);
	return;
}

