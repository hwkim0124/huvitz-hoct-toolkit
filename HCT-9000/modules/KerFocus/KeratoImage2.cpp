#include "stdafx.h"
#include "KeratoImage2.h"
#include "KeratoPoint.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace KerFocus;
using namespace std;
using namespace cv;


struct KeratoImage2::KeratoImage2Impl
{
	CvImage image;
	CvImage sample;

	int valMean;
	int threshOfPupil;
	int threshOfSpot;

	int eyeCx;
	int eyeCy;
	bool eyeFound;

	float focusLen;
	bool focusFound;

	float mireCx;
	float mireCy;
	float mireMl;
	float mireNl;
	float mireAx;
	bool ellipseFound;

	KerRectVect kerRects;
	KerSpotVect kerSpots;

	KeratoImage2Impl() {
	};
};



KeratoImage2::KeratoImage2()
	: d_ptr(make_unique<KeratoImage2Impl>())
{
}


KeratoImage2::~KeratoImage2()
{
}


KerFocus::KeratoImage2::KeratoImage2(KeratoImage2 && rhs) = default;
KeratoImage2 & KerFocus::KeratoImage2::operator=(KeratoImage2 && rhs) = default;


bool KerFocus::KeratoImage2::loadFile(const std::wstring & path)
{
	bool ret = getImage().fromFile(path);

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Load image file: " << wtoa(path) << ", result: " << ret;
	}

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool KerFocus::KeratoImage2::loadData(const unsigned char * pBits, int width, int height, int padding)
{
	bool ret = getImage().fromBitsData(pBits, width, height, padding);

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Load image data: w: " << width << ", h: " << height << ", result: " << ret;
	}

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool KerFocus::KeratoImage2::isEmpty(void) const
{
	return (getImage().isEmpty());
}


int KerFocus::KeratoImage2::getKeratoSpotsCount(void) const
{
	return (int)getImpl().kerSpots.size();
}


KerSpotVect & KerFocus::KeratoImage2::getKeratoSpots(void) const
{
	return getImpl().kerSpots;
}


KeratoImage2::KeratoImage2Impl & KerFocus::KeratoImage2::getImpl(void) const
{
	return *d_ptr;
}


void KerFocus::KeratoImage2::createSampleImage(void)
{
	int width = SAMPLE_IMAGE_WIDTH;
	int height = SAMPLE_IMAGE_HEIGHT;

	resize(getImage().getCvMat(), getSample().getCvMat(), Size(width, height));
	return;
}


void KerFocus::KeratoImage2::applyDenoiseFilter(void)
{
	int wsize = DENOISE_FILTER_SIZE;
	int hsize = DENOISE_FILTER_SIZE;
	wsize += (wsize % 2 == 0 ? 1 : 0);
	hsize += (hsize % 2 == 0 ? 1 : 0);

	//GaussianBlur(getSample().getCvMat(), getSample().getCvMat(), Size(wsize, hsize), 0.0);
	
	Scalar mean = cv::mean(getSample().getCvMat());
	getImpl().valMean = (int)mean(0);
	int thresh = (int)(getImpl().valMean * PUPIL_THRESHOLD_TO_MEAN_RATIO);
	thresh = (thresh < PUPIL_THRESHOLD_MIN ? PUPIL_THRESHOLD_MIN : thresh);
	thresh = (thresh > PUPIL_THRESHOLD_MAX ? PUPIL_THRESHOLD_MAX : thresh);
	getImpl().threshOfPupil = thresh;
	getImpl().threshOfSpot = SCAN_SPOT_THRESHOLD;

	getImpl().eyeFound = false;
	getImpl().focusFound = false;
	getImpl().ellipseFound = false;

	getImpl().kerRects.clear();
	getImpl().kerSpots.clear();
	return;
}


int KerFocus::KeratoImage2::scanKeratoSpots(void)
{
	Mat sample = getSample().getCvMat();
	int bgThresh = getImpl().threshOfPupil;
	int sgThresh = getImpl().threshOfSpot;

	for (int r = 20; r < SAMPLE_IMAGE_HEIGHT-20; r++) {
		const uchar* ptr = sample.ptr(r);
		bool found = false;
		int bgSize = 0;
		int bgDist = 0;
		int sgStart = 0;
		int sgEnd = 0;

		for (int c = 20; c < SAMPLE_IMAGE_WIDTH-20; c++) {
			if (!found) {
				if (ptr[c] < bgThresh) {
					bgSize++;
				}
				else {
					if (ptr[c] >= sgThresh) {
						if (bgSize >= SCAN_SPOT_BACK_SIZE) {
							sgStart = sgEnd = c;
							found = true;
						}
						bgSize = bgDist = 0;
					}
					else {
						bgSize = (bgSize < 0 ? 0 : (bgSize - 1));
					}
				}
			}
			else {
				if (ptr[c] >= sgThresh) {
					sgEnd = c;
				}
				else {
					if (ptr[c] < bgThresh) {
						if (++bgSize >= SCAN_SPOT_BACK_SIZE) {
							int sgSize = (sgEnd - sgStart + 1);
							if (sgSize >= SCAN_SPOT_HORZ_SIZE_MIN && sgSize <= SCAN_SPOT_HORZ_SIZE_MAX) {
								feedKeratoSpotRect(sgStart, r, sgEnd, r);
							}
							found = false;
						}
					}
					else {
						if (++bgDist > SCAN_SPOT_BACK_DISTANCE) {
							found = false;
						}
					}
				}
			}
		}
	}
	return findKeratoSpotCenters();
}


bool KerFocus::KeratoImage2::feedKeratoSpotRect(int sx, int sy, int ex, int ey)
{
	for (int i = 0; i < getImpl().kerRects.size(); i++) {
		if (getImpl().kerRects[i].enlargeToNextHorzLine(sx, sy, ex, ey)) {
			return false;
		}
	}
	getImpl().kerRects.emplace_back( sx, sy, ex, ey );
	return true;
}


int KerFocus::KeratoImage2::findKeratoSpotCenters(void)
{
	Mat sample = getSample().getCvMat();
	int thresh = KER_SPOT_THRESHOLD;

	KerRectVect& rects = getImpl().kerRects;
	for (int i = 0; i < rects.size(); i++) {
		float w = (float)rects[i].getWidth();
		float h = (float)rects[i].getHeight();
		float eccen = (w > h ? (w / h) : (h / w));
		if (eccen > KER_SPOT_ECCENT_MAX) {
			continue;
		}
	
		int xsum = 0;
		int ysum = 0;
		int wsum = 0;
		int wght = 0;
		int size = 0;
		rects[i].expand(SCAN_SPOT_RECT_OFFSET);
		if (rects[i]._sy >= 0 && rects[i]._ey < SAMPLE_IMAGE_HEIGHT
			&& rects[i]._sx >= 0 && rects[i]._ex < SAMPLE_IMAGE_WIDTH) 
		{
			for (int y = rects[i]._sy; y <= rects[i]._ey && y < SAMPLE_IMAGE_HEIGHT; y++) {
				const uchar* ptr = sample.ptr(y);
				for (int x = rects[i]._sx; x <= rects[i]._ex; x++) {
					if (ptr[x] >= thresh) {
						wght = ptr[x] - thresh + 1;
						xsum += x * wght;
						ysum += y * wght;
						wsum += wght;
						size++;
					}
				}
			}
		}
		
		if (size >= KER_SPOT_SIZE_MIN && size <= KER_SPOT_SIZE_MAX) {
			float ratio = 1.0f / SAMPLE_IMAGE_SIZE_RATIO;
			float cx = (float)xsum / (float)wsum;
			float cy = (float)ysum / (float)wsum;
			cx *= ratio;
			cy *= ratio;
			rects[i].resize(ratio, ratio);
			getImpl().kerSpots.emplace_back(cx, cy, size, thresh, rects[i]);
		}
	}

	return getKeratoSpotsCount();
}


bool KerFocus::KeratoImage2::estimateCenterOfEye(void)
{
	if (getKeratoSpotsCount() < 3) {
		return false;
	}

	float xsum = 0.0f;
	float ysum = 0.0f;
	int size = 0;
	for (auto spot : getImpl().kerSpots) {
		xsum += spot._cx;
		ysum += spot._cy;
		size++;
	}

	getImpl().eyeCx = (int)(xsum / size);
	getImpl().eyeCy = (int)(ysum / size);
	getImpl().eyeFound = true;

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Eye center: " << getImpl().eyeCx << ", " << getImpl().eyeCy;
	}
	return true;
}


bool KerFocus::KeratoImage2::isEyeFound(void) const
{
	return getImpl().eyeFound;
}


bool KerFocus::KeratoImage2::getEyeCenter(float & cx, float & cy) const
{
	if (isEyeFound()) {
		// If pupil not found, return with image center.
		cx = (float)(d_ptr->eyeCx);
		cy = (float)(d_ptr->eyeCy);
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage2::getCenterOffset(float & cx, float & cy) const
{
	float x, y, m, n, a;
	if (!getMireEllipse(x, y, m, n, a)) {
		if (!getEyeCenter(x, y)) {
			return false;
		}
	}
	
	cx = x - CAMERA_IMAGE_CENTER_X;
	cy = y - CAMERA_IMAGE_CENTER_Y;
	return true;
}


bool KerFocus::KeratoImage2::isEyeCenterAligned(void)
{
	if (!getImpl().eyeFound) {
		return false;
	}

	int cx = getImpl().eyeCx;
	int cy = getImpl().eyeCy;
	if (cx <= (CAMERA_IMAGE_WIDTH / 3) || cx >= (CAMERA_IMAGE_WIDTH * 2 / 3)) {
		return false;
	}
	if (cy <= (CAMERA_IMAGE_HEIGHT / 3) || cy >= (CAMERA_IMAGE_HEIGHT * 2 / 3)) {
		return false;
	}
	return true;
}


bool KerFocus::KeratoImage2::assignFocusSpots(void)
{
	if (getKeratoSpotsCount() < (KER_MIRE_SPOTS_NUM - 1)) {
		return false;
	}

	KerSpotVect& spots = getKeratoSpots();
	int dnVal = getImpl().eyeCy;
	int dnIdx = -1;
	int upVal = getImpl().eyeCy;
	int upIdx = -1;
	for (int i = 0; i < spots.size(); i++) {
		if ((spots[i]._cx >= (getImpl().eyeCx - KER_SPOTS_OFFSET)) &&
			(spots[i]._cx <= (getImpl().eyeCx + KER_SPOTS_OFFSET))) {
			if (spots[i]._cy > dnVal) {
				dnVal = (int)spots[i]._cy;
				dnIdx = i;
			}
			if (spots[i]._cy < upVal) {
				upVal = (int)spots[i]._cy;
				upIdx = i;
			}
		}
	}

	if (upIdx < 0 && dnIdx < 0) {
		return false;
	}

	int iStart = (upIdx < 0 ? 0 : upIdx);
	int lfIdx[3] = { 0 };
	int rgIdx[3] = { 0 };
	int lfSize = 0;
	int rgSize = 0;
	for (int i = iStart; i < spots.size(); i++) {
		if (spots[i]._cx < (getImpl().eyeCx - KER_SPOTS_OFFSET)) {
			if (lfSize < 3) {
				lfIdx[lfSize] = i;
				lfSize++;
			}
		}
		if (spots[i]._cx > (getImpl().eyeCx + KER_SPOTS_OFFSET)) {
			if (rgSize < 3) {
				rgIdx[rgSize] = i;
				rgSize++;
			}
		}
	}

	if (lfSize >= 3 && rgSize >= 3) {
		if (fabs(spots[lfIdx[1]]._cy - spots[rgIdx[1]]._cy) <= KER_SPOTS_OFFSET) {
			spots[lfIdx[1]]._focus = true;
			spots[rgIdx[1]]._focus = true;

			float xlen = spots[rgIdx[1]]._cx - spots[lfIdx[1]]._cx;
			float ylen = spots[rgIdx[1]]._cy - spots[lfIdx[1]]._cy;
			getImpl().focusLen = sqrt(xlen*xlen + ylen*ylen);
			getImpl().focusFound = true;
		}
	}

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Left spots: " << lfSize << ", Right spots: " << rgSize;
	}
	return true;
}


int KerFocus::KeratoImage2::getMireSpotsCount(void) const
{
	int count = 0;
	for (auto spot : getImpl().kerSpots) {
		if (spot._focus == false) {
			count++;
		}
	}
	return count;
}


int KerFocus::KeratoImage2::isFocusLength(void) const
{
	return getImpl().focusFound;
}


bool KerFocus::KeratoImage2::getFocusLength(float& flen) const
{
	flen = 0.0f;
	if (isFocusLength()) {
		flen = getImpl().focusLen;
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage2::measureMireRing(void)
{
	int snum = getMireSpotsCount();
	if (snum < (KER_MIRE_SPOTS_NUM - 1)) {
		return false;
	}

	Mat mat = Mat::zeros(snum, 2, CV_32F);
	int scnt = 0;
	for (auto spot : getImpl().kerSpots) {
		if (!spot.isFocus()) {
			mat.at<float>(scnt, 0) = spot._cx;
			mat.at<float>(scnt, 1) = spot._cy;
			scnt++;
		}
	}

	RotatedRect e = cv::fitEllipse(mat);
	d_ptr->mireCx = e.center.x;
	d_ptr->mireCy = e.center.y;
	d_ptr->mireMl = e.size.width;
	d_ptr->mireNl = e.size.height;
	d_ptr->mireAx = e.angle;
	d_ptr->ellipseFound = true;

	if (d_ptr->mireMl < d_ptr->mireNl) {
		float t = d_ptr->mireMl;
		d_ptr->mireMl = d_ptr->mireNl;
		d_ptr->mireNl = t;
	}

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Mire ellipse: " << e.size.width << ", " << e.size.height << ", " << e.angle << ", center: " << e.center.x << ", " << e.center.y;
	}
	return true;
}


bool KerFocus::KeratoImage2::isMireEllipse(void) const
{
	return d_ptr->ellipseFound;
}


bool KerFocus::KeratoImage2::getMireEllipse(float & cx, float & cy, float & m, float & n, float & a) const
{
	if (isMireEllipse() == false) {
		return false;
	}
	else {
		cx = d_ptr->mireCx;
		cy = d_ptr->mireCy;
		m = d_ptr->mireMl;
		n = d_ptr->mireNl;
		a = d_ptr->mireAx;
		return true;
	}
}


bool KerFocus::KeratoImage2::getMireLength(float & mire) const
{
	if (isMireEllipse() == false) {
		return false;
	}

	mire = (float)CppUtil::NumericFunc::horizontalLengthOfEllipse(d_ptr->mireMl, d_ptr->mireNl, d_ptr->mireAx);
	return true;
}


bool KerFocus::KeratoImage2::getFocusDifference(float & diff) const
{
	float mire, flen;
	diff = 0.0f;
	if (getMireLength(mire) && getFocusLength(flen)) {
		diff = mire - flen;
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage2::performAnalysis(void)
{
	if (isEmpty()) {
		return false;
	}

	createSampleImage();
	applyDenoiseFilter();
	scanKeratoSpots();
	printKeratoSpots();

	if (estimateCenterOfEye()) {
		if (isEyeCenterAligned()) {
			if (assignFocusSpots()) {
				measureMireRing();
				if (isFocusLength()) {
					if (isMireEllipse()) {
					}
				}
			}
		}
	}
	return true;
}


void KerFocus::KeratoImage2::printKeratoSpots(void) const
{
	KerSpotVect& spots = getImpl().kerSpots;
	for (int i = 0; i < spots.size(); i++) {
		if (KerFocus::DEBUG_OUT) {
			DebugOut2() << "Ker spots: " << i << ", cx: " << spots[i]._cx << ", cy: " << spots[i]._cy << ", size: " << spots[i]._size;
		}
	}
	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Ker spots " << spots.size() << " found";
	}
	return;
}


CvImage & KerFocus::KeratoImage2::getImage(void) const
{
	return d_ptr->image;
}


CvImage & KerFocus::KeratoImage2::getSample(void) const
{
	return d_ptr->sample;
}
