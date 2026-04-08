#include "stdafx.h"
#include "MeasureScan.h"
#include "PatternFrame.h"
#include "LineTrace.h"

#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct MeasureScan::MeasureScanImpl
{
	MeasureScanImpl() {

	}
};


MeasureScan::MeasureScan() :
	d_ptr(make_unique<MeasureScanImpl>())
{
}


OctPattern::MeasureScan::~MeasureScan() = default;
OctPattern::MeasureScan::MeasureScan(MeasureScan && rhs) = default;
MeasureScan & OctPattern::MeasureScan::operator=(MeasureScan && rhs) = default;


OctPattern::MeasureScan::MeasureScan(const MeasureScan & rhs)
	: d_ptr(make_unique<MeasureScanImpl>(*rhs.d_ptr))
{
}


MeasureScan & OctPattern::MeasureScan::operator=(const MeasureScan & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctPattern::MeasureScan::buildPattern(bool hidden)
{
	clearPatternFrames();

	// Build line traces for preview and measure scan pattern.
	switch (getPatternType()) {
	case PatternType::Point:
		buildPatternPoint();
		break;
	case PatternType::HorzLine:
		buildPatternLine(false);
		break;
	case PatternType::VertLine:
		buildPatternLine(true);
		break;
	case PatternType::Circle:
		buildPatternCircle();
		break;
	case PatternType::Cross:
		buildPatternCross();
		break;
	case PatternType::Radial: 
		buildPatternRadial();
		break;
	case PatternType::HorzRaster:
		buildPatternRaster(false);
		break;
	case PatternType::HorzCube:
		buildPatternCube(false);
		break;
	case PatternType::VertRaster:
		buildPatternRaster(true);
		break;
	case PatternType::VertCube:
		buildPatternCube(true);
		break;
	default:
		return false;
	}
	return true;
}


void OctPattern::MeasureScan::buildPatternPoint(void)
{
	float xStart = PATTERN_SCAN_CENTER_X;
	float xClose = PATTERN_SCAN_CENTER_X;
	float yStart = PATTERN_SCAN_CENTER_Y;
	float yClose = PATTERN_SCAN_CENTER_Y;

	xStart += getScanOffsetX(true);
	xClose += getScanOffsetX(true);
	yStart += getScanOffsetY(true);
	yClose += getScanOffsetY(true);

	int numPoints = getNumberOfScanPoints();
	int numRepeats = getNumberOfScanOverlaps();

	LineTrace line;
	line.initLine(TRACE_ID_MEASURE_LINE,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints, 
		numRepeats);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::MeasureScan::buildPatternLine(bool isVert, int lineCount)
{
	float radiusX, radiusY;

	if (isVert) {
		radiusX = 0.0f;
		radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	}
	else {
		radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
		radiusY = 0.0f;
	}

	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	double angle = getScanAngle();
	if (std::fabs(angle) > 0.0001) {
		double radian = NumericFunc::degreeToRadian(angle);
		float centerX = PATTERN_SCAN_CENTER_X;
		float centerY = PATTERN_SCAN_CENTER_Y;
		float dxStart = xStart - centerX;
		float dyStart = yStart - centerY;
		float dxClose = xClose - centerX;
		float dyClose = yClose - centerY;
		float cosA = (float)cos(radian);
		float sinA = (float)sin(radian);
		xStart = centerX + (dxStart * cosA - dyStart * sinA);
		yStart = centerY + (dxStart * sinA + dyStart * cosA);
		xClose = centerX + (dxClose * cosA - dyClose * sinA);
		yClose = centerY + (dxClose * sinA + dyClose * cosA);
	}

	xStart += getScanOffsetX(true);
	xClose += getScanOffsetX(true);
	yStart += getScanOffsetY(true);
	yClose += getScanOffsetY(true);

	int numPoints = getNumberOfScanPoints();
	int numRepeats = getNumberOfScanOverlaps();

	LineTrace line;
	line.initLine(TRACE_ID_MEASURE_LINE + lineCount,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints, 
		numRepeats); // PREVIEW_LINE_NUM_POINTS);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::MeasureScan::buildPatternCircle(void)
{
	float radiusX, radiusY;

	radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	xStart += getScanOffsetX(true);
	xClose += getScanOffsetX(true);
	yStart += getScanOffsetY(true);
	yClose += getScanOffsetY(true);

	int numPoints = getNumberOfScanPoints();
	int numRepeats = getNumberOfScanOverlaps();

	LineTrace line;
	line.initCircle(TRACE_ID_MEASURE_CIRCLE,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints,
		numRepeats);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::MeasureScan::buildPatternCross(void)
{
	float radiusX, radiusY;

	radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY ;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY ;

	float xInterval = getScanLineSpace() * getScanScaleX();
	float yInterval = getScanLineSpace() * getScanScaleY();

	int numPoints = getNumberOfScanPoints();
	int numRepeats = getNumberOfScanOverlaps();
	int numLines = getNumberOfScanLines() ;
	int numFrames = numLines;
	int halfSize = numLines / 2;

	float x1, y1, x2, y2;
	double angle = getScanAngle();
	int frameIdx = 0;
	int lineIdx = 0;

	// Horizontal lines.
	for (; lineIdx < halfSize; lineIdx++)
	{
		x1 = xStart;
		x2 = xClose;
		y1 = PATTERN_SCAN_CENTER_Y + yInterval * (lineIdx - halfSize/2);
		y2 = PATTERN_SCAN_CENTER_Y + yInterval * (lineIdx - halfSize/2);

		x1 += getScanOffsetX(true);
		x2 += getScanOffsetX(true);
		y1 += getScanOffsetY(true);
		y2 += getScanOffsetY(true);

		LineTrace line;
		line.initLine(TRACE_ID_MEASURE_CROSS + lineIdx,
			OctPoint(x1, y1),
			OctPoint(x2, y2),
			numPoints,
			numRepeats); 

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}

	// Vertical lines.
	for (; lineIdx < numLines; lineIdx++)
	{
		x1 = PATTERN_SCAN_CENTER_X + xInterval * ((lineIdx - halfSize) - halfSize / 2);
		x2 = PATTERN_SCAN_CENTER_X + xInterval * ((lineIdx - halfSize) - halfSize / 2);
		y1 = yStart;
		y2 = yClose;

		x1 += getScanOffsetX(true);
		x2 += getScanOffsetX(true);
		y1 += getScanOffsetY(true);
		y2 += getScanOffsetY(true);

		LineTrace line;
		line.initLine(TRACE_ID_MEASURE_CROSS + lineIdx,
			OctPoint(x1, y1),
			OctPoint(x2, y2),
			numPoints,
			numRepeats);

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}
	return;
}


void OctPattern::MeasureScan::buildPatternRadial(void)
{
	float radiusX, radiusY;

	radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	int numPoints = getNumberOfScanPoints();
	int numRepeats = getNumberOfScanOverlaps();
	int numLines = getNumberOfScanLines();

	double angle = 180.0 / numLines;
	float x1, y1, x2, y2;

	for (int lineIdx = 0; lineIdx < numLines; lineIdx++)
	{
		double degree = angle * lineIdx;
		double radian = NumericFunc::degreeToRadian(degree);
		x1 = (float)(radiusX * cos(radian) * -1.0);
		y1 = (float)(radiusY * sin(radian) * -1.0);
		x2 = (float)(radiusX * cos(radian) * +1.0);
		y2 = (float)(radiusY * sin(radian) * +1.0);

		x1 += getScanOffsetX(true);
		x2 += getScanOffsetX(true);
		y1 += getScanOffsetY(true);
		y2 += getScanOffsetY(true);

		LineTrace line;
		line.initLine(TRACE_ID_MEASURE_RADIAL + lineIdx,
			OctPoint(x1, y1),
			OctPoint(x2, y2),
			numPoints,
			numRepeats);

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}
	return;
}


void OctPattern::MeasureScan::buildPatternRaster(bool isVert)
{
	float radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	float radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	// For Angio point profiles.
	// radiusX = radiusY = 0.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	xStart += getScanOffsetX(true);
	xClose += getScanOffsetX(true);
	yStart += getScanOffsetY(true);
	yClose += getScanOffsetY(true);

	int numPoints = getNumberOfScanPoints();
	int numLines = getNumberOfScanLines();
	int numRepeats = getNumberOfScanOverlaps();

	float yInterval = (yClose - yStart) / (numLines - 1);
	float xInterval = (xClose - xStart) / (numLines - 1);

	float x1, y1, x2, y2;
	int traceId;

	int frameIdx = 0;
	int numFrames = numLines;
	resizePatternFrames(numFrames);

	for (int lineIdx = 0; lineIdx < numLines; lineIdx++) {
		if (isVert) {
			x1 = xStart + xInterval*lineIdx;
			y1 = yStart;
			x2 = xStart + xInterval*lineIdx;
			y2 = yClose;
		}
		else {
			x1 = xStart;
			y1 = yStart + yInterval*lineIdx;
			x2 = xClose;
			y2 = yStart + yInterval*lineIdx;
		}

		if (lineIdx != 0) {
			frameIdx++;
		}

		traceId = (TRACE_ID_MEASURE_RASTER + lineIdx) % TRACE_ID_MAX_LINES;

		LineTrace line;
		if (isVert) {
			line.initVertRaster(TRACE_ID_MEASURE_CUBE + frameIdx * 1, // + frameIdx,
				OctPoint(x1, y1),
				OctPoint(x2, y2),
				numPoints,
				numRepeats);
		}
		else {
			line.initHorzRaster(TRACE_ID_MEASURE_CUBE + frameIdx * 1, // + frameIdx,
				OctPoint(x1, y1),
				OctPoint(x2, y2),
				numPoints,
				numRepeats);
		}

		PatternFrame* pFrame = getPatternFrame(frameIdx);
		pFrame->addLineTrace(line);
	}
	return;
}


void OctPattern::MeasureScan::buildPatternCube(bool isVert)
{
	float radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	float radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	xStart += getScanOffsetX(true);
	xClose += getScanOffsetX(true);
	yStart += getScanOffsetY(true);
	yClose += getScanOffsetY(true);

	int numPoints = getNumberOfScanPoints();
	int numLines = getNumberOfScanLines();
	int numRepeats = getNumberOfScanOverlaps();

	int numFrames = getNumberOfFramesFromPoints(numPoints, numLines, numRepeats, false);
	int maxFrames = (useFastRasters() ? (PATTERN_FRAMES_MEASURE_MAX / 2) : PATTERN_FRAMES_MEASURE_MAX);
	if (numFrames <= 0 || numFrames > maxFrames) {
		// return;
	}
	int frameSize = getFrameCapacityFromPoints(numPoints, numLines, numRepeats, false);

	LogD() << "Pattern Cube, numFrames: " << numFrames << ", frameSize: " << frameSize << ", numLines: " << numLines;

	float yInterval = (yClose - yStart) / (numLines - 1);
	float xInterval = (xClose - xStart) / (numLines - 1);

	float x1, y1, x2, y2;
	int frameIdx = 0;

	resizePatternFrames(numFrames);

	for (int lineIdx = 0; lineIdx < numLines; lineIdx++) {
		if (isVert) {
			x1 = xStart + xInterval*lineIdx;
			y1 = yStart;
			x2 = xStart + xInterval*lineIdx;
			y2 = yClose;
		}
		else {
			x1 = xStart;
			y1 = yStart + yInterval*lineIdx;
			x2 = xClose;
			y2 = yStart + yInterval*lineIdx;
		}

		if (lineIdx != 0 && (lineIdx % frameSize) == 0) {
			frameIdx++;
		}

		int traceId = 0;
		if (useFastRasters()) {
			traceId = TRACE_ID_MEASURE_CUBE + frameIdx * 2;
		}
		else {
			traceId = TRACE_ID_MEASURE_CUBE + frameIdx * 1;
		}

		if (traceId >= TRACE_ID_MAX_LINES) {
			traceId = traceId % TRACE_ID_MAX_LINES;
		}

		// Increase trace id corresponding to frame as galvano profile index.
		LineTrace line;
		if (isVert) {
			if (useFastRasters()) {
				line.initVertRasterFast(traceId, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints, 
					numRepeats);
			}
			else {
				line.initVertRaster(traceId, // + frameIdx,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints,
					numRepeats);
			}
		}
		else {
			if (useFastRasters()) {
				line.initHorzRasterFast(traceId, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints, 
					numRepeats);
			}
			else {
				line.initHorzRaster(traceId, // + frameIdx,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints, 
					numRepeats);
			}
		}

		PatternFrame* pFrame = getPatternFrame(frameIdx);
		pFrame->addLineTrace(line);
	}
	return;
}


MeasureScan::MeasureScanImpl & OctPattern::MeasureScan::getImpl(void) const
{
	return *d_ptr;
}


