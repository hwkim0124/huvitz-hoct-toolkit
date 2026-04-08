#include "stdafx.h"
#include "SegmImage.h"
#include "CppUtil2.h"

using namespace RetSegm;
using namespace CppUtil;
using namespace cv;
using namespace std;


struct SegmImage::SegmImageImpl
{
	std::vector<int> colMeans;
	std::vector<float> colStddev;
	std::vector<int> colMaxVals;
	std::vector<int> colMaxLocs;

	std::vector<float> colBgdStdevs;
	std::vector<float> colBgdMeans;
	std::vector<float> colObjMeans;
	std::vector<float> colSnRatios;

	float mean;
	float stddev;
	float sampleRatioWidth;
	float sampleRatioHeight;

	SegmImageImpl() :
		mean(0.0f), stddev(0.0f), sampleRatioWidth(1.0f), sampleRatioHeight(1.0f) 
	{
	}
};


SegmImage::SegmImage() :
	d_ptr(make_unique<SegmImageImpl>())
{
}


RetSegm::SegmImage::~SegmImage() = default;
RetSegm::SegmImage::SegmImage(SegmImage && rhs) = default;
SegmImage & RetSegm::SegmImage::operator=(SegmImage && rhs) = default;


RetSegm::SegmImage::SegmImage(const SegmImage & rhs)
	: d_ptr(make_unique<SegmImageImpl>(*rhs.d_ptr))
{
}


SegmImage & RetSegm::SegmImage::operator=(const SegmImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void RetSegm::SegmImage::setSampleRatio(float wRatio, float hRatio)
{
	d_ptr->sampleRatioWidth = wRatio;
	d_ptr->sampleRatioHeight = hRatio;
	return;
}


float RetSegm::SegmImage::getSampleRatioWidth(void) const
{
	return d_ptr->sampleRatioWidth;
}


float RetSegm::SegmImage::getSampleRatioHeight(void) const
{
	return d_ptr->sampleRatioHeight;
}


float RetSegm::SegmImage::getWidthRatioToSample(void) const
{
	float ratio = (float) getWidth() / SAMPLE_IMAGE_WIDTH;
	return ratio;
}


float RetSegm::SegmImage::getHeightRatioToSample(void) const
{
	float ratio = (float)getHeight() / SAMPLE_IMAGE_HEIGHT;
	return ratio;
}


float RetSegm::SegmImage::getMean(void) const
{
	return d_ptr->mean;
}


float RetSegm::SegmImage::getStddev(void) const
{
	return d_ptr->stddev;
}


void RetSegm::SegmImage::setMeanStddev(float mean, float stddev)
{
	d_ptr->mean = mean;
	d_ptr->stddev = stddev;
	return;
}


const std::vector<int>& RetSegm::SegmImage::getColumMaxLocs(void) const
{
	return d_ptr->colMaxLocs;
}


const std::vector<int>& RetSegm::SegmImage::getColumMaxVals(void) const
{
	return d_ptr->colMaxVals;
}


const std::vector<int>& RetSegm::SegmImage::getColumMeans(void) const
{
	return d_ptr->colMeans;
}

const std::vector<float>& RetSegm::SegmImage::getColumnSnRatios(void) const
{
	return d_ptr->colSnRatios;
}


void RetSegm::SegmImage::setColumnMax(const std::vector<int>& maxLocs, const std::vector<int>& maxVals)
{
	d_ptr->colMaxLocs = maxLocs;
	d_ptr->colMaxVals = maxVals;
	return;
}


void RetSegm::SegmImage::setColumnMean(const std::vector<int>& means)
{
	d_ptr->colMeans = means;
	return;
}


void RetSegm::SegmImage::setColumnStddev(const std::vector<float>& stddev)
{
	d_ptr->colStddev = stddev;
	return;
}

void RetSegm::SegmImage::updateImageStats(void)
{
	Mat srcMat = getCvMatConst();

	// Mat_<type> can be more convinient to access elements with shorter notation. 
	// http://docs.opencv.org/master/df/dfc/classcv_1_1Mat__.html#details

	// Vec is a template class representing short numerical vectors. 
	// http://docs.opencv.org/master/d6/dcf/classcv_1_1Vec.html#details

	// Scalar is a tuple of 4 doubles. 
	// http://docs.opencv.org/3.0-beta/modules/core/doc/basic_structures.html#Scalar_
	Scalar mean, stddev;
	meanStdDev(srcMat, mean, stddev);

	// These result from the first channel of image. 
	setMeanStddev((float)mean(0), (float)stddev(0));
	return;
}

void RetSegm::SegmImage::updateColumnStats(void)
{
	Mat srcMat = getCvMatConst();

	// Column-wise mean, maximum of all columns.
	// reduce(matSrc, pInMat->m_maxOfCols, 0, CV_REDUCE_MAX);

	Mat means;
	reduce(srcMat, means, 0, CV_REDUCE_AVG);

	auto meanVect = std::vector<int>(means.cols);
	for (int c = 0; c < means.cols; c++) {
		meanVect[c] = means.at<uchar>(0, c);
	}
	setColumnMean(meanVect);

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
	setColumnMax(std::vector<int>(pIdxs, pIdxs + maxIdxs.cols), std::vector<int>(pVals, pVals + maxVals.cols));
	return ;
}

void RetSegm::SegmImage::computeColumnSnRatios(void)
{
	Mat srcMat = getCvMatConst();

    // Get colBgdMeans, colBgdStdevs by column percentile 25 (lower), and colObjMeans by column percentile 99 (upper).
    int rows = srcMat.rows;
    int cols = srcMat.cols;

    std::vector<float> bgdMeans(cols, 0.0f);
    std::vector<float> bgdStdevs(cols, 0.0f);
    std::vector<float> objMeans(cols, 0.0f);
	std::vector<float> snRatios(cols, 0.0f);

    for (int c = 0; c < cols; ++c) {
        // Extract column as a vector<float>
        std::vector<float> colValues(rows);
        for (int r = 0; r < rows; ++r) {
            colValues[r] = static_cast<float>(srcMat.at<uchar>(r, c));
        }
        // Sort to compute percentiles
        std::sort(colValues.begin(), colValues.end());

        // Percentile indices (clamped)
        int idx25 = static_cast<int>(0.25f * (rows - 1) + 0.5f);
        int idx99 = static_cast<int>(0.99f * (rows - 1) + 0.5f);
        idx25 = std::max(0, std::min(rows - 1, idx25));
        idx99 = std::max(0, std::min(rows - 1, idx99));

        // Compute mean and stdev of lower 25% (bgd)
        int bgdCount = idx25 + 1;
        if (bgdCount > 0) {
            float sum = 0.0f;
            for (int i = 0; i < bgdCount; ++i) {
				sum += colValues[i];
			}
            float mean = sum / static_cast<float>(bgdCount);
            bgdMeans[c] = mean;

            float sumSq = 0.0f;
            for (int i = 0; i < bgdCount; ++i) {
                sumSq += (colValues[i] - mean) * (colValues[i] - mean);
			}
            bgdStdevs[c] = sqrt(sumSq / (bgdCount > 1 ? (bgdCount - 1) : 1));
        } else {
            bgdMeans[c] = 0.0f;
            bgdStdevs[c] = 0.0f;
        }

        // Compute mean of upper 1% (obj)
        int objStart = idx99;
        int objCount = rows - objStart;
        if (objCount > 0) {
            float sum = 0.0f;
            for (int i = objStart; i < rows; ++i) {
				sum += colValues[i];
			} 
            objMeans[c] = sum / static_cast<float>(objCount);
        } else {
            objMeans[c] = 0.0f;
        }

		snRatios[c] = (objMeans[c] - bgdMeans[c]) / (bgdStdevs[c] + 0.0001f);
    }

    d_ptr->colBgdMeans = bgdMeans;
    d_ptr->colBgdStdevs = bgdStdevs;
    d_ptr->colObjMeans = objMeans;
	d_ptr->colSnRatios = snRatios;
	return;
}
