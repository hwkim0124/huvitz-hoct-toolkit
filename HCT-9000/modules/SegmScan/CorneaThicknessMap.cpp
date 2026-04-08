#include "stdafx.h"
#include "CorneaThicknessMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaThicknessMap::CorneaThicknessMapImpl
{
	CorneaThicknessMapImpl()
	{
	}
};



CorneaThicknessMap::CorneaThicknessMap() :
	d_ptr(make_unique<CorneaThicknessMapImpl>())
{
}


SegmScan::CorneaThicknessMap::~CorneaThicknessMap() = default;
SegmScan::CorneaThicknessMap::CorneaThicknessMap(CorneaThicknessMap && rhs) = default;
CorneaThicknessMap & SegmScan::CorneaThicknessMap::operator=(CorneaThicknessMap && rhs) = default;


SegmScan::CorneaThicknessMap::CorneaThicknessMap(const CorneaThicknessMap & rhs)
	: d_ptr(make_unique<CorneaThicknessMapImpl>(*rhs.d_ptr))
{
}


CorneaThicknessMap & SegmScan::CorneaThicknessMap::operator=(const CorneaThicknessMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaThicknessMap::CorneaThicknessMapImpl & SegmScan::CorneaThicknessMap::getImpl(void) const
{
	return *d_ptr;
}
