#include "stdafx.h"
#include "MacularETDRSChart.h"

#include "RetParam2.h"
#include "NormData2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;
using namespace NormData;


struct MacularETDRSChart::MacularETDRSChartImpl
{
	RetParam::SectorChart chart;

	MacularETDRSChartImpl()
	{
	}
};


MacularETDRSChart::MacularETDRSChart() :
	d_ptr(make_unique<MacularETDRSChartImpl>())
{
	setEnfaceChart(&getImpl().chart);
	getImpl().chart.setCorneaThickness(false);
	setSectionSize();
}


SegmScan::MacularETDRSChart::~MacularETDRSChart() = default;
SegmScan::MacularETDRSChart::MacularETDRSChart(MacularETDRSChart && rhs) = default;
MacularETDRSChart & SegmScan::MacularETDRSChart::operator=(MacularETDRSChart && rhs) = default;


SegmScan::MacularETDRSChart::MacularETDRSChart(const MacularETDRSChart & rhs)
	: d_ptr(make_unique<MacularETDRSChartImpl>(*rhs.d_ptr))
{
}


MacularETDRSChart & SegmScan::MacularETDRSChart::operator=(const MacularETDRSChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::MacularETDRSChart::setSectionSize(float center, float inner, float outer)
{
	d_ptr->chart.setCenterDiameter(center);
	d_ptr->chart.setInnerDiameter(inner);
	d_ptr->chart.setOuterDiameter(outer);
	return;
}


float SegmScan::MacularETDRSChart::centerThickness(void) const
{
	return d_ptr->chart.meanCenter();
}


float SegmScan::MacularETDRSChart::minCenterThickness(void) const
{
	return d_ptr->chart.minCenter();
}


float SegmScan::MacularETDRSChart::maxCenterThickness(void) const
{
	return d_ptr->chart.maxCenter();
}


float SegmScan::MacularETDRSChart::innerThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 3 : index);
		index = (section == 3 ? 1 : index);
	}

	return d_ptr->chart.meanInnerSector(index);
}


float SegmScan::MacularETDRSChart::outerThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 3 : index);
		index = (section == 3 ? 1 : index);
	}

	return d_ptr->chart.meanOuterSector(index);
}


float SegmScan::MacularETDRSChart::hexagonalThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 5 : index);
		index = (section == 2 ? 4 : index);
		index = (section == 4 ? 2 : index);
		index = (section == 5 ? 1 : index);
	}

	return d_ptr->chart.meanHexagonalSector(index);
}


float SegmScan::MacularETDRSChart::centerThicknessPercentile(void) const
{
	float value = centerThickness();

	if (value <= 210.0f) {
		return CHART_PERCENTILE_1;
	}
	else if (value <= 220.0f) {
		return CHART_PERCENTILE_5;
	}
	else if (value <= 300.0f) {
		return CHART_PERCENTILE_95;
	}
	else if (value <= 315.0f) {
		return CHART_PERCENTILE_99;
	}
	else {
		return CHART_PERCENTILE_100;
	}
	return 0.0f;
}


float SegmScan::MacularETDRSChart::innerThicknessPercentile(int section) const
{
	float value = innerThickness(section);

	switch (section) {
	case 0:
		if (value <= 270.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 285.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 300.0f) {
			return CHART_PERCENTILE_95;
		}
		if (value <= 350.0f) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 1:
		if (value <= (eyeSide() == EyeSide::OD ? 260.0f : 265.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 280.0f : 285.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 300.0f : 305.0f)) {
			return CHART_PERCENTILE_95;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 345.0f : 350.0f)) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 2:
		if (value <= 275.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 290.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 310.0f) {
			return CHART_PERCENTILE_95;
		}
		if (value <= 360.0f) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 3:
		if (value <= (eyeSide() == EyeSide::OD ? 265.0f : 260.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 285.0f : 280.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 305.0f : 300.0f)) {
			return CHART_PERCENTILE_95;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 350.0f : 345.0f)) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;
	}

	return 0.0f;
}


float SegmScan::MacularETDRSChart::outerThicknessPercentile(int section) const
{
	float value = outerThickness(section);

	switch (section) {
	case 0:
		if (value <= 250.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 270.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 290.0f) {
			return CHART_PERCENTILE_95;
		}
		if (value <= 300.0f) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 1:
		if (value <= (eyeSide() == EyeSide::OD ? 230.0f : 240.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 245.0f : 255.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 300.0f : 315.0f)) {
			return CHART_PERCENTILE_95;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 315.0f : 330.0f)) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 2:
		if (value <= 235.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 245.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 295.0f) {
			return CHART_PERCENTILE_95;
		}
		if (value <= 305.0f) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;

	case 3:
		if (value <= (eyeSide() == EyeSide::OD ? 240.0f : 230.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 255.0f : 245.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 315.0f : 300.0f)) {
			return CHART_PERCENTILE_95;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 330.0f : 315.0f)) {
			return CHART_PERCENTILE_99;
		}
		return CHART_PERCENTILE_100;
	}

	return 0.0f;
}


float SegmScan::MacularETDRSChart::hexagonalThicknessPercentile(int section) const
{
	float value = hexagonalThickness(section);
	float range = 0.0f;

	switch (section) {
	case 0:
		if (value <= 70.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 75.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 90.0f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;

	case 1:
	case 2:
		if (value <= (eyeSide() == EyeSide::OD ? 70.0f : 75.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 75.0f : 80.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 90.0f : 95.0f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;

	case 3:
		if (value <= 70.0f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 75.0f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 95.0f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;

	case 4:
	case 5:
		if (value <= (eyeSide() == EyeSide::OD ? 75.0f : 70.0f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 80.0f : 75.0f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 95.0f : 90.0f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	}

	return 0.0f;
}


float SegmScan::MacularETDRSChart::averageThickness(void) const
{
	return d_ptr->chart.meanTotal();
}


float SegmScan::MacularETDRSChart::superiorThickness(void) const
{
	return d_ptr->chart.meanSuperior();
}


float SegmScan::MacularETDRSChart::inferiorThickness(void) const
{
	return d_ptr->chart.meanInferior();
}


MacularETDRSChart::MacularETDRSChartImpl & SegmScan::MacularETDRSChart::getImpl(void) const
{
	return *d_ptr;
}
