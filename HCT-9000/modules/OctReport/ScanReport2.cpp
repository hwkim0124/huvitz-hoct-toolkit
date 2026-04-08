#include "stdafx.h"
#include "ScanReport2.h"

#include <forward_list>

#include "CppUtil2.h"
#include "OctResult2.h"
#include "OctData2.h"
#include "SegmScan2.h"

using namespace OctReport;
using namespace OctData;
using namespace OctResult;
using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct ScanReport2::ScanReport2Impl
{
	ProtocolData* data;
	forward_list<unique_ptr<OcularEnfaceImage>> shots;
	forward_list<unique_ptr<OcularEnfaceMap>> tmaps;

	ScanReport2Impl() : data(nullptr)
	{
	}
};


OctReport::ScanReport2::ScanReport2() :
	d_ptr(make_unique<ScanReport2Impl>())
{
}


OctReport::ScanReport2::~ScanReport2() = default;
OctReport::ScanReport2::ScanReport2(ScanReport2 && rhs) = default;
ScanReport2 & OctReport::ScanReport2::operator=(ScanReport2 && rhs) = default;




void OctReport::ScanReport2::setProtocolData(OctData::ProtocolData * data, EyeSide side, bool update)
{
	d_ptr->data = data;
	setEyeSide(side);

	if (update) {
		updateContents();
	}
	return;
}


OctData::ProtocolData * OctReport::ScanReport2::getProtocolData(void) const
{
	return d_ptr->data;
}


OctScanPattern * OctReport::ScanReport2::getDescript(void) const
{
	if (getProtocolData()) {
		return &getProtocolData()->getDescript();
	}
	return nullptr;
}


bool OctReport::ScanReport2::isEmpty(void) const
{
	return (d_ptr->data == nullptr);
}


std::unique_ptr<SegmScan::OcularEnfaceImage> OctReport::ScanReport2::createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	return nullptr;
}


std::unique_ptr<SegmScan::OcularEnfaceMap> OctReport::ScanReport2::createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	return nullptr;
}


SegmScan::OcularEnfaceImage * OctReport::ScanReport2::prepareEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	if (isEmpty()) {
		return nullptr;
	}

	if (getProtocolData()->getDescript().isEnfaceScan()) {
	}
	else {
		return nullptr;
	}

	auto p = findEnfaceImage(upper, lower, upperOffset, lowerOffset);
	if (p) {
		return p;
	}

	auto uptr = createEnfaceImage(upper, lower, upperOffset, lowerOffset);
	if (uptr != nullptr) {
		if (registEnfaceImage(std::move(uptr))) {
			return findEnfaceImage(upper, lower, upperOffset, lowerOffset);
		}
	}
	return nullptr;
}


SegmScan::OcularEnfaceImage * OctReport::ScanReport2::findEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	for (const auto& enf : d_ptr->shots) {
		if (enf->isIdentical(upper, lower, upperOffset, lowerOffset)) {
			return enf.get();
		}
	}
	return nullptr;
}


bool OctReport::ScanReport2::registEnfaceImage(std::unique_ptr<SegmScan::OcularEnfaceImage> enface)
{
	if (enface) {
		getImpl().shots.push_front(move(enface));
		return true;
	}
	return false;
}


SegmScan::OcularEnfaceMap * OctReport::ScanReport2::prepareEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	if (isEmpty()) {
		return nullptr;
	}

	if (getProtocolData()->getDescript().isEnfaceScan() ||
		getProtocolData()->getDescript().isRadialScan() ||
		getProtocolData()->getDescript().isRasterScan()) {
	}
	else {
		return nullptr;
	}

	auto p = findEnfaceMap(upper, lower, upperOffset, lowerOffset);
	if (p) {
		return p;
	}

	auto uptr = createEnfaceMap(upper, lower, upperOffset, lowerOffset);
	if (uptr != nullptr) {
		if (registEnfaceMap(std::move(uptr))) {
			return findEnfaceMap(upper, lower, upperOffset, lowerOffset);
		}
	}
	return nullptr;
}


SegmScan::OcularEnfaceMap * OctReport::ScanReport2::findEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	for (const auto& tmap : d_ptr->tmaps) {
		if (tmap->isIdentical(upper, lower, upperOffset, lowerOffset)) {
			return tmap.get();
		}
	}
	return nullptr;
}


bool OctReport::ScanReport2::registEnfaceMap(std::unique_ptr<SegmScan::OcularEnfaceMap> tmap)
{
	if (tmap) {
		getImpl().tmaps.push_front(move(tmap));
		return true;
	}
	return false;
}


ScanReport2::ScanReport2Impl & OctReport::ScanReport2::getImpl(void) const
{
	return *d_ptr;
}