#include "stdafx.h"
#include "CorneaEplot.h"
#include "CorneaBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaEplot::CorneaEplotImpl
{
	CorneaEplotImpl()
	{
	}
};



CorneaEplot::CorneaEplot() :
	d_ptr(make_unique<CorneaEplotImpl>())
{
}


SegmScan::CorneaEplot::~CorneaEplot() = default;
SegmScan::CorneaEplot::CorneaEplot(CorneaEplot && rhs) = default;
CorneaEplot & SegmScan::CorneaEplot::operator=(CorneaEplot && rhs) = default;


SegmScan::CorneaEplot::CorneaEplot(const CorneaEplot & rhs)
	: d_ptr(make_unique<CorneaEplotImpl>(*rhs.d_ptr))
{
}


CorneaEplot & SegmScan::CorneaEplot::operator=(const CorneaEplot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaEplot::CorneaEplotImpl & SegmScan::CorneaEplot::getImpl(void) const
{
	return *d_ptr;
}
