#include "stdafx.h"
#include "MacularThicknessMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularThicknessMap::MacularThicknessMapImpl
{
	MacularThicknessMapImpl()
	{
	}
};


MacularThicknessMap::MacularThicknessMap() :
	d_ptr(make_unique<MacularThicknessMapImpl>())
{
}


SegmScan::MacularThicknessMap::~MacularThicknessMap() = default;
SegmScan::MacularThicknessMap::MacularThicknessMap(MacularThicknessMap && rhs) = default;
MacularThicknessMap & SegmScan::MacularThicknessMap::operator=(MacularThicknessMap && rhs) = default;


SegmScan::MacularThicknessMap::MacularThicknessMap(const MacularThicknessMap & rhs)
	: d_ptr(make_unique<MacularThicknessMapImpl>(*rhs.d_ptr))
{
}


MacularThicknessMap & SegmScan::MacularThicknessMap::operator=(const MacularThicknessMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularThicknessMap::MacularThicknessMapImpl & SegmScan::MacularThicknessMap::getImpl(void) const
{
	return *d_ptr;
}
