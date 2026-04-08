#include "stdafx.h"
#include "CorneaChart.h"
#include "CorneaEplot.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaChart::CorneaChartImpl
{
	CorneaChartImpl()
	{
	}
};


CorneaChart::CorneaChart() :
	d_ptr(make_unique<CorneaChartImpl>())
{
}


SegmScan::CorneaChart::~CorneaChart() = default;
SegmScan::CorneaChart::CorneaChart(CorneaChart && rhs) = default;
CorneaChart & SegmScan::CorneaChart::operator=(CorneaChart && rhs) = default;


SegmScan::CorneaChart::CorneaChart(const CorneaChart & rhs)
	: d_ptr(make_unique<CorneaChartImpl>(*rhs.d_ptr))
{
}


CorneaChart & SegmScan::CorneaChart::operator=(const CorneaChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}

CorneaChart::CorneaChartImpl & SegmScan::CorneaChart::getImpl(void) const
{
	return *d_ptr;
}
