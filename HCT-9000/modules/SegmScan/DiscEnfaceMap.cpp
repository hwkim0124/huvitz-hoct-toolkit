#include "stdafx.h"
#include "DiscEnfaceMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscEnfaceMap::DiscEnfaceMapImpl
{
	DiscEnfaceMapImpl()
	{
	}
};


DiscEnfaceMap::DiscEnfaceMap() :
	d_ptr(make_unique<DiscEnfaceMapImpl>())
{
}


SegmScan::DiscEnfaceMap::~DiscEnfaceMap() = default;
SegmScan::DiscEnfaceMap::DiscEnfaceMap(DiscEnfaceMap && rhs) = default;
DiscEnfaceMap & SegmScan::DiscEnfaceMap::operator=(DiscEnfaceMap && rhs) = default;


SegmScan::DiscEnfaceMap::DiscEnfaceMap(const DiscEnfaceMap & rhs)
	: d_ptr(make_unique<DiscEnfaceMapImpl>(*rhs.d_ptr))
{
}


DiscEnfaceMap & SegmScan::DiscEnfaceMap::operator=(const DiscEnfaceMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEnfaceMap::DiscEnfaceMapImpl & SegmScan::DiscEnfaceMap::getImpl(void) const
{
	return *d_ptr;
}
