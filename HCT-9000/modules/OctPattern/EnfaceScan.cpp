#include "stdafx.h"
#include "EnfaceScan.h"
#include "PatternFrame.h"
#include "LineTrace.h"

#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct EnfaceScan::EnfaceScanImpl
{
	EnfaceScanImpl() {

	}
};


EnfaceScan::EnfaceScan() :
	d_ptr(make_unique<EnfaceScanImpl>())
{
}


OctPattern::EnfaceScan::~EnfaceScan() = default;
OctPattern::EnfaceScan::EnfaceScan(EnfaceScan && rhs) = default;
EnfaceScan & OctPattern::EnfaceScan::operator=(EnfaceScan && rhs) = default;


OctPattern::EnfaceScan::EnfaceScan(const EnfaceScan & rhs)
	: d_ptr(make_unique<EnfaceScanImpl>(*rhs.d_ptr))
{
}


EnfaceScan & OctPattern::EnfaceScan::operator=(const EnfaceScan & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctPattern::EnfaceScan::buildPattern(void)
{
	clearPatternFrames();

	switch (getPatternType()) {
	case PatternType::VertRaster:
	case PatternType::VertCube:
		buildEnfaceCube(true);
		break;

	case PatternType::HorzRaster:
	case PatternType::HorzCube:
	default:
		buildEnfaceCube(false);
		break;
	}
	return true;
}


void OctPattern::EnfaceScan::buildEnfaceCube(bool isVert)
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

	int numFrames = getNumberOfFramesFromPoints(numPoints, numLines, true);
	int maxFrames = (useFastRasters() ? (PATTERN_FRAMES_ENFACE_MAX/2) : PATTERN_FRAMES_ENFACE_MAX);
	if (numFrames <= 0 || numFrames > maxFrames) {
		return;
	}
	int frameSize = getFrameCapacityFromPoints(numPoints, numLines, true);

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

		// Increase trace id corresponding to frame as galvano profile index.
		LineTrace line;
		if (isVert) {
			if (useFastRasters()) {
				line.initVertRasterFast(TRACE_ID_ENFACE_CUBE + frameIdx * 2, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
			else {
				line.initVertRaster(TRACE_ID_ENFACE_CUBE + frameIdx, // + frameIdx,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
		}
		else {
			if (useFastRasters()) {
				line.initHorzRasterFast(TRACE_ID_ENFACE_CUBE + frameIdx * 2, // + frameIdx * 2,
					OctPoint(x1, y1),
					OctPoint(x2, y2),
					numPoints);
			}
			else {
				line.initHorzRaster(TRACE_ID_ENFACE_CUBE + frameIdx, // + frameIdx,
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


EnfaceScan::EnfaceScanImpl & OctPattern::EnfaceScan::getImpl(void) const
{
	return *d_ptr;
}


