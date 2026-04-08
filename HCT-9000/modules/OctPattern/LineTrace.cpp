#include "stdafx.h"
#include "LineTrace.h"

#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct LineTrace::LineTraceImpl
{
	int traceId;
	LineTraceType type;
	OctRoute path;
	ScanSpeed speed;

	int numScanPoints;
	int numForePaddings;
	int numPostPaddings;
	int numRepeats;

	int16_t  galvPosX1[8192];
	int16_t  galvPosY1[8192];
	int16_t  galvPosX2[8192];
	int16_t  galvPosY2[8192];

	int countOfPosX;
	int countOfPosY;
	bool hidden;
	bool lineHD;
	

	LineTraceImpl() : traceId(0), type(LineTraceType::Line),
		numScanPoints(0), numRepeats(1),
		numForePaddings(TRIGGER_FORE_PADDING_POINTS), numPostPaddings(TRIGGER_POST_PADDING_POINTS),
		galvPosX1{ 0 }, galvPosY1{ 0 }, galvPosX2{ 0 }, galvPosY2{ 0 },
		countOfPosX(0), countOfPosY(0), speed(ScanSpeed::Fastest), hidden(false), lineHD(false)
	{
	}

};


LineTrace::LineTrace() :
	d_ptr(make_unique<LineTraceImpl>())
{
}


OctPattern::LineTrace::~LineTrace() = default;
OctPattern::LineTrace::LineTrace(LineTrace && rhs) = default;
LineTrace & OctPattern::LineTrace::operator=(LineTrace && rhs) = default;


OctPattern::LineTrace::LineTrace(const LineTrace & rhs)
	: d_ptr(make_unique<LineTraceImpl>(*rhs.d_ptr))
{
}


LineTrace & OctPattern::LineTrace::operator=(const LineTrace & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctPattern::LineTrace::initLine(int traceId, OctPoint start, OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::Line;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::initCircle(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::Circle;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::initHorzRaster(int traceId, OctPoint start, OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::HorzRaster;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::initVertRaster(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::VertRaster;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::initHorzRasterFast(int traceId, OctPoint start, OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::HorzRasterFast;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::initVertRasterFast(int traceId, OctGlobal::OctPoint start, OctGlobal::OctPoint end, int numPoints, int numRepeats, int forePadds, int postPadds)
{
	getImpl().traceId = traceId;
	getImpl().type = LineTraceType::VertRasterFast;
	getImpl().path.setLine(start, end);
	getImpl().numScanPoints = numPoints;
	getImpl().numRepeats = numRepeats;
	setPaddings(forePadds, postPadds);
	return;
}


void OctPattern::LineTrace::setPaddings(int forePadds, int postPadds, int numPoints)
{
	if (numPoints != 0) {
		forePadds = (int)(numPoints * TRIGGER_FORE_PADDING_RATIO);
		postPadds = (int)(numPoints * TRIGGER_POST_PADDING_RATIO);
		// forePadds = max(forePadds, TRIGGER_FORE_PADDING_POINTS);
		// postPadds = max(postPadds, TRIGGER_POST_PADDING_POINTS);
	}

	getImpl().numForePaddings = forePadds;
	getImpl().numPostPaddings = postPadds;
	// LogD() << "Set paddings: " << forePadds << ", " << postPadds << ", " << numPoints;
	return;
}


OctRoute OctPattern::LineTrace::getRouteOfScan(void) const
{
	OctRoute route;
	if (isCircle()) {
		route.setLine(getStartX(), getStartY(), getCloseX(), getCloseY());
	}
	else {
		route.setLine(getStartX(), getStartY(), getCloseX(), getCloseY());
	}
	return route;
}


LineTraceType OctPattern::LineTrace::getTraceType(void) const
{
	return getImpl().type;
}


std::string OctPattern::LineTrace::getTraceTypeStr(void) const
{
	if (isLine()) {
		return string("line");
	}
	if (isCircle()) {
		return string("circle");
	}
	if (isRasterX()) {
		return string("rasterX");
	}
	if (isRasterY()) {
		return string("rasterY");
	}
	if (isRasterFastX()) {
		return string("rasterFastX");
	}
	if (isRasterFastY()) {
		return string("rasterFastY");
	}
	return string("unknown");
}


int OctPattern::LineTrace::getTraceId(void) const
{
	return getImpl().traceId;
}


int OctPattern::LineTrace::getForePaddings(void) const
{
	return getImpl().numForePaddings;
}


int OctPattern::LineTrace::getPostPaddings(void) const
{
	return getImpl().numPostPaddings;
}


int OctPattern::LineTrace::getNumberOfScanPoints(bool repeats) const
{
	if (!repeats) {
		return getImpl().numScanPoints;
	}
	else {
		return (getImpl().numScanPoints * getNumberOfRepeats());
	}
}


int OctPattern::LineTrace::getNumberOfRepeats(void) const
{
	return getImpl().numRepeats;
}


void OctPattern::LineTrace::setScanSpeed(ScanSpeed speed)
{
	d_ptr->speed = speed;
	return;
}


ScanSpeed OctPattern::LineTrace::getScanSpeed(void)
{
	return d_ptr->speed;
}


float OctPattern::LineTrace::getStartX(void) const
{
	return getImpl().path.start()._x;
}


float OctPattern::LineTrace::getStartY(void) const
{
	return getImpl().path.start()._y;
}


float OctPattern::LineTrace::getCloseX(void) const
{
	return getImpl().path.close()._x;
}


float OctPattern::LineTrace::getCloseY(void) const
{
	return getImpl().path.close()._y;
}


short OctPattern::LineTrace::getPositionX(int index) const
{
	if (index < 0 || index >= getImpl().countOfPosX) {
		return 0;
	}
	return getImpl().galvPosX1[index];
}


short OctPattern::LineTrace::getPositionY(int index) const
{
	if (index < 0 || index >= getImpl().countOfPosY) {
		return 0;
	}
	return getImpl().galvPosY1[index];
}


short * OctPattern::LineTrace::getGalvanoPositionsX(bool reversed) const
{
	if (reversed) {
		getImpl().galvPosX2;
	}
	return getImpl().galvPosX1;
}


short * OctPattern::LineTrace::getGalvanoPositionsY(bool reversed) const
{
	if (reversed) {
		getImpl().galvPosY2;
	}
	return getImpl().galvPosY1;
}


void OctPattern::LineTrace::setGalvanoPositionsX(short * posXs, short count)
{
	/*
	if (count > 0) {
		memcpy(getImpl().galvPosXs, posXs, sizeof(short)*count);
	}
	*/
	for (int i = 0; i < count; i++) {
		d_ptr->galvPosX1[i] = posXs[i];
		d_ptr->galvPosX2[i] = posXs[count - i - 1];
	}

	getImpl().countOfPosX = count;
	return;
}


void OctPattern::LineTrace::setGalvanoPositionsY(short * posYs, short count)
{
	/*
	if (count > 0) {
		memcpy(getImpl().galvPosYs, posYs, sizeof(short)*count);
	}
	*/
	for (int i = 0; i < count; i++) {
		d_ptr->galvPosY1[i] = posYs[i];
		d_ptr->galvPosY2[i] = posYs[count - i - 1];
	}

	getImpl().countOfPosY = count;
	return;
}


int OctPattern::LineTrace::getCountOfPositionsX(void) const
{
	return getImpl().countOfPosX;
}


int OctPattern::LineTrace::getCountOfPositionsY(void) const
{
	return getImpl().countOfPosY;
}


bool OctPattern::LineTrace::isLine(void) const
{
	return (getImpl().type == LineTraceType::Line);
}


bool OctPattern::LineTrace::isCircle(void) const
{
	return (getImpl().type == LineTraceType::Circle);
}


bool OctPattern::LineTrace::isRasterX(void) const
{
	return (getImpl().type == LineTraceType::HorzRaster);
}


bool OctPattern::LineTrace::isRasterY(void) const
{
	return (getImpl().type == LineTraceType::VertRaster);
}


bool OctPattern::LineTrace::isRasterFastX(void) const
{
	return (getImpl().type == LineTraceType::HorzRasterFast);
}


bool OctPattern::LineTrace::isRasterFastY(void) const
{
	return (getImpl().type == LineTraceType::VertRasterFast);
}

bool OctPattern::LineTrace::isHidden(void) const
{
	return getImpl().hidden;
}

bool OctPattern::LineTrace::isLineHD(void) const
{
	return getImpl().lineHD;
}

void OctPattern::LineTrace::setHidden(bool flag)
{
	getImpl().hidden = flag;
	return;
}

void OctPattern::LineTrace::setLineHD(bool flag)
{
	getImpl().lineHD = flag;
	return;
}


LineTrace::LineTraceImpl & OctPattern::LineTrace::getImpl(void) const
{
	return *d_ptr;
}

