#include "stdafx.h"
#include "MacularEplot.h"
#include "MacularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularEplot::MacularEplotImpl
{
	RetParam::EnfaceTmap enfaceTmap;

	MacularEplotImpl()
	{
	}
};


MacularEplot::MacularEplot() :
	d_ptr(make_unique<MacularEplotImpl>())
{
}


SegmScan::MacularEplot::~MacularEplot() = default;
SegmScan::MacularEplot::MacularEplot(MacularEplot && rhs) = default;
MacularEplot & SegmScan::MacularEplot::operator=(MacularEplot && rhs) = default;


SegmScan::MacularEplot::MacularEplot(const MacularEplot & rhs)
	: d_ptr(make_unique<MacularEplotImpl>(*rhs.d_ptr))
{
}


MacularEplot & SegmScan::MacularEplot::operator=(const MacularEplot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEplot::MacularEplotImpl & SegmScan::MacularEplot::getImpl(void) const
{
	return *d_ptr;
}
