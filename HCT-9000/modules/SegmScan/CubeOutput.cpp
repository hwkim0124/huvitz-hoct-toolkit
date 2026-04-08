#include "stdafx.h"
#include "CubeOutput.h"


using namespace SegmScan;
using namespace std;


struct CubeOutput::CubeOutputImpl 
{
	bool vertical;

	CubeOutputImpl() :
		vertical(false)
	{
	}
};


CubeOutput::CubeOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, bool vertical, float axialRes) :
	d_ptr(make_unique<CubeOutputImpl>()), PatternOutput(side, region, PatternType::HorzCube, start, end, size, axialRes)
{
	d_ptr->vertical = vertical;
}


SegmScan::CubeOutput::~CubeOutput() = default;
SegmScan::CubeOutput::CubeOutput(CubeOutput && rhs) = default;
CubeOutput & SegmScan::CubeOutput::operator=(CubeOutput && rhs) = default;


SegmScan::CubeOutput::CubeOutput(const CubeOutput & rhs)
	: d_ptr(make_unique<CubeOutputImpl>(*rhs.d_ptr)), PatternOutput(rhs)
{
}


CubeOutput & SegmScan::CubeOutput::operator=(const CubeOutput & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::CubeOutput::isVerticalDirection(void) const
{
	return d_ptr->vertical;
}


int SegmScan::CubeOutput::getBscanIndexFromPosition(float mmX, float mmY) const
{
	int scans = getDescript()->getNumberOfBscans();
	float width = getDescript()->getWidth();
	float height = getDescript()->getHeight();
	if (scans <= 1 || width <= 0.0f || height <= 0.0f) {
		return -1;
	}

	// Input coordinates are supposed to the center origin in millimeters.  
	float pos_x = mmX + width / 2.0f;
	float pos_y = mmY + height / 2.0f;
	
	if (pos_x < 0.0f || pos_x > width) {
		return -1;
	}

	// Region between the two consecutive scan lines. 
	float rod_h = height / (float)(scans - 1);
	float rem_y = pos_y - rod_h / 2.0f;

	int index = 0;
	if (rem_y >= 0.0f) {
		index = (int)(rem_y / rod_h) + 1;
	}
	
	index = (index > (scans - 1) ? (scans - 1) : index);
	return index;
}


int SegmScan::CubeOutput::getBscanIndexFromPosition(int posX, int posY, int width, int height) const
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


bool SegmScan::CubeOutput::getBscanStartPosition(int index, float * mmX, float * mmY, bool center) const
{
	int scans = getDescript()->getNumberOfBscans();
	float width = getDescript()->getWidth();
	float height = getDescript()->getHeight();
	if (index < 0 || index >= scans || width <= 0.0f || height <= 0.0f) {
		return false;
	}

	// Starting from left end of pattern region.
	float pos_x = 0.0f;
	float pos_y = 0.0f;

	if (index == (scans - 1)) {
		pos_y = height;
	}
	else {
		float rod_h = height / (float)scans;
		pos_y = index * rod_h;
	}

	// Convert to center origin coordinates.
	if (center) {
		*mmX = pos_x - width / 2.0f;
		*mmY = pos_y - height / 2.0f;
	}
	else {
		*mmX = pos_x;
		*mmY = pos_y;
	}
	return true;
}


bool SegmScan::CubeOutput::getBscanStartPosition(int index, int width, int height, int * posX, int * posY) const
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


bool SegmScan::CubeOutput::getBscanEndPosition(int index, float * mmX, float * mmY, bool center) const
{
	int scans = getDescript()->getNumberOfBscans();
	float width = getDescript()->getWidth();
	float height = getDescript()->getHeight();
	if (index < 0 || index >= scans || width <= 0.0f || height <= 0.0f) {
		return false;
	}

	// Ending with right end of pattern region.
	float pos_x = width;
	float pos_y = 0.0f;

	if (index == (scans - 1)) {
		pos_y = height;
	}
	else {
		float rod_h = height / (float)scans;
		pos_y = index * rod_h;
	}

	// Convert to center origin coordinates.
	if (center) {
		*mmX = pos_x - width / 2.0f;
		*mmY = pos_y - height / 2.0f;
	}
	else {
		*mmX = pos_x;
		*mmY = pos_y;
	}
	return true;
}


bool SegmScan::CubeOutput::getBscanEndPosition(int index, int width, int height, int * posX, int * posY) const
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





