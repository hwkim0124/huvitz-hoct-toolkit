#include "stdafx.h"
#include "PrepCornea.h"
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


PrepCornea::PrepCornea()
{
}


PrepCornea::~PrepCornea()
{
}


bool RetSegm::PrepCornea::createSampleImage(const SegmImage * imgSrc, SegmImage * imgOut, int width, int height)
{
	if (width == imgSrc->getWidth() && height == imgSrc->getHeight()) {
		imgSrc->getCvMatConst().copyTo(imgOut->getCvMat());
	}
	else {
		Size size(width, height);
		resize(imgSrc->getCvMatConst(), imgOut->getCvMat(), size);

		imgOut->applyGuidedFilter();
	}
	return true;
}


bool RetSegm::PrepCornea::checkIfValidScanImage(SegmImage * imgSrc)
{
	float mean = imgSrc->getMean();
	float stddev = imgSrc->getStddev();

	if (mean < SAMPLE_IMAGE_MEAN_MIN || mean > SAMPLE_IMAGE_MEAN_MAX) {
		LogD() << "Invalid mean level: " << mean;
		return false;
	}

	auto peaks = imgSrc->getColumMaxVals();
	auto means = imgSrc->getColumMeans();
	int under = 0;
	for (int c = 0; c < peaks.size(); c++) {
		float ratio = peaks[c] / (means[c] + 1.0f);
		if (ratio < SAMPLE_PEAK_TO_MEAN_RATIO_MIN_CORNEA) {
			under++;
		}
	}

	float poor = (float)under / peaks.size();
	LogD() << "Poor SNR ratio: " << poor;
	if (poor >= SAMPLE_POOR_PEAK_TO_MEAN_AREA_MIN) {
		return false;
	}
	return true;
}
