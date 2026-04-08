#include "stdafx.h"
#include "PatternDescript.h"
#include "ScanRange.h"

using namespace SegmScan;
using namespace std;


struct PatternDescript::PatternDescriptImpl
{
	EyeSide side;
	ScanRegion region;
	PatternType type;
	float axialRes;
	ScanPoint startPos;
	ScanPoint endPos;
	ScanRange range;
	int numOfBscans;

	PatternDescriptImpl() :
		side(EyeSide::OD), region(ScanRegion::Macular), type(PatternType::HorzLine), numOfBscans(0), axialRes(AXIAL_RESOLUTION) {
	}

};


PatternDescript::PatternDescript() :
	d_ptr(make_unique<PatternDescriptImpl>())
{
}


SegmScan::PatternDescript::PatternDescript(EyeSide side, ScanRegion region, PatternType type, int size, float axialRes) :
	d_ptr(make_unique<PatternDescriptImpl>())
{
	d_ptr->side = side;
	d_ptr->region = region;
	d_ptr->type = type;
	d_ptr->numOfBscans = size;
	d_ptr->axialRes = axialRes;
}


SegmScan::PatternDescript::PatternDescript(EyeSide side, ScanRegion region, PatternType type, ScanPoint start, ScanPoint end, int size, float axialRes) :
	PatternDescript(side, region, type, size, axialRes)
{
	setRange(ScanRange(start, end));
}


SegmScan::PatternDescript::~PatternDescript() = default;
SegmScan::PatternDescript::PatternDescript(PatternDescript && rhs) = default;
PatternDescript & SegmScan::PatternDescript::operator=(PatternDescript && rhs) = default;


SegmScan::PatternDescript::PatternDescript(const PatternDescript & rhs)
	: d_ptr(make_unique<PatternDescriptImpl>(*rhs.d_ptr))
{
}


PatternDescript & SegmScan::PatternDescript::operator=(const PatternDescript & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


EyeSide SegmScan::PatternDescript::getSide(void) const
{
	return d_ptr->side;
}


ScanRegion SegmScan::PatternDescript::getRegion(void) const
{
	return d_ptr->region;
}


PatternType SegmScan::PatternDescript::getType(void) const
{
	return d_ptr->type;
}


int SegmScan::PatternDescript::getNumberOfBscans(void) const
{
	return d_ptr->numOfBscans;
}


float SegmScan::PatternDescript::getAxialResolution(void) const
{
	return d_ptr->axialRes;
}


void SegmScan::PatternDescript::setSide(EyeSide side)
{
	d_ptr->side = side;
	return;
}


void SegmScan::PatternDescript::setRegion(ScanRegion region)
{
	d_ptr->region = region;
	return;
}


void SegmScan::PatternDescript::setType(PatternType type)
{
	d_ptr->type = type;
	return;
}


void SegmScan::PatternDescript::setNumberOfBscans(int size)
{
	d_ptr->numOfBscans = size;
	return;
}


void SegmScan::PatternDescript::setAxialResolution(float axialRes)
{
	d_ptr->axialRes = axialRes;
	return;
}


void SegmScan::PatternDescript::setRange(ScanRange range)
{
	d_ptr->range = range;
	return;
}


ScanPoint & SegmScan::PatternDescript::getStartPos(void) const
{
	return d_ptr->range.getStart();
}


ScanPoint & SegmScan::PatternDescript::getEndPos(void) const
{
	return d_ptr->range.getEnd();
}


float SegmScan::PatternDescript::getWidth(void) const
{
	return d_ptr->range.getWidth();
}


float SegmScan::PatternDescript::getHeight(void) const
{
	return d_ptr->range.getHeight();
}


bool SegmScan::PatternDescript::isLinePattern(void) const
{
	return (getType() == PatternType::HorzLine);
}


bool SegmScan::PatternDescript::isCubePattern(void) const
{
	return (getType() == PatternType::HorzCube);
}