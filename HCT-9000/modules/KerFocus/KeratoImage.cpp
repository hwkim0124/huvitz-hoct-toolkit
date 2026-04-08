#include "stdafx.h"
#include "KeratoImage.h"
#include "KerRect.h"
#include "KerSpot.h"
#include "CppUtil2.h"

#include <algorithm>

using namespace CppUtil;
using namespace KerFocus;
using namespace std;
using namespace cv;


struct KeratoImage::KeratoImageImpl
{
	CvImage image;
	CvImage sample;

	KerSpot2Vect<float> spots;
	KerRect2Vect<float> rects;

	KerPoint2<float> eyeCenter;
	KerPoint2<float> spotsCenter;
	KerPoint2<float> pupilCenter;
	KerPoint2<float> spotsLeft;
	KerPoint2<float> spotsRght;
	int pupilSize;

	int pupilThresh;
	int spotsThresh;
	int backsThresh;

	int pupilStartX;
	int pupilCloseX;
	int pupilWidth;
	float lightExtent;

	bool isModelEye;
	bool isPupilFound;
	bool isEyeFound;

	float focusLen;
	bool isFocusFound;

	float mireCx;
	float mireCy;
	float mireMl;
	float mireNl;
	float mireAx;
	bool isMireEllipse;

	int histGray[HISTOGRAM_BINS];
	int histXsum[HISTOGRAM_BINS];
	int histYsum[HISTOGRAM_BINS];
	int histSize;

	KerSpot2Vect<float> mireLefts;
	KerSpot2Vect<float> mireRghts;
	KerSpot2Vect<float> mireCents;
	KerSpot2Vect<float> focusSpots;

	KeratoImageImpl() : isPupilFound(false), isMireEllipse(false), isFocusFound(false), isEyeFound(false),
		histGray{0}, histSize(0), histXsum{0}, histYsum{0}
	{
	};
};



KeratoImage::KeratoImage() 
	: d_ptr(make_unique<KeratoImageImpl>())
{
}


KerFocus::KeratoImage::KeratoImage(const std::wstring & path)
{
	loadFile(path);
}


KerFocus::KeratoImage::~KeratoImage()
{
	// Destructor should be defined for unique_ptr to delete KeratoImageImpl as an incomplete type.
}


KerFocus::KeratoImage::KeratoImage(KeratoImage && rhs) = default;
KeratoImage & KerFocus::KeratoImage::operator=(KeratoImage && rhs) = default;





bool KerFocus::KeratoImage::loadFile(const std::wstring & path)
{
	bool ret = getImage().fromFile(path);

	if (DEBUG_OUT) {
		DebugOut2() << "Kerato image file: " << wtoa(path) << ", result: " << ret;
	}

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool KerFocus::KeratoImage::loadData(const unsigned char * pBits, int width, int height, int padding)
{
	bool ret = getImage().fromBitsData(pBits, width, height, padding);

	if (DEBUG_OUT) {
		DebugOut2() << "Kerato image data: w: " << width << ", h: " << height << ", result: " << ret;
	}

	if (ret) {
		performAnalysis();
	}
	return ret;
}


bool KerFocus::KeratoImage::isEmpty(void) const
{
	return (getImage().isEmpty());
}

bool KerFocus::KeratoImage::isFocusLength(void) const
{
	return d_ptr->isFocusFound;
}


bool KerFocus::KeratoImage::getFocusLength(float & flen) const
{
	flen = 0.0f;
	if (isFocusLength()) {
		flen = getImpl().focusLen;
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage::isMireEllipse(void) const
{
	return d_ptr->isMireEllipse;
}

bool KerFocus::KeratoImage::getMireEllipse(float & cx, float & cy, float & m, float & n, float & a) const
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

bool KerFocus::KeratoImage::getMireLength(float & mire) const
{
	if (isMireEllipse() == false) {
		return false;
	}

	mire = (float)CppUtil::NumericFunc::horizontalLengthOfEllipse(d_ptr->mireMl, d_ptr->mireNl, d_ptr->mireAx);
	return true;
}


bool KerFocus::KeratoImage::getFocusDifference(float & diff) const
{
	float mire, flen;
	diff = 0.0f;
	if (getMireLength(mire) && getFocusLength(flen)) {
		diff = mire - flen;
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage::getMireSpot(int index, KerSpot2<float>& spot) const
{
	int size = 0;

	for (const auto& mire : d_ptr->mireLefts) {
		if (mire.valid_) {
			if (size++ == index) {
				spot = mire;
				return true;
			}
		}
	}

	for (const auto& mire : d_ptr->mireRghts) {
		if (mire.valid_) {
			if (size++ == index) {
				spot = mire;
				return true;
			}
		}
	}

	for (const auto& mire : d_ptr->mireCents) {
		if (mire.valid_) {
			if (size++ == index) {
				spot = mire;
				return true;
			}
		}
	}

	return false;
}


bool KerFocus::KeratoImage::getFocusSpot(int index, KerSpot2<float>& spot) const
{
	int size = 0;

	for (const auto& focus : d_ptr->focusSpots) {
		if (focus.valid_) {
			if (size++ == index) {
				spot = focus;
				return true;
			}
		}
	}
	return false;
}


int KerFocus::KeratoImage::getCountOfMireSpots(void) const
{
	return countMireSpots();
}


int KerFocus::KeratoImage::getCountOfFocusSpots(void) const
{
	return countFocusSpots();
}


bool KerFocus::KeratoImage::isEyeFound(void) const
{
	return d_ptr->isEyeFound;
}


bool KerFocus::KeratoImage::getEyeCenter(float & cx, float & cy) const
{
	if (isEyeFound()) {
		cx = d_ptr->eyeCenter.x_;
		cy = d_ptr->eyeCenter.y_;
		return true;
	}
	return false;
}


bool KerFocus::KeratoImage::getCenterOffset(float & cx, float & cy) const
{
	if (getEyeCenter(cx, cy)) {
		cx = cx - CAMERA_IMAGE_CENTER_X;
		cy = cy - CAMERA_IMAGE_CENTER_Y;
		return true;
	}
	return false;
}


float KerFocus::KeratoImage::getLightExtent(void) const
{
	return d_ptr->lightExtent;
}


int KerFocus::KeratoImage::getPupilStartX(void) const
{
	return d_ptr->pupilStartX;
}


int KerFocus::KeratoImage::getPupilCloseX(void) const
{
	return d_ptr->pupilCloseX;
}


KeratoImage::KeratoImageImpl & KerFocus::KeratoImage::getImpl(void) const
{
	return *d_ptr;
}



void KerFocus::KeratoImage::initialize(void)
{
	d_ptr->spots.clear();
	d_ptr->rects.clear();

	d_ptr->isModelEye = false;
	d_ptr->isPupilFound = false;
	d_ptr->isMireEllipse = false;
	d_ptr->isFocusFound = false;
	d_ptr->isEyeFound = false;

	d_ptr->pupilStartX = 0;
	d_ptr->pupilCloseX = 0;
	d_ptr->pupilWidth = 640;
	d_ptr->lightExtent = 1.0f;

	d_ptr->mireLefts.clear();
	d_ptr->mireRghts.clear();
	d_ptr->mireCents.clear();
	d_ptr->focusSpots.clear();

	memset(d_ptr->histGray, 0, sizeof(int)*HISTOGRAM_BINS);
	memset(d_ptr->histXsum, 0, sizeof(int)*HISTOGRAM_BINS);
	memset(d_ptr->histYsum, 0, sizeof(int)*HISTOGRAM_BINS);
	d_ptr->histSize = 0;
	return;
}



bool KerFocus::KeratoImage::performAnalysis(void)
{
	initialize();

	if (isEmpty()) {
		return false;
	}

	createSampleImage();
	makeHistogram();
	decideThresholds();

	assignPupilCenter();
	measurePupilDiameter();
	measureLightingArea();

	assignSpotWindows();
	if (findSpotCenters() < NUM_KER_SPOTS_MIN) {
		return false;
	}

	if (checkIfModelEye()) {
		if (decideEyeCenter()) {
			d_ptr->pupilCenter = d_ptr->eyeCenter;
		}
	}

	sortSpotsByDistance();
	if (matchHorzPairsOfSpots() > 0) {
		assignMireSpotsOnCenter();
		assignMireSpotsOnLeft();
		assignMireSpotsOnRight();
		assignFocusSpots();

		calculateFocusLength();
		calculateMireRing();
	}
	
	decideEyeCenter();
	return true;
}


bool KerFocus::KeratoImage::createSampleImage(void)
{
	getSample().getCvMat() = getImage().getCvMat();
	return true;
}


void KerFocus::KeratoImage::makeHistogram(void)
{
	Mat sample = getSample().getCvMat();

	for (int r = ROW_START; r <= ROW_END; r += ROW_STEP_HIST) {
		const uchar* ptr = sample.ptr(r);
		for (int c = COL_START; c <= COL_END; c += COL_STEP_HIST) {
			int ibin = ptr[c];
			d_ptr->histGray[ibin] += 1;
			d_ptr->histSize += 1;
			d_ptr->histXsum[ibin] += c;
			d_ptr->histYsum[ibin] += r;
		}
	}
	return;
}


void KerFocus::KeratoImage::decideThresholds(void)
{
	int ibin, count, size, thresh;

	count = 0;
	size = (int)(d_ptr->histSize * PUPIL_RATIO);
	for (ibin = 0; ibin < HISTOGRAM_BINS; ibin++) {
		count += d_ptr->histGray[ibin];
		if (count > size) {
			break;
		}
	}

	thresh = ibin + (5 - ibin % 5);
	thresh = min(max(thresh, PUPIL_THRESHOLD_MIN), PUPIL_THRESHOLD_MAX);
	d_ptr->pupilThresh = thresh;
	d_ptr->pupilSize = size;

	count = 0;
	size = (int)(d_ptr->histSize * SPOTS_RATIO);
	for (ibin = HISTOGRAM_BINS-1; ibin >= 0; ibin--) {
		count += d_ptr->histGray[ibin];
		if (count > size) {
			break;
		}
	}

	thresh = ibin + (5 - ibin % 5);
	thresh = min(max(thresh, SPOTS_THRESHOLD_MIN), SPOTS_THRESHOLD_MAX);
	d_ptr->spotsThresh = thresh;

	d_ptr->backsThresh = (d_ptr->pupilThresh + d_ptr->spotsThresh) / 2;

	if (DEBUG_OUT) {
		DebugOut2() << "Pupil threshold: " << d_ptr->pupilThresh << ", size: " << d_ptr->pupilSize;
		DebugOut2() << "Spots threshold: " << d_ptr->spotsThresh;
		DebugOut2() << "Backs threshold: " << d_ptr->backsThresh;
	}
	return;
}


bool KerFocus::KeratoImage::assignPupilCenter(void)
{
	int xsum, ysum, wsum, size, wght;

	xsum = ysum = wsum = size = 0;
	for (int ibin = 0; ibin <= d_ptr->pupilThresh; ibin++) {
		wght = d_ptr->pupilThresh - ibin + 1;
		size += d_ptr->histGray[ibin] ;
		xsum += (d_ptr->histXsum[ibin] * wght);
		ysum += (d_ptr->histYsum[ibin] * wght);
		wsum += (d_ptr->histGray[ibin] * wght);
	}

	if (size <= 0) {
		return false;
	}

	d_ptr->isPupilFound = true;
	d_ptr->pupilCenter = KerPoint2<float>((float)xsum / wsum, (float)ysum / wsum);
	d_ptr->pupilSize = size;

	if (DEBUG_OUT) {
		DebugOut2() << "Pupil center: " << d_ptr->pupilCenter.x_ << ", " << d_ptr->pupilCenter.y_ << ", size: " << size;
	}
	return true;
}


bool KerFocus::KeratoImage::measurePupilDiameter(void)
{
	Mat sample = getSample().getCvMat();

	int pup_cx = (int)d_ptr->pupilCenter.x_;
	int pup_cy = (int)d_ptr->pupilCenter.y_;
	int thresh = (int)(d_ptr->pupilThresh * 1.45f);

	int row_s, row_e;

	row_s = (pup_cy - 75);
	row_e = (pup_cy + 75);
	row_s = max(row_s, 0);
	row_e = min(row_e, sample.rows - 1);

	int pup_start = pup_cx;
	int pup_close = pup_cx;
	int out_count = 0;

	for (int c = pup_cx; c >= 0; c--) {
		int overs = 0;
		int total = 0;
		for (int r = row_s; r <= row_e; r += 4) {
			if (sample.at<uchar>(r, c) > thresh) {
				overs++;
			}
			total++;
		}

		float ratio = (float)overs / (float)total;
		if (ratio > 0.95f) {
			if (++out_count > 5) {
				break;
			}
		}
		else {
			pup_start = c;
			out_count = 0;
		}
	}

	out_count = 0;
	for (int c = pup_cx; c < sample.cols; c++) {
		int overs = 0;
		int total = 0;
		for (int r = row_s; r <= row_e; r += 4) {
			if (sample.at<uchar>(r, c) > thresh) {
				overs++;
			}
			total++;
		}

		float ratio = (float)overs / (float)total;
		if (ratio > 0.95f) {
			if (++out_count > 5) {
				break;
			}
		}
		else {
			pup_close = c;
			out_count = 0;
		}
	}

	d_ptr->pupilStartX = pup_start;
	d_ptr->pupilCloseX = pup_close;
	d_ptr->pupilWidth = (pup_close - pup_start + 1);

	if (DEBUG_OUT) {
		DebugOut2() << "Pupil width: " << d_ptr->pupilWidth << ", start: " << d_ptr->pupilStartX << ", close: " << d_ptr->pupilCloseX;
	}
	return true;
}


bool KerFocus::KeratoImage::measureLightingArea(void)
{
	int normRadius = 75;
	int testRadius = 0;

	int centX = SAMPLE_IMAGE_CENTER_X;
	int centY = SAMPLE_IMAGE_CENTER_Y;

	if (d_ptr->pupilStartX <= (centX - normRadius) &&
		d_ptr->pupilCloseX >= (centX + normRadius)) {
		testRadius = max((centX - d_ptr->pupilStartX), (d_ptr->pupilCloseX - centX));
	}
	else {
		testRadius = normRadius;
	}

	int testX1 = max(centX - testRadius, 0);
	int testX2 = min(centX + testRadius, SAMPLE_IMAGE_WIDTH - 1);
	int testY1 = max(centY - testRadius, 0);
	int testY2 = min(centY + testRadius, SAMPLE_IMAGE_HEIGHT - 1);
	int extent = testRadius * testRadius;
	int offset = 0;

	Mat sample = getSample().getCvMat();
	int thresh = (int)(d_ptr->pupilThresh * 2.0f);
	int pupil = 0;

	for (int y = testY1; y <= testY2; y += 4) {
		for (int x = testX1; x <= testX2; x += 4) {
			if (sample.at<uchar>(y, x) < thresh) {
				offset = (centX - x) * (centX - x) + (centY - y) * (centY - y);
				if (offset <= extent) {
					pupil++;
				}
			}
		}
	}

	float normArea = 2.0f * 3.14f * normRadius * normRadius / 16.0f;
	float pupilArea = (float) pupil;
	float lightArea = pupilArea / normArea;

	d_ptr->lightExtent = lightArea; // *100.0f;
	return true;
}


bool KerFocus::KeratoImage::checkIfModelEye(void)
{
	if (d_ptr->pupilWidth > MODEL_EYE_PUPIL_SIZE_MIN || d_ptr->pupilThresh <= MODEL_EYE_PUPIL_THRESHOLD_MAX) {
		d_ptr->isModelEye = true;
	}
	else {
		d_ptr->isModelEye = false;
	}

	return d_ptr->isModelEye;
}


int KerFocus::KeratoImage::assignSpotWindows(void)
{
	Mat sample = getSample().getCvMat();

	bool ready, found;
	int x1, x2, backs;

	d_ptr->rects.clear();
	for (int r = ROW_START; r <= ROW_END; r += ROW_STEP_SPOT) {
		const uchar* ptr = sample.ptr(r);
		ready = found = false;
		x1 = x2 = backs = 0;
		for (int c = COL_START; c <= COL_END; c += COL_STEP_SPOT) {
			if (!ready) {
				if (ptr[c] <= d_ptr->backsThresh) {
					ready = true; backs = 0;
				}
			}
			if (ready) {
				if (!found) {
					if (ptr[c] >= d_ptr->spotsThresh) {
						x1 = x2 = c; found = true; 
						backs = 0;
					}
					else if (ptr[c] >= d_ptr->backsThresh) {
						if (++backs > 5) {
							ready = found = false;
						}
					}
				}
				else {
					if (ptr[c] >= d_ptr->spotsThresh) {
						x2 = c;
						backs = 0;
					}
					else if (ptr[c] <= d_ptr->backsThresh) {
						expandSpotWindows(x1, r, x2, r);
						ready = found = false;
						x1 = x2 = 0;
					}
					else {
						if (++backs > 5) {
							ready = found = false;
						}
					}
				}
			}
		}
	}

	int size = filterSpotWindows();
	return size;
}



void KerFocus::KeratoImage::expandSpotWindows(int sx, int sy, int ex, int ey)
{
	KerRect2<float> spot(KerPoint2<float>((float)sx, (float)sy), KerPoint2<float>((float)ex, (float)ey));

	bool found = false;
	for (auto& rect : d_ptr->rects) {
		if (rect.alignedX(spot, 9.0f) && rect.alignedY(spot, 3.0f)) {
			rect.merge(spot);
			found = true;
			break;
		}
	}

	if (!found) {
		d_ptr->rects.push_back(spot);
	}
	return;
}


int KerFocus::KeratoImage::filterSpotWindows(void)
{

	if (DEBUG_OUT) {
		for (const auto& rect : d_ptr->rects) {
			DebugOut2() << "Spot found: " << rect.center().x_ << ", " << rect.center().y_ << ", w: " << rect.width() << ", h: " << rect.height();
		}
	}

	decltype(d_ptr->rects) vect;

	for (auto& rect : d_ptr->rects) {
		if ((rect.width() >= SPOT_WIDTH_MIN && rect.width() <= SPOT_WIDTH_MAX) &&
			(rect.height() >= SPOT_HEIGHT_MIN && rect.height() <= SPOT_HEIGHT_MAX))
		{
			if ((rect.width() >= SPOT_WIDTH_STD && rect.height() >= SPOT_HEIGHT_STD) &&
				rect.sizeRatio() >= SPOT_SIZE_RATIO_MAX) {
				continue;
			}

			if (distanceBetween(rect, d_ptr->pupilCenter) >= PUPIL_SPOT_DISTANCE_MAX) {
				continue;
			}
			vect.push_back(rect);
		}
	}

	d_ptr->rects = vect;
	int size = (int)d_ptr->rects.size();

	if (DEBUG_OUT) {
		for (const auto& rect : d_ptr->rects) {
			DebugOut2() << "Spot filtered: " << rect.center().x_ << ", " << rect.center().y_ << ", w: " << rect.width() << ", h: " << rect.height();
		}
	}
	return size;
}


int KerFocus::KeratoImage::findSpotCenters(void)
{
	for (const auto& rect : d_ptr->rects) {
		d_ptr->spots.push_back({ rect });
	}

	Mat sample = getSample().getCvMat();
	int thresh = d_ptr->spotsThresh;
	int xsum, ysum, wsum, wght, size;

	for (auto& spot : d_ptr->spots) {
		xsum = ysum = wsum = size = 0;
		for (int r = (int)spot.pt1().y_-5; r <= (int)spot.pt2().y_+5; r++) {
			const uchar* ptr = sample.ptr(r);
			for (int c = (int)spot.pt1().x_-5; c <= (int)spot.pt2().x_+5; c++) {
				if (ptr[c] >= thresh) {
					wght = ptr[c] - thresh + 1;
					xsum += c * wght;
					ysum += r * wght;
					wsum += wght;
					size += 1;
				}
			}
		}

		spot.center_ = KerPoint2<float>((float)xsum / wsum, (float)ysum / wsum);
		spot.thresh_ = thresh;
		spot.size_ = size;
		spot.valid_ = true;
	}

	if (DEBUG_OUT) {
		for (const auto& rect : d_ptr->spots) {
			DebugOut2() << "Spot center: " << rect.center().x_ << ", " << rect.center().y_;
		}
	}

	size = (int)d_ptr->spots.size();
	return size;
}


void KerFocus::KeratoImage::sortSpotsByDistance(void)
{
	decltype(d_ptr->pupilCenter) origin = d_ptr->pupilCenter;

	sort(d_ptr->spots.begin(), d_ptr->spots.end(), 
		[origin](const KerSpot2<float>& sp1, const KerSpot2<float>& sp2) {
		float dist1 = distanceBetween(sp1, origin);
		float dist2 = distanceBetween(sp2, origin);
		return (dist1 < dist2);
	});

	if (DEBUG_OUT) {
		for (const auto& rect : d_ptr->spots) {
			DebugOut2() << "Spot sorted: " << rect.center().x_ << ", " << rect.center().y_ ;
		}
	}
	return;
}


int KerFocus::KeratoImage::matchHorzPairsOfSpots(void)
{
	size_t testSize = d_ptr->spots.size() - 1;
	testSize = (testSize > HORZ_PAIRS_TEST_SIZE ? HORZ_PAIRS_TEST_SIZE : testSize);

	std::vector<std::pair<int, int>> pairs;

	int count = 0;
	int lidx, ridx;
	for (int i = 0; i < testSize; i++) {
		// for (int j = 0; j < d_ptr->spots.size(); j++) {
		for (int j = 0; j < testSize; j++) {
			if (i == j) {
				continue;
			}
			if (d_ptr->spots[i].window_.alignedY(d_ptr->spots[j].window_, 5.0f)) {
				float dist = distanceBetween(d_ptr->spots[i], d_ptr->spots[j]);
				if (dist >= HORZ_PAIRS_DISTANCE_MIN && dist <= HORZ_PAIRS_DISTANCE_MAX) {
					if (d_ptr->spots[i].center().isLeft(d_ptr->spots[j].center().x_)) {
						lidx = i; ridx = j;
					}
					else {
						lidx = j; ridx = i;
					}

					bool exist = false;
					for (auto& pair : pairs) {
						if (pair.first == lidx && pair.second == ridx) {
							exist = true;
							break;
						}
					}
					if (exist) {
						continue;
					}

					if (d_ptr->spots[lidx].center().isLeft(d_ptr->pupilCenter.x_) && 
						d_ptr->spots[ridx].center().isRight(d_ptr->pupilCenter.x_)) {
						d_ptr->mireLefts.push_back(d_ptr->spots[lidx]);
						d_ptr->mireRghts.push_back(d_ptr->spots[ridx]);

						pairs.push_back(std::pair<int, int>(lidx, ridx));

						count += 1;
						if (count >= HORZ_PAIRS_COUNT_MAX) {
							break;
						}
					}
				}
			}
		}
	}

	if (count > 0) {
		float leftXs = 0.0f, leftYs = 0.0f;
		float rghtXs = 0.0f, rghtYs = 0.0f;
		for (int i = 0; i < count; i++) {
			leftXs += d_ptr->mireLefts[i].center().x_;
			leftYs += d_ptr->mireLefts[i].center().y_;
			rghtXs += d_ptr->mireRghts[i].center().x_;
			rghtYs += d_ptr->mireRghts[i].center().y_;
		}

		d_ptr->spotsLeft = KerPoint2<float>(leftXs / count, leftYs / count);
		d_ptr->spotsRght = KerPoint2<float>(rghtXs / count, rghtYs / count);
		d_ptr->spotsCenter = KerPoint2<float>(
			(d_ptr->spotsLeft.x_ + d_ptr->spotsRght.x_) / 2.0f,
			(d_ptr->spotsLeft.y_ + d_ptr->spotsRght.y_) / 2.0f);

		if (DEBUG_OUT) {
			DebugOut2() << "Spots left: " << d_ptr->spotsLeft.x_ << ", " << d_ptr->spotsLeft.y_;
			DebugOut2() << "Spots rght: " << d_ptr->spotsRght.x_ << ", " << d_ptr->spotsRght.y_;
			DebugOut2() << "Spots center: " << d_ptr->spotsCenter.x_ << ", " << d_ptr->spotsCenter.y_;
		}
	}
	return count;
}


int KerFocus::KeratoImage::assignMireSpotsOnCenter(void)
{
	float cx, cy;

	cx = d_ptr->spotsCenter.x_;
	cy = d_ptr->spotsCenter.y_;

	auto vect = KerSpot2Vect<float>(NUM_MIRE_SPOTS_CENTER);

	for (const auto& spot : d_ptr->spots) {
		if (spot.center().isAbove(cy - VERT_PAIRS_DISTANCE_MIN / 2) && spot.window_.includeX(cx)) {
			vect[0] = spot;
			if (DEBUG_OUT) {
				DebugOut2() << "Center spot-0: " << spot.center().x_ << ", " << spot.center().y_;
			}
			break;
		}
	}

	for (const auto& spot : d_ptr->spots) {
		if (spot.center().isBelow(cy - VERT_PAIRS_DISTANCE_MIN / 2) && spot.window_.includeX(cx)) {
			vect[1] = spot;
			if (DEBUG_OUT) {
				DebugOut2() << "Center spot-1: " << spot.center().x_ << ", " << spot.center().y_;
			}
			break;
		}
	}

	d_ptr->mireCents = vect;
	int size = (int) d_ptr->mireCents.size();

	if (size == NUM_MIRE_SPOTS_CENTER) {
		d_ptr->spotsCenter.y_ = (vect[0].center().y_ + vect[1].center().y_) / 2.0f;
	}
	return size;
}


int KerFocus::KeratoImage::assignMireSpotsOnLeft(void)
{
	float cx, cy, y1, y2;

	cx = d_ptr->spotsLeft.x_;
	cy = d_ptr->spotsLeft.y_;

	y1 = (d_ptr->mireCents[0].valid_ ? d_ptr->mireCents[0].center().y_ : 0);
	y2 = (d_ptr->mireCents[1].valid_ ? d_ptr->mireCents[1].center().y_ : SAMPLE_IMAGE_HEIGHT - 1);

	auto vect = KerSpot2Vect<float>(NUM_MIRE_SPOTS_LEFT);

	int count = 0;
	for (const auto& spot : d_ptr->spots) {
		if (spot.window().includeX(cx, 7.0f) && spot.center().inBetweenY(y1, y2)) {
			vect[count++] = spot;
			if (DEBUG_OUT) {
				DebugOut2() << "Left spot-" << count << ": " << spot.center().x_ << ", " << spot.center().y_;
			}
			if (count >= NUM_MIRE_SPOTS_LEFT) {
				break;
			}
		}
	}

	d_ptr->mireLefts = vect;
	return count;
}


int KerFocus::KeratoImage::assignMireSpotsOnRight(void)
{
	float cx, cy, y1, y2;

	cx = d_ptr->spotsRght.x_;
	cy = d_ptr->spotsRght.y_;

	y1 = (d_ptr->mireCents[0].valid_ ? d_ptr->mireCents[0].center().y_ : 0);
	y2 = (d_ptr->mireCents[1].valid_ ? d_ptr->mireCents[1].center().y_ : SAMPLE_IMAGE_HEIGHT - 1);

	auto vect = KerSpot2Vect<float>(NUM_MIRE_SPOTS_RIGHT);

	int count = 0;
	for (const auto& spot : d_ptr->spots) {
		if (spot.window().includeX(cx, 7.0f) && spot.center().inBetweenY(y1, y2)) {
			vect[count++] = spot;
			if (DEBUG_OUT) {
				DebugOut2() << "Rght spot-" << count << ": " << spot.center().x_ << ", " << spot.center().y_;
			}
			if (count >= NUM_MIRE_SPOTS_RIGHT) {
				break;
			}
		}
	}

	d_ptr->mireRghts = vect;
	return count;
}


int KerFocus::KeratoImage::assignFocusSpots(void)
{
	auto vect = KerSpot2Vect<float>(NUM_FOCUS_SPOTS);

	if (countMireSpotsOnLeft() == NUM_MIRE_SPOTS_LEFT) {
		sort(d_ptr->mireLefts.begin(), d_ptr->mireLefts.end(),
			[](const KerSpot2<float>& sp1, const KerSpot2<float>& sp2) {
			return (sp1.center().y_ < sp2.center().y_);
		});

		vect[0] = d_ptr->mireLefts[1];
		d_ptr->mireLefts.erase(d_ptr->mireLefts.begin() + 1);
		if (DEBUG_OUT) {
			DebugOut2() << "Focus spot-0: " << vect[0].center().x_ << ", " << vect[0].center().y_;
		}
	}

	if (countMireSpotsOnRight() == NUM_MIRE_SPOTS_RIGHT) {
		sort(d_ptr->mireRghts.begin(), d_ptr->mireRghts.end(),
			[](const KerSpot2<float>& sp1, const KerSpot2<float>& sp2) {
			return (sp1.center().y_ < sp2.center().y_);
		});

		vect[1] = d_ptr->mireRghts[1];
		d_ptr->mireRghts.erase(d_ptr->mireRghts.begin() + 1);
		if (DEBUG_OUT) {
			DebugOut2() << "Focus spot-1: " << vect[1].center().x_ << ", " << vect[1].center().y_;
		}
	}

	d_ptr->focusSpots = vect;

	int size = countFocusSpots();
	return size;
}


bool KerFocus::KeratoImage::calculateFocusLength(void)
{
	int size = countFocusSpots();
	if (size == NUM_FOCUS_SPOTS) 
	{
		if (d_ptr->focusSpots[0].window().alignedY(d_ptr->focusSpots[1].window(), 5.0f)) {
			d_ptr->focusLen = distanceBetween(d_ptr->focusSpots[0], d_ptr->focusSpots[1]);
			d_ptr->isFocusFound = true;
		}
		else {
			d_ptr->mireLefts[1] = d_ptr->focusSpots[0];
			d_ptr->mireRghts[1] = d_ptr->focusSpots[1];
			d_ptr->focusSpots.clear();
			d_ptr->isFocusFound = false;
		}
	}
	return false;
}


bool KerFocus::KeratoImage::calculateMireRing(void)
{
	int size = (countMireSpots() < ELLIPSE_SPOTS_MIN ? countAllSpots() : countMireSpots());
	if (size < ELLIPSE_SPOTS_MIN) {
		return false;
	}

	Mat mat = Mat::zeros(size, 2, CV_32F);
	int sidx = 0;
	for (const auto& spot : d_ptr->mireLefts) {
		if (spot.valid_) {
			mat.at<float>(sidx, 0) = spot.center().x_;
			mat.at<float>(sidx, 1) = spot.center().y_;
			sidx++;
		}
	}

	for (const auto& spot : d_ptr->mireCents) {
		if (spot.valid_) {
			mat.at<float>(sidx, 0) = spot.center().x_;
			mat.at<float>(sidx, 1) = spot.center().y_;
			sidx++;
		}
	}

	for (const auto& spot : d_ptr->mireRghts) {
		if (spot.valid_) {
			mat.at<float>(sidx, 0) = spot.center().x_;
			mat.at<float>(sidx, 1) = spot.center().y_;
			sidx++;
		}
	}

	if (sidx < size) {
		for (const auto& spot : d_ptr->focusSpots) {
			if (spot.valid_) {
				mat.at<float>(sidx, 0) = spot.center().x_;
				mat.at<float>(sidx, 1) = spot.center().y_;
				sidx++;
			}
		}
	}

	RotatedRect e = cv::fitEllipse(mat);
	d_ptr->mireCx = e.center.x;
	d_ptr->mireCy = e.center.y;
	d_ptr->mireMl = e.size.width;
	d_ptr->mireNl = e.size.height;
	d_ptr->mireAx = e.angle;
	d_ptr->isMireEllipse = true;

	if (d_ptr->mireMl < d_ptr->mireNl) {
		float t = d_ptr->mireMl;
		d_ptr->mireMl = d_ptr->mireNl;
		d_ptr->mireNl = t;
		d_ptr->mireAx += 90.0f;
		if (d_ptr->mireAx >= 180.0f) {
			d_ptr->mireAx -= 180.0f;
		}
	}

	if (KerFocus::DEBUG_OUT) {
		DebugOut2() << "Mire ellipse: " << e.size.width << ", " << e.size.height << ", " << e.angle << ", center: " << e.center.x << ", " << e.center.y;
	}
	return true;
}


bool KerFocus::KeratoImage::decideEyeCenter(void)
{
	if (isMireEllipse()) {
		d_ptr->eyeCenter.x_ = d_ptr->mireCx;
		d_ptr->eyeCenter.y_ = d_ptr->mireCy;
		d_ptr->isEyeFound = true;
	}
	else if (d_ptr->spots.size() >= NUM_KER_SPOTS_MIN) {
		float xsum = 0.0f, ysum = 0.0f;
		int size = 0;
		for (const auto& spot : d_ptr->spots) {
			xsum += spot.center().x_;
			ysum += spot.center().y_;
			size += 1;
		}
		d_ptr->eyeCenter.x_ = xsum / size;
		d_ptr->eyeCenter.y_ = ysum / size;
		d_ptr->isEyeFound = true;
	}
	else {
		d_ptr->isEyeFound = false;
	}

	return d_ptr->isEyeFound;
}


int KerFocus::KeratoImage::countMireSpotsOnCenter(void) const
{
	int count = 0;
	for (const auto& spot : d_ptr->mireCents) {
		if (spot.valid_) {
			count++;
		}
	}
	return count;
}


int KerFocus::KeratoImage::countMireSpotsOnLeft(void) const
{
	int count = 0;
	for (const auto& spot : d_ptr->mireLefts) {
		if (spot.valid_) {
			count++;
		}
	}
	return count;
}


int KerFocus::KeratoImage::countMireSpotsOnRight(void) const
{
	int count = 0;
	for (const auto& spot : d_ptr->mireRghts) {
		if (spot.valid_) {
			count++;
		}
	}
	return count;
}


int KerFocus::KeratoImage::countMireSpots(void) const
{
	return (countMireSpotsOnLeft() + countMireSpotsOnCenter() + countMireSpotsOnRight());
}


int KerFocus::KeratoImage::countFocusSpots(void) const
{
	int count = 0;
	for (const auto& spot : d_ptr->focusSpots) {
		if (spot.valid_) {
			count++;
		}
	}
	return count;
}


int KerFocus::KeratoImage::countAllSpots(void) const
{
	return (countFocusSpots() + countMireSpots());
}


CvImage & KerFocus::KeratoImage::getImage(void) const
{
	return d_ptr->image;
}


CvImage & KerFocus::KeratoImage::getSample(void) const
{
	return d_ptr->sample;
}

