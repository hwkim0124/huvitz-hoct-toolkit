#include "stdafx.h"
#include "MacularEnface.h"
#include "MacularBsegm.h"

#include "CppUtil2.h"

using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct MacularEnface::MacularEnfaceImpl
{
	MacularEnfaceImpl()
	{
	}
};


MacularEnface::MacularEnface() :
	d_ptr(make_unique<MacularEnfaceImpl>())
{
}


SegmScan::MacularEnface::~MacularEnface() = default;
SegmScan::MacularEnface::MacularEnface(MacularEnface && rhs) = default;
MacularEnface & SegmScan::MacularEnface::operator=(MacularEnface && rhs) = default;


SegmScan::MacularEnface::MacularEnface(const MacularEnface & rhs)
	: d_ptr(make_unique<MacularEnfaceImpl>(*rhs.d_ptr))
{
}


MacularEnface & SegmScan::MacularEnface::operator=(const MacularEnface & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEnface::MacularEnfaceImpl & SegmScan::MacularEnface::getImpl(void) const
{
	return *d_ptr;
}
