#include "stdafx.h"
#include "CorneaReport2.h"

#include "CppUtil2.h"
#include "OctResult2.h"
#include "OctData2.h"
#include "SegmScan2.h"

using namespace OctReport;
using namespace OctResult;
using namespace OctData;
using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct CorneaReport2::CorneaReport2Impl
{
	CorneaEplot radiusPlot;
	CorneaRadiusMap radiusMap;

	CorneaReport2Impl()
	{
	}
};


OctReport::CorneaReport2::CorneaReport2() :
	d_ptr(make_unique<CorneaReport2Impl>())
{
}


OctReport::CorneaReport2::~CorneaReport2() = default;
OctReport::CorneaReport2::CorneaReport2(CorneaReport2 && rhs) = default;
CorneaReport2 & OctReport::CorneaReport2::operator=(CorneaReport2 && rhs) = default;

bool OctReport::CorneaReport2::updateContents(void)
{
	return true;
}


void OctReport::CorneaReport2::clearContents(void)
{
	return;
}


SegmScan::CorneaThicknessMap * OctReport::CorneaReport2::getThicknessMap(OcularLayerType upper, OcularLayerType lower)
{
	auto tmap = prepareEnfaceMap(upper, lower);
	return static_cast<CorneaThicknessMap*>(tmap);
}


SegmScan::CorneaEplot * OctReport::CorneaReport2::getCorneaEplot(OcularLayerType upper, OcularLayerType lower)
{
	auto tmap = getThicknessMap(upper, lower);
	return static_cast<CorneaEplot*>(tmap);
}


SegmScan::CorneaEplot * OctReport::CorneaReport2::getCurvatureRadiusPlot(void)
{
	if (d_ptr->radiusPlot.isEmpty()) {
		const auto& desc = getProtocolData()->getDescript();
		const auto& bsegms = getProtocolData()->getBscanSegmList();

		bool result = d_ptr->radiusPlot.setupRadiusMap(desc, bsegms);
	}

	return &d_ptr->radiusPlot;
}


SegmScan::CorneaRadiusMap * OctReport::CorneaReport2::getCurvatureRadiusMap(void)
{
	if (d_ptr->radiusMap.isEmpty()) {
		const auto& desc = getProtocolData()->getDescript();
		const auto& bsegms = getProtocolData()->getBscanSegmList();

		bool result = d_ptr->radiusMap.setupRadiusMap(desc, bsegms);
	}

	return &d_ptr->radiusMap;
}


SegmScan::CorneaETDRSChart OctReport::CorneaReport2::makeETDRSChart(OcularLayerType upper, OcularLayerType lower, float centerX, float centerY)
{
	CorneaETDRSChart chart;

	auto tmap = getCorneaEplot(upper, lower);
	if (tmap) {
		chart.setEnfaceData(tmap->getThicknessMap());
		chart.setLocation(centerX, centerY);
		chart.setEyeSide(getEyeSide());
		chart.updateMetrics();
	}

	return chart;
}


SegmScan::CorneaChart OctReport::CorneaReport2::makeCorneaChart(OcularLayerType upper, OcularLayerType lower, float centerX, float centerY)
{
	CorneaChart chart;

	auto tmap = getCorneaEplot(upper, lower);
	if (tmap) {
		chart.setEnfaceData(tmap->getThicknessMap());
		chart.setLocation(centerX, centerY);
		chart.setEyeSide(getEyeSide());
		chart.updateMetrics();
	}

	return chart;
}


std::unique_ptr<SegmScan::OcularEnfaceMap> OctReport::CorneaReport2::createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	auto plot = unique_ptr<CorneaEnfaceMap>(new CorneaEnfaceMap);

	const auto& desc = getProtocolData()->getDescript();
	const auto& bsegms = getProtocolData()->getBscanSegmList();

	bool result = plot->setupThicknessMap(desc, upper, lower, upperOffset, lowerOffset, bsegms);

	if (result) {
		return std::move(plot);
	}
	return nullptr;
}


CorneaReport2::CorneaReport2Impl & OctReport::CorneaReport2::getImpl(void) const
{
	return *d_ptr;
}