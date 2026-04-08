#include "stdafx.h"
#include "PreviewScan.h"
#include "PatternFrame.h"
#include "LineTrace.h"


#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct PreviewScan::PreviewScanImpl
{
	PreviewScanImpl() {
	}
};


PreviewScan::PreviewScan() :
	d_ptr(make_unique<PreviewScanImpl>())
{
}


OctPattern::PreviewScan::~PreviewScan() = default;
OctPattern::PreviewScan::PreviewScan(PreviewScan && rhs) = default;
PreviewScan & OctPattern::PreviewScan::operator=(PreviewScan && rhs) = default;


OctPattern::PreviewScan::PreviewScan(const PreviewScan & rhs)
	: d_ptr(make_unique<PreviewScanImpl>(*rhs.d_ptr))
{
}


PreviewScan & OctPattern::PreviewScan::operator=(const PreviewScan & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



bool OctPattern::PreviewScan::buildPattern(bool hidden)
{
	clearPatternFrames();

	if (usePreviewPattern())
	{
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
		
		if (hidden) {
			buildPreviewHidden();
		}
	}
	else
	{
		// Build line traces for preview and measure scan pattern.
		switch (getPatternType()) {
		case PatternType::Point:
			buildPreviewPoint();
			break;
		case PatternType::HorzLine:
			buildPreviewLine(false);
			break;
		case PatternType::VertLine:
			buildPreviewLine(true);
			break;
		case PatternType::Cross:
			buildPreviewCross();
			break;
		default:
			buildPreviewLine(false);
			break;
		}

		if (hidden) {
			buildPreviewHidden();
		}
	}
	return true;
}


void OctPattern::PreviewScan::buildPatternPoint(void)
{
	buildPreviewPoint();
	return;
}


void OctPattern::PreviewScan::buildPatternLine(bool isVert)
{
	buildPreviewLine(isVert);
	return;
}


void OctPattern::PreviewScan::buildPatternCircle(void)
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

	LineTrace line;
	line.initCircle(TRACE_ID_MEASURE_CIRCLE,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::PreviewScan::buildPatternCross(void)
{
	float radiusX, radiusY;

	radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	//radiusX *= -1.0f;

	float xStart = PATTERN_SCAN_CENTER_X - radiusX;
	float xClose = PATTERN_SCAN_CENTER_X + radiusX;
	float yStart = PATTERN_SCAN_CENTER_Y - radiusY;
	float yClose = PATTERN_SCAN_CENTER_Y + radiusY;

	float xInterval = getScanLineSpace() * getScanScaleX();
	float yInterval = getScanLineSpace() * getScanScaleY();

	int numPoints = getNumberOfScanPoints();
	int numLines = getNumberOfScanLines();
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
		y1 = PATTERN_SCAN_CENTER_Y + yInterval * (lineIdx - halfSize / 2);
		y2 = PATTERN_SCAN_CENTER_Y + yInterval * (lineIdx - halfSize / 2);

		x1 += getScanOffsetX(true);
		x2 += getScanOffsetX(true);
		y1 += getScanOffsetY(true);
		y2 += getScanOffsetY(true);

		LineTrace line;
		line.initLine(TRACE_ID_MEASURE_CROSS + lineIdx,
			OctPoint(x1, y1),
			OctPoint(x2, y2),
			numPoints);

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
			numPoints);

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}
	return;
}


void OctPattern::PreviewScan::buildPatternRadial(void)
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
			numPoints);

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}
	return;
}


void OctPattern::PreviewScan::buildPatternRaster(bool isVert)
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

	float yInterval = (yClose - yStart) / (numLines - 1);
	float xInterval = (xClose - xStart) / (numLines - 1);

	float x1, y1, x2, y2;
	int traceId;

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

		traceId = (TRACE_ID_MEASURE_RASTER + lineIdx) % TRACE_ID_MAX_LINES;

		LineTrace line;
		line.initLine(traceId,
			OctPoint(x1, y1),
			OctPoint(x2, y2),
			numPoints);

		PatternFrame frame;
		frame.addLineTrace(line);
		addPatternFrame(frame);
	}
	return;
}


void OctPattern::PreviewScan::buildPatternCube(bool isVert)
{
	float radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
	float radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();

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

	int numFrames = getNumberOfFramesFromPoints(numPoints, numLines, numRepeats);
	int maxFrames = (useFastRasters() ? TRACE_ID_MAX_REPEATS : TRACE_ID_MAX_LINES);
	if (numFrames <= 0 || numFrames > maxFrames) {
		return;
	}
	int frameSize = getFrameCapacityFromPoints(numPoints, numLines, numRepeats);

	float yInterval = (yClose - yStart) / (numLines - 1);
	float xInterval = (xClose - xStart) / (numLines - 1);

	float x1, y1, x2, y2;
	double angle = getScanAngle();
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

		if (std::fabs(angle) > 0.0001) {
			double radian = NumericFunc::degreeToRadian(angle);
			double radius = (isVert ? radiusY : radiusX);
			xStart = (float)(x1 * cos(radian) * -1.0);
			yStart = (float)(y1 * sin(radian) * -1.0);
			xClose = (float)(x2 * cos(radian) * +1.0);
			yClose = (float)(y2 * sin(radian) * +1.0);
		}

		if (lineIdx != 0 && (lineIdx % frameSize) == 0) {
			frameIdx++;
		}

		// Increase trace id corresponding to frame as galvano profile index.
		LineTrace line;
		if (isVert) {
			if (useFastRasters()) {
				line.initVertRasterFast(TRACE_ID_PREVIEW_CUBE, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
			else {
				line.initVertRaster(TRACE_ID_PREVIEW_CUBE, // + frameIdx,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
		}
		else {
			if (useFastRasters()) {
				line.initHorzRasterFast(TRACE_ID_PREVIEW_CUBE, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
			else {
				line.initHorzRaster(TRACE_ID_PREVIEW_CUBE, // + frameIdx,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
		}

		PatternFrame* pFrame = getPatternFrame(frameIdx);
		pFrame->addLineTrace(line);
	}
	return;
}


void OctPattern::PreviewScan::buildPreviewPoint(void)
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

	LineTrace line;
	line.initLine(TRACE_ID_PREVIEW_POINT,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints); // PREVIEW_LINE_NUM_POINTS);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::PreviewScan::buildPreviewHidden(void)
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

	LineTrace line;
	line.initLine(TRACE_ID_PREVIEW_HIDDEN,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints); // PREVIEW_LINE_NUM_POINTS);

	line.setHidden(true);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);
	return;
}


void OctPattern::PreviewScan::buildPreviewLine(bool isVert, int lineCount)
{
	float radiusX, radiusY;

	if (isVert) {
		radiusX = 0.0f ;
		radiusY = (getScanRangeY() / 2.0f) * getScanScaleY();
	}
	else {
		radiusX = (getScanRangeX() / 2.0f) * getScanScaleX();
		radiusY = 0.0f ;
	}

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

	int numPoints = getNumberOfScanPoints(); // PATTERN_PREVIEW_LINE_NUM_POINTS; // getNumberOfScanPoints();
	setNumberOfScanPoints(numPoints);

	LineTrace line;
	line.initLine(TRACE_ID_PREVIEW_LINE + lineCount,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints); // PREVIEW_LINE_NUM_POINTS);

	PatternFrame frame;
	frame.addLineTrace(line);
	addPatternFrame(frame);

	// Scan speed for preview is fixed as the fastest. 
	const int NUM_LINES_HD = 5; 

	LineTrace line2;
	line2.initLine(TRACE_ID_PREVIEW_LINE_HD + lineCount,
		OctPoint(xStart, yStart),
		OctPoint(xClose, yClose),
		numPoints, 
		NUM_LINES_HD); // PREVIEW_LINE_NUM_POINTS);
	line2.setLineHD(true);

	PatternFrame frame2;
	frame2.addLineTrace(line2);
	addPatternFrame(frame2);
	return;
}


void OctPattern::PreviewScan::buildPreviewCircle(void)
{
	buildPreviewLine(false, 0);
	return;
}


void OctPattern::PreviewScan::buildPreviewCross(void)
{
	buildPreviewLine(false, 0);
	buildPreviewLine(true, 1);
	return;
}


void OctPattern::PreviewScan::buildPreviewRadial(void)
{
	buildPreviewCross();
	return;
}


void OctPattern::PreviewScan::buildPreviewRaster(bool isVert)
{
	buildPreviewLine(isVert);
	return;
}


void OctPattern::PreviewScan::buildPreviewCube(bool isVert)
{
	buildPreviewLine(isVert);
	return;
}


PreviewScan::PreviewScanImpl & OctPattern::PreviewScan::getImpl(void) const
{
	return *d_ptr;
}


