#include "stdafx.h"
#include "CorTracker.h"
#include "CppUtil2.h"
#include "RetSegm2.h"
#include "SegmScan2.h"
#include "SegmScan2.h"

using namespace CorTopo;
using namespace CppUtil;
using namespace RetSegm;
using namespace SegmScan;
using namespace cv;
using namespace std;


struct CorTracker::CorTrackerImpl
{
	SegmImage imgCornea;
	SegmImage imgSample;
	SegmImage imgAscent;

	std::vector<int> corneaAntes;
	int topCorneaCenterX = -1;
	int topCorneaCenterY = -1;
	float corneaCenterPosX = 0.0f;
	float corneaCenterPosY = 0.0f;

	bool isResult = false;
	OctScanPattern pattern;

	PreviewCorneaCallback callback;

	CorTrackerImpl() {
	}
};


CorTracker::CorTracker() :
	d_ptr(make_unique<CorTrackerImpl>())
{
	getImpl().callback = std::bind(&CorTracker::callbackPreviewImage, this, std::placeholders::_1, std::placeholders::_2, 
		std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7);
}


CorTopo::CorTracker::~CorTracker() = default;
CorTopo::CorTracker::CorTracker(CorTracker && rhs) = default;
CorTracker & CorTopo::CorTracker::operator=(CorTracker && rhs) = default;


CorTracker::CorTrackerImpl & CorTopo::CorTracker::getImpl(void) const
{
	return *d_ptr;
}


CorTopo::CorTracker::CorTracker(const CorTracker & rhs)
	: d_ptr(make_unique<CorTrackerImpl>(*rhs.d_ptr))
{
}


CorTracker & CorTopo::CorTracker::operator=(const CorTracker & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


PreviewCorneaCallback * CorTopo::CorTracker::getCallbackFunction(void)
{
	return &getImpl().callback;
}


void CorTopo::CorTracker::callbackPreviewImage(const unsigned char * data, unsigned int width, unsigned int height, 
	float quality, float sigRatio, unsigned int refPoint, unsigned int idxOfImage)
{
	getImpl().isResult = false; 

	if (data == nullptr) {
		return;
	}

	if (quality >= VALID_PREVIEW_SNR_MIN) {
		if (loadImage(data, width, height)) {
			if (process()) {
				GlobalRecord::writeCorneaTrackAnteriorCenter((float)getImpl().topCorneaCenterX, (float)getImpl().topCorneaCenterY, idxOfImage);
				GlobalRecord::writeCorneaTrackAnteriorCurve(getImpl().corneaAntes, idxOfImage);
				GlobalRecord::writeCorneaTrackTargetFound(true, idxOfImage);
				getImpl().isResult = true;
				return;
			}
		}
	}
	GlobalRecord::writeCorneaTrackTargetFound(false, idxOfImage);
	getImpl().isResult = false;
	return;
}


void CorTopo::CorTracker::setScanPreviewPattern(OctScanPattern pattern)
{
	getImpl().pattern = pattern;
	return;
}


void CorTopo::CorTracker::initialize(OctScanPattern pattern)
{
	getImpl().isResult = false;

	setScanPreviewPattern(pattern);
	return;
}


bool CorTopo::CorTracker::isResult(void) const
{
	return getImpl().isResult;
}


bool CorTopo::CorTracker::isImagesLoaded(void) const
{
	if (imageCornea()->isEmpty()) {
		return false;
	}
	return true;
}


bool CorTopo::CorTracker::process(void)
{
	vector<int> antes;

	int centerX, centerY;
	float rangeX = getImpl().pattern.getScanRangeX();

	if (!segmentAnteriorBorderLine(rangeX, centerX, centerY, antes)) {
		centerX = -1;
		centerY = -1;
		antes.clear();
		return false;
	}

	if (upscaleBorderLine(rangeX, antes)) {
		return true;
	}
	return false;
}



bool CorTopo::CorTracker::loadImage(const unsigned char * bits, int width, int height)
{
	try {
		if (!bits || !imageCornea()->fromBitsData(bits, width, height)) {
			// LogD() << "Failed to load cornea bitmap";
			return false;
		}

		if (!prepareSamples()) {
			return false;
		}
		return true;
	}
	catch (...) {
	}
	return false;
}


bool CorTopo::CorTracker::prepareSamples(void)
{
	if (!isImagesLoaded()) {
		return false;
	}

	imageCornea()->flipVertical();

	Size size(SAMPLE_IMAGE_WIDTH, SAMPLE_IMAGE_HEIGHT);
	resize(imageCornea()->getCvMatConst(), imageSample()->getCvMat(), size);

	imageSample()->applyGuidedFilter();

	PrepCornea::collectImageStats(imageSample());
	PrepCornea::collectColumnStats(imageSample());

	CoarseCornea::createGradientMapOfCornea(imageSample(), imageAscent());
	PrepCornea::collectImageStats(imageAscent());
	PrepCornea::collectColumnStats(imageAscent());
	return true;
}


bool CorTopo::CorTracker::segmentAnteriorBorderLine(float rangeX, int & centerX, int & centerY, std::vector<int>& antes)
{
	vector<int> inner;
	vector<int> outer;
	vector<int> fitts;
	inner.resize(imageSample()->getWidth(), -1);
	outer.resize(imageSample()->getWidth(), -1);
	fitts.resize(imageSample()->getWidth(), -1);

	int hingeY = -1;
	int reflectX1 = -1;
	int reflectX2 = -1;

	if (!CoarseCornea::locateCenterReflection(imageSample(), rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::locateCenterEdgesOfCornea(imageSample(), imageAscent(), inner, outer, 
												rangeX, reflectX1, reflectX2)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInCenterEdges(imageAscent(), inner, outer, rangeX)) {
		return false;
	}

	if (!CoarseCornea::composeInnerCurveByCenterEdges(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::locateInnerBorderLine(imageSample(), inner, outer, rangeX, hingeY, reflectX1, reflectX2, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::removeFractionsInInnerBorderLine(inner, rangeX, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::removeOutliersInInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!CoarseCornea::interpolateInnerBorderLine(inner, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	antes = inner;
	return true;
}


bool CorTopo::CorTracker::upscaleBorderLine(float rangeX, std::vector<int> antes)
{
	int img_w = imageCornea()->getWidth();
	int img_h = imageCornea()->getHeight();

	auto line1 = std::vector<int>(img_w, -1);

	int size = (int)antes.size();
	float ratioH = (float)img_w / size;
	float ratioV = (float)img_h / 384;

	for (int i = 0; i < img_w / 2; i++) {
		int idx = min(max((int)(i*(1.0f / ratioH)), 0), size - 1);
		line1[i] = (int)(antes[idx] * ratioV);
	}

	{
		auto size = img_w;
		auto dataX = vector<int>(size, -1);
		auto dataY = vector<int>(size, -1);
		auto fittY = vector<int>(size, -1);

		for (int c = 0; c < size; c++) {
			dataX[c] = c;
			dataY[c] = (line1[c] < 0 ? 0 : line1[c]);
		}

		vector<double> coeffs;
		if (CppUtil::Regressor::chevyPolyCurve(dataX, dataY, 2, true, fittY, coeffs)) {
			line1 = fittY;
		}
	}

	if (line1.size() > 0) {
		int size = (int)line1.size();
		int min_val = 9999;
		int min_pos = -1;
		for (int i = 0; i < size; i++) {
			if (line1[i] < min_val) {
				min_val = line1[i];
				min_pos = i;
			}
		}
		int xsum = 0;
		int wsum = 0;
		for (int i = min_pos; i < size; i++) {
			if (line1[i] == min_val) {
				xsum += i;
				wsum += 1;
			}
		}

		int centerX = xsum / wsum;
		int centerY = min_val;
		getImpl().topCorneaCenterX = centerX;
		getImpl().topCorneaCenterY = centerY;
		getImpl().corneaAntes = line1;

		float pixelPerMM = img_w / rangeX;
		float offsetX = (float)(centerX - img_w / 2); // / pixelPerMM;
		getImpl().corneaCenterPosX = offsetX;
		getImpl().corneaCenterPosY = 0.0f;
		return true;
	}
	else {
		getImpl().topCorneaCenterX = -1;
		getImpl().topCorneaCenterY = -1;
		getImpl().corneaAntes.clear();
		return false;
	}
}


RetSegm::SegmImage * CorTopo::CorTracker::imageCornea(void) const
{
	return &(getImpl().imgCornea);
}


RetSegm::SegmImage * CorTopo::CorTracker::imageSample(void) const
{
	return &(getImpl().imgSample);
}


RetSegm::SegmImage * CorTopo::CorTracker::imageAscent(void) const
{
	return &(getImpl().imgAscent);
}

