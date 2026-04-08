#include "stdafx.h"
#include "MacularEnfaceImage.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularEnfaceImage::MacularEnfaceImageImpl
{
	MacularEnfaceImageImpl()
	{
	}
};


MacularEnfaceImage::MacularEnfaceImage() :
	d_ptr(make_unique<MacularEnfaceImageImpl>())
{
}


SegmScan::MacularEnfaceImage::~MacularEnfaceImage() = default;
SegmScan::MacularEnfaceImage::MacularEnfaceImage(MacularEnfaceImage && rhs) = default;
MacularEnfaceImage & SegmScan::MacularEnfaceImage::operator=(MacularEnfaceImage && rhs) = default;


SegmScan::MacularEnfaceImage::MacularEnfaceImage(const MacularEnfaceImage & rhs)
	: d_ptr(make_unique<MacularEnfaceImageImpl>(*rhs.d_ptr))
{
}


MacularEnfaceImage & SegmScan::MacularEnfaceImage::operator=(const MacularEnfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


MacularEnfaceImage::MacularEnfaceImageImpl & SegmScan::MacularEnfaceImage::getImpl(void) const
{
	return *d_ptr;
}
