#include "stdafx.h"
#include "MacularEshot.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularEshot::MacularEshotImpl
{

	MacularEshotImpl()
	{
	}
};


MacularEshot::MacularEshot() :
	d_ptr(make_unique<MacularEshotImpl>())
{
}


SegmScan::MacularEshot::~MacularEshot() = default;
SegmScan::MacularEshot::MacularEshot(MacularEshot && rhs) = default;
MacularEshot & SegmScan::MacularEshot::operator=(MacularEshot && rhs) = default;


SegmScan::MacularEshot::MacularEshot(const MacularEshot & rhs)
	: d_ptr(make_unique<MacularEshotImpl>(*rhs.d_ptr))
{
}


MacularEshot & SegmScan::MacularEshot::operator=(const MacularEshot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEshot::MacularEshotImpl & SegmScan::MacularEshot::getImpl(void) const
{
	return *d_ptr;
}
