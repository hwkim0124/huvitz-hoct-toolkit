#include "stdafx.h"
#include "CorneaETDRSChart.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaETDRSChart::CorneaETDRSChartImpl
{
	RetParam::SectorChart chart;

	CorneaETDRSChartImpl()
	{
	}
};


CorneaETDRSChart::CorneaETDRSChart() :
	d_ptr(make_unique<CorneaETDRSChartImpl>())
{
	setEnfaceChart(&getImpl().chart);
	getImpl().chart.setCorneaThickness(true);
	setSectionSize();
}


SegmScan::CorneaETDRSChart::~CorneaETDRSChart() = default;
SegmScan::CorneaETDRSChart::CorneaETDRSChart(CorneaETDRSChart && rhs) = default;
CorneaETDRSChart & SegmScan::CorneaETDRSChart::operator=(CorneaETDRSChart && rhs) = default;


SegmScan::CorneaETDRSChart::CorneaETDRSChart(const CorneaETDRSChart & rhs)
	: d_ptr(make_unique<CorneaETDRSChartImpl>(*rhs.d_ptr))
{

}


CorneaETDRSChart & SegmScan::CorneaETDRSChart::operator=(const CorneaETDRSChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::CorneaETDRSChart::setSectionSize(float center, float inner, float outer)
{
	d_ptr->chart.setCenterDiameter(center);
	d_ptr->chart.setInnerDiameter(inner);
	d_ptr->chart.setOuterDiameter(outer);
	return;
}


float SegmScan::CorneaETDRSChart::centerThickness(void) const
{
	return d_ptr->chart.meanCenter();
}


float SegmScan::CorneaETDRSChart::innerThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 7 : index);
		index = (section == 2 ? 6 : index);
		index = (section == 3 ? 5 : index);
	}

	return d_ptr->chart.meanInnerSector(section);
}


float SegmScan::CorneaETDRSChart::outerThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 7 : index);
		index = (section == 2 ? 6 : index);
		index = (section == 3 ? 5 : index);
	}

	return d_ptr->chart.meanOuterSector(section);
}


float SegmScan::CorneaETDRSChart::averageThickness(void) const
{
	return d_ptr->chart.meanTotal();
}


float SegmScan::CorneaETDRSChart::superiorThickness(void) const
{
	return d_ptr->chart.meanSuperior();
}


float SegmScan::CorneaETDRSChart::inferiorThickness(void) const
{
	return d_ptr->chart.meanInferior();
}


CorneaETDRSChart::CorneaETDRSChartImpl & SegmScan::CorneaETDRSChart::getImpl(void) const
{
	return *d_ptr;
}
