#include "stdafx.h"
#include "MacularEnfaceChart.h"
#include "MacularEnfaceMap.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularEnfaceChart::MacularEnfaceChartImpl
{
	MacularEnfaceChartImpl() 
	{
	}
};


MacularEnfaceChart::MacularEnfaceChart() :
	d_ptr(make_unique<MacularEnfaceChartImpl>())
{
}


SegmScan::MacularEnfaceChart::~MacularEnfaceChart() = default;
SegmScan::MacularEnfaceChart::MacularEnfaceChart(MacularEnfaceChart && rhs) = default;
MacularEnfaceChart & SegmScan::MacularEnfaceChart::operator=(MacularEnfaceChart && rhs) = default;


SegmScan::MacularEnfaceChart::MacularEnfaceChart(const MacularEnfaceChart & rhs)
	: d_ptr(make_unique<MacularEnfaceChartImpl>(*rhs.d_ptr))
{
}


MacularEnfaceChart & SegmScan::MacularEnfaceChart::operator=(const MacularEnfaceChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEnfaceChart::MacularEnfaceChartImpl & SegmScan::MacularEnfaceChart::getImpl(void) const
{
	return *d_ptr;
}
