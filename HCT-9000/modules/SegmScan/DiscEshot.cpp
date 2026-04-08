#include "stdafx.h"
#include "DiscEshot.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscEshot::DiscEshotImpl
{
	DiscEshotImpl()
	{
	}
};


DiscEshot::DiscEshot() :
	d_ptr(make_unique<DiscEshotImpl>())
{
}


SegmScan::DiscEshot::~DiscEshot() = default;
SegmScan::DiscEshot::DiscEshot(DiscEshot && rhs) = default;
DiscEshot & SegmScan::DiscEshot::operator=(DiscEshot && rhs) = default;


SegmScan::DiscEshot::DiscEshot(const DiscEshot & rhs)
	: d_ptr(make_unique<DiscEshotImpl>(*rhs.d_ptr))
{
}


DiscEshot & SegmScan::DiscEshot::operator=(const DiscEshot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEshot::DiscEshotImpl & SegmScan::DiscEshot::getImpl(void) const
{
	return *d_ptr;
}
