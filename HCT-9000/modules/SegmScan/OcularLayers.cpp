#include "stdafx.h"
#include "OcularLayers.h"

#include "RetSegm2.h"
#include "CppUtil2.h"


using namespace SegmScan;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;


struct OcularLayers::OcularLayersImpl
{
	std::map<unsigned short, OcularLayer> layers;
	int width;
	int height;

	OcularLayersImpl() : width(0), height(0)
	{
	}
};


OcularLayers::OcularLayers() :
	d_ptr(make_unique<OcularLayersImpl>())
{
}


SegmScan::OcularLayers::~OcularLayers() = default;
SegmScan::OcularLayers::OcularLayers(OcularLayers && rhs) = default;
OcularLayers & SegmScan::OcularLayers::operator=(OcularLayers && rhs) = default;


SegmScan::OcularLayers::OcularLayers(const OcularLayers & rhs)
	: d_ptr(make_unique<OcularLayersImpl>(*rhs.d_ptr))
{
}


OcularLayers & SegmScan::OcularLayers::operator=(const OcularLayers & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::OcularLayers::initialize(unsigned int width, unsigned int height)
{
	return;
}


std::vector<int> SegmScan::OcularLayers::getLayerPoints(OcularLayerType layer, int width, int height) const
{
	return std::vector<int>(width, -1);
}

void SegmScan::OcularLayers::setLayerPoints(OcularLayerType layer, const std::vector<int>& points)
{
	return;
}


bool SegmScan::OcularLayers::isLayerPoints(OcularLayerType layer) const
{
	return false;
}


OcularLayer * SegmScan::OcularLayers::getILM(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getNFL(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getIPL(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getOPL(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getIOS(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getRPE(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getBRM(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getBASE(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getInner(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getOuter(void) const
{
	return nullptr;
}

OcularLayer * SegmScan::OcularLayers::getOPR(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getEPI(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getBOW(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getEND(void) const
{
	return nullptr;
}


OcularLayer * SegmScan::OcularLayers::getLayer(unsigned short type) const
{
	auto iter = d_ptr->layers.find(type);
	if (iter != end(d_ptr->layers)) {
		return &iter->second;
	}
	return nullptr;
}


std::vector<int> SegmScan::OcularLayers::getLayerPoints(unsigned short type, int width, int height) const
{
	if (d_ptr->width == 0 || d_ptr->height == 0) {
		return vector<int>();
	}

	auto layer = OcularLayers::getLayer(type);
	int reg_w = (width == 0 ? d_ptr->width : width);
	int reg_h = (height == 0 ? d_ptr->height : height);

	auto data = std::vector<int>(reg_w, -1);

	if (layer != nullptr) {
		if (reg_w == d_ptr->width && reg_h == d_ptr->height) {
			data = layer->getYs();
		}
		else {
			data = layer->getYsResized(reg_w, reg_h);
		}
	}

	return data;
}


void SegmScan::OcularLayers::setLayerPoints(unsigned short type, const std::vector<int>& points)
{
	auto layer = OcularLayers::getLayer(type);
	layer->initialize(points, getRegionWidth(), getRegionHeight());
	return;
}


void SegmScan::OcularLayers::initializeAsRetina(unsigned int width, unsigned int height)
{
	d_ptr->width = width;
	d_ptr->height = height;

	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::ILM)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::NFL)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::IPL)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::OPL)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::IOS)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::RPE)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::BRM)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::BASE)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::INNER)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::OUTER)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::OPR)] = OcularLayer(width, height);
	return;
}


void SegmScan::OcularLayers::initializeAsCornea(unsigned int width, unsigned int height)
{
	d_ptr->width = width;
	d_ptr->height = height;

	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::INNER)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::OUTER)] = OcularLayer(width, height);

	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::EPI)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::BOW)] = OcularLayer(width, height);
	d_ptr->layers[static_cast<unsigned short>(OcularLayerType::END)] = OcularLayer(width, height);

	return;
}


void SegmScan::OcularLayers::applyRegionSize(unsigned int width, unsigned int height)
{
	for (auto iter = d_ptr->layers.begin(); iter != d_ptr->layers.end(); iter++) {
		auto layer = iter->second;
		// if (layer != nullptr) {
			layer.resize(width, height);
		// }
	}

	setRegionSize(width, height);
	return;
}


void SegmScan::OcularLayers::setRegionSize(unsigned int width, unsigned int height)
{
	d_ptr->width = width;
	d_ptr->height = height;
	return;
}


int SegmScan::OcularLayers::getNumberOfLayers(void)
{
	return (int)d_ptr->layers.size();
}


int SegmScan::OcularLayers::getRegionWidth(void)
{
	return d_ptr->width;
}


int SegmScan::OcularLayers::getRegionHeight(void)
{
	return d_ptr->height;
}


void SegmScan::OcularLayers::clear(void)
{
	d_ptr->layers.clear();
	return;
}


OcularLayers::OcularLayersImpl & SegmScan::OcularLayers::getImpl(void) const
{
	return *d_ptr;
}
