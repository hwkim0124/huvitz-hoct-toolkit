#include "stdafx.h"
#include "DiscChart.h"
#include "DiscEplot.h"

#include "RetParam2.h"
#include "CppUtil2.h"
#include "NormData2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace NormData;
using namespace std;


struct DiscChart::DiscChartImpl
{
	DiscChartImpl() 
	{
	}
};


DiscChart::DiscChart() :
	d_ptr(make_unique<DiscChartImpl>())
{
}


SegmScan::DiscChart::~DiscChart() = default;
SegmScan::DiscChart::DiscChart(DiscChart && rhs) = default;
DiscChart & SegmScan::DiscChart::operator=(DiscChart && rhs) = default;


SegmScan::DiscChart::DiscChart(const DiscChart & rhs)
	: d_ptr(make_unique<DiscChartImpl>(*rhs.d_ptr))
{
}


DiscChart & SegmScan::DiscChart::operator=(const DiscChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscChart::DiscChartImpl & SegmScan::DiscChart::getImpl(void) const
{
	return *d_ptr;
}
