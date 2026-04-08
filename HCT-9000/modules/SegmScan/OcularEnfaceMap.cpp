#include "stdafx.h"
#include "OcularEnfaceMap.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct OcularEnfaceMap::OcularEnfaceMapImpl
{
	RetParam::EnfaceTmap enfaceTmap;

	OcularEnfaceMapImpl()
	{
	}
};


OcularEnfaceMap::OcularEnfaceMap() :
	d_ptr(make_unique<OcularEnfaceMapImpl>())
{
}


SegmScan::OcularEnfaceMap::~OcularEnfaceMap() = default;
SegmScan::OcularEnfaceMap::OcularEnfaceMap(OcularEnfaceMap && rhs) = default;
OcularEnfaceMap & SegmScan::OcularEnfaceMap::operator=(OcularEnfaceMap && rhs) = default;


SegmScan::OcularEnfaceMap::OcularEnfaceMap(const OcularEnfaceMap & rhs)
	: d_ptr(make_unique<OcularEnfaceMapImpl>(*rhs.d_ptr))
{
}


OcularEnfaceMap & SegmScan::OcularEnfaceMap::operator=(const OcularEnfaceMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool SegmScan::OcularEnfaceMap::setupThicknessMap(const OctScanPattern & pattern, OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, const std::vector<OcularBsegm*>& bsegms)
{
	auto lines = vector<vector<float>>();
	auto flags = vector<bool>();
	auto thick = vector<float>();
	bool result;

	for (const auto& b : bsegms) {
		if (b != nullptr) {
			result = b->getThicknessLine(upper, lower, thick, 0, 0, upperOffset, lowerOffset);
			lines.push_back(std::move(thick));
			flags.push_back(result);
		}
		else {
			return false;
		}
	}

	result = getImpl().enfaceTmap.setupData(pattern, upper, lower, upperOffset, lowerOffset, lines, flags);
	return result;
}


bool SegmScan::OcularEnfaceMap::isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const
{
	return getImpl().enfaceTmap.isIdentical(upper, lower, upperOffset, lowerOffset);
}


bool SegmScan::OcularEnfaceMap::isEmpty(void) const
{
	return getImpl().enfaceTmap.isEmpty();
}


CppUtil::CvImage SegmScan::OcularEnfaceMap::makeImage(int width, int height) const
{
	auto image = CppUtil::CvImage();
	getImpl().enfaceTmap.copyToImage(&image, width, height);
	return image;
}


RetParam::EnfaceTmap * SegmScan::OcularEnfaceMap::getThicknessMap(void) const
{
	return &getImpl().enfaceTmap;
}


OcularEnfaceMap::OcularEnfaceMapImpl & SegmScan::OcularEnfaceMap::getImpl(void) const
{
	return *d_ptr;
}
