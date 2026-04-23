#include "stdafx.h"
#include "BscanImageData.h"

#include "CppUtil2.h"
#include "SemtRetina2.h"

using namespace SemtRetina;
using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;



struct BscanImageData::BscanImageDataImpl
{
	vector<float> col_means;
	vector<float> col_stdev;
    vector<float> row_means;

	vector<float> col_bgd_stdev;
	vector<float> col_bgd_means;
	vector<float> col_obj_means;
    vector<float> col_obj_stdev;
	vector<float> col_snr_ratio;

	float img_mean = 0.0f;
	float img_stdev = 0.0f;

	BscanImageDataImpl()
	{
	}
};


BscanImageData::BscanImageData() :
	d_ptr(make_unique<BscanImageDataImpl>())
{
}


SemtRetina::BscanImageData::~BscanImageData() = default;
SemtRetina::BscanImageData::BscanImageData(BscanImageData&& rhs) = default;
BscanImageData& SemtRetina::BscanImageData::operator=(BscanImageData&& rhs) = default;


SemtRetina::BscanImageData::BscanImageData(const BscanImageData& rhs)
	: d_ptr(make_unique<BscanImageDataImpl>(*rhs.d_ptr))
{
}


BscanImageData& SemtRetina::BscanImageData::operator=(const BscanImageData& rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}

void SemtRetina::BscanImageData::estimateStatitics(void)
{
	Mat matSrc = getCvMatConst();

	Scalar mean, stdev;
	meanStdDev(matSrc, mean, stdev);
	impl().img_mean = (float)mean[0];
	impl().img_stdev = (float)stdev[0];

	Mat matOut;
	reduce(matSrc, matOut, 0, CV_REDUCE_AVG);
	auto means = std::vector<float>(matOut.cols);
	for (int c = 0; c < matOut.cols; c++) {
		means[c] = (float)matOut.at<uchar>(0, c);
	}
	impl().col_means = means;

	reduce(matSrc, matOut, 1, CV_REDUCE_AVG);
	auto row_means = std::vector<float>(matOut.rows);
    for (int r = 0; r < matOut.rows; r++) {
        row_means[r] = (float)matOut.at<uchar>(r, 0);
    }
	impl().row_means = row_means;
    return;
}

void SemtRetina::BscanImageData::estimateColSnRatios(float percentBgd, float percentObj)
{
    Mat matSrc = getCvMatConst();

    int rows = matSrc.rows;
    int cols = matSrc.cols;

    std::vector<float> bgdMeans(cols, 0.0f);
    std::vector<float> bgdStdevs(cols, 0.0f);
    std::vector<float> objMeans(cols, 0.0f);
    std::vector<float> objStdevs(cols, 0.0f);
    std::vector<float> snRatios(cols, 0.0f);

    for (int c = 0; c < cols; ++c) {
        // Extract column as a vector<float>
        std::vector<float> colValues(rows);
        for (int r = 0; r < rows; ++r) {
            colValues[r] = static_cast<float>(matSrc.at<uchar>(r, c));
        }
        // Sort to compute percentiles
        std::sort(colValues.begin(), colValues.end());

        // Percentile indices (clamped)
        int idx25 = static_cast<int>(percentBgd * (rows - 1) + 0.5f);
        int idx99 = static_cast<int>(percentObj * (rows - 1) + 0.5f);
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
        }
        else {
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
            float mean = sum / static_cast<float>(objCount);
            objMeans[c] = mean;

            float sumSq = 0.0f;
            for (int i = objStart; i < rows; ++i) {
                sumSq += (colValues[i] - mean) * (colValues[i] - mean);
            }
            objStdevs[c] = sqrt(sumSq / (objCount > 1 ? (objCount - 1) : 1));
        }
        else {
            objMeans[c] = 0.0f;
            objStdevs[c] = 0.0f;
        }

        snRatios[c] = (objMeans[c] - bgdMeans[c]) / (bgdStdevs[c] + 0.0001f);
    }

    impl().col_bgd_means = bgdMeans;
    impl().col_bgd_stdev = bgdStdevs;
    impl().col_obj_means = objMeans;
    impl().col_obj_stdev = objStdevs;
    impl().col_snr_ratio = snRatios;
    return;
}

void SemtRetina::BscanImageData::resizeToMatchSample(int sampleWidth, int sampleHeight)
{
	int srcWidth = getWidth();

	Mat matDst;
    cv::resize(getCvMatConst(), matDst, Size(sampleWidth, sampleHeight), 0.0, 0.0, INTER_LINEAR);
	getCvMat() = matDst;

    if (!impl().col_bgd_means.empty() && impl().col_bgd_means.size() != sampleWidth) {
		// Interpolate column statistics to match new width
		std::vector<float> newColBgdMeans(sampleWidth);
		std::vector<float> newColBgdStdev(sampleWidth);
		std::vector<float> newColObjMeans(sampleWidth);
		std::vector<float> newColObjStdev(sampleWidth);
		std::vector<float> newColSnrRatio(sampleWidth);

		float scaleX = static_cast<float>(srcWidth) / static_cast<float>(sampleWidth);

        for (int i = 0; i < sampleWidth; ++i) {
            // Calculate the floating-point index in the original array
            float srcIdx = i * scaleX;
            int x1 = static_cast<int>(std::floor(srcIdx));
            int x2 = std::min(x1 + 1, static_cast<int>(impl().col_bgd_means.size()) - 1);

            // Calculate interpolation weight
            float weight = srcIdx - x1;

            // Linear interpolation: (1 - w) * val1 + w * val2
            newColBgdMeans[i] = (1.0f - weight) * impl().col_bgd_means[x1] + weight * impl().col_bgd_means[x2];
			newColBgdStdev[i] = (1.0f - weight) * impl().col_bgd_stdev[x1] + weight * impl().col_bgd_stdev[x2];
			newColObjMeans[i] = (1.0f - weight) * impl().col_obj_means[x1] + weight * impl().col_obj_means[x2];
			newColObjStdev[i] = (1.0f - weight) * impl().col_obj_stdev[x1] + weight * impl().col_obj_stdev[x2];
			newColSnrRatio[i] = (1.0f - weight) * impl().col_snr_ratio[x1] + weight * impl().col_snr_ratio[x2];
        }
        
        impl().col_bgd_means = std::move(newColBgdMeans);
		impl().col_bgd_stdev = std::move(newColBgdStdev);
		impl().col_obj_means = std::move(newColObjMeans);
		impl().col_obj_stdev = std::move(newColObjStdev);
		impl().col_snr_ratio = std::move(newColSnrRatio);
    }
	return;
}

void SemtRetina::BscanImageData::upscaleStatistics(int width, int height)
{
    if (!impl().col_bgd_means.empty() && impl().col_bgd_means.size() != width) {
        // Interpolate column statistics to match new width
        std::vector<float> newColBgdMeans(width);
        std::vector<float> newColBgdStdev(width);
        std::vector<float> newColObjMeans(width);
        std::vector<float> newColObjStdev(width);
        std::vector<float> newColSnrRatio(width);

		auto srcWidth = impl().col_bgd_means.size();
        float scaleX = static_cast<float>(srcWidth) / static_cast<float>(width);

        for (int i = 0; i < width; ++i) {
            // Calculate the floating-point index in the original array
            float srcIdx = i * scaleX;
            int x1 = static_cast<int>(std::floor(srcIdx));
            int x2 = std::min(x1 + 1, static_cast<int>(impl().col_bgd_means.size()) - 1);

            // Calculate interpolation weight
            float weight = srcIdx - x1;

            // Linear interpolation: (1 - w) * val1 + w * val2
            newColBgdMeans[i] = (1.0f - weight) * impl().col_bgd_means[x1] + weight * impl().col_bgd_means[x2];
            newColBgdStdev[i] = (1.0f - weight) * impl().col_bgd_stdev[x1] + weight * impl().col_bgd_stdev[x2];
            newColObjMeans[i] = (1.0f - weight) * impl().col_obj_means[x1] + weight * impl().col_obj_means[x2];
            newColObjStdev[i] = (1.0f - weight) * impl().col_obj_stdev[x1] + weight * impl().col_obj_stdev[x2];
            newColSnrRatio[i] = (1.0f - weight) * impl().col_snr_ratio[x1] + weight * impl().col_snr_ratio[x2];
        }

        impl().col_bgd_means = std::move(newColBgdMeans);
        impl().col_bgd_stdev = std::move(newColBgdStdev);
        impl().col_obj_means = std::move(newColObjMeans);
        impl().col_obj_stdev = std::move(newColObjStdev);
        impl().col_snr_ratio = std::move(newColSnrRatio);
    }
    return;
}

float SemtRetina::BscanImageData::imageMean(void) const
{
    return impl().img_mean;
}

float SemtRetina::BscanImageData::imageStdev(void) const
{
    return impl().img_stdev;
}

const std::vector<float>& SemtRetina::BscanImageData::columnMeans(void) const
{
    return impl().col_means;
}

const std::vector<float>& SemtRetina::BscanImageData::columnBgdMeans(void) const
{
    return impl().col_bgd_means;
}

const std::vector<float>& SemtRetina::BscanImageData::columnBgdStdev(void) const
{
    return impl().col_bgd_stdev;
}

const std::vector<float>& SemtRetina::BscanImageData::columnObjMeans(void) const
{
    return impl().col_obj_means;
}

const std::vector<float>& SemtRetina::BscanImageData::columnObjStdev(void) const
{
    return impl().col_obj_stdev;
}

const std::vector<float>& SemtRetina::BscanImageData::columnSnRatios(void) const
{
    return impl().col_snr_ratio;
}

const std::vector<float>& SemtRetina::BscanImageData::rowMeans(void) const
{
    return impl().row_means;
}

BscanImageData::BscanImageDataImpl& SemtRetina::BscanImageData::impl(void) const
{
	return *d_ptr;
}

