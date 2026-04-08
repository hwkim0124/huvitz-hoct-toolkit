#include "stdafx.h"
#include "CorneaRadiusMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaRadiusMap::CorneaRadiusMapImpl
{
	CorneaRadiusMapImpl()
	{
	}
};



CorneaRadiusMap::CorneaRadiusMap() :
	d_ptr(make_unique<CorneaRadiusMapImpl>())
{
}


SegmScan::CorneaRadiusMap::~CorneaRadiusMap() = default;
SegmScan::CorneaRadiusMap::CorneaRadiusMap(CorneaRadiusMap && rhs) = default;
CorneaRadiusMap & SegmScan::CorneaRadiusMap::operator=(CorneaRadiusMap && rhs) = default;


SegmScan::CorneaRadiusMap::CorneaRadiusMap(const CorneaRadiusMap & rhs)
	: d_ptr(make_unique<CorneaRadiusMapImpl>(*rhs.d_ptr))
{
}


CorneaRadiusMap & SegmScan::CorneaRadiusMap::operator=(const CorneaRadiusMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaRadiusMap::CorneaRadiusMapImpl & SegmScan::CorneaRadiusMap::getImpl(void) const
{
	return *d_ptr;
}
