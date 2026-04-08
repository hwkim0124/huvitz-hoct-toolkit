#include "stdafx.h"
#include "GCCThicknessChart.h"
#include "MacularEplot.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct GCCThicknessChart::GCCThicknessChartImpl
{
	RetParam::DonutChart chart;

	GCCThicknessChartImpl() 
	{
	}
};


GCCThicknessChart::GCCThicknessChart() :
	d_ptr(make_unique<GCCThicknessChartImpl>())
{
	setEnfaceChart(&getImpl().chart);
}


SegmScan::GCCThicknessChart::~GCCThicknessChart() = default;
SegmScan::GCCThicknessChart::GCCThicknessChart(GCCThicknessChart && rhs) = default;
GCCThicknessChart & SegmScan::GCCThicknessChart::operator=(GCCThicknessChart && rhs) = default;


SegmScan::GCCThicknessChart::GCCThicknessChart(const GCCThicknessChart & rhs)
	: d_ptr(make_unique<GCCThicknessChartImpl>(*rhs.d_ptr))
{
}


GCCThicknessChart & SegmScan::GCCThicknessChart::operator=(const GCCThicknessChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::GCCThicknessChart::setSectionSize(float inner, float outer)
{
	d_ptr->chart.setInnerDiameter(inner);
	d_ptr->chart.setOuterDiameter(outer);
	return;
}


float SegmScan::GCCThicknessChart::sectionThickness(int section) const
{
	int index = section;
	if (eyeSide() == EyeSide::OS) {
		index = (section == 1 ? 5 : index);
		index = (section == 2 ? 4 : index);
		index = (section == 4 ? 2 : index);
		index = (section == 5 ? 1 : index);
	}
	return d_ptr->chart.meanSection(index);
}


float SegmScan::GCCThicknessChart::averageThickness(void) const
{
	return d_ptr->chart.meanTotal();
}


float SegmScan::GCCThicknessChart::superiorThickness(void) const
{
	return d_ptr->chart.meanSuperior();
}


float SegmScan::GCCThicknessChart::inferiorThickness(void) const
{
	return d_ptr->chart.meanInferior();
}


GCCThicknessChart::GCCThicknessChartImpl & SegmScan::GCCThicknessChart::getImpl(void) const
{
	return *d_ptr;
}
