#include "stdafx.h"
#include "CorneaEnfaceMap.h"
#include "CorneaBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaEnfaceMap::CorneaEnfaceMapImpl
{
	float radiusCentHorz;
	float radiusCentVert;
	float averageCenter;

	CorneaEnfaceMapImpl() : radiusCentHorz(0.0f), radiusCentVert(0.0f), averageCenter(0.0f)
	{
	}
};


CorneaEnfaceMap::CorneaEnfaceMap() :
	d_ptr(make_unique<CorneaEnfaceMapImpl>())
{
}


SegmScan::CorneaEnfaceMap::~CorneaEnfaceMap() = default;
SegmScan::CorneaEnfaceMap::CorneaEnfaceMap(CorneaEnfaceMap && rhs) = default;
CorneaEnfaceMap & SegmScan::CorneaEnfaceMap::operator=(CorneaEnfaceMap && rhs) = default;


SegmScan::CorneaEnfaceMap::CorneaEnfaceMap(const CorneaEnfaceMap & rhs)
	: d_ptr(make_unique<CorneaEnfaceMapImpl>(*rhs.d_ptr))
{
}


CorneaEnfaceMap & SegmScan::CorneaEnfaceMap::operator=(const CorneaEnfaceMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::CorneaEnfaceMap::setupThicknessMap(const OctScanPattern & pattern, OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, const std::vector<OcularBsegm*>& bsegms)
{
	if (!OcularEnfaceMap::setupThicknessMap(pattern, upper, lower, upperOffset, lowerOffset, bsegms)) {
		return false;
	}

	getImpl().averageCenter = getThicknessMap()->getAverageOnCenter(3.0f);
	return true;
}


bool SegmScan::CorneaEnfaceMap::setupRadiusMap(const OctScanPattern & pattern, const std::vector<OcularBsegm*>& bsegms)
{
	auto lines = vector<vector<float>>();
	auto flags = vector<bool>();
	auto radius = vector<float>();
	bool result;

	for (const auto& b : bsegms) {
		if (b != nullptr) {
			result = ((CorneaBsegm*)b)->getCurvatureRadius(radius);
			lines.push_back(std::move(radius));
			flags.push_back(result);
		}
		else {
			return false;
		}
	}

	OcularLayerType upper = OcularLayerType::EPI;
	OcularLayerType lower = OcularLayerType::END;
	result = getThicknessMap()->setupData(pattern, upper, lower, 0.0f, 0.0f, lines, flags);
	if (result) {
		getImpl().radiusCentHorz = getThicknessMap()->getAverageOnHorzLine(3.0f);
		getImpl().radiusCentVert = getThicknessMap()->getAverageOnVertLine(3.0f);
	}
	return result;

}


float SegmScan::CorneaEnfaceMap::getCurvatureRadiusOnCenterHorz(void) const
{
	return getImpl().radiusCentHorz;
}


float SegmScan::CorneaEnfaceMap::getCurvatureRadiusOnCenterVert(void) const
{
	return getImpl().radiusCentVert;
}


float SegmScan::CorneaEnfaceMap::getAverageThicknessOnCenter(void) const
{
	return getImpl().averageCenter;
}


CorneaEnfaceMap::CorneaEnfaceMapImpl & SegmScan::CorneaEnfaceMap::getImpl(void) const
{
	return *d_ptr;
}
