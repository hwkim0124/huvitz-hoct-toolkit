#include "stdafx.h"
#include "RetinaFrame.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace RetFocus;
using namespace std;
using namespace cv;


struct RetinaFrame::RetinaFrameImpl
{
	CvImage image;
	CvImage sample;

	float splitOffset;
	bool splitFound;
	float uppSplitX;
	float uppSplitY;
	float lowSplitX;
	float lowSplitY;
	int uppWinCenX;
	int lowWinCenX;
	bool isUppSplit;
	bool isLowSplit;

	float upperPeak;
	float lowerPeak;
	float upperFwhm;
	float lowerFwhm;
	float upperSNR;
	float lowerSNR;

	int centerX;
	int centerY;
	int kernelWidth;
	int kernelHeight;
	
	int uppRangeX[2];
	int uppRangeY[2];
	int lowRangeX[2];
	int lowRangeY[2];

	float wDotXs[2];
	float wDotYs[2];
	int wDotSizes[2];
	int wDotMeans[2];
	bool wDotValids[2];

	pair<int, int> discRegionPos1;
	pair<int, int> discRegionPos2;
	pair<float, float> discCenterPos;

	array<float, SPLIT_CONVS_RANGE> upperConvs;
	array<float, SPLIT_CONVS_RANGE> lowerConvs;

	RetinaFrameImpl() : splitFound(false), upperConvs{ 0.0f }, lowerConvs{ 0.0f }, 
		isUppSplit(false), isLowSplit(false)
	{
	};
};

 
RetinaFrame::RetinaFrame()
	: d_ptr(make_unique<RetinaFrameImpl>())
{
}


RetFocus::RetinaFrame::RetinaFrame(const std::wstring & path)
{
	loadFile(path);
}


RetinaFrame::~RetinaFrame()
{
	// Destructor should be defined for unique_ptr to delete RetinaFrameImpl as an incomplete type.
}


RetFocus::RetinaFrame::RetinaFrame(RetinaFrame && rhs) = default;
RetinaFrame & RetFocus::RetinaFrame::operator=(RetinaFrame && rhs) = default;



bool RetFocus::RetinaFrame::loadFile(const std::wstring & path)
{
	bool ret = getImage().fromFile(path);

	if (DEBUG_OUT) {
		DebugOut2() << "Retina image file: " << wtoa(path) << ", result: " << ret;
	}

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool RetFocus::RetinaFrame::loadData(const unsigned char * pBits, int width, int height, int padding)
{
	bool ret = getImage().fromBitsData(pBits, width, height, padding);

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool RetFocus::RetinaFrame::isEmpty(void) const
{
	return (getImage().isEmpty());
}

bool RetFocus::RetinaFrame::isOpticDiscFound(void) const
{
	return (getImpl().discCenterPos.first > 0 && getImpl().discCenterPos.second > 0);
}

bool RetFocus::RetinaFrame::getOpticDiscCenter(float & cx, float & cy)
{
	if (!isOpticDiscFound()) {
		return false;
	}

	cx = getImpl().discCenterPos.first;
	cy = getImpl().discCenterPos.second;
	return true;
}

bool RetFocus::RetinaFrame::getOpticDiscRegion(int & x1, int & y1, int & x2, int & y2)
{
	if (!isOpticDiscFound()) {
		return false;
	}

	x1 = getImpl().discRegionPos1.first;
	y1 = getImpl().discRegionPos1.second;
	x2 = getImpl().discRegionPos2.first;
	y2 = getImpl().discRegionPos2.second;
	return true;
}


bool RetFocus::RetinaFrame::isSplitFocus(void) const
{
	return (isUpperSplit() || isLowerSplit());
}


bool RetFocus::RetinaFrame::isUpperSplit(void) const
{
	return d_ptr->isUppSplit;
}


bool RetFocus::RetinaFrame::isLowerSplit(void) const
{
	return d_ptr->isLowSplit;
}


float RetFocus::RetinaFrame::getSplitOffset(void) const
{
	return d_ptr->splitOffset;
}


float RetFocus::RetinaFrame::getSplitUpperX(void) const
{
	return d_ptr->uppSplitX;
}


float RetFocus::RetinaFrame::getSplitUpperY(void) const
{
	return d_ptr->uppSplitY;
}


float RetFocus::RetinaFrame::getSplitLowerX(void) const
{
	return d_ptr->lowSplitX;
}


float RetFocus::RetinaFrame::getSplitLowerY(void) const
{
	return d_ptr->lowSplitY;
}


float RetFocus::RetinaFrame::getSplitUpperFwhm(void) const
{
	return d_ptr->upperFwhm;
}


float RetFocus::RetinaFrame::getSplitLowerFwhm(void) const
{
	return d_ptr->lowerFwhm;
}


float RetFocus::RetinaFrame::getSplitUpperSNR(void) const
{
	return d_ptr->upperSNR;
}


float RetFocus::RetinaFrame::getSplitLowerSNR(void) const
{
	return d_ptr->lowerSNR;
}


float RetFocus::RetinaFrame::getSplitUpperPeak(void) const
{
	return d_ptr->upperPeak;
}


float RetFocus::RetinaFrame::getSplitLowerPeak(void) const
{
	return d_ptr->lowerPeak;
}


bool RetFocus::RetinaFrame::getSplitWindow(int index, int & x1, int & y1, int & x2, int & y2)
{
	if (index == 0) {
		if (!isUpperSplit()) {
			return false;
		}
		int cx = (int)d_ptr->uppWinCenX;
		x1 = cx - SPLIT_FOCUS_WIDTH / 2;
		x2 = cx + SPLIT_FOCUS_WIDTH / 2;
		y1 = d_ptr->uppRangeY[0];
		y2 = d_ptr->uppRangeY[1];
	}
	else {
		if (!isLowerSplit()) {
			return false;
		}
		int cx = (int)d_ptr->lowWinCenX;
		x1 = cx - SPLIT_FOCUS_WIDTH / 2;
		x2 = cx + SPLIT_FOCUS_WIDTH / 2;
		y1 = d_ptr->lowRangeY[0];
		y2 = d_ptr->lowRangeY[1];
	}
	return true;
}


bool RetFocus::RetinaFrame::getWorkingDot(int index, float & x, float & y, int & size, int & mean)
{
	if (index < 0 || index > 1) {
		return false;
	}

	if (!d_ptr->wDotValids[index]) {
		return false;
	}

	x = d_ptr->wDotXs[index];
	y = d_ptr->wDotYs[index];
	size = d_ptr->wDotSizes[index];
	mean = d_ptr->wDotMeans[index];
	return true;
}


RetinaFrame::RetinaFrameImpl & RetFocus::RetinaFrame::getImpl(void) const
{
	return *d_ptr;
}


void RetFocus::RetinaFrame::initialize(void)
{
	d_ptr->splitFound = false;
	d_ptr->isUppSplit = false;
	d_ptr->isLowSplit = false;

	d_ptr->uppSplitX = 0.0f;
	d_ptr->uppSplitY = 0.0f;
	d_ptr->lowSplitX = 0.0f;
	d_ptr->lowSplitY = 0.0f;
	d_ptr->splitOffset = 0.0f;

	auto center = GlobalSettings::splitFocusCenter();
	auto kernel = GlobalSettings::splitFocusKernel();

	d_ptr->centerX = SPLIT_CENTER_X;
	d_ptr->centerY = SPLIT_CENTER_Y;

	if (center.first >= SPLIT_CENTER_X_MIN && center.first <= SPLIT_CENTER_X_MAX) {
		d_ptr->centerX = center.first;
	}
	if (center.second >= SPLIT_CENTER_Y_MIN && center.second <= SPLIT_CENTER_Y_MAX) {
		d_ptr->centerY = center.second;
	}

	d_ptr->kernelWidth = SPLIT_KERNEL_WIDTH; // (kernel.first <= 0 ? SPLIT_KERNEL_WIDTH : kernel.first);
	d_ptr->kernelHeight = SPLIT_KERNEL_HEIGHT; // (kernel.second <= 0 ? SPLIT_KERNEL_HEIGHT : kernel.second);

	d_ptr->uppRangeY[0] = d_ptr->centerY - d_ptr->kernelHeight;
	d_ptr->uppRangeY[1] = d_ptr->centerY - SPLIT_KERNEL_OFFSET;
	d_ptr->lowRangeY[0] = d_ptr->centerY + SPLIT_KERNEL_OFFSET;
	d_ptr->lowRangeY[1] = d_ptr->centerY + d_ptr->kernelHeight;

	d_ptr->uppRangeX[0] = d_ptr->centerX - SPLIT_CONVS_RANGE / 2;
	d_ptr->uppRangeX[1] = d_ptr->centerX + SPLIT_CONVS_RANGE / 2;
	d_ptr->lowRangeX[0] = d_ptr->centerX - SPLIT_CONVS_RANGE / 2;
	d_ptr->lowRangeX[1] = d_ptr->centerX + SPLIT_CONVS_RANGE / 2;

	if (DEBUG_OUT) {
		LogD() << "Center x, y: " << d_ptr->centerX << ", " << d_ptr->centerY;
		LogD() << "Upper range, x: " << d_ptr->uppRangeX[0] << ", " << d_ptr->uppRangeX[1];
		LogD() << "Upper range, y: " << d_ptr->uppRangeY[0] << ", " << d_ptr->uppRangeY[1];
		LogD() << "Lower range, x: " << d_ptr->lowRangeX[0] << ", " << d_ptr->lowRangeX[1];
		LogD() << "Lower range, y: " << d_ptr->lowRangeY[0] << ", " << d_ptr->lowRangeY[1];
	}

	d_ptr->upperConvs.fill(0.0f);
	d_ptr->lowerConvs.fill(0.0f);

	d_ptr->wDotValids[0] = false;
	d_ptr->wDotValids[1] = false;

	d_ptr->discRegionPos1 = pair<int, int>(0, 0);
	d_ptr->discRegionPos2 = pair<int, int>(0, 0);
	d_ptr->discCenterPos = pair<float, float>(0.0f, 0.0f);
	return;
}


bool RetFocus::RetinaFrame::performAnalysis(void)
{
	bool detectSplit = true; // false;
	bool detectWdots = false;
	bool detectOdisc = false;// true;

	initialize();

	if (isEmpty()) {
		return false;
	}

	createSampleImage();

	if (detectSplit) {
		locateWorkingDots();
		makeUpperConvolutions();
		makeLowerConvolutions();

		assignUpperSplit();
		assignLowerSplit();

		if (isUpperSplit() || isLowerSplit()) {
			d_ptr->splitFound = true;
			decideSplitOffset();
		}
	}
	if (detectOdisc) {
		if (transformFeatureImage()) {
			locateOpticDiscFeature();
		}
	}
	return true;
}


bool RetFocus::RetinaFrame::createSampleImage(void)
{
	getSample().getCvMat() = getImage().getCvMat();
	return true;
}


bool RetFocus::RetinaFrame::locateWorkingDots(void)
{
	Mat sample = getSample().getCvMat();

	int thresh = 150;
	int gsum = 0; 
	int gcnt = 0;
	int gwgh = 0;
	int xsum = 0;
	int ysum = 0;

	for (int r = 160; r < 320; r += 2) {
		const uchar* ptr = sample.ptr(r);
		for (int c = 20; c < 200; c += 2) {
			if (ptr[c] > thresh) {
				gwgh = ptr[c];
				xsum += (c * gwgh);
				ysum += (r * gwgh);
				gsum += gwgh;
				gcnt += 1;
			}
		}
	}

	if (gcnt > 0) {
		d_ptr->wDotXs[0] = (float)xsum / (float)gsum;
		d_ptr->wDotYs[0] = (float)ysum / (float)gsum;
		d_ptr->wDotSizes[0] = gcnt * 4;
		d_ptr->wDotMeans[0] = gsum / gcnt;
		d_ptr->wDotValids[0] = true;
	}

	gsum = gcnt = gwgh = 0;
	xsum = ysum = 0;

	for (int r = 160; r < 320; r += 2) {
		const uchar* ptr = sample.ptr(r);
		for (int c = 440; c < 620; c += 2) {
			if (ptr[c] > thresh) {
				gwgh = ptr[c];
				xsum += (c * gwgh);
				ysum += (r * gwgh);
				gsum += gwgh;
				gcnt += 1;
			}
		}
	}

	if (gcnt > 0) {
		d_ptr->wDotXs[1] = (float)xsum / (float)gsum;
		d_ptr->wDotYs[1] = (float)ysum / (float)gsum;
		d_ptr->wDotSizes[1] = gcnt * 4;
		d_ptr->wDotMeans[1] = gsum / gcnt;
		d_ptr->wDotValids[1] = true;
	}

	return true;
}


bool RetFocus::RetinaFrame::transformFeatureImage(void)
{
	float mean, stdev;
	getSample().getMeanStddevInCircle(&mean, &stdev, 180);
	int thresh = (int)(mean + stdev * RETINA_FEAT_THRESHOLD_RATIO);
	int permax = getSample().getPercentile(93, 4, 180);

	thresh = min(thresh, permax);
	thresh = max(min(thresh, RETINA_FEAT_THRESHOLD_MAX), RETINA_FEAT_THRESHOLD_MIN);

	if (DEBUG_OUT) {
		LogD() << "Retina Ir image, mean: " << (int)mean << ", stdev: " << (int)stdev << ", per99: " << permax << " => thresh: " << thresh;
	}

	if (mean < RETINA_FEAT_MEAN_MIN || stdev < RETINA_FEAT_STDEV_MIN) {
		return false;
	}

	getSample().applyBinarized(thresh);
	getSample().applyMorphologyTransform(RETINA_FEAT_KERNEL_SIZE);
	return true;
}


bool RetFocus::RetinaFrame::locateOpticDiscFeature(void)
{
	Mat sample = getSample().getCvMat();
	int cent_y = sample.rows / 2;
	int cent_x = sample.cols / 2;

	int radest = (int)(min(cent_x, cent_y));
	int radius = min(RETINA_FEAT_SCAN_RADIUS, radest);
	int radsqr = radius * radius;

	struct FeatRect {
		int x1;
		int y1;
		int x2;
		int y2;
		int size = 0;
		float aspect = 0.0f;
		float ratio = 0.0f;

		int centerX() { return (x2 + x1) / 2; };
		int centerY() { return (y2 + y1) / 2; };
	};

	std::vector<FeatRect> feats;

	for (int dy = -45; dy <= +45; dy++) {
		int x1 = 9999, x2 = -1;	
		int y = dy + cent_y;
		auto ptr = sample.ptr(y);

		for (int dx = -radius; dx <= radius; dx++) {
			if ((dx*dx + dy*dy) > radsqr) {
				continue;
			}

			int x = dx + cent_x;
			if (ptr[x] > 0) {
				x1 = min(x, x1);
				x2 = max(x, x2);
			}
			else if (x2 > 0) {
				bool found = false;
				for (auto& feat : feats) {
					if (((x1 <= feat.x1) && (feat.x1 <= x2)) ||
						((x1 <= feat.x2) && (feat.x2 <= x2)) ||
						((feat.x1 <= x1) && (x1 <= feat.x2))) {
						if ((y - feat.y2) <= 1) {
							feat.x1 = min(x1, feat.x1);
							feat.x2 = max(x2, feat.x2);
							feat.y2 = max(y, feat.y2);
							feat.size += (x2 - x1 + 1);
							found = true;
							break;
						}
					}
				}
				
				if (!found) {
					FeatRect rect;
					rect.x1 = x1;
					rect.x2 = x2;
					rect.y1 = y;
					rect.y2 = y;
					rect.size += (x2 - x1 + 1);
					feats.push_back(rect);
				}
				x1 = 9999; x2 = -1;
			}
		}
	}

	for (auto& feat : feats) {
		for (int x = feat.x1-1; x >= 0; x--) {
			int count = 0;
			for (int y = feat.y1; y <= feat.y2; y++) {
				auto ptr = sample.ptr(y);
				if ((ptr[x + 1] * ptr[x]) > 0) {
					count++;
				}
			}
			if (count > 0) {
				feat.x1 = x;
			}
			else {
				break;
			}
		}

		for (int x = feat.x2+1; x < sample.cols; x++) {
			int count = 0;
			for (int y = feat.y1; y <= feat.y2; y++) {
				auto ptr = sample.ptr(y);
				if ((ptr[x - 1] * ptr[x]) > 0) {
					count++;
				}
			}
			if (count > 0) {
				feat.x2 = x;
			}
			else {
				break;
			}
		}

		for (int y = feat.y1 - 1; y >= 0; y--) {
			int count = 0;
			for (int x = feat.x1; x <= feat.x2; x++) {
				auto ptr1 = sample.ptr(y);
				auto ptr2 = sample.ptr(y + 1);
				if ((ptr1[x] * ptr2[x]) > 0) {
					count++;
				}
			}
			if (count > 0) {
				feat.y1 = y;
			}
			else {
				break;
			}
		}

		for (int y = feat.y2 + 1; y < sample.rows; y++) {
			int count = 0;
			for (int x = feat.x1; x <= feat.x2; x++) {
				auto ptr1 = sample.ptr(y);
				auto ptr2 = sample.ptr(y - 1);
				if ((ptr1[x] * ptr2[x]) > 0) {
					count++;
				}
			}
			if (count > 0) {
				feat.y2 = y;
			}
			else {
				break;
			}
		}

		int count = 0;
		for (int y = feat.y1; y <= feat.y2; y++) {
			auto ptr = sample.ptr(y);
			for (int x = feat.x1; x <= feat.x2; x++) {
				if (ptr[x] > 0) {
					count++;
				}
			}
		}
		feat.size = count;
	}

	for (auto& feat : feats) {
		int w = feat.x2 - feat.x1 + 1;
		int h = feat.y2 - feat.y1 + 1;
		feat.ratio = (feat.size) / (float)(w * h);
		feat.aspect = (float)h / (float)w;
	}

	FeatRect* featSel = NULL;

	for (auto& feat : feats) {
		if (feat.size < RETINA_FEAT_SCAN_SIZE_MIN || feat.size > RETINA_FEAT_SCAN_SIZE_MAX) {
			continue;
		}
		if (feat.ratio < RETINA_FEAT_SCAN_RATIO_MIN) {
			continue;
		}
		if (feat.aspect < RETINA_FEAT_SCAN_ASPECT_MIN || feat.aspect > RETINA_FEAT_SCAN_ASPECT_MAX) {
			continue;
		}

		if (DEBUG_OUT) {
			LogD() << "Retina feature, size: " << feat.size << ", ratio: " << feat.ratio << ", aspect: " << feat.aspect;
		}

		if (featSel == NULL) {
			featSel = &feat;
		}
		else {
			if (featSel->size < feat.size) {
				featSel = &feat;
			}
		}
	}

	if (featSel != NULL) {
		getImpl().discRegionPos1 = pair<int, int>(featSel->x1, featSel->y1);
		getImpl().discRegionPos2 = pair<int, int>(featSel->x2, featSel->y2);
		getImpl().discCenterPos = pair<float, float>(featSel->centerX(), featSel->centerY());
		return true;
	}
	return false;
}


bool RetFocus::RetinaFrame::locateOpticDiscRegion(void)
{
	Mat sample = getSample().getCvMat();
	int regX1 = (int)(sample.cols * RETINA_FEAT_SCAN_REGION_X1);
	int regX2 = (int)(sample.cols * RETINA_FEAT_SCAN_REGION_X2);
	int regY1 = (int)(sample.cols * RETINA_FEAT_SCAN_REGION_Y1);
	int regY2 = (int)(sample.cols * RETINA_FEAT_SCAN_REGION_Y2);

	auto xpos = vector<int>();
	auto ypos = vector<int>();

	for (int r = regY1; r < regY2; r += RETINA_FEAT_SCAN_OFFSET_Y) {
		auto ptr = sample.ptr(r);
		for (int c = regX1; c < regX2; c += RETINA_FEAT_SCAN_OFFSET_X) {
			if (ptr[c] > 0) {
				xpos.push_back(c);
				ypos.push_back(r);
			}
		}
	}

	if (xpos.size() <= 0 || ypos.size() <= 0) {
		return false;
	}


	sort(xpos.begin(), xpos.end());
	sort(ypos.begin(), ypos.end());
	auto midd = xpos.size() / 2;

	int xcen = xpos[midd];
	int ycen = ypos[midd];
	int wind_x1, wind_x2;
	int wind_y1, wind_y2;

	int x1 = max(xcen - RETINA_FEAT_WIND_EXTENT, 0);
	int x2 = min(xcen + RETINA_FEAT_WIND_EXTENT, sample.cols - 1);
	int y1 = max(ycen - RETINA_FEAT_WIND_EXTENT, 0);
	int y2 = min(ycen + RETINA_FEAT_WIND_EXTENT, sample.rows - 1);

	for (int r = ycen; r >= 0; r--) {
		auto ptr = sample.ptr(r);
		for (int c = x1; c < x2; c += RETINA_FEAT_WIND_OFFSET) {
			if (ptr[c] > 0) {
				wind_y1 = r; 
				break;
			}
		}
	}

	for (int r = ycen; r < sample.rows; r++) {
		auto ptr = sample.ptr(r);
		for (int c = x1; c < x2; c += RETINA_FEAT_WIND_OFFSET) {
			if (ptr[c] > 0) {
				wind_y2 = r;
				break;
			}
		}
	}

	for (int c = xcen; c >= 0; c--) {
		for (int r = y1; r < y2; r += RETINA_FEAT_WIND_OFFSET) {
			auto ptr = sample.ptr(r);
			if (ptr[c] > 0) {
				wind_x1 = c;
				break;
			}
		}
	}

	for (int c = xcen; c < sample.cols; c++) {
		for (int r = y1; r < y2; r += RETINA_FEAT_WIND_OFFSET) {
			auto ptr = sample.ptr(r);
			if (ptr[c] > 0) {
				wind_x2 = c;
				break;
			}
		}
	}

	int wind_w = wind_x2 - wind_x1 + 1;
	int wind_h = wind_y2 - wind_y1 + 1;

	int pixels = 0;
	for (int r = wind_y1; r <= wind_y2; r++) {
		auto ptr = sample.ptr(r);
		for (int c = wind_x1; c <= wind_x2; c++) {
			if (ptr[c] > 0) {
				pixels++;
			}
		}
	}

	float ratio = (float)pixels / (wind_w * wind_h);
	
	if (DEBUG_OUT) {
		LogD() << "Retina feature, width: " << wind_w << ", height: " << wind_h << ", ratio: " << ratio;
	}

	/*
	if (wind_w < RETINA_FEAT_DISC_WIDTH_MIN || wind_w > RETINA_FEAT_DISC_WIDTH_MAX ||
		wind_h < RETINA_FEAT_DISC_HEIGHT_MIN || wind_h > RETINA_FEAT_DISC_HEIGHT_MAX ||
		ratio < RETINA_FEAT_DISC_PIXELS_RATIO_MIN) {
		return false;
	}
	*/

	getImpl().discRegionPos1 = pair<int, int>(wind_x1, wind_y1);
	getImpl().discRegionPos2 = pair<int, int>(wind_x2, wind_y2);
	getImpl().discCenterPos = pair<int, int>((wind_x2 + wind_x1) / 2, (wind_y2 + wind_y1) / 2);
	return true;
}


bool RetFocus::RetinaFrame::makeUpperConvolutions(void)
{
	Mat sample = getSample().getCvMat();

	int scnt = 0;
	for (int c = d_ptr->uppRangeX[0]; c < d_ptr->uppRangeX[1]; c += SPLIT_CONVS_X_STEP, scnt++) {
		int gsum = 0, gcnt = 0;
		for (int r = d_ptr->uppRangeY[0]; r <= d_ptr->uppRangeY[1]; r += SPLIT_CONVS_Y_STEP) {
			const uchar* ptr = sample.ptr(r);
			for (int k = (c - d_ptr->kernelWidth/2); k < (c + d_ptr->kernelWidth/2); k++) {
				gsum += ptr[k];
				gcnt += 1;
			}
		}
		d_ptr->upperConvs[scnt] = (float)gsum / gcnt;
	}
	return true;
}


bool RetFocus::RetinaFrame::makeLowerConvolutions(void)
{
	Mat sample = getSample().getCvMat();

	int scnt = 0;
	for (int c = d_ptr->lowRangeX[0]; c < d_ptr->lowRangeX[1]; c += SPLIT_CONVS_X_STEP, scnt++) {
		int gsum = 0, gcnt = 0;
		for (int r = d_ptr->lowRangeY[0]; r <= d_ptr->lowRangeY[1]; r += SPLIT_CONVS_Y_STEP) {
			const uchar* ptr = sample.ptr(r);
			for (int k = (c - d_ptr->kernelWidth / 2); k < (c + d_ptr->kernelWidth / 2); k++) {
				gsum += ptr[k];
				gcnt += 1;
			}
		}
		d_ptr->lowerConvs[scnt] = (float)gsum / gcnt;
	}
	return true;
}


bool RetFocus::RetinaFrame::assignUpperSplit(void)
{
	int pidx;
	float peak, fwhm, snRatio;

	findPeakInData(d_ptr->upperConvs, &pidx, &peak, &fwhm, &snRatio);

	d_ptr->upperFwhm = fwhm;
	d_ptr->upperSNR = snRatio;

	pidx += (d_ptr->uppRangeX[0]);
	d_ptr->uppWinCenX = pidx;

	if (DEBUG_OUT) {
		DebugOut2() << "Upper split, pidx: " << pidx << ", peak: " << peak << ", fwhm: " << fwhm << ", snRatio: " << snRatio;
	}

	if (!checkIfValidPeak(peak, fwhm, snRatio)) {
		return false;
	}

	if (!findUpperSplitPosition(pidx)) {
		return false;
	}

	d_ptr->isUppSplit = true;
	return true;
}


bool RetFocus::RetinaFrame::assignLowerSplit(void)
{
	int pidx;
	float peak, fwhm, snRatio;

	findPeakInData(d_ptr->lowerConvs, &pidx, &peak, &fwhm, &snRatio);

	d_ptr->lowerFwhm = fwhm;
	d_ptr->lowerSNR = snRatio;

	pidx += (d_ptr->lowRangeX[0]);
	d_ptr->lowWinCenX = pidx;

	if (DEBUG_OUT) {
		DebugOut2() << "Lower split, pidx: " << pidx << ", peak: " << peak << ", fwhm: " << fwhm << ", snRatio: " << snRatio;
	}

	if (!checkIfValidPeak(peak, fwhm, snRatio)) {
		return false;
	}

	if (!findLowerSplitPosition(pidx)) {
		return false;
	}

	d_ptr->isLowSplit = true;
	return true;
}



bool RetFocus::RetinaFrame::findPeakInData(const SplitConvsArray & data, int * peakPos, float * peakVal, float * peakSize, float * snRatio)
{
	float dsum;
	float peak_val = 0.0f;
	int peak_idx = 0;
	int count = 0;

	dsum = accumulate(cbegin(data), cend(data), 0.0f, [&](float dacc, const float elem) {
		if (elem >= peak_val) {
			peak_idx = count;
			peak_val = elem;
		}
		count++;
		return (dacc + elem);
	});

	float bgrd_val = dsum / count;
	float thresh = bgrd_val + (peak_val - bgrd_val) * 0.75f;

	auto peak_s = SplitConvsArrayConstIter();
	auto peak_e = SplitConvsArrayConstIter();
	bool found_s = false;
	bool found_e = false;

	for (auto iter = data.cbegin() + peak_idx; iter != data.cbegin(); --iter) {
		if (*iter >= thresh) {
			peak_s = iter;
			// peak_s = distance(data.begin(), iter);
			found_s = true;
		}
		else {
			break;
		}
	}

	for (auto iter = data.cbegin() + peak_idx; iter != data.cend(); ++iter) {
		if (*iter >= thresh) {
			peak_e = iter;
			// peak_e = distance(data.begin(), iter);
			found_e = true;
		}
		else {
			break;
		}
	}

	int index = 0;
	float peak_sum = 0;
	float peak_cnt = 0;
	float bgrd_cnt = 0;
	float bgrd_sum = 0.0f;
	float wght = 0;

	if (found_s && found_e) {
		for (auto iter = data.cbegin(); iter != data.cend(); ++iter, ++index) {
			if (iter >= peak_s && iter <= peak_e) {
				wght = (*iter - thresh + 1);
				peak_sum += (index * wght);
				peak_cnt += wght;
			}
			else {
				bgrd_sum += *iter;
				bgrd_cnt += 1;
			}
		}
	}
	else {
		for (auto iter = data.cbegin(); iter != data.cend(); ++iter, ++index) {
			bgrd_sum += *iter;
			bgrd_cnt += 1;
		}
	}

	*peakPos = 0;
	*peakVal = 0;
	*peakSize = 0.0f;
	*snRatio = 0.0f;

	if (peak_cnt > 0) {
		*peakPos = (int)(peak_sum / peak_cnt);
		*peakSize = (float)distance(peak_s, peak_e) + 1;
		*peakVal = peak_val;
		if (bgrd_cnt > 0) {
			bgrd_val = (bgrd_sum / bgrd_cnt);
			*snRatio = peak_val / bgrd_val;
		}
	}
	return true;
}


bool RetFocus::RetinaFrame::checkIfValidPeak(float peak, float fwhm, float snRatio)
{
	if (peak > SPLIT_PEAK_MAX) {
	}
	else {
		if (peak < SPLIT_PEAK_MIN) {
			return false;
		}
		if (snRatio < SPLIT_SNR_MIN) {
			return false;
		}
	}

	if (fwhm < SPLIT_FWHM_MIN || fwhm > SPLIT_FWHM_MAX) {
		return false;
	}
	return true;
}


bool RetFocus::RetinaFrame::findUpperSplitPosition(int pidx)
{
	Mat sample = getSample().getCvMat();

	int xsum = 0, ysum = 0, wsum = 0, wght = 0, gcnt = 0;
	for (int r = d_ptr->uppRangeY[0]; r <= d_ptr->uppRangeY[1]; r++) {
		const uchar* ptr = sample.ptr(r);
		for (int c = pidx - (SPLIT_FOCUS_WIDTH / 2); c <= pidx + (SPLIT_FOCUS_WIDTH / 2); c++) {
			wght = ptr[c];
			xsum += c * wght;
			ysum += r * wght;
			wsum += wght;
			gcnt += 1;
		}
	}

	d_ptr->uppSplitX = (float)xsum / (float)wsum;
	d_ptr->uppSplitY = (float)ysum / (float)wsum;
	d_ptr->upperPeak = (float)wsum / (float)gcnt;

	if (DEBUG_OUT) {
		DebugOut2() << "Upper split pos.: " << d_ptr->uppSplitX << ", " << d_ptr->uppSplitY;
	}
	return true;
}


bool RetFocus::RetinaFrame::findLowerSplitPosition(int pidx)
{
	Mat sample = getSample().getCvMat();

	int xsum = 0, ysum = 0, wsum = 0, wght = 0, gcnt = 0;
	for (int r = d_ptr->lowRangeY[0]; r <= d_ptr->lowRangeY[1]; r++) {
		const uchar* ptr = sample.ptr(r);
		for (int c = pidx - (SPLIT_FOCUS_WIDTH / 2); c <= pidx + (SPLIT_FOCUS_WIDTH / 2); c++) {
			wght = ptr[c];
			xsum += c * wght;
			ysum += r * wght;
			wsum += wght;
			gcnt += 1;
		}
	}

	d_ptr->lowSplitX = (float)xsum / (float)wsum;
	d_ptr->lowSplitY = (float)ysum / (float)wsum;
	d_ptr->lowerPeak = (float)wsum / (float)gcnt;

	if (DEBUG_OUT) {
		DebugOut2() << "Lower split pos.: " << d_ptr->lowSplitX << ", " << d_ptr->lowSplitY;
	}
	return true;
}
 

bool RetFocus::RetinaFrame::decideSplitOffset(void)
{
	if (!d_ptr->isLowSplit && !d_ptr->isUppSplit) {
		return false;
	}

	float offset = 0.0f;
	if (d_ptr->isLowSplit && d_ptr->isUppSplit) {
		float uppOffs = d_ptr->uppSplitX - d_ptr->centerX;
		float lowOffs = d_ptr->lowSplitX - d_ptr->centerX;

		// Split focus center could be shifted along with eye movement.
		if (true) { // uppOffs * lowOffs < 0.0f) {
			if (fabs(uppOffs - lowOffs) < SPLIT_CENTER_OFFSET_DIFF_MAX) {
				offset = (d_ptr->uppSplitX - d_ptr->lowSplitX) * 0.5f;
			}
			else {
				if (fabs(uppOffs) < fabs(lowOffs)) {
					offset = (d_ptr->uppSplitX - d_ptr->centerX);
				}
				else {
					offset = (d_ptr->centerX - d_ptr->lowSplitX);
				}
			}
		}
		else {
			if (fabs(uppOffs) < fabs(lowOffs)) {
				offset = (d_ptr->uppSplitX - d_ptr->centerX);
			}
			else {
				offset = (d_ptr->centerX - d_ptr->lowSplitX);
			}
		}
	}
	else {
		if (d_ptr->isUppSplit) {
			offset = (d_ptr->uppSplitX - d_ptr->centerX);
		}
		else {
			offset = (d_ptr->centerX - d_ptr->lowSplitX);
		}
	}

	d_ptr->splitOffset = offset;

	if (DEBUG_OUT) {
		DebugOut2() << "Split focus offset: " << offset;
	}
	return true;
}



CvImage & RetFocus::RetinaFrame::getImage(void) const
{
	return d_ptr->image;
}


CvImage & RetFocus::RetinaFrame::getSample(void) const
{
	return d_ptr->sample;
}
