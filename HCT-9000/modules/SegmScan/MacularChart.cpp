#include "stdafx.h"
#include "MacularChart.h"
#include "MacularEplot.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularChart::MacularChartImpl
{
	MacularChartImpl() 
	{
	}
};


MacularChart::MacularChart() :
	d_ptr(make_unique<MacularChartImpl>())
{
}


SegmScan::MacularChart::~MacularChart() = default;
SegmScan::MacularChart::MacularChart(MacularChart && rhs) = default;
MacularChart & SegmScan::MacularChart::operator=(MacularChart && rhs) = default;


SegmScan::MacularChart::MacularChart(const MacularChart & rhs)
	: d_ptr(make_unique<MacularChartImpl>(*rhs.d_ptr))
{
}


MacularChart & SegmScan::MacularChart::operator=(const MacularChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



MacularChart::MacularChartImpl & SegmScan::MacularChart::getImpl(void) const
{
	return *d_ptr;
}
