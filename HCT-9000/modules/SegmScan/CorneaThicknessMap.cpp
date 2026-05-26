#include "stdafx.h"
#include "CorneaThicknessMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;
using namespace cv;

struct CorneaThicknessMap::CorneaThicknessMapImpl
{
	CorneaThicknessMapImpl()
	{
	}
};



CorneaThicknessMap::CorneaThicknessMap() :
	d_ptr(make_unique<CorneaThicknessMapImpl>())
{
}


SegmScan::CorneaThicknessMap::~CorneaThicknessMap() = default;
SegmScan::CorneaThicknessMap::CorneaThicknessMap(CorneaThicknessMap && rhs) = default;
CorneaThicknessMap & SegmScan::CorneaThicknessMap::operator=(CorneaThicknessMap && rhs) = default;


SegmScan::CorneaThicknessMap::CorneaThicknessMap(const CorneaThicknessMap & rhs)
	: d_ptr(make_unique<CorneaThicknessMapImpl>(*rhs.d_ptr))
{
}


CorneaThicknessMap & SegmScan::CorneaThicknessMap::operator=(const CorneaThicknessMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::CorneaThicknessMap::applyTearFilmSubtraction(void)
{
	auto* tmap = getThicknessMap();
	if (!tmap || tmap->isEmpty()) {
		return false;
	}
	if (tmap->upperLayer() != OcularLayerType::EPI || tmap->lowerLayer() != OcularLayerType::BOW) {
		return false;
	}

	auto* image = tmap->getDataImage();
	auto w = image->getWidth();
	auto h = image->getHeight();

	auto thicks = image->copyDataInFloats();
	if (thicks.empty()) {
		return false;
	}

	const float TEAR_FILM_MIN = 30.0f;
	const float TEAR_DIFF_MIN = 0.0f;
	const float TEAR_BLUR_SIZE = 11.0f;

	auto diffs = vector<float>(thicks.size(), 0.0f);
	for (int y = 0; y < h / 2; y++) {
		for (int x = 0; x < w; x++) {
			int idx1 = y * w + x;
			int idx2 = (h - 1 - y) * w + x;
			auto val1 = thicks[idx1];
			auto val2 = thicks[idx2];
			auto tear = val2 - val1;
			if (val1 > TEAR_FILM_MIN && val2 > TEAR_FILM_MIN && tear > TEAR_DIFF_MIN) {
				diffs[idx2] = tear;
			}
		}
	}

	/*
	LogD() << "\n\n";
	for (int y = 0; y < h; y += 5) {
		CString text, str;
		for (int x = 0; x < w; x += 5) {
			int idx = y * w + x;
			str.Format(_T("%03d "), (int)thicks[idx]);
			text += str;
		}
		LogD() << wtoa(text);
	}
	LogD() << "\n\n";

	LogD() << "\n\n";
	for (int y = 0; y < h; y += 5) {
		CString text, str;
		for (int x = 0; x < w; x += 5) {
			int idx = y * w + x;
			str.Format(_T("%03d "), (int)diffs[idx]);
			text += str;
		}
		LogD() << wtoa(text) ;
	}
	LogD() << "\n\n";
	*/

	CvImage tears;
	tears.fromFloat32((const unsigned char*)diffs.data(), w, h);
	tears.applyGaussianBlur(TEAR_BLUR_SIZE);

	auto ratio = tears.copyDataInFloats();
	auto thick2 = thicks;
	for (int y = h / 2; y < h; y++) {
		for (int x = 0; x < w; x++) {
			int idx = y * w + x;
			auto tval = thicks[idx] - ratio[idx];
			if (tval > 0.0f) {
				thick2[idx] = tval;
			}
		}
	}

	image->fromFloat32((const unsigned char*)thick2.data(), w, h);

	/*
	LogD() << "\n\n";
	for (int y = 0; y < h; y += 5) {
		CString text, str;
		for (int x = 0; x < w; x += 5) {
			int idx = y * w + x;
			str.Format(_T("%03d "), (int)thick2[idx]);
			text += str;
		}
		LogD() << wtoa(text);
	}
	LogD() << "\n\n";
	*/
	return true;
}


CorneaThicknessMap::CorneaThicknessMapImpl & SegmScan::CorneaThicknessMap::getImpl(void) const
{
	return *d_ptr;
}
