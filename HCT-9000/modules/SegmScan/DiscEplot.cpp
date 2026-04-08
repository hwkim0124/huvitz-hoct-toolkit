#include "stdafx.h"
#include "DiscEplot.h"
#include "DiscBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscEplot::DiscEplotImpl
{
	RetParam::EnfaceTmap enfaceTmap;

	DiscEplotImpl()
	{
	}
};


DiscEplot::DiscEplot() :
	d_ptr(make_unique<DiscEplotImpl>())
{
}


SegmScan::DiscEplot::~DiscEplot() = default;
SegmScan::DiscEplot::DiscEplot(DiscEplot && rhs) = default;
DiscEplot & SegmScan::DiscEplot::operator=(DiscEplot && rhs) = default;


SegmScan::DiscEplot::DiscEplot(const DiscEplot & rhs)
	: d_ptr(make_unique<DiscEplotImpl>(*rhs.d_ptr))
{
}


DiscEplot & SegmScan::DiscEplot::operator=(const DiscEplot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEplot::DiscEplotImpl & SegmScan::DiscEplot::getImpl(void) const
{
	return *d_ptr;
}
