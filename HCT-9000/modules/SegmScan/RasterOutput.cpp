#include "stdafx.h"
#include "RasterOutput.h"


using namespace SegmScan;
using namespace std;


struct RasterOutput::RasterOutputImpl
{

};


RasterOutput::RasterOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, float axialRes) :
	d_ptr(make_unique<RasterOutputImpl>()), PatternOutput(side, region, PatternType::HorzRaster, start, end, size, axialRes)
{
}


SegmScan::RasterOutput::~RasterOutput() = default;
SegmScan::RasterOutput::RasterOutput(RasterOutput && rhs) = default;
RasterOutput & SegmScan::RasterOutput::operator=(RasterOutput && rhs) = default;


SegmScan::RasterOutput::RasterOutput(const RasterOutput & rhs)
	: d_ptr(make_unique<RasterOutputImpl>(*rhs.d_ptr)), PatternOutput(rhs)
{
}


RasterOutput & SegmScan::RasterOutput::operator=(const RasterOutput & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


RasterOutput::RasterOutputImpl & SegmScan::RasterOutput::getImpl(void) const
{
	return *d_ptr;
}


int SegmScan::RasterOutput::getBscanIndexFromPosition(float mmX, float mmY) const
{
	for (int i = 0; i < getResultCount(); i++) {
		ScanRange* range = getResult(i)->getRange();
		if (range->hitTest(mmX, mmY)) {
			return i;
		}
	}
	return -1;
}


int SegmScan::RasterOutput::getBscanIndexFromPosition(int posX, int posY, int width, int height) const
{
	if (width <= 0.0f || height <= 0.0f || posX < 0 || posY < 0) {
		return -1;
	}

	float scan_w = getDescript()->getWidth();
	float scan_h = getDescript()->getHeight();

	// Convert to center origin coordinates in millimeter.
	float pos_x = ((float)posX / (float)width) * scan_w;
	float pos_y = ((float)posY / (float)height) * scan_h;

	pos_x -= scan_w / 2.0f;
	pos_y -= scan_h / 2.0f;

	return getBscanIndexFromPosition(pos_x, pos_y);
}


bool SegmScan::RasterOutput::getBscanStartPosition(int index, float * mmX, float * mmY, bool center) const
{
	BscanResult* p = getResult(index);
	if (p == nullptr) {
		return false;
	}

	float pos_x = p->getRange()->getStart().x;
	float pos_y = p->getRange()->getStart().y;

	// Convert to center origin coordinates.
	if (center) {
		*mmX = pos_x ;
		*mmY = pos_y ;
	}
	else {
		float width = getDescript()->getWidth();
		float height = getDescript()->getHeight();
		*mmX = pos_x + width / 2.0f;
		*mmY = pos_y + height / 2.0f;
	}
	return true;
}


bool SegmScan::RasterOutput::getBscanStartPosition(int index, int width, int height, int * posX, int * posY) const
{
	float mmX, mmY;
	if (!getBscanStartPosition(index, &mmX, &mmY, false)) {
		return false;
	}

	float pos_x = (mmX / getDescript()->getWidth());
	float pos_y = (mmY / getDescript()->getHeight());
	*posX = (int)(pos_x * width);
	*posY = (int)(pos_y * height);
	return true;
}


bool SegmScan::RasterOutput::getBscanEndPosition(int index, float * mmX, float * mmY, bool center) const
{
	BscanResult* p = getResult(index);
	if (p == nullptr) {
		return false;
	}

	float pos_x = p->getRange()->getEnd().x;
	float pos_y = p->getRange()->getEnd().y;

	// Convert to center origin coordinates.
	if (center) {
		*mmX = pos_x;
		*mmY = pos_y;
	}
	else {
		float width = getDescript()->getWidth();
		float height = getDescript()->getHeight();
		*mmX = pos_x + width / 2.0f;
		*mmY = pos_y + height / 2.0f;
	}
	return true;
}


bool SegmScan::RasterOutput::getBscanEndPosition(int index, int width, int height, int * posX, int * posY) const
{
	float mmX, mmY;
	if (!getBscanEndPosition(index, &mmX, &mmY, false)) {
		return false;
	}

	float pos_x = (mmX / getDescript()->getWidth());
	float pos_y = (mmY / getDescript()->getHeight());
	*posX = (int)(pos_x * width);
	*posY = (int)(pos_y * height);
	return true;
}
