#include "stdafx.h"
#include "DiscThicknessMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscThicknessMap::DiscThicknessMapImpl
{
	DiscThicknessMapImpl()
	{
	}
};


DiscThicknessMap::DiscThicknessMap() :
	d_ptr(make_unique<DiscThicknessMapImpl>())
{
}


SegmScan::DiscThicknessMap::~DiscThicknessMap() = default;
SegmScan::DiscThicknessMap::DiscThicknessMap(DiscThicknessMap && rhs) = default;
DiscThicknessMap & SegmScan::DiscThicknessMap::operator=(DiscThicknessMap && rhs) = default;


SegmScan::DiscThicknessMap::DiscThicknessMap(const DiscThicknessMap & rhs)
	: d_ptr(make_unique<DiscThicknessMapImpl>(*rhs.d_ptr))
{
}


DiscThicknessMap & SegmScan::DiscThicknessMap::operator=(const DiscThicknessMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscThicknessMap::DiscThicknessMapImpl & SegmScan::DiscThicknessMap::getImpl(void) const
{
	return *d_ptr;
}
