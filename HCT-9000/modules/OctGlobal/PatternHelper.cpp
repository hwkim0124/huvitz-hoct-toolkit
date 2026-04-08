#include "stdafx.h"
#include "PatternHelper.h"

#include <string>
#include <vector>

using namespace OctGlobal;
using namespace std;


struct PatternHelper::PatternHelperImpl
{
	std::vector<int> patternCodes;
	std::vector<string> patternNames;


	PatternHelperImpl()
	{

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<PatternHelper::PatternHelperImpl> PatternHelper::d_ptr(new PatternHelperImpl());



PatternHelper::PatternHelper()
{
	initializePatternHelper();
}


PatternHelper::~PatternHelper()
{
}


int OctGlobal::PatternHelper::getNumberOfPatternTypes(void)
{
	return (int)d_ptr->patternCodes.size();
}


int OctGlobal::PatternHelper::getPatternCodeFromIndex(int index)
{
	if (index < 0 || index >= getNumberOfPatternTypes()) {
		return -1;
	}
	return d_ptr->patternCodes[index];
}


int OctGlobal::PatternHelper::getPatternIndexFromCode(int code)
{
	for (int index = 0; index < d_ptr->patternCodes.size(); index++) {
		if (d_ptr->patternCodes[index] == code) {
			return index;
		}
	}
	return -1;
}


const char * OctGlobal::PatternHelper::getPatternNameString(PatternName name)
{
	int code = getPatternCodeFromName(name);
	return getPatternNameStringFromCode(code);
}


const char * OctGlobal::PatternHelper::getPatternNameStringFromCode(int code)
{
	int index = getPatternIndexFromCode(code);
	const char* pstr = getPatternNameStringFromIndex(index);
	return pstr;
}


const char * OctGlobal::PatternHelper::getPatternNameStringFromIndex(int index)
{
	if (index < 0 || index >= getNumberOfPatternTypes()) {
		return "";
	}
	return d_ptr->patternNames[index].c_str();
}


PatternDomain OctGlobal::PatternHelper::getPatternDomainFromCode(int code)
{
	switch (code) {
		case PATTERN_CALIBRATION_POINT_CODE:
			return PatternDomain::Mirror;
		case PATTERN_MACULAR_POINT_CODE:
		case PATTERN_MACULAR_LINE_CODE:
		case PATTERN_MACULAR_CROSS_CODE:
		case PATTERN_MACULAR_RADIAL_CODE:
		case PATTERN_MACULAR_RASTER_CODE:
		case PATTERN_MACULAR_CIRCLE_CODE:
		case PATTERN_MACULAR_3D_CODE:
		case PATTERN_MACULAR_ANGIO_CODE:
		case PATTERN_MACULAR_DISC_CODE:
		case PATTERN_ANTERIOR_AL_CODE:
			return PatternDomain::Macular;
		case PATTERN_DISC_POINT_CODE:
		case PATTERN_DISC_RADIAL_CODE:
		case PATTERN_DISC_RASTER_CODE:
		case PATTERN_DISC_3D_CODE:
		case PATTERN_DISC_CIRCLE_CODE:
		case PATTERN_DISC_ANGIO_CODE:
			return PatternDomain::OpticDisc;
		case PATTERN_ANTERIOR_POINT_CODE:
		case PATTERN_ANTERIOR_LINE_CODE:
		case PATTERN_ANTERIOR_RADIAL_CODE:
		case PATTERN_ANTERIOR_3D_CODE:
		case PATTERN_ANTERIOR_ANGIO_CODE:
		case PATTERN_TOPOGRAPHY_CODE:
		case PATTERN_ANTERIOR_WIDE_CODE:
		case PATTERN_ANTERIOR_LT_CODE:
			return PatternDomain::Cornea;
	}
	return PatternDomain::Unknown;
}


PatternName OctGlobal::PatternHelper::getPatternNameFromCode(int code)
{
	switch (code) {
	case PATTERN_CALIBRATION_POINT_CODE:
		return PatternName::CalibrationPoint;
	case PATTERN_MACULAR_POINT_CODE:
		return PatternName::MacularPoint;
	case PATTERN_MACULAR_LINE_CODE:
		return PatternName::MacularLine;
	case PATTERN_MACULAR_CROSS_CODE:
		return PatternName::MacularCross;
	case PATTERN_MACULAR_RADIAL_CODE:
		return PatternName::MacularRadial;
	case PATTERN_MACULAR_RASTER_CODE:
		return PatternName::MacularRaster;
	case PATTERN_MACULAR_CIRCLE_CODE:
		return PatternName::MacularCircle;
	case PATTERN_MACULAR_3D_CODE:
		return PatternName::Macular3D;
	case PATTERN_MACULAR_ANGIO_CODE:
		return PatternName::MacularAngio;
	case PATTERN_DISC_POINT_CODE:
		return PatternName::DiscPoint;
	case PATTERN_DISC_RADIAL_CODE:
		return PatternName::DiscRadial;
	case PATTERN_DISC_3D_CODE:
		return PatternName::Disc3D;
	case PATTERN_DISC_CIRCLE_CODE:
		return PatternName::DiscCircle;
	case PATTERN_DISC_RASTER_CODE:
		return PatternName::DiscRaster;
	case PATTERN_DISC_ANGIO_CODE:
		return PatternName::DiscAngio;
	case PATTERN_ANTERIOR_POINT_CODE:
		return PatternName::AnteriorPoint;
	case PATTERN_ANTERIOR_LINE_CODE:
		return PatternName::AnteriorLine;
	case PATTERN_ANTERIOR_RADIAL_CODE:
		return PatternName::AnteriorRadial;
	case PATTERN_ANTERIOR_3D_CODE:
		return PatternName::Anterior3D;
	case PATTERN_ANTERIOR_ANGIO_CODE:
		return PatternName::AnteriorAngio;
	case PATTERN_MACULAR_DISC_CODE:
		return PatternName::MacularDisc;
	case PATTERN_TOPOGRAPHY_CODE:
		return PatternName::Topography;
	case PATTERN_ANTERIOR_WIDE_CODE:
		return PatternName::AnteriorFull;
	case PATTERN_ANTERIOR_AL_CODE:
		return PatternName::AxialLength;
	case PATTERN_ANTERIOR_LT_CODE:
		return PatternName::LensThickness;
	}
	return PatternName::Unknown;
}


PatternType OctGlobal::PatternHelper::getPatternTypeFromCode(int code, int direction)
{
	switch (code) {
	case PATTERN_CALIBRATION_POINT_CODE:
	case PATTERN_MACULAR_POINT_CODE:
	case PATTERN_DISC_POINT_CODE:
	case PATTERN_ANTERIOR_POINT_CODE:
		return PatternType::Point;
	
	case PATTERN_MACULAR_LINE_CODE:
	case PATTERN_ANTERIOR_LINE_CODE:
		return (direction == PATTERN_DIRECTION_X_Y ? PatternType::HorzLine : PatternType::VertLine);
	case PATTERN_ANTERIOR_WIDE_CODE:
	case PATTERN_ANTERIOR_AL_CODE:
	case PATTERN_ANTERIOR_LT_CODE:
		return PatternType::HorzLine;
	case PATTERN_MACULAR_CROSS_CODE:
		return PatternType::Cross;
	case PATTERN_MACULAR_RADIAL_CODE:
	case PATTERN_DISC_RADIAL_CODE:
	case PATTERN_ANTERIOR_RADIAL_CODE:
	case PATTERN_TOPOGRAPHY_CODE:
		return PatternType::Radial;

	case PATTERN_MACULAR_RASTER_CODE:
	case PATTERN_DISC_RASTER_CODE:
		return (direction == PATTERN_DIRECTION_X_Y ? PatternType::HorzRaster : PatternType::VertRaster);
	
	case PATTERN_MACULAR_3D_CODE:
	case PATTERN_DISC_3D_CODE:
	case PATTERN_ANTERIOR_3D_CODE:
	case PATTERN_MACULAR_DISC_CODE:
	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:
		return (direction == PATTERN_DIRECTION_X_Y ? PatternType::HorzCube : PatternType::VertCube);
	
	case PATTERN_MACULAR_CIRCLE_CODE:
	case PATTERN_DISC_CIRCLE_CODE:
		return PatternType::Circle;
	}
	return PatternType::Unknown;
}


int OctGlobal::PatternHelper::getPatternCodeFromName(PatternName name)
{
	switch (name) {
	case PatternName::CalibrationPoint:
		return PATTERN_CALIBRATION_POINT_CODE;
	case PatternName::MacularPoint:
		return PATTERN_MACULAR_POINT_CODE;
	case PatternName::MacularLine:
		return PATTERN_MACULAR_LINE_CODE;
	case PatternName::MacularCross:
		return PATTERN_MACULAR_CROSS_CODE;
	case PatternName::MacularRadial:
		return PATTERN_MACULAR_RADIAL_CODE;
	case PatternName::MacularRaster:
		return PATTERN_MACULAR_RASTER_CODE;
	case PatternName::MacularCircle:
		return PATTERN_MACULAR_CIRCLE_CODE;
	case PatternName::Macular3D:
		return PATTERN_MACULAR_3D_CODE;
	case PatternName::MacularAngio:
		return PATTERN_MACULAR_ANGIO_CODE;

	case PatternName::DiscPoint:
		return PATTERN_DISC_POINT_CODE;
	case PatternName::DiscRadial:
		return PATTERN_DISC_RADIAL_CODE;
	case PatternName::DiscRaster:
		return PATTERN_DISC_RASTER_CODE;
	case PatternName::Disc3D:
		return PATTERN_DISC_3D_CODE;
	case PatternName::DiscCircle:
		return PATTERN_DISC_CIRCLE_CODE;
	case PatternName::DiscAngio:
		return PATTERN_DISC_ANGIO_CODE;

	case PatternName::AnteriorPoint:
		return PATTERN_ANTERIOR_POINT_CODE;
	case PatternName::AnteriorLine:
		return PATTERN_ANTERIOR_LINE_CODE;
	case PatternName::AnteriorRadial:
		return PATTERN_ANTERIOR_RADIAL_CODE;
	case PatternName::Anterior3D:
		return PATTERN_ANTERIOR_3D_CODE;
	case PatternName::AnteriorAngio:
		return PATTERN_ANTERIOR_ANGIO_CODE;

	case PatternName::MacularDisc:
		return PATTERN_MACULAR_DISC_CODE;

	case PatternName::Topography:
		return PATTERN_TOPOGRAPHY_CODE;
	case PatternName::AnteriorFull:
		return PATTERN_ANTERIOR_WIDE_CODE;
	case PatternName::AxialLength:
		return PATTERN_ANTERIOR_AL_CODE;
	case PatternName::LensThickness:
		return PATTERN_ANTERIOR_LT_CODE;
	}
	return -1;
}


std::vector<float> OctGlobal::PatternHelper::getPatternRangesFromCode(int code)
{
	std::vector<float> ranges;

	switch (code) {
	case PATTERN_MACULAR_CIRCLE_CODE:
	case PATTERN_DISC_CIRCLE_CODE:
		ranges.push_back(PATTERN_BSCAN_RANGE4);
		break;

	case PATTERN_MACULAR_RADIAL_CODE:
	case PATTERN_MACULAR_RASTER_CODE:
	case PATTERN_MACULAR_3D_CODE:
	case PATTERN_DISC_RADIAL_CODE:
	case PATTERN_DISC_RASTER_CODE:
	case PATTERN_DISC_3D_CODE:
	case PATTERN_ANTERIOR_LINE_CODE:
	case PATTERN_ANTERIOR_RADIAL_CODE:
	case PATTERN_ANTERIOR_3D_CODE:
	case PATTERN_MACULAR_DISC_CODE:
		ranges.push_back(PATTERN_BSCAN_RANGE1);
		ranges.push_back(PATTERN_BSCAN_RANGE2);
		break;

	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:

	case PATTERN_TOPOGRAPHY_CODE:
		ranges.push_back(PATTERN_BSCAN_RANGE5);
	default:
		ranges.push_back(PATTERN_BSCAN_RANGE1);
		ranges.push_back(PATTERN_BSCAN_RANGE2);
		ranges.push_back(PATTERN_BSCAN_RANGE3);
		break;
	}

	return ranges;
}


std::vector<int> OctGlobal::PatternHelper::getPatternPointsFromCode(int code)
{
	std::vector<int> points;

	switch (code) {
	case PATTERN_MACULAR_CROSS_CODE:
	case PATTERN_MACULAR_RADIAL_CODE:
	case PATTERN_MACULAR_RASTER_CODE:
	case PATTERN_DISC_RADIAL_CODE:
	case PATTERN_DISC_RASTER_CODE:
	case PATTERN_ANTERIOR_RADIAL_CODE:
		points.push_back(PATTERN_ASCAN_POINTS1);
		points.push_back(PATTERN_ASCAN_POINTS2);
		points.push_back(PATTERN_ASCAN_POINTS3);
		points.push_back(PATTERN_ASCAN_POINTS4);
		break;

	case PATTERN_MACULAR_3D_CODE:
	case PATTERN_DISC_3D_CODE:
	case PATTERN_ANTERIOR_3D_CODE:
	case PATTERN_MACULAR_DISC_CODE:
		points.push_back(PATTERN_ASCAN_POINTS1);
		points.push_back(PATTERN_ASCAN_POINTS2);
		points.push_back(PATTERN_ASCAN_POINTS3);
		break;

	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:
		// points.push_back(PATTERN_ASCAN_POINTS_ANGIO);
		points.push_back(PATTERN_ASCAN_POINTS1_ANGIO);
		points.push_back(PATTERN_ASCAN_POINTS2_ANGIO);
		points.push_back(PATTERN_ASCAN_POINTS3_ANGIO);
		points.push_back(PATTERN_ASCAN_POINTS4_ANGIO);
		points.push_back(PATTERN_ASCAN_POINTS5_ANGIO);
		break;

	case PATTERN_MACULAR_LINE_CODE:
	case PATTERN_DISC_CIRCLE_CODE:
	case PATTERN_ANTERIOR_LINE_CODE:

	case PATTERN_TOPOGRAPHY_CODE:
		points.push_back(PATTERN_ASCAN_POINTS1);
		points.push_back(PATTERN_ASCAN_POINTS2);
		break;
	default:
		points.push_back(PATTERN_ASCAN_POINTS1);
		break;
	}

	return points;
}


std::vector<int> OctGlobal::PatternHelper::getPatternOverlapsFromCode(int code, int points)
{
	std::vector<int> overlap;

	switch (code) {
	case PATTERN_MACULAR_LINE_CODE:
	case PATTERN_MACULAR_CIRCLE_CODE:
	case PATTERN_DISC_CIRCLE_CODE:
	case PATTERN_ANTERIOR_LINE_CODE:
		if (points <= PATTERN_ASCAN_POINTS2) {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS4);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS5);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS6);
		}
		else {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS2);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS3);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS4);
		}
		break;

	case PATTERN_MACULAR_CROSS_CODE:
	case PATTERN_MACULAR_RADIAL_CODE:
	case PATTERN_DISC_RADIAL_CODE:
	case PATTERN_ANTERIOR_RADIAL_CODE:
		if (points <= PATTERN_ASCAN_POINTS2) {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS2);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS3);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS4);
		}
		else {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS2);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS3);
		}
		break;

	case PATTERN_MACULAR_RASTER_CODE:
	case PATTERN_DISC_RASTER_CODE:
		if (points <= PATTERN_ASCAN_POINTS2) {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS2);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS3);
		}
		else {
			overlap.push_back(PATTERN_BSCAN_OVERLAPS1);
			overlap.push_back(PATTERN_BSCAN_OVERLAPS2);
		}
		break;

	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO1);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO2);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO3);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO4);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO5);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO6);
		overlap.push_back(PATTERN_BSCAN_OVERLAPS_ANGIO7);
		break;

	case PATTERN_MACULAR_3D_CODE:
	case PATTERN_DISC_3D_CODE:
	case PATTERN_ANTERIOR_3D_CODE:
	case PATTERN_MACULAR_DISC_CODE:
	default:
		overlap.push_back(PATTERN_CSCAN_OVERLAPS1);
		overlap.push_back(PATTERN_CSCAN_OVERLAPS2);
		overlap.push_back(PATTERN_CSCAN_OVERLAPS3);
		break;
	}

	return overlap;
}


std::vector<int> OctGlobal::PatternHelper::getPatternLinesFromCode(int code, int points)
{
	std::vector<int> lines;

	switch (code) {
	case PATTERN_MACULAR_CROSS_CODE:
		lines.push_back(PATTERN_CROSS_LINES);
		break;
	case PATTERN_MACULAR_RADIAL_CODE:
	case PATTERN_DISC_RADIAL_CODE:
	case PATTERN_ANTERIOR_RADIAL_CODE:
		lines.push_back(PATTERN_RADIAL_LINES1);
		lines.push_back(PATTERN_RADIAL_LINES2);
		lines.push_back(PATTERN_RADIAL_LINES3);
		break;
	case PATTERN_TOPOGRAPHY_CODE:
		lines.push_back(PATTERN_TOPOGRAPHY_LINES1);
		lines.push_back(PATTERN_TOPOGRAPHY_LINES2);
		lines.push_back(PATTERN_TOPOGRAPHY_LINES3);
		break;
	case PATTERN_MACULAR_RASTER_CODE:
	case PATTERN_DISC_RASTER_CODE:
		lines.push_back(PATTERN_RASTER_LINES1);
		lines.push_back(PATTERN_RASTER_LINES2);
		lines.push_back(PATTERN_RASTER_LINES3);
		lines.push_back(PATTERN_RASTER_LINES4);
		break;
	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES1);
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES2);
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES3);
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES4);
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES5);
		lines.push_back(PATTERN_BSCAN_ANGIO_LINES6);
		break;
	case PATTERN_MACULAR_3D_CODE:
	case PATTERN_DISC_3D_CODE:
	case PATTERN_ANTERIOR_3D_CODE:
	case PATTERN_MACULAR_DISC_CODE:
		if (points >= PATTERN_ASCAN_POINTS1) {
			lines.push_back(PATTERN_BSCAN_LINES4);
		}
		else if (points >= PATTERN_ASCAN_POINTS2) {
			lines.push_back(PATTERN_BSCAN_LINES2);
			lines.push_back(PATTERN_BSCAN_LINES3);
			lines.push_back(PATTERN_BSCAN_LINES4);
		}
		else {
			lines.push_back(PATTERN_BSCAN_LINES1);
			lines.push_back(PATTERN_BSCAN_LINES2);
			lines.push_back(PATTERN_BSCAN_LINES3);
		}
		break;
	case PATTERN_MACULAR_LINE_CODE:
	case PATTERN_ANTERIOR_LINE_CODE:
	case PATTERN_MACULAR_CIRCLE_CODE:
	case PATTERN_DISC_CIRCLE_CODE:
	default:
		lines.push_back(PATTERN_LINE_LINES);
		break;
	}
	return lines;
}


EyeRegion OctGlobal::PatternHelper::getEyeRegionFromDomain(PatternDomain domain)
{
	switch (domain) {
	case PatternDomain::Macular:
		return EyeRegion::Macular;
	case PatternDomain::Cornea:
		return EyeRegion::Cornea;
	case PatternDomain::OpticDisc:
		return EyeRegion::OpticDisc;
	default:
		return EyeRegion::Unknown;
	}
}


bool OctGlobal::PatternHelper::isAngioPatternCode(int code)
{
	switch (code) {
	case PATTERN_MACULAR_ANGIO_CODE:
	case PATTERN_DISC_ANGIO_CODE:
	case PATTERN_ANTERIOR_ANGIO_CODE:
		return true;
	}
	return false;
}


void OctGlobal::PatternHelper::initializePatternHelper(void)
{
	// pattern code
	d_ptr->patternCodes.clear();
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_LINE_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_CROSS_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_RADIAL_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_RASTER_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_3D_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_ANGIO_CODE);

	d_ptr->patternCodes.push_back(PATTERN_DISC_RADIAL_CODE);
	d_ptr->patternCodes.push_back(PATTERN_DISC_RASTER_CODE);
	d_ptr->patternCodes.push_back(PATTERN_DISC_3D_CODE);
	d_ptr->patternCodes.push_back(PATTERN_DISC_CIRCLE_CODE);
	d_ptr->patternCodes.push_back(PATTERN_DISC_ANGIO_CODE);

	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_LINE_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_RADIAL_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_3D_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_ANGIO_CODE);
	d_ptr->patternCodes.push_back(PATTERN_MACULAR_DISC_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_WIDE_CODE);
	d_ptr->patternCodes.push_back(PATTERN_TOPOGRAPHY_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_AL_CODE);
	d_ptr->patternCodes.push_back(PATTERN_ANTERIOR_LT_CODE);
	// pattern name
	d_ptr->patternNames.clear();
	d_ptr->patternNames.push_back(PATTERN_MACULAR_LINE_NAME);
	d_ptr->patternNames.push_back(PATTERN_MACULAR_CROSS_NAME);
	d_ptr->patternNames.push_back(PATTERN_MACULAR_RADIAL_NAME);
	d_ptr->patternNames.push_back(PATTERN_MACULAR_RASTER_NAME);
	d_ptr->patternNames.push_back(PATTERN_MACULAR_3D_NAME);
	d_ptr->patternNames.push_back(PATTERN_MACULAR_ANGIO_NAME);

	d_ptr->patternNames.push_back(PATTERN_DISC_RADIAL_NAME);
	d_ptr->patternNames.push_back(PATTERN_DISC_RASTER_NAME);
	d_ptr->patternNames.push_back(PATTERN_DISC_3D_NAME);
	d_ptr->patternNames.push_back(PATTERN_DISC_CIRCLE_NAME);
	d_ptr->patternNames.push_back(PATTERN_DISC_ANGIO_NAME);

	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_LINE_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_RADIAL_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_3D_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_ANGIO_NAME);

	d_ptr->patternNames.push_back(PATTERN_MACULAR_DISC_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_WIDE_NAME);
	d_ptr->patternNames.push_back(PATTERN_TOPOGRAPHY_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_AL_NAME);
	d_ptr->patternNames.push_back(PATTERN_ANTERIOR_LT_NAME);
	return;
}


PatternHelper::PatternHelperImpl & OctGlobal::PatternHelper::getImpl(void)
{
	return *d_ptr;
}
