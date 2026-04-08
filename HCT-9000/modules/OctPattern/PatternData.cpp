#include "stdafx.h"
#include "PatternData.h"
#include "PreviewScan.h"
#include "MeasureScan.h"
#include "EnfaceScan.h"
#include "LineTrace.h"


#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct PatternData::PatternDataImpl
{
	PreviewScan preview;
	MeasureScan measure;
	EnfaceScan enface;
	bool initiated;
	bool useEnface;

	float dispOffsetX;
	float dispOffsetY;
	float dispAngle;
	float dispScaleX;
	float dispScaleY;

	int enfacePoints;
	int enfaceLines;
	float enfaceRangeX;
	float enfaceRangeY;

	PatternDataImpl() : initiated(false), useEnface(false),
					dispOffsetX(0.0f), dispOffsetY(0.0f), dispAngle(0.0f),
					dispScaleX(1.0f), dispScaleY(1.0f), 
					enfacePoints(PATTERN_ENFACE_NUM_ASCAN_POINTS),
					enfaceLines(PATTERN_ENFACE_NUM_BSCAN_LINES),
					enfaceRangeX(PATTERN_ENFACE_RANGE_X), 
					enfaceRangeY(PATTERN_ENFACE_RANGE_Y)
	{
	}
};


PatternData::PatternData() :
	d_ptr(make_unique<PatternDataImpl>())
{
}


OctPattern::PatternData::~PatternData() = default;
OctPattern::PatternData::PatternData(PatternData && rhs) = default;
PatternData & OctPattern::PatternData::operator=(PatternData && rhs) = default;


OctPattern::PatternData::PatternData(const PatternData & rhs)
	: d_ptr(make_unique<PatternDataImpl>(*rhs.d_ptr))
{
}


PatternData & OctPattern::PatternData::operator=(const PatternData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool OctPattern::PatternData::setupPattern(PatternDomain domain, PatternType type,
										int numPoints, int numLines, float rangeX, float rangeY, 
										int overlaps, float lineSpace, bool useEnface,
										bool usePattern, bool useFaster)
{
	float offsetX = d_ptr->dispOffsetX;
	float offsetY = d_ptr->dispOffsetY;
	float angle = d_ptr->dispAngle;
	float scaleX = d_ptr->dispScaleX;
	float scaleY = d_ptr->dispScaleY;

	getImpl().preview.initialize(domain, type, numPoints, numLines, 
									rangeX, rangeY, overlaps, lineSpace,
									offsetX, offsetY, angle, scaleX, scaleY);
	getImpl().preview.usePreviewPattern(true, usePattern);
	getImpl().preview.useFastRasters(true, useFaster);
	
	if (!getImpl().preview.buildPattern()) {
		return false;
	}

	getImpl().measure.initialize(domain, type, numPoints, numLines, rangeX, rangeY, overlaps, lineSpace,
									offsetX, offsetY, angle, scaleX, scaleY);
	getImpl().measure.useFastRasters(true, useFaster);

	/*
	if (!getImpl().measure.buildPattern()) {
		return false;
	}
	*/

	if (usePattern) {
		useEnface = false;
	}

	if (useEnface)
	{
		int enfPoints = d_ptr->enfacePoints;
		int enfLines = d_ptr->enfaceLines;
		float enfRangeX = d_ptr->enfaceRangeX;
		float enfRangeY = d_ptr->enfaceRangeY;

		getImpl().enface.initialize(PatternDomain::Macular, PatternType::HorzCube, 
									enfPoints, enfLines, enfRangeX, enfRangeY,
									overlaps, lineSpace,
									offsetX, offsetY, angle, scaleX, scaleY);
		getImpl().enface.useFastRasters(true, useFaster);

		if (!getImpl().enface.buildPattern()) {
			return false;
		}
	}
	else {
		getImpl().enface.clearPatternFrames();
	}

	getImpl().useEnface = useEnface;
	getImpl().initiated = true;
	return true;
}


void OctPattern::PatternData::setupEnface(int numPoints, int numLines, float rangeX, float rangeY)
{
	getImpl().enfacePoints = numPoints;
	getImpl().enfaceLines = numLines;
	getImpl().enfaceRangeX = rangeX;
	getImpl().enfaceRangeY = rangeY;
	return;
}


void OctPattern::PatternData::setupDisplacement(float offsetX, float offsetY, float angle, float scaleX, float scaleY)
{
	getImpl().dispOffsetX = offsetX;
	getImpl().dispOffsetY = offsetY;
	getImpl().dispAngle = angle;
	getImpl().dispScaleX = scaleX;
	getImpl().dispScaleY = scaleY;
	return;
}


int OctPattern::PatternData::getPreviewScanPoints(void) const
{
	return getImpl().preview.getNumberOfScanPoints();
}


int OctPattern::PatternData::getMeasureScanPoints(void) const
{
	return getImpl().measure.getNumberOfScanPoints();
}


int OctPattern::PatternData::getEnfaceScanPoints(void) const
{
	return getImpl().enface.getNumberOfScanPoints();
}


int OctPattern::PatternData::getPreviewScanLines(void) const
{
	return getImpl().preview.getNumberOfScanLines();
}


int OctPattern::PatternData::getMeasureScanLines(void) const
{
	return getImpl().measure.getNumberOfScanLines();
}


int OctPattern::PatternData::getEnfaceScanLines(void) const
{
	return getImpl().enface.getNumberOfScanLines();
}


bool OctPattern::PatternData::isInitiated(void) const
{
	return getImpl().initiated;
}


bool OctPattern::PatternData::isEnfaceScan(void) const
{
	return getImpl().useEnface;
}


void OctPattern::PatternData::clear(void)
{
	getImpl().initiated = false;
	return;
}


bool OctPattern::PatternData::isCornea(void)
{
	return getImpl().preview.isCornea();
}


PreviewScan & OctPattern::PatternData::getPreviewScan(void) const
{
	return getImpl().preview;
}


MeasureScan & OctPattern::PatternData::getMeasureScan(void) const
{
	return getImpl().measure;
}


EnfaceScan & OctPattern::PatternData::getEnfaceScan(void) const
{
	return getImpl().enface;
}


PatternData::PatternDataImpl & OctPattern::PatternData::getImpl(void) const
{
	return *d_ptr;
}
