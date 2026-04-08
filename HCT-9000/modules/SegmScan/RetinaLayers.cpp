#include "stdafx.h"
#include "RetinaLayers.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SegmScan;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;


struct RetinaLayers::RetinaLayersImpl
{
	RetinaLayersImpl()
	{
	}
};


RetinaLayers::RetinaLayers() :
	d_ptr(make_unique<RetinaLayersImpl>())
{
}


SegmScan::RetinaLayers::~RetinaLayers() = default;
SegmScan::RetinaLayers::RetinaLayers(RetinaLayers && rhs) = default;
RetinaLayers & SegmScan::RetinaLayers::operator=(RetinaLayers && rhs) = default;


SegmScan::RetinaLayers::RetinaLayers(const RetinaLayers & rhs)
	: d_ptr(make_unique<RetinaLayersImpl>(*rhs.d_ptr))
{
}


RetinaLayers & SegmScan::RetinaLayers::operator=(const RetinaLayers & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::RetinaLayers::initialize(unsigned int width, unsigned int height)
{
	OcularLayers::initializeAsRetina(width, height);
	return;
}


std::vector<int> SegmScan::RetinaLayers::getLayerPoints(OcularLayerType layer, int width, int height) const
{
	auto type = static_cast<unsigned short>(layer);
	return OcularLayers::getLayerPoints(type, width, height);
}


void SegmScan::RetinaLayers::setLayerPoints(OcularLayerType layer, const std::vector<int>& points)
{
	auto type = static_cast<unsigned short>(layer);
	OcularLayers::setLayerPoints(type, points);
	return;
}


bool SegmScan::RetinaLayers::isLayerPoints(OcularLayerType layer) const
{
	auto type = static_cast<unsigned short>(layer);
	auto data = OcularLayers::getLayerPoints(type);
	bool none = std::all_of(data.cbegin(), data.cend(), [](int v) { return v < 0; });
	return !none;
	// return (data.size() > 0);
}


OcularLayer * SegmScan::RetinaLayers::getILM(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::ILM);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getNFL(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::NFL);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getIPL(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::IPL);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getOPL(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::OPL);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getIOS(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::IOS);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getRPE(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::RPE);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getBRM(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::BRM);
	return getLayer(type);
}

OcularLayer * SegmScan::RetinaLayers::getBASE(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::BASE);
	return getLayer(type);
}

OcularLayer * SegmScan::RetinaLayers::getInner(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::INNER);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getOuter(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::OUTER);
	return getLayer(type);
}


OcularLayer * SegmScan::RetinaLayers::getOPR(void) const
{
	auto type = static_cast<unsigned short>(OcularLayerType::OPR);
	return getLayer(type);
}


RetinaLayers::RetinaLayersImpl & SegmScan::RetinaLayers::getImpl(void) const
{
	return *d_ptr;
}
