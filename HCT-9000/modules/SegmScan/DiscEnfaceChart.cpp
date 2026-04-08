#include "stdafx.h"
#include "DiscEnfaceChart.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscEnfaceChart::DiscEnfaceChartImpl
{
	DiscEnfaceChartImpl()
	{
	}
};


DiscEnfaceChart::DiscEnfaceChart() :
	d_ptr(make_unique<DiscEnfaceChartImpl>())
{
}


SegmScan::DiscEnfaceChart::~DiscEnfaceChart() = default;
SegmScan::DiscEnfaceChart::DiscEnfaceChart(DiscEnfaceChart && rhs) = default;
DiscEnfaceChart & SegmScan::DiscEnfaceChart::operator=(DiscEnfaceChart && rhs) = default;


SegmScan::DiscEnfaceChart::DiscEnfaceChart(const DiscEnfaceChart & rhs)
	: d_ptr(make_unique<DiscEnfaceChartImpl>(*rhs.d_ptr))
{
}


DiscEnfaceChart & SegmScan::DiscEnfaceChart::operator=(const DiscEnfaceChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEnfaceChart::DiscEnfaceChartImpl & SegmScan::DiscEnfaceChart::getImpl(void) const
{
	return *d_ptr;
}
