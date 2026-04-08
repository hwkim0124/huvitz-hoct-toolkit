#include "stdafx.h"
#include "PatternScan.h"
#include "PatternFrame.h"
#include "LineTrace.h"

#include "CppUtil2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct PatternScan::PatternScanImpl
{
	PatternDomain domain;
	PatternType type;
	LineTraceVect lines;
	PatternFrameVect frames;

	int numPoints;
	int numLines;
	int overlaps;
	float rangeX;
	float rangeY;
	float lineSpace;
	float angle;
	float offsetX;
	float offsetY;
	float scaleX;
	float scaleY;
	float posMoveX;
	float posMoveY;

	bool usePattern;
	bool useFaster;
	bool useForeDist;

	PatternScanImpl() : type(PatternType::HorzLine), domain(PatternDomain::Macular), 
						numPoints(1024), numLines(1), overlaps(1), 
						rangeX(6.0f), rangeY(6.0f), lineSpace(0.0f), angle(0.0f),
						offsetX(0.0f), offsetY(0.0f), scaleX(0.0f), scaleY(0.0f),
						posMoveX(0.0f), posMoveY(0.0f), 
						usePattern(false), useFaster(false), useForeDist(false)
	{
	}
};


PatternScan::PatternScan() :
	d_ptr(make_unique<PatternScanImpl>())
{
}


OctPattern::PatternScan::~PatternScan() = default;
OctPattern::PatternScan::PatternScan(PatternScan && rhs) = default;
PatternScan & OctPattern::PatternScan::operator=(PatternScan && rhs) = default;


OctPattern::PatternScan::PatternScan(const PatternScan & rhs)
	: d_ptr(make_unique<PatternScanImpl>(*rhs.d_ptr))
{
}


PatternScan & OctPattern::PatternScan::operator=(const PatternScan & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctPattern::PatternScan::initialize(PatternDomain domain, PatternType type, int numPoints, int numLines, 
										 float rangeX, float rangeY, int overlaps, float lineSpace, 
										 float offsetX, float offsetY, float angle, float scaleX, float scaleY, 
										 float posMoveX, float posMoveY)
{
	getImpl().type = type;
	getImpl().domain = domain;

	setScanRange(rangeX, rangeY);
	setScanOverlaps(overlaps);
	setScanLineSpace(lineSpace);
	setScanAngle(angle);
	setScanOffset(offsetX, offsetY);
	setScanScale(scaleX, scaleY);
	setNumberOfScanPoints(numPoints);
	setNumberOfScanLines(numLines);

	d_ptr->posMoveX = posMoveX;
	d_ptr->posMoveY = posMoveY;

	// initializeAsPattern();
	return;
}


bool OctPattern::PatternScan::buildPattern(bool hidden)
{
	clearPatternFrames();

	// Build line traces for preview and measure scan pattern.
	switch (getImpl().type) {
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
	case PatternType::HorzCube:
		buildPatternCube(false);
		break;
	case PatternType::VertRaster:
	case PatternType::VertCube:
		buildPatternCube(true);
		break;
	default:
		return false;
	}
	return true;
}


bool OctPattern::PatternScan::usePreviewPattern(bool isset, bool flag)
{
	if (isset) {
		getImpl().usePattern = flag;
	}
	return getImpl().usePattern;
}


bool OctPattern::PatternScan::useFastRasters(bool isset, bool flag)
{
	if (isset) {
		getImpl().useFaster = flag;
	}
	return getImpl().useFaster;
}


bool OctPattern::PatternScan::useForeDistance(bool isset, bool flag)
{
	if (isset) {
		getImpl().useForeDist = flag;
	}
	return getImpl().useForeDist;
}


void OctPattern::PatternScan::setScanRange(float rangeX, float rangeY)
{
	float value = rangeX;
	value = (value < PATTERN_SCAN_RANGE_X_MIN ? PATTERN_SCAN_RANGE_X_MIN : value);
	value = (value > PATTERN_SCAN_RANGE_X_MAX ? PATTERN_SCAN_RANGE_X_MAX : value);
	getImpl().rangeX = value;
	
	value = rangeY;
	value = (value < PATTERN_SCAN_RANGE_Y_MIN ? PATTERN_SCAN_RANGE_Y_MIN : value);
	value = (value > PATTERN_SCAN_RANGE_Y_MAX ? PATTERN_SCAN_RANGE_Y_MAX : value);
	getImpl().rangeY = value;
	return;
}


void OctPattern::PatternScan::setScanAngle(float angle)
{
	float value = angle;
	value = (value < PATTERN_SCAN_ANGLE_MIN ? PATTERN_SCAN_ANGLE_MIN : value);
	value = (value > PATTERN_SCAN_ANGLE_MAX ? PATTERN_SCAN_ANGLE_MAX : value);
	getImpl().angle = value;
	return;
}


void OctPattern::PatternScan::setScanOffset(float offsetX, float offsetY)
{
	getImpl().offsetX = offsetX;
	getImpl().offsetY = offsetY;
	return;
}


void OctPattern::PatternScan::setScanScale(float scaleX, float scaleY)
{
	getImpl().scaleX = scaleX;
	getImpl().scaleY = scaleY;
	return;
}


void OctPattern::PatternScan::setScanOverlaps(int overlaps)
{
	getImpl().overlaps = overlaps;
	return;
}


void OctPattern::PatternScan::setScanLineSpace(float space)
{
	getImpl().lineSpace = space;
	return;
}


void OctPattern::PatternScan::setNumberOfScanPoints(int numPoints)
{
	int value = numPoints;
	value = (value < PATTERN_SCAN_POINTS_MIN ? PATTERN_SCAN_POINTS_MIN : value);
	value = (value > PATTERN_SCAN_POINTS_MAX ? PATTERN_SCAN_POINTS_MAX : value);
	getImpl().numPoints = value;
	return;
}


void OctPattern::PatternScan::setNumberOfScanLines(int numLines)
{
	int value = numLines;
	value = (value < PATTERN_SCAN_LINES_MIN ? PATTERN_SCAN_LINES_MIN : value);
	value = (value > PATTERN_SCAN_LINES_MAX ? PATTERN_SCAN_LINES_MAX : value);
	getImpl().numLines = value;
	return;
}


float OctPattern::PatternScan::getScanRangeX(void) const
{
	return getImpl().rangeX;
}


float OctPattern::PatternScan::getScanRangeY(void) const
{
	return getImpl().rangeY;
}


int OctPattern::PatternScan::getNumberOfScanOverlaps(void) const
{
	return getImpl().overlaps;
}


float OctPattern::PatternScan::getScanLineSpace(void) const
{
	return getImpl().lineSpace;
}


float OctPattern::PatternScan::getScanAngle(void) const
{
	return getImpl().angle;
}


float OctPattern::PatternScan::getScanOffsetX(bool valid) const
{
	if (!valid) {
		return getImpl().offsetX;
	}
	else {
		float avail = (PATTERN_SCAN_RANGE_X_MAX - getScanRangeX()) / 2.0f;
		float offset = getImpl().offsetX;
		if (fabs(offset) > avail) {
			// offset = (offset > 0.0f ? avail : (avail*-1));
		}
		return offset;
	}
}


float OctPattern::PatternScan::getScanOffsetY(bool valid) const
{
	if (!valid) {
		return getImpl().offsetY;
	}
	else {
		float avail = (PATTERN_SCAN_RANGE_Y_MAX - getScanRangeY()) / 2.0f;
		float offset = getImpl().offsetY;
		if (fabs(offset) > avail) {
			// offset = (offset > 0.0f ? avail : (avail*-1));
		}
		return offset;
	}
}


float OctPattern::PatternScan::getScanScaleX(void) const
{
	return getImpl().scaleX;
}


float OctPattern::PatternScan::getScanScaleY(void) const
{
	return getImpl().scaleY;
}


float OctPattern::PatternScan::getScanMoveX(void) const
{
	return getImpl().posMoveX;
}


float OctPattern::PatternScan::getScanMoveY(void) const
{
	return getImpl().posMoveY;
}


int OctPattern::PatternScan::getNumberOfScanPoints(void) const
{
	return getImpl().numPoints;
}


int OctPattern::PatternScan::getNumberOfScanLines(void) const
{
	return getImpl().numLines;
}


int OctPattern::PatternScan::getFrameCapacityFromPoints(int numPoints, int numLines, int numOverlaps, bool enface) const
{
	int frameSize;
	if (enface)
	{
		if (numPoints >= PATTERN_ASCAN_POINTS1) {
			frameSize = 4; // 8;
		}
		else if (numPoints >= PATTERN_ASCAN_POINTS2) {
			frameSize = 8;  16; // 8;
		}
		else {
			frameSize = 8;  16; // 32;// 64; // 16;
		}
	}
	else
	{
		if (numPoints >= PATTERN_ASCAN_POINTS1) {
			frameSize = 4; // 8;
		}
		else if (numPoints >= PATTERN_ASCAN_POINTS2) {
			// frameSize = (numOverlaps > 1 ? 4 : 8); // 16; 24; //  32; // 8;
			frameSize = (numOverlaps > 1 ? 2 : 8); ;
		}
		else {
			// frameSize = (numOverlaps > 1 ? 8 : 16); // 16; 32; // 64; // 64; // 16;
			// frameSize = (numOverlaps > 1 ? 4 : 16); // 16; 32; // 64; // 64; // 16;
			if (numOverlaps == 1) {
				frameSize = 16;
			}
			else if (numOverlaps == 2) {
				frameSize = 8;
			}
			else { // if (numOverlaps >= 3) {
				frameSize = 4;
			}
		}
	}

	frameSize = (frameSize > numLines ? numLines : frameSize);
	return frameSize;
}


int OctPattern::PatternScan::getNumberOfFramesFromPoints(int numPoints, int numLines, int numOverlaps, bool enface) const
{
	int frameSize = getFrameCapacityFromPoints(numPoints, numLines, numOverlaps, enface);

	int numFrames = numLines / frameSize + (numLines % frameSize != 0 ? 1 : 0);
	return numFrames;
}


bool OctPattern::PatternScan::isVertical(void) const
{
	switch (getImpl().type) {
	case PatternType::VertLine:
	case PatternType::VertCube:
	case PatternType::VertRaster:
		return true;
	}
	return false;
}


bool OctPattern::PatternScan::isCornea(void) const
{
	if (getImpl().domain == PatternDomain::Cornea) {
		return true;
	}
	return false;
}


bool OctPattern::PatternScan::isScan3D(void) const
{
	switch (getPatternType()) {
	case PatternType::HorzRaster:
	case PatternType::VertRaster:
	case PatternType::HorzCube:
	case PatternType::VertCube:
		return true;
	}
	return false;
}


bool OctPattern::PatternScan::isRaster(void) const
{
	switch (getPatternType()) {
	case PatternType::HorzRaster:
	case PatternType::VertRaster:
		return true;
	}
	return false;
}


PatternType OctPattern::PatternScan::getPatternType(void) const
{
	return getImpl().type;
}


std::vector<PatternFrame> & OctPattern::PatternScan::getPatternFrames(void) const
{
	return getImpl().frames;
}


void OctPattern::PatternScan::clearPatternFrames(void)
{
	getImpl().frames.clear();
	return;
}


int OctPattern::PatternScan::resizePatternFrames(int size)
{
	clearPatternFrames();
	for (int i = 0; i < size; i++) {
		getImpl().frames.emplace_back();
	}
	return getNumberOfPatternFrames();
}


int OctPattern::PatternScan::getNumberOfPatternFrames(void) const
{
	return (int)getImpl().frames.size();
}


PatternFrame * OctPattern::PatternScan::getPatternFrame(int index) const
{
	if (index >= 0 && index < getNumberOfPatternFrames()) {
		return &getImpl().frames[index];
	}
	return nullptr;
}


void OctPattern::PatternScan::addPatternFrame(PatternFrame & frame)
{
	getImpl().frames.push_back(move(frame));
	return;
}


std::vector<int> OctPattern::PatternScan::getLateralSizeListOfFrame(int index, bool repeat) const
{
	PatternFrame* p = getPatternFrame(index);
	if (p != nullptr) {
		return p->getLateralSizeList(repeat);
	}
	return std::vector<int>();
}


LineTrace * OctPattern::PatternScan::getLineTraceFromImageIndex(int index, bool repeat, bool preview) const
{
	int rsize = index;
	int lsize = 0;
	for (const auto& frame : d_ptr->frames) {
		for (auto& line : frame.getLineTraceList()) {
			if (preview) {
				if (repeat && !line.isLineHD()) {
					continue;
				}
			}
			lsize = (repeat ? line.getNumberOfRepeats() : 1);
			rsize -= lsize;
			if (rsize < 0) {
				return &line;
			}
		}
	}
	return nullptr;
}


int OctPattern::PatternScan::getLineIndexFromImageIndex(int index, bool repeat, bool preview) const
{
	int rsize = index;
	int lsize = 0;
	int nline = 0;
	for (const auto& frame : d_ptr->frames) {
		for (auto& line : frame.getLineTraceList()) {
			if (preview) {
				if (repeat && !line.isLineHD()) {
					continue;
				}
			}
			lsize = (repeat ? line.getNumberOfRepeats() : 1);
			rsize -= lsize;
			if (rsize < 0) {
				return nline;
			}
			nline++;
		}
	}
	return -1;
}


int OctPattern::PatternScan::getOverlapIndexFromImageIndex(int index, bool repeat, bool preview) const
{
	if (!repeat) {
		return 0;
	}

	int rsize = index;
	int lsize = 0;
	int nline = 0;
	for (const auto& frame : d_ptr->frames) {
		for (auto& line : frame.getLineTraceList()) {
			if (preview) {
				if (!line.isLineHD()) {
					continue;
				}
			}
			lsize = line.getNumberOfRepeats();
			rsize -= lsize;
			if (rsize < 0) {
				return (rsize + lsize);
			}
			nline++;
		}
	}
	return 0;
}


void OctPattern::PatternScan::buildPatternPoint(void)
{
	return;
}


void OctPattern::PatternScan::buildPatternLine(bool isVert, int lineCount)
{
	return;
}


void OctPattern::PatternScan::buildPatternCircle(void)
{
	return;
}


void OctPattern::PatternScan::buildPatternCross(void)
{
	return;
}


void OctPattern::PatternScan::buildPatternRadial(void)
{
	return;
}


void OctPattern::PatternScan::buildPatternRaster(bool isVert)
{
	return;
}


void OctPattern::PatternScan::buildPatternCube(bool isVert)
{
	return;
}



PatternScan::PatternScanImpl & OctPattern::PatternScan::getImpl(void) const
{
	return *d_ptr;
}

