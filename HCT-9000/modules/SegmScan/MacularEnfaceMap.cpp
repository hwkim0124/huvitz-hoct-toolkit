#include "stdafx.h"
#include "MacularEnfaceMap.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularEnfaceMap::MacularEnfaceMapImpl
{
	MacularEnfaceMapImpl()
	{
	}
};


MacularEnfaceMap::MacularEnfaceMap() :
	d_ptr(make_unique<MacularEnfaceMapImpl>())
{
}


SegmScan::MacularEnfaceMap::~MacularEnfaceMap() = default;
SegmScan::MacularEnfaceMap::MacularEnfaceMap(MacularEnfaceMap && rhs) = default;
MacularEnfaceMap & SegmScan::MacularEnfaceMap::operator=(MacularEnfaceMap && rhs) = default;


SegmScan::MacularEnfaceMap::MacularEnfaceMap(const MacularEnfaceMap & rhs)
	: d_ptr(make_unique<MacularEnfaceMapImpl>(*rhs.d_ptr))
{
}


MacularEnfaceMap & SegmScan::MacularEnfaceMap::operator=(const MacularEnfaceMap & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEnfaceMap::MacularEnfaceMapImpl & SegmScan::MacularEnfaceMap::getImpl(void) const
{
	return *d_ptr;
}
