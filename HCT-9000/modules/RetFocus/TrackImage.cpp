#include "stdafx.h"
#include "TrackImage.h"
#include "CppUtil2.h"


using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct TrackImage::TrackImageImpl
{
	CvImage image;
	CvImage target;

	float sampleRatioX = 1.0f;
	float sampleRatioY = 1.0f;

	float targetStdev;
	int targetMean;
	int targetCx;
	int targetCy;

	int upperRegionMean = 0;
	int lowerRegionMean = 0;

	float matchValue;
	int matchCx;
	int matchCy;
	bool matched;

	TrackImageImpl() : targetCx(0), targetCy(0), targetMean(0), 
		upperRegionMean(0), lowerRegionMean(0), matched(false)
	{
	};
};


TrackImage::TrackImage()
	: d_ptr(make_unique<TrackImageImpl>())
{
}


TrackImage::~TrackImage()
{
}


RetFocus::TrackImage::TrackImage(TrackImage && rhs) = default;
TrackImage & RetFocus::TrackImage::operator=(TrackImage && rhs) = default;


bool RetFocus::TrackImage::checkIfEyelidsCovered(void) const
{
	const int EYELID_REGION_SX = (220 / 2);
	const int EYELID_REGION_EX = (420 / 2);
	const int UPPER_REGION_SY = (60 / 2);
	const int UPPER_REGION_EY = (160 / 2);
	const int LOWER_REGION_SY = (320 / 2);
	const int LOWER_REGION_EY = (420 / 2);
	const int LOOP_OFFSET = 4;

	Mat srcMat = getSource().getCvMat();

	int dsum1 = 0, dsum2 = 0;
	int dcnt1 = 0, dcnt2 = 0;
	for (int y = UPPER_REGION_SY; y <= UPPER_REGION_EY; y += LOOP_OFFSET) {
		const uchar* ptr = srcMat.ptr(y);
		for (int x = EYELID_REGION_SX; x <= EYELID_REGION_EX; x += LOOP_OFFSET) {
			dsum1 += ptr[x];
			dcnt1 += 1;
		}
	}

	for (int y = LOWER_REGION_SY; y <= LOWER_REGION_EY; y += LOOP_OFFSET) {
		const uchar* ptr = srcMat.ptr(y);
		for (int x = EYELID_REGION_SX; x <= EYELID_REGION_EX; x += LOOP_OFFSET) {
			dsum2 += ptr[x];
			dcnt2 += 1;
		}
	}

	int davg1 = dsum1 / dcnt1;
	int davg2 = dsum2 / dcnt2;

	getImpl().upperRegionMean = davg1;
	getImpl().lowerRegionMean = davg2;

	const int EYELIDS_COVERED_MEAN_MIN = 245;
	if (davg1 >= EYELIDS_COVERED_MEAN_MIN || davg2 >= EYELIDS_COVERED_MEAN_MIN) {
		return true;
	}
	return false;
}


bool RetFocus::TrackImage::decideTargetRegion(void)
{
	if (isEmpty()) {
		return false;
	}

	const int BLUR_SIZE = 17;
	getSource().applyGaussianBlur(0.0, BLUR_SIZE, BLUR_SIZE);

	Mat srcMat = getSource().getCvMat();
	const int RETINA_REGION_CX = (322 / 2); // (640 / 2);
	const int RETINA_REGION_CY = (240 / 2); // (480 / 2);
	const int RETINA_REGION_SX = (RETINA_REGION_CX - (210 / 2));
	const int RETINA_REGION_EX = (RETINA_REGION_CX + (210 / 2));
	const int RETINA_REGION_SY = (RETINA_REGION_CY - (80 / 2));
	const int RETINA_REGION_EY = (RETINA_REGION_CY + (80 / 2));
	const int KERNEL_W = (60 / 2) + 1;
	const int KERNEL_H = (80 / 2) + 1;
	const int KERNEL_OFFSET = 2;
	const int TARGET_VALUE_MIN = 45;

	int ry1 = RETINA_REGION_SY ;
	int ry2 = RETINA_REGION_EY - KERNEL_H;
	int rx1 = RETINA_REGION_SX; // +KERNEL_W;
	int rx2 = RETINA_REGION_EX - KERNEL_W;

	int pad_y = KERNEL_H / 2;
	int pad_x = KERNEL_W / 2;

	int max_dvar = 0;
	int max_mean = 0;
	int max_x = (rx1 + rx2) / 2;
	int max_y = (ry1 + ry2) / 2;
	float max_val = TARGET_VALUE_MIN;
	float max_stdev = 0.0f;

	for (int y = ry1; y <= ry2; y += KERNEL_OFFSET) {
		const uchar* ptr = srcMat.ptr(y);
		for (int x = rx1; x <= rx2; x += KERNEL_OFFSET) {
			if ((RETINA_REGION_CX - KERNEL_W - 15) <= x && x <= (RETINA_REGION_CX + 15)) { 
				continue;
			}

			/*
			if (ptr[x] < max_val) {
				continue;
			}
			max_val = ptr[x];
			max_x = x;
			max_y = y;
			*/

			int dsum = 0;
			int dcnt = 0;
			for (int ky = (y-pad_y); ky <= (y+pad_y); ky += KERNEL_OFFSET) {
				const uchar* ptr = srcMat.ptr(ky);
				for (int kx = (x-pad_x); kx <= (x+pad_x); kx += KERNEL_OFFSET) {
					dsum += ptr[kx];
					dcnt += 1;
				}
			}
			int davg = dsum / dcnt;
			int dvar = 0;
			for (int ky = (y - pad_y); ky <= (y + pad_y); ky += KERNEL_OFFSET) {
				const uchar* ptr = srcMat.ptr(ky);
				for (int kx = (x - pad_x); kx <= (x + pad_x); kx += KERNEL_OFFSET) {
					dvar += (int)pow((ptr[kx] - davg), 2.0f);
					dcnt += 1;
				}
			}

			float stdev = (float)sqrt(dvar);
			float val = stdev * (davg / 255.0f);

			if (val > max_val) {
				max_val = val;
				max_x = x;
				max_y = y;
				max_dvar = dvar;
				max_mean = davg;
				max_stdev = stdev;
			}
		}
	}

	int x = max_x;
	int y = max_y;
	max_val = 0.0f;
	for (int ky = (y - pad_y); ky <= (y + pad_y); ky++) {
		const uchar* ptr = srcMat.ptr(ky);
		for (int kx = (x - pad_x); kx <= (x + pad_x); kx++) {
			if (ptr[kx] >= max_val) {
				max_val = ptr[kx];
				max_x = kx;
				max_y = ky;
			}
		}
	}

	getImpl().targetMean = max_mean;
	getImpl().targetStdev = max_stdev;
	getImpl().targetCx = max_x;
	getImpl().targetCy = max_y;

	LogD() << "Retina Track target, mean: " << max_mean << ", stdev: " << max_stdev << ", cx: " << max_x << ", cy: " << max_y;


	const int TARGET_WIDTH = KERNEL_W;
	const int TARGET_HEIGHT = KERNEL_H;
	int hw = TARGET_WIDTH / 2;
	int hh = TARGET_HEIGHT / 2;

	CvImage patch;
	getImpl().image.copyTo(&patch);
	patch.crop(max_x - hw, max_y - hh, TARGET_WIDTH, TARGET_HEIGHT);
	patch.copyTo(&getImpl().target);
	return true;
}


bool RetFocus::TrackImage::measureSimilarity(void)
{
	if (getSource().isEmpty() || getTarget().isEmpty()) {
		return false;
	}

	Mat img = getSource().getCvMat();
	Mat templ = getTarget().getCvMat();

	// Mat img = image.getCvMat();

	/*
	float mean, stdev;
	float mean2, stdev2;
	getTarget().getMeanStddev(&mean, &stdev);
	getImage().getMeanStddev(&mean2, &stdev2);

	LogD() << "Target, " << mean << ", " << stdev << ", Image, " << mean2 << ", " << stdev2;
	*/

	Mat result;
	int resRows = img.rows - templ.rows + 1;
	int resCols = img.cols - templ.cols + 1;
	result.create(resCols, resRows, CV_32FC1);

	cv::matchTemplate(img, templ, result, TM_CCOEFF_NORMED);
	// normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc); // , cv::Mat());
	matchLoc = maxLoc;

	getImpl().matchValue = (float)maxVal;
	getImpl().matchCx = matchLoc.x + templ.cols / 2;
	getImpl().matchCy = matchLoc.y + templ.rows / 2;
	getImpl().matched = true;

	// LogD() << "Target matched, value: " << getImpl().matchValue << ", " << getImpl().matchCx << ", " << getImpl().matchCy;
	return true;
}


bool RetFocus::TrackImage::isValidTarget(void) const
{
	const int TARGET_MEAN_MIN = 45;
	const int TARGET_MEAN_MAX = 245;

	/*
	if (getImpl().targetMean <= TARGET_MEAN_MIN ||  
		getImpl().targetMean >= TARGET_MEAN_MAX) {
		return false;
	}
	*/

	if (getImpl().targetCx <= 0 || getImpl().targetCy <= 0) {
		return false;
	}
	if (isEmpty()) {
		return false;
	}
	return true;
}


bool RetFocus::TrackImage::isEmpty(void) const
{
	return getImpl().image.isEmpty();
}


float RetFocus::TrackImage::getTargetStdev(void) const
{
	return getImpl().targetStdev;
}


int RetFocus::TrackImage::getTargetMean(void) const
{
	return getImpl().targetMean;
}


int RetFocus::TrackImage::getTargetCx(void) const
{
	return (int)(getImpl().targetCx * getImpl().sampleRatioX);
}


int RetFocus::TrackImage::getTargetCy(void) const
{
	return (int)(getImpl().targetCy * getImpl().sampleRatioY);
}


int RetFocus::TrackImage::getTargetWidth(void) const
{
	return (int)(getTarget().getWidth() * getImpl().sampleRatioX);
}


int RetFocus::TrackImage::getTargetHeight(void) const
{
	return (int)(getTarget().getHeight() * getImpl().sampleRatioY);
}


int RetFocus::TrackImage::getMatchedCx(void) const
{
	return (int)(getImpl().matchCx * getImpl().sampleRatioX);
}


int RetFocus::TrackImage::getMatchedCy(void) const
{
	return (int)(getImpl().matchCy * getImpl().sampleRatioY);
}


float RetFocus::TrackImage::getMatchedValue(void) const
{
	return getImpl().matchValue;
}


bool RetFocus::TrackImage::isMatched(void) const
{
	return getImpl().matched;
}


void RetFocus::TrackImage::setSource(CppUtil::CvImage & image)
{
	CvImage sample;
	const int SAMPLE_WIDTH = 322;
	const int SAMPLE_HEIGHT = 240;

	getImpl().sampleRatioX = (float)image.getWidth() / SAMPLE_WIDTH;
	getImpl().sampleRatioY = (float)image.getHeight() / SAMPLE_HEIGHT;

	image.resizeTo(&sample, SAMPLE_WIDTH, SAMPLE_HEIGHT);
	getImpl().image = sample;
	getImpl().matched = false;
	return;
}


void RetFocus::TrackImage::setTarget(CppUtil::CvImage & image)
{
	getImpl().target = image;
	return;
}


CppUtil::CvImage & RetFocus::TrackImage::getSource(void) const
{
	return d_ptr->image;
}


CppUtil::CvImage & RetFocus::TrackImage::getTarget(void) const
{
	return d_ptr->target;
}

TrackImage::TrackImageImpl & RetFocus::TrackImage::getImpl(void) const
{
	return *d_ptr;
}
