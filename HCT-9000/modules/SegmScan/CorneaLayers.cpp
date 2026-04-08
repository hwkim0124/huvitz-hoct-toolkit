#include "stdafx.h"
#include "CorneaLayers.h"

#include "CppUtil2.h"

using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct CorneaLayers::CorneaLayersImpl
{
	CorneaLayersImpl()
	{
	}
};


CorneaLayers::CorneaLayers() :
	d_ptr(make_unique<CorneaLayersImpl>())
{
}


SegmScan::CorneaLayers::~CorneaLayers() = default;
SegmScan::CorneaLayers::CorneaLayers(CorneaLayers && rhs) = default;
CorneaLayers & SegmScan::CorneaLayers::operator=(CorneaLayers && rhs) = default;


SegmScan::CorneaLayers::CorneaLayers(const CorneaLayers & rhs)
: d_ptr(make_unique<CorneaLayersImpl>(*rhs.d_ptr))
{
}


CorneaLayers & SegmScan::CorneaLayers::operator=(const CorneaLayers & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::CorneaLayers::initialize(unsigned int width, unsigned int height)
{
	OcularLayers::initializeAsCornea(width, height);
	return;
}


std::vector<int> SegmScan::CorneaLayers::getLayerPoints(OcularLayerType layer, int width, int height) const
{
	auto type = static_cast<unsigned short>(layer);
	return OcularLayers::getLayerPoints(type, width, height);
}


void SegmScan::CorneaLayers::setLayerPoints(OcularLayerType layer, const std::vector<int>& points)
{
	auto type = static_cast<unsigned short>(layer);
	OcularLayers::setLayerPoints(type, points);
	return;
}


bool SegmScan::CorneaLayers::isLayerPoints(OcularLayerType layer) const
{
	auto type = static_cast<unsigned short>(layer);
	auto data = OcularLayers::getLayerPoints(type);
	bool none = std::all_of(data.cbegin(), data.cend(), [](int v) { return v < 0; });
	return !none;
	// return (data.size() > 0);
}


OcularLayer * SegmScan::CorneaLayers::getEPI(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::EPI);
	return getLayer(type);
}


OcularLayer * SegmScan::CorneaLayers::getBOW(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::BOW);
	return getLayer(type);
}


OcularLayer * SegmScan::CorneaLayers::getEND(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::END);
	return getLayer(type);
}


OcularLayer * SegmScan::CorneaLayers::getInner(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::INNER);
	return getLayer(type);
}


OcularLayer * SegmScan::CorneaLayers::getOuter(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::OUTER);
	return getLayer(type);
}


CorneaLayers::CorneaLayersImpl & SegmScan::CorneaLayers::getImpl(void) const
{
	return *d_ptr;
}
