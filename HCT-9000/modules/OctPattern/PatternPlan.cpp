#include "stdafx.h"
#include "PatternPlan.h"
#include "PreviewScan.h"
#include "MeasureScan.h"
#include "EnfaceScan.h"
#include "LineTrace.h"


#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctPattern;
using namespace CppUtil;
using namespace std;


struct PatternPlan::PatternPlanImpl
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

	PatternPlanImpl() : initiated(false), useEnface(false),
		dispOffsetX(0.0f), dispOffsetY(0.0f), dispAngle(0.0f),
		dispScaleX(1.0f), dispScaleY(1.0f),
		enfacePoints(PATTERN_ENFACE_ASCAN_POINTS),
		enfaceLines(PATTERN_ENFACE_BSCAN_LINES),
		enfaceRangeX(PATTERN_ENFACE_RANGE_X),
		enfaceRangeY(PATTERN_ENFACE_RANGE_Y)
	{
	}
};


PatternPlan::PatternPlan() :
	d_ptr(make_unique<PatternPlanImpl>())
{
}


OctPattern::PatternPlan::~PatternPlan() = default;
OctPattern::PatternPlan::PatternPlan(PatternPlan && rhs) = default;
PatternPlan & OctPattern::PatternPlan::operator=(PatternPlan && rhs) = default;


OctPattern::PatternPlan::PatternPlan(const PatternPlan & rhs)
	: d_ptr(make_unique<PatternPlanImpl>(*rhs.d_ptr))
{
}


PatternPlan & OctPattern::PatternPlan::operator=(const PatternPlan & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctPattern::PatternPlan::setupEnface(int numPoints, int numLines, float rangeX, float rangeY)
{
	getImpl().enfacePoints = numPoints;
	getImpl().enfaceLines = numLines;
	getImpl().enfaceRangeX = rangeX;
	getImpl().enfaceRangeY = rangeY;
	return;
}


void OctPattern::PatternPlan::setupDisplacement(float offsetX, float offsetY, float angle, float scaleX, float scaleY)
{
	getImpl().dispOffsetX = offsetX;
	getImpl().dispOffsetY = offsetY;
	getImpl().dispAngle = angle;
	getImpl().dispScaleX = scaleX;
	getImpl().dispScaleY = scaleY;
	return;
}



bool OctPattern::PatternPlan::buildMeasure(PatternDomain domain, PatternType type,
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

	if (!getImpl().measure.buildPattern()) {
		return false;
	}

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


bool OctPattern::PatternPlan::buildMeasure(OctScanMeasure & measure)
{
	bool useFaster = measure.useFastRaster();
	bool usePattern = measure.usePreviewPattern();
	bool useEnface = measure.usePhaseEnface();

	if (!buildPreviewPattern(measure)) {
		return false;
	}

	if (!buildMeasurePattern(measure)) {
		return false;
	}

	if (usePattern) {
		useEnface = false;
	}

	if (useEnface)
	{
		if (!buildEnfacePattern(measure)) {
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


bool OctPattern::PatternPlan::buildPreviewPattern(OctScanMeasure& measure)
{
	bool useFaster = measure.useFastRaster();
	bool usePattern = measure.usePreviewPattern();
	bool useEnface = measure.usePhaseEnface();

	OctScanPattern pattern = measure.getPattern();
	OctScanOffset offset = pattern.getScanOffset();

	float offsetX = (offset._mmAsUnit ? offset._offsetX : 0.0f);
	float offsetY = (offset._mmAsUnit ? offset._offsetY : 0.0f);
	float moveX = (offset._mmAsUnit ? 0.0f : offset._offsetX);
	float moveY = (offset._mmAsUnit ? 0.0f : offset._offsetY);
	float angle = offset._angle;
	float scaleX = offset._scaleX;
	float scaleY = offset._scaleY;

	updatePatternScale(measure.getPattern(), scaleX, scaleY);
	updatePatternOffset(measure.getPattern(), offsetX, offsetY);

	// DebugOut2() << "Pattern offset: " << offsetX << ", " << offsetY << " angle: " << angle << " scale: " << scaleX << ", " << scaleY;

	PatternDomain domain = pattern._domain;
	PatternType type = pattern._type;

	int numPoints = PATTERN_PREVIEW_LINE_NUM_POINTS; // pattern._numPoints;
	int numLines = pattern._numLines;
	float rangeX = pattern._rangeX;
	float rangeY = pattern._rangeY;
	int overlaps = pattern._overlaps;
	float lineSpace = pattern._lineSpace;

	if (!usePattern) {
		if (type != PatternType::Point) {
			type = pattern.getPreviewType();
			numLines = (type == PatternType::Cross ? 2 : 1);
		}
		if (measure.getPattern().isAngioScan()) {
			numPoints = pattern._numPoints;
		}
	}

	getImpl().preview.initialize(domain, type, numPoints, numLines,
		rangeX, rangeY, overlaps, lineSpace,
		offsetX, offsetY, angle, scaleX, scaleY, moveX, moveY);
	getImpl().preview.usePreviewPattern(true, usePattern);
	getImpl().preview.useFastRasters(true, useFaster);

	if (!getImpl().preview.buildPattern(true)) {
		LogD() << "Failed to build preview pattern";
		return false;
	}
	else {
		PreviewScan& scan = getPreviewScan();
		LogD() << "[ Preview Scan ]";
		LogD() << "X-Y range: " << scan.getScanRangeX() << ", " << scan.getScanRangeY();
		LogD() << "X-Y scale: " << scan.getScanScaleX() << ", " << scan.getScanScaleY();
		LogD() << "X-Y offset: " << scan.getScanOffsetX() << ", " << scan.getScanOffsetY() << " angle: " << scan.getScanAngle();
		LogD() << "Points/lines: " << scan.getNumberOfScanPoints() << ", " << scan.getNumberOfScanLines();
		LogD() << "Pattern type: " << static_cast<int>(type) << ", horiz.: " << measure.getPattern().isHorizontal();
		LogD() << "Line space: " << scan.getScanLineSpace() << " overlaps: " << scan.getNumberOfScanOverlaps();
		LogD() << "Pattern frames: " << scan.getNumberOfPatternFrames();
	}
	
	return true;
}


bool OctPattern::PatternPlan::buildMeasurePattern(OctScanMeasure& measure)
{
	bool useFaster = measure.useFastRaster();
	bool usePattern = measure.usePreviewPattern();
	bool useEnface = measure.usePhaseEnface();

	OctScanPattern pattern = measure.getPattern();
	OctScanOffset offset = pattern.getScanOffset();

	float offsetX = (offset._mmAsUnit ? offset._offsetX : 0.0f);
	float offsetY = (offset._mmAsUnit ? offset._offsetY : 0.0f);
	float moveX = (offset._mmAsUnit ? 0.0f : offset._offsetX);
	float moveY = (offset._mmAsUnit ? 0.0f : offset._offsetY);
	float angle = offset._angle;
	float scaleX = offset._scaleX;
	float scaleY = offset._scaleY;

	updatePatternScale(measure.getPattern(), scaleX, scaleY);
	updatePatternOffset(measure.getPattern(), offsetX, offsetY);

	PatternDomain domain = pattern._domain;
	PatternType type = pattern._type;

	int numPoints = pattern._numPoints;
	int numLines = pattern._numLines;
	float rangeX = pattern._rangeX;
	float rangeY = pattern._rangeY;
	int overlaps = pattern._overlaps;
	float lineSpace = pattern._lineSpace;

	getImpl().measure.initialize(domain, type, numPoints, numLines, rangeX, rangeY, overlaps, lineSpace,
		offsetX, offsetY, angle, scaleX, scaleY, moveX, moveY);
	getImpl().measure.useFastRasters(true, useFaster);

	if (pattern.isAngioScan()) {
		// getImpl().measure.useForeDistance(true, pattern.isAngioScan());
	}

	if (!getImpl().measure.buildPattern()) {
		LogD() << "Failed to build measure pattern";
		return false;
	}
	else {
		MeasureScan& scan = getMeasureScan();
		LogD() << "[ Measure Scan ]";
		LogD() << "X-Y range: " << scan.getScanRangeX() << ", " << scan.getScanRangeY();
		LogD() << "X-Y scale: " << scan.getScanScaleX() << ", " << scan.getScanScaleY();
		LogD() << "X-Y offset: " << scan.getScanOffsetX() << ", " << scan.getScanOffsetY() << " angle: " << scan.getScanAngle();
		LogD() << "Points/lines: " << scan.getNumberOfScanPoints() << ", " << scan.getNumberOfScanLines();
		LogD() << "Pattern type: " << static_cast<int>(type) << ", horiz.: " << measure.getPattern().isHorizontal();
		LogD() << "Line space: " << scan.getScanLineSpace() << " overlaps: " << scan.getNumberOfScanOverlaps();
		LogD() << "Pattern frames: " << scan.getNumberOfPatternFrames() << ", enface: " << useEnface << ", faster: " << useFaster;
		LogD() << "Angio scan: " << pattern.isAngioScan();
	}
	return true;
}


bool OctPattern::PatternPlan::buildEnfacePattern(OctScanMeasure& measure)
{
	bool useFaster = measure.useFastRaster();
	bool usePattern = measure.usePreviewPattern();
	bool useEnface = measure.usePhaseEnface();

	OctScanPattern enface = measure.getEnface();
	OctScanOffset offset = enface.getScanOffset();

	float offsetX = (offset._mmAsUnit ? offset._offsetX : 0.0f);
	float offsetY = (offset._mmAsUnit ? offset._offsetY : 0.0f);
	float moveX = (offset._mmAsUnit ? 0.0f : offset._offsetX);
	float moveY = (offset._mmAsUnit ? 0.0f : offset._offsetY);
	float angle = offset._angle;
	float scaleX = offset._scaleX;
	float scaleY = offset._scaleY;

	updatePatternScale(measure.getPattern(), scaleX, scaleY);
	updatePatternOffset(measure.getPattern(), offsetX, offsetY);

	PatternDomain domain = measure.getPattern().getPatternDomain();// enface._domain;
	PatternType type = enface.getPatternType();

	int enfPoints = enface._numPoints;
	int enfLines = enface._numLines;
	float enfRangeX = enface._rangeX;
	float enfRangeY = enface._rangeY;
	int overlaps = 1;
	float lineSpace = 0.0f;

	// auto type = (measure.getPattern().isPreviewVertLine() ? PatternType::VertCube : PatternType::HorzCube);
	auto enfType = PatternType::HorzCube;
	getImpl().enface.initialize(domain, enfType,
		enfPoints, enfLines, enfRangeX, enfRangeY,
		overlaps, lineSpace,
		offsetX, offsetY, angle, scaleX, scaleY, moveX, moveY);
	getImpl().enface.useFastRasters(true, useFaster);

	if (!getImpl().enface.buildPattern()) {
		LogD() << "Failed to build enface pattern";
		return false;
	}
	else {
		EnfaceScan& scan = getEnfaceScan();
		LogD() << "[ Enface Scan ]";
		LogD() << "X-Y range: " << scan.getScanRangeX() << ", " << scan.getScanRangeY();
		LogD() << "X-Y scale: " << scan.getScanScaleX() << ", " << scan.getScanScaleY();
		LogD() << "X-Y offset: " << scan.getScanOffsetX() << ", " << scan.getScanOffsetY() << " angle: " << scan.getScanAngle();
		LogD() << "Pattern type: " << static_cast<int>(type) << ", horiz.: " << measure.getPattern().isHorizontal();
		LogD() << "Points/lines: " << scan.getNumberOfScanPoints() << ", " << scan.getNumberOfScanLines();
		LogD() << "Line space: " << scan.getScanLineSpace() << " overlaps: " << scan.getNumberOfScanOverlaps();
		LogD() << "Pattern frames: " << scan.getNumberOfPatternFrames();
	}

	return true;
}


void OctPattern::PatternPlan::updatePatternScale(const OctScanPattern & pattern, float & scaleX, float & scaleY)
{
	int index = 0;

	if (scaleX != 1.0f || scaleY != 1.0f) {
		return;
	}

	if (pattern.isCorneaScan())
	{
		if (pattern.isNormalSpeed()) {
			index = 0;
		}
		else if (pattern.isFasterSpeed()) {
			index = 1;
		}
		else {
			index = 2;
		}
		if (pattern.getPatternName() == PatternName::Topography) {
			scaleX = GlobalSettings::topographyPatternScale(index).first;
			scaleY = GlobalSettings::topographyPatternScale(index).second;
		}
		else {
			if (GlobalSettings::useTopoCalibration(false)) {
				scaleX = GlobalSettings::topographyPatternScale(index).first;
				scaleY = GlobalSettings::topographyPatternScale(index).second;
			}
			else {
				scaleX = GlobalSettings::corneaPatternScale(index).first;
				scaleY = GlobalSettings::corneaPatternScale(index).second;
			}
		}
		
	}
	else 
	{
		if (pattern.isNormalSpeed()) {
			index = 0;
		}
		else if (pattern.isFasterSpeed()) {
			index = 1;
		}
		else {
			index = 2;
		}
		scaleX = GlobalSettings::retinaPatternScale(index).first;
		scaleY = GlobalSettings::retinaPatternScale(index).second;
	}
	return;
}


void OctPattern::PatternPlan::updatePatternOffset(const OctScanPattern & pattern, float & offsetX, float & offsetY)
{
	int index = 0;

	if (offsetX != 0.0f || offsetY != 0.0f) {
		return;
	}

	if (pattern.isCorneaScan())
	{
		if (pattern.isNormalSpeed()) {
			index = 0;
		}
		else if (pattern.isFasterSpeed()) {
			index = 1;
		}
		else {
			index = 2;
		}

		if (pattern.getPatternName() == PatternName::Topography) {
			offsetX = GlobalSettings::topographyPatternOffset(index).first;
			offsetY = GlobalSettings::topographyPatternOffset(index).second;
		}
		else {
			if (GlobalSettings::useTopoCalibration(false)) {
				offsetX = GlobalSettings::topographyPatternOffset(index).first;
				offsetY = GlobalSettings::topographyPatternOffset(index).second;
			}
			else {
				offsetX = GlobalSettings::corneaPatternOffset(index).first;
				offsetY = GlobalSettings::corneaPatternOffset(index).second;
			}
		}
	}
	else
	{
		if (pattern.isNormalSpeed()) {
			index = 0;
		}
		else if (pattern.isFasterSpeed()) {
			index = 1;
		}
		else {
			index = 2;
		}
		offsetX = GlobalSettings::retinaPatternOffset(index).first;
		offsetY = GlobalSettings::retinaPatternOffset(index).second;
	}
	return;
}



int OctPattern::PatternPlan::getPreviewScanPoints(void) const
{
	return getImpl().preview.getNumberOfScanPoints();
}


int OctPattern::PatternPlan::getMeasureScanPoints(void) const
{
	return getImpl().measure.getNumberOfScanPoints();
}


int OctPattern::PatternPlan::getEnfaceScanPoints(void) const
{
	return getImpl().enface.getNumberOfScanPoints();
}


int OctPattern::PatternPlan::getPreviewScanLines(void) const
{
	return getImpl().preview.getNumberOfScanLines();
}


int OctPattern::PatternPlan::getMeasureScanLines(void) const
{
	return getImpl().measure.getNumberOfScanLines();
}


int OctPattern::PatternPlan::getEnfaceScanLines(void) const
{
	return getImpl().enface.getNumberOfScanLines();
}


bool OctPattern::PatternPlan::isInitiated(void) const
{
	return getImpl().initiated;
}


void OctPattern::PatternPlan::clear(void)
{
	getImpl().initiated = false;
	return;
}


bool OctPattern::PatternPlan::isCornea(void) const
{
	return getImpl().preview.isCornea();
}


bool OctPattern::PatternPlan::isEnfaceScan(void) const
{
	return getImpl().useEnface;
}


PreviewScan & OctPattern::PatternPlan::getPreviewScan(void) const
{
	return getImpl().preview;
}


MeasureScan & OctPattern::PatternPlan::getMeasureScan(void) const
{
	return getImpl().measure;
}


EnfaceScan & OctPattern::PatternPlan::getEnfaceScan(void) const
{
	return getImpl().enface;
}


PatternPlan::PatternPlanImpl & OctPattern::PatternPlan::getImpl(void) const
{
	return *d_ptr;
}
