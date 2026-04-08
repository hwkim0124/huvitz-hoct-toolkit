#include "stdafx.h"

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

#include "OctHttp.h"

#define __OCTHTTP_SETUP__
#include "OctHttpSetup.h"

using namespace octhttp;

void OctHttpSetup::setIpAddress(const std::wstring &ip)
{
    OctHttp::setIpAddress(ip);
}

void OctHttpSetup::setPort(const unsigned short p)
{
    OctHttp::setPort(p);
}

std::wstring OctHttpSetup::patternToString(MeasurementType pat)
{
    std::wstring scanPatternStr;
    switch (pat) {
    case MeasurementType::Macular_Line:
        scanPatternStr = std::wstring(L"Macular Line");
        break;
    case MeasurementType::Macular_Cross:
        scanPatternStr = std::wstring(L"Macular Cross");
        break;
    case MeasurementType::Macular_Radial:
        scanPatternStr = std::wstring(L"Macular Radial");
        break;
    case MeasurementType::Macular_3D:
        scanPatternStr = std::wstring(L"Macular 3D");
        break;
    case MeasurementType::Macular_Raster:
        scanPatternStr = std::wstring(L"Macular Raster");
        break;
    case MeasurementType::Disc_Radial:
        scanPatternStr = std::wstring(L"Disc Radial");
        break;
    case MeasurementType::Disc_Raster:
        scanPatternStr = std::wstring(L"Disc Raster");
        break;
    case MeasurementType::Disc_3D:
        scanPatternStr = std::wstring(L"Disc 3D");
        break;
    case MeasurementType::Disc_Circle:
        scanPatternStr = std::wstring(L"Disc Circle");
        break;
    case MeasurementType::Anterior_Line:
        scanPatternStr = std::wstring(L"Anterior Line");
        break;
    case MeasurementType::Anterior_Radial:
        scanPatternStr = std::wstring(L"Anterior Radial");
        break;
    case MeasurementType::Anterior_3D:
        scanPatternStr = std::wstring(L"Anterior 3D");
        break;
    case MeasurementType::Color_Fundus:
        scanPatternStr = std::wstring(L"Color Fundus");
        break;
	case MeasurementType::Macular_Disc:
		scanPatternStr = std::wstring(L"Macular Wide");
		break;
	case MeasurementType::Macular_Angio:
		scanPatternStr = std::wstring(L"Macular Angio");
		break;
	case MeasurementType::Disc_Angio:
		scanPatternStr = std::wstring(L"Disc Angio");
		break;
	case MeasurementType::Anterior_Full:
		scanPatternStr = std::wstring(L"Anterior Wide");
		break;
	case MeasurementType::Axial_Length:
		scanPatternStr = std::wstring(L"Axial Length");
		break;
	case MeasurementType::Lens_Thickness:
		scanPatternStr = std::wstring(L"Lens Thickness");
		break;
	case MeasurementType::Topography:
		scanPatternStr = std::wstring(L"Topography");
	}
    return scanPatternStr;
}

std::wstring OctHttpSetup::eyeSideToString(EyeSide es)
{
    std::wstring eyeSideStr;
    if (es == EyeSide::OS) {
        eyeSideStr = std::wstring(L"OS");
    }
    else if (es == EyeSide::OD) {
        eyeSideStr = std::wstring(L"OD");
    }
	else {
		eyeSideStr = std::wstring(L"UN");
	}

    return eyeSideStr;
}

std::wstring OctHttpSetup::genderToString(Gender g)
{
    std::wstring genderStr;
    if (g == Gender::Male) {
        genderStr = std::wstring(L"M");
    }
    else {
        genderStr = std::wstring(L"F");
    }

    return genderStr;
}

std::wstring OctHttpSetup::raceToString(Race r)
{
    std::wstring raceStr;

    switch (r) {
    case Race::Asian:
        raceStr = std::wstring(L"Asian");
        break;
    case Race::Caucasian:
        raceStr = std::wstring(L"Caucasian");
        break;
    case Race::Latino:
        raceStr = std::wstring(L"Latino");
        break;
    case Race::Blacks:
        raceStr = std::wstring(L"Blacks");
        break;
    case Race::Indian:
        raceStr = std::wstring(L"Indian");
        break;
    case Race::Arab:
        raceStr = std::wstring(L"Arab");
        break;
	case Race::Other:
		raceStr = std::wstring(L"Other");
		break;
	//case Race::Mixed:
	//	raceStr = std::wstring(L"Mixed");
	//	break;
    }

    return raceStr;
}

std::wstring OctHttpSetup::timeToString(ATL::COleDateTime t)
{
	CString temp;
	temp = t.Format(_T("%Y-%m-%d %H:%M:%S"));
    
	std::wstring stringTime = CT2CW(temp);

	return stringTime;
}