#include "stdafx.h"
#include "DiscEnface.h"

#include "CppUtil2.h"

using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct DiscEnface::DiscEnfaceImpl
{
	DiscEnfaceImpl()
	{
	}
};


DiscEnface::DiscEnface() :
	d_ptr(make_unique<DiscEnfaceImpl>())
{
}


SegmScan::DiscEnface::~DiscEnface() = default;
SegmScan::DiscEnface::DiscEnface(DiscEnface && rhs) = default;
DiscEnface & SegmScan::DiscEnface::operator=(DiscEnface && rhs) = default;


SegmScan::DiscEnface::DiscEnface(const DiscEnface & rhs)
	: d_ptr(make_unique<DiscEnfaceImpl>(*rhs.d_ptr))
{
}


DiscEnface & SegmScan::DiscEnface::operator=(const DiscEnface & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEnface::DiscEnfaceImpl & SegmScan::DiscEnface::getImpl(void) const
{
	return *d_ptr;
}
