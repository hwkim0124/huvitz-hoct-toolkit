#include "stdafx.h"
#include "TrackImage2.h"
#include "CppUtil2.h"


using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct TrackImage2::TrackImage2Impl
{
	CvImage frame;
	CvImage target;

	float sampleRatioX = 1.0f;
	float sampleRatioY = 1.0f;

	float targetVal = 0.0f;
	float targetCx = -1;
	float targetCy = -1;
	float targetVal2 = 0.0f;
	float targetCx2 = -1;
	float targetCy2 = -1;

	float matchVal = 0.0f;
	float matchCx = -1;
	float matchCy = -1;

	TrackImage2Impl() {
	};
};


TrackImage2::TrackImage2()
	: d_ptr(make_unique<TrackImage2Impl>())
{
}


TrackImage2::~TrackImage2()
{
}


RetFocus::TrackImage2::TrackImage2(TrackImage2 && rhs) = default;
TrackImage2 & RetFocus::TrackImage2::operator=(TrackImage2 && rhs) = default;


void RetFocus::TrackImage2::initializeTrackImage(void)
{
	getImpl().sampleRatioX = 1.0f;
	getImpl().sampleRatioY = 1.0f;

	getImpl().targetVal = 0.0f;
	getImpl().targetCx = -1;
	getImpl().targetCy = -1;
	getImpl().targetVal2 = 0.0f;
	getImpl().targetCx2 = -1;
	getImpl().targetCy2 = -1;

	getImpl().matchVal = 0.0f;
	getImpl().matchCx = -1;
	getImpl().matchCy = -1;
	return;
}


void RetFocus::TrackImage2::setFrame(CppUtil::CvImage & image)
{
	if (image.isEmpty()) {
		return;
	}

	const int SAMPLE_WIDTH = 644;
	const int SAMPLE_HEIGHT = 480;

	if (image.getWidth() != SAMPLE_WIDTH || image.getHeight() != SAMPLE_HEIGHT) {
		getImpl().sampleRatioX = (float)image.getWidth() / SAMPLE_WIDTH;
		getImpl().sampleRatioY = (float)image.getHeight() / SAMPLE_HEIGHT;

		CvImage sample;
		image.resizeTo(&sample, SAMPLE_WIDTH, SAMPLE_HEIGHT);
		getFrame() = sample;
	}
	else {
		getFrame() = image;
	}
	return;
}


void RetFocus::TrackImage2::setTarget(CppUtil::CvImage & image)
{
	getImpl().target = image;
	return;
}


CppUtil::CvImage & RetFocus::TrackImage2::getFrame(void) const
{
	return d_ptr->frame;
}


CppUtil::CvImage & RetFocus::TrackImage2::getTarget(void) const
{
	return d_ptr->target;
}


bool RetFocus::TrackImage2::isTarget(void)
{
	return targetVal() >= 0.15f;
}


bool RetFocus::TrackImage2::getTargetInfo(float & cx, float & cy, float & w, float & h, float & val, int idx)
{
	if (isTarget()) {
		cx = targetCx(idx);
		cy = targetCy(idx);
		h = targetHeight();
		w = targetWidth();
		val = targetVal(idx);
		return true;
	}
	return false;
}


float RetFocus::TrackImage2::targetVal(int idx) const
{
	return (idx == 0 ? getImpl().targetVal : getImpl().targetVal2);
}


float RetFocus::TrackImage2::targetCx(int idx) const
{
	return ((idx == 0 ? getImpl().targetCx : getImpl().targetCx2) * getImpl().sampleRatioX);
}


float RetFocus::TrackImage2::targetCy(int idx) const
{
	return ((idx == 0 ? getImpl().targetCy : getImpl().targetCy2) * getImpl().sampleRatioY);
}


float RetFocus::TrackImage2::targetWidth(void) const
{
	// return (int)(getTarget().getWidth() * getImpl().sampleRatioX);
	return (99 * getImpl().sampleRatioX);
}


float RetFocus::TrackImage2::targetHeight(void) const
{
	// return (int)(getTarget().getHeight() * getImpl().sampleRatioY);
	return (99 * getImpl().sampleRatioY);
}


bool RetFocus::TrackImage2::isMatched(void)
{
	return matchedVal() > 0.85f; //  0.90f; //  0.95f;
}


bool RetFocus::TrackImage2::getMatchedInfo(float & cx, float & cy, float& w, float& h, float & val)
{
	if (isMatched()) {
		cx = matchedCx();
		cy = matchedCy();
		h = targetHeight();
		w = targetWidth();
		val = matchedVal();
		return true;
	}
	return false;
}


float RetFocus::TrackImage2::matchedCx(void) const
{
	return (getImpl().matchCx * getImpl().sampleRatioX);
}


float RetFocus::TrackImage2::matchedCy(void) const
{
	return (getImpl().matchCy * getImpl().sampleRatioY);
}


float RetFocus::TrackImage2::matchedVal(void) const
{
	return getImpl().matchVal;
}


bool RetFocus::TrackImage2::appointTargetOnFrame(EyeSide side, bool isDisc)
{
	auto& frame = getFrame();
	if (frame.isEmpty()) {
		return false;
	}

	CvImage image, grads;
	frame.copyTo(&image);
	image.applyGaussianBlur(9.0f);
	image.gradients(5, &grads);

	int w = image.getWidth();
	int h = image.getHeight();

	const int KERNEL_W = 99;
	const int KERNEL_H = 99;

	const int REGION_CX = (w / 2);
	const int REGION_CY = (h / 2);
	const int REGION_SX = (REGION_CX - 140);
	const int REGION_EX = (REGION_CX + 140);
	const int REGION_SY = (REGION_CY - 50);
	const int REGION_EY = (REGION_CY + 50);

	const int MASK_SX = REGION_CX - 70;
	const int MASK_EX = REGION_CX + 70;
	const int MASK_SY = REGION_CY - 16;
	const int MASK_EY = REGION_CY + 16;

	int kw2 = KERNEL_W / 2;
	int kh2 = KERNEL_H / 2;
	float max_val = -9999.0f;
	float max_g = -9999.0f;
	float max_w = -9999.0f;
	int max_x = -1;
	int max_y = -1;

	Mat matGrad = grads.getCvMat();
	Mat matInts = image.getCvMat();
	Mat matSrc = frame.getCvMat();

	for (int y = REGION_SY; y < REGION_EY; y += 2) {
		for (int x = REGION_SX; x < REGION_EX; x += 2) {
			if (isDisc) {
				if (x < MASK_SX || x > MASK_EX) {
					continue;
				}
			}
			else {
				if (x >= MASK_SX && x <= MASK_EX) {
					continue;
				}
				if (side == EyeSide::OD && x <= MASK_EX) {
					continue;
				}
				if (side == EyeSide::OS && x >= MASK_SX) {
					continue;
				}
			}

			// Accumulate gradients multiplied by intensity. 
			float dsum = 0.0f;
			float gsum = 0.0f;
			float wsum = 0.0f;
			for (int i = (y - kh2); i <= (y + kh2); i += 2) {
				const uchar* ptr = matGrad.ptr(i);
				const uchar* ptr2 = matInts.ptr(i);
				for (int j = (x - kw2); j <= (x + kw2); j += 2) {
					dsum += (ptr[j] * ptr2[j] / 128.0f);
					gsum += (ptr2[j]);
					wsum += (ptr[j]);
					// dsum += (ptr2[j] / 255.0f);
				}
			}
			
			float davg = dsum / (float)(kw2 * kh2);
			float wavg = wsum / (float)(kw2 * kh2);
			float gavg = gsum / (float)(kw2 * kh2);

			if (max_val < davg) {
				max_val = davg;
				max_g = gavg;
				max_w = wavg;
				max_x = x;
				max_y = y;
			}
		}
	}

	getImpl().targetVal2 = max_val;
	getImpl().targetCx2 = (float)max_x;
	getImpl().targetCy2 = (float)max_y;

	// LogD() << "Retina regist region1, val: " << max_val << ", g: " << max_g << ", w: " << max_w;

	float gsum = 0.0f;
	float xsum = 0.0f;
	float ysum = 0.0f;
	float cent_x = 0.0f; // xsum / gsum;
	float cent_y = 0.0f; // ysum / gsum;

	for (int i = max_y - kh2; i <= max_y + kh2; i++) {
		const uchar* ptr2 = matSrc.ptr(i);
		for (int j = max_x - kw2; j <= max_x + kw2; j++) {
			float w = max(ptr2[j] - max_g, 1.0f);
			// float w = ptr2[j];
			w = w * w;
			xsum += (w * j);
			ysum += (w * i);
			gsum += w;
		}
	}

	// LogD() << "Retina regist region2, val: " << gsum << ", x: " << xsum << ", y: " << ysum;

	cent_x = xsum / gsum;
	cent_y = ysum / gsum;

	// LogD() << "Retina regist region2, val: " << gsum << ", x: " << cent_x << ", y: " << cent_y;

	getImpl().targetVal = max_val;
	getImpl().targetCx = cent_x;
	getImpl().targetCy = cent_y;

	CvImage patch;
	getImpl().frame.copyTo(&patch);
	patch.crop((int)(cent_x - kw2), (int)(cent_y - kh2), KERNEL_W, KERNEL_H);
	patch.copyTo(&getImpl().target);
	return true;
}


bool RetFocus::TrackImage2::measureSimilarity(void)
{
	if (getFrame().isEmpty()) { 
		return false;
	}
	if (getTarget().isEmpty()) {
		return false;
	}

	Mat frame = getFrame().getCvMat();
	Mat templ = getTarget().getCvMat();

	// LogD() << "frame: " << frame.rows << ", " << frame.cols << ", templ: " << templ.rows << ", " << templ.cols;

	Mat result, mask;
	int resRows = frame.rows - templ.rows + 1;
	int resCols = frame.cols - templ.cols + 1;
	result.create(resCols, resRows, CV_32FC1);

	/*
	frame.copyTo(mask);
	mask.setTo(0);
	
	const int REGION_CX = (frame.cols / 2);
	const int REGION_CY = (frame.rows / 2);
	const int REGION_SX = (REGION_CX - 180);
	const int REGION_EX = (REGION_CX + 180);
	const int REGION_SY = (REGION_CY - 80);
	const int REGION_EY = (REGION_CY + 80);

	const int MASK_SX = REGION_CX - 20;
	const int MASK_EX = REGION_CX + 20;

	cv::rectangle(mask, Rect(REGION_SX, REGION_SY, (MASK_SX - REGION_SX), (REGION_EY - REGION_SY)), 255, -1);
	cv::rectangle(mask, Rect(MASK_EX, REGION_SY, (REGION_EX-MASK_EX), (REGION_EY - REGION_SY)), 255, -1);
	*/

	cv::matchTemplate(frame, templ, result, TM_CCOEFF_NORMED);

	// Localizing the best match with minMaxLoc
	double minVal; double maxVal; cv::Point minLoc; cv::Point maxLoc;
	cv::Point matchLoc;

	minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc); // , cv::Mat());
	matchLoc = maxLoc;

	getImpl().matchVal = (float)maxVal;
	getImpl().matchCx = (float)matchLoc.x + templ.cols / 2;
	getImpl().matchCy = (float)matchLoc.y + templ.rows / 2;

	// LogD() << "Target matched, value: " << getImpl().matchVal << ", " << getImpl().matchCx << ", " << getImpl().matchCy;
	return true;
}


TrackImage2::TrackImage2Impl & RetFocus::TrackImage2::getImpl(void) const
{
	return *d_ptr;
}
