#include "stdafx.h"
#include "CorneaEnfaceChart.h"
#include "MacularEnfaceMap.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaEnfaceChart::CorneaEnfaceChartImpl
{
	CorneaEnfaceChartImpl()
	{
	}
};


CorneaEnfaceChart::CorneaEnfaceChart() :
	d_ptr(make_unique<CorneaEnfaceChartImpl>())
{
}


SegmScan::CorneaEnfaceChart::~CorneaEnfaceChart() = default;
SegmScan::CorneaEnfaceChart::CorneaEnfaceChart(CorneaEnfaceChart && rhs) = default;
CorneaEnfaceChart & SegmScan::CorneaEnfaceChart::operator=(CorneaEnfaceChart && rhs) = default;


SegmScan::CorneaEnfaceChart::CorneaEnfaceChart(const CorneaEnfaceChart & rhs)
	: d_ptr(make_unique<CorneaEnfaceChartImpl>(*rhs.d_ptr))
{
}


CorneaEnfaceChart & SegmScan::CorneaEnfaceChart::operator=(const CorneaEnfaceChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaEnfaceChart::CorneaEnfaceChartImpl & SegmScan::CorneaEnfaceChart::getImpl(void) const
{
	return *d_ptr;
}
