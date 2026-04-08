#include "stdafx.h"
#include "ScanRange.h"

using namespace SegmScan;
using namespace std;


struct ScanRange::ScanRangeImpl
{
	ScanPoint startPos;
	ScanPoint endPos;
	ScanPoint centerPos;
	float degree;
	float radius;


	ScanRangeImpl() :
		degree(0.0f), radius(0.0f)
	{
	}


	ScanRangeImpl(const ScanPoint& start, const ScanPoint& end) :
		startPos(start), endPos(end), degree(0.0f), radius(0.0f)
	{
	}


	ScanRangeImpl(const ScanPoint& start, const ScanPoint& end, float deg) :
		startPos(start), endPos(end), degree(deg), radius(0.0f)
	{
	}


	ScanRangeImpl(const ScanPoint& center, float rad) :
		centerPos(center), radius(rad)
	{
		startPos = ScanPoint(centerPos.x - radius, centerPos.y - radius);
		endPos = ScanPoint(centerPos.x + radius, centerPos.y + radius);
	}
};



ScanRange::ScanRange() :
	d_ptr(make_unique<ScanRangeImpl>())
{
}


SegmScan::ScanRange::~ScanRange() = default;
SegmScan::ScanRange::ScanRange(ScanRange && rhs) = default;
ScanRange & SegmScan::ScanRange::operator=(ScanRange && rhs) = default;


SegmScan::ScanRange::ScanRange(const ScanRange & rhs)
	: d_ptr(make_unique<ScanRangeImpl>(*rhs.d_ptr))
{
}


ScanRange & SegmScan::ScanRange::operator=(const ScanRange & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


SegmScan::ScanRange::ScanRange(const ScanPoint & start, const ScanPoint & end) :
	d_ptr(make_unique<ScanRangeImpl>(start, end))
{
}


SegmScan::ScanRange::ScanRange(float startX, float startY, float endX, float endY) :
	d_ptr(make_unique<ScanRangeImpl>(ScanPoint(startX, startY), ScanPoint(endX, endY)))
{
}


SegmScan::ScanRange::ScanRange(float centerX, float centerY, float radius) :
	d_ptr(make_unique<ScanRangeImpl>(ScanPoint(centerX, centerY), radius))
{
}


void SegmScan::ScanRange::set(const ScanPoint & start, const ScanPoint & end)
{
	d_ptr->startPos = start;
	d_ptr->endPos = end;
	return;
}


void SegmScan::ScanRange::set(const ScanPoint & start, const ScanPoint & end, float degree)
{
	set(start, end);
	d_ptr->degree = degree;
	return;
}


void SegmScan::ScanRange::set(const ScanPoint & center, float radius)
{
	d_ptr->centerPos = center;
	d_ptr->radius = radius;
	return;
}


ScanPoint & SegmScan::ScanRange::getStart(void) const
{
	return d_ptr->startPos;
}


ScanPoint & SegmScan::ScanRange::getEnd(void) const
{
	return d_ptr->endPos;
}


ScanPoint & SegmScan::ScanRange::getCenter(void) const
{
	return d_ptr->centerPos;
}


float SegmScan::ScanRange::getDegree(void) const
{
	return d_ptr->degree;
}


float SegmScan::ScanRange::getRadius(void) const
{
	return d_ptr->radius;
}


float SegmScan::ScanRange::getWidth(void) const
{
	return (getEnd().x - getStart().x);
}


float SegmScan::ScanRange::getHeight(void) const
{
	return (getEnd().y - getStart().y);
}


bool SegmScan::ScanRange::isHorizontalLine(void) const
{
	return (getStart().y == getEnd().y);
}


bool SegmScan::ScanRange::isVerticalLine(void) const
{
	return (getStart().x == getEnd().x);
}


bool SegmScan::ScanRange::isCircle(void) const
{
	return (getRadius() != 0.0f);
}


bool SegmScan::ScanRange::hitTest(float xPos, float yPos, float offset) const
{
	float x1, x2, y1, y2;

	if (isHorizontalLine()) {
		y1 = getStart().y - offset;
		y2 = getEnd().y + offset;
		if (getStart().x < getEnd().x) {
			x1 = getStart().x;
			x2 = getEnd().x;
		}
		else {
			x1 = getEnd().x ;
			x2 = getStart().x;
		}
		if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
			return true;
		}
	}
	else if (isVerticalLine()) {
		x1 = getStart().x - offset;
		x2 = getEnd().x + offset;
		if (getStart().y < getEnd().y) {
			y1 = getStart().y;
			y2 = getEnd().y;
		}
		else {
			y1 = getEnd().y;
			y2 = getStart().y;
		}
		if (x1 <= xPos && xPos <= x2 && y1 <= yPos && yPos <= y2) {
			return true;
		}
	}
	else if (isCircle()) {

	}
	else {
		x1 = getStart().x;
		y1 = getStart().y;
		x2 = getEnd().x;
		y2 = getEnd().y;

		float a = (y2 - y1);
		float b = (x2 - x1) * -1.0f;
		float c = (a * x1 + b * y1) * -1.0f;
		float dist = fabs(a * xPos + b * yPos + c) / sqrt(a*a + b*b);
		if (dist <= offset) {
			return true;
		}
	}
	return false;
}