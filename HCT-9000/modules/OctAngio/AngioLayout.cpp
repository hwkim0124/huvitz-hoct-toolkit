#include "stdafx.h"
#include "AngioLayout.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;


struct AngioLayout::AngioLayoutImpl
{
	int lines = 0;
	int repeats = 0;
	int points = 0;
	bool vertical = false;
	bool isDisc = false;

	float rangeX = 3.0f;
	float rangeY = 3.0f;
	float foveaX = 0.0f;
	float foveaY = 0.0f;

	AngioLayoutImpl() {

	};
};



AngioLayout::AngioLayout()
	: d_ptr(make_unique<AngioLayoutImpl>())
{
}


OctAngio::AngioLayout::AngioLayout(int lines, int points, int repeats, bool vertical)
	: d_ptr(make_unique<AngioLayoutImpl>())
{
	getImpl().lines = lines;
	getImpl().points = points;
	getImpl().repeats = repeats;
	getImpl().vertical = vertical;
}

AngioLayout::~AngioLayout()
{
	// Destructor should be defined for unique_ptr to delete AngioLayoutImpl as an incomplete type.
}


OctAngio::AngioLayout::AngioLayout(AngioLayout && rhs) = default;
AngioLayout & OctAngio::AngioLayout::operator=(AngioLayout && rhs) = default;

OctAngio::AngioLayout::AngioLayout(const AngioLayout & rhs)
	: d_ptr(make_unique<AngioLayoutImpl>(*rhs.d_ptr))
{
}


AngioLayout & OctAngio::AngioLayout::operator=(const AngioLayout & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctAngio::AngioLayout::setupLayout(int lines, int points, int repeats, bool vertical)
{
	getImpl().lines = lines;
	getImpl().points = points;
	getImpl().repeats = repeats;
	getImpl().vertical = vertical;
}

void OctAngio::AngioLayout::setupRange(float rangeX, float rangeY, float centerX, float centerY, bool isDisc)
{
	getImpl().rangeX = rangeX;
	getImpl().rangeY = rangeY;
	getImpl().foveaX = (fabs(centerX) < (rangeX / 2.0f) ? centerX : 0.0f);
	getImpl().foveaY = (fabs(centerY) < (rangeY / 2.0f) ? centerY : 0.0f);
	getImpl().isDisc = isDisc;
}

int OctAngio::AngioLayout::numberOfLines(void) const
{
	return getImpl().lines;
}

int OctAngio::AngioLayout::numberOfPoints(void) const
{
	return getImpl().points;
}

int OctAngio::AngioLayout::numberOfRepeats(void) const
{
	return getImpl().repeats;
}

bool OctAngio::AngioLayout::isVerticalScan(void) const
{
	return getImpl().vertical;
}

bool OctAngio::AngioLayout::isDiscScan(void) const
{
	return getImpl().isDisc;
}

bool OctAngio::AngioLayout::isMacularScan(void) const
{
	return !isDiscScan();
}

bool OctAngio::AngioLayout::isFoveaCenter(void) const
{
	if (!isMacularScan()) {
		return false;
	}

	if (scanRangeX() > 9.0f || scanRangeY() > 9.0f) {
		return false;
	}
	return true;
}

float OctAngio::AngioLayout::scanRangeX(void) const
{
	return getImpl().rangeX;
}

float OctAngio::AngioLayout::scanRangeY(void) const
{
	return getImpl().rangeY;
}

float OctAngio::AngioLayout::foveaCenterX(void) const
{
	return getImpl().foveaX;
}

float OctAngio::AngioLayout::foveaCenterY(void) const
{
	return getImpl().foveaY;
}

bool OctAngio::AngioLayout::getFoveaCenterInPixel(int& cx, int& cy) const
{
	if (isMacularScan()) {
		float rangeX = scanRangeX();
		float rangeY = scanRangeY();
		float fov_cx = foveaCenterX();
		float fov_cy = foveaCenterY();

		float pos_x = rangeX / 2.0f + fov_cx;
		float pos_y = rangeY / 2.0f + fov_cy;
		float w = getWidth();
		float h = getHeight();

		if (rangeX <= 0.0f || rangeY <= 0.0f) {
			cx = (int)(w / 2.0f);
			cy = (int)(h / 2.0f);
		}
		else {
			cx = (int)((w / rangeX) * pos_x);
			cy = (int)((h / rangeY) * pos_y);
		}
		return true;
	}
	return false;
}

bool OctAngio::AngioLayout::getFoveaRadiusInPixel(int& size1, int& size2, float radius) const
{
	if (isMacularScan()) {
		float FOVEA_RADIUS_IN_MM = radius;
		const float SCAN_RANGE_X = 4.5f;
		const float SCAN_RANGE_Y = 4.5f;

		float rangeX = scanRangeX();
		float rangeY = scanRangeY();
		float fov_cx = foveaCenterX();
		float fov_cy = foveaCenterY();

		float pos_x = rangeX / 2.0f + fov_cx;
		float pos_y = rangeY / 2.0f + fov_cy;
		float w = getWidth();
		float h = getHeight();

		if (rangeX <= 0.0f || rangeY <= 0.0f) {
			size1 = (int)((w / SCAN_RANGE_X) * FOVEA_RADIUS_IN_MM);
			size2 = (int)((h / SCAN_RANGE_Y) * FOVEA_RADIUS_IN_MM);
		}
		else {
			size1 = (int)((w / rangeX) * FOVEA_RADIUS_IN_MM);
			size2 = (int)((h / rangeY) * FOVEA_RADIUS_IN_MM);
		}
		return true;
	}
	return false;
}

int OctAngio::AngioLayout::getWidth(void) const
{
	return (isVerticalScan() ? numberOfLines() : numberOfPoints());
}

int OctAngio::AngioLayout::getHeight(void) const
{
	return (isVerticalScan() ? numberOfPoints() : numberOfLines());
}

int OctAngio::AngioLayout::getSize(void) const
{
	return getImpl().lines * getImpl().points;
}

float OctAngio::AngioLayout::getPixelSizeX(void) const
{
	float rangeX = scanRangeX();
	float width = getWidth();

	if (rangeX <= 0.0f || width <= 0) {
		return 0.0f;
	}
	else {
		return rangeX / width;
	}
}

float OctAngio::AngioLayout::getPixelSizeY(void) const
{
	float rangeY = scanRangeY();
	float height = getHeight();

	if (rangeY <= 0.0f || height <= 0) {
		return 0.0f;
	}
	else {
		return rangeY / height;
	}
}

AngioLayout::AngioLayoutImpl & OctAngio::AngioLayout::getImpl(void) const
{
	return *d_ptr;
}
