#include "stdafx.h"
#include "DiscEnfaceImage.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct DiscEnfaceImage::DiscEnfaceImageImpl
{
	DiscEnfaceImageImpl()
	{
	}
};


DiscEnfaceImage::DiscEnfaceImage() :
	d_ptr(make_unique<DiscEnfaceImageImpl>())
{
}


SegmScan::DiscEnfaceImage::~DiscEnfaceImage() = default;
SegmScan::DiscEnfaceImage::DiscEnfaceImage(DiscEnfaceImage && rhs) = default;
DiscEnfaceImage & SegmScan::DiscEnfaceImage::operator=(DiscEnfaceImage && rhs) = default;


SegmScan::DiscEnfaceImage::DiscEnfaceImage(const DiscEnfaceImage & rhs)
	: d_ptr(make_unique<DiscEnfaceImageImpl>(*rhs.d_ptr))
{
}


DiscEnfaceImage & SegmScan::DiscEnfaceImage::operator=(const DiscEnfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


DiscEnfaceImage::DiscEnfaceImageImpl & SegmScan::DiscEnfaceImage::getImpl(void) const
{
	return *d_ptr;
}
