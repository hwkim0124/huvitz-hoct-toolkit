#include "stdafx.h"
#include "CorneaEnface.h"
#include "CorneaBsegm.h"

#include "CppUtil2.h"

using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct CorneaEnface::CorneaEnfaceImpl
{
	CorneaEnfaceImpl()
	{
	}
};


CorneaEnface::CorneaEnface() :
	d_ptr(make_unique<CorneaEnfaceImpl>())
{
}


SegmScan::CorneaEnface::~CorneaEnface() = default;
SegmScan::CorneaEnface::CorneaEnface(CorneaEnface && rhs) = default;
CorneaEnface & SegmScan::CorneaEnface::operator=(CorneaEnface && rhs) = default;


SegmScan::CorneaEnface::CorneaEnface(const CorneaEnface & rhs)
	: d_ptr(make_unique<CorneaEnfaceImpl>(*rhs.d_ptr))
{
}


CorneaEnface & SegmScan::CorneaEnface::operator=(const CorneaEnface & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaEnface::CorneaEnfaceImpl & SegmScan::CorneaEnface::getImpl(void) const
{
	return *d_ptr;
}
