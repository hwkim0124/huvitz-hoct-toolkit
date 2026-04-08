#include "stdafx.h"
#include "CorneaEnfaceImage.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct CorneaEnfaceImage::CorneaEnfaceImageImpl
{
	CorneaEnfaceImageImpl()
	{
	}
};


CorneaEnfaceImage::CorneaEnfaceImage() :
	d_ptr(make_unique<CorneaEnfaceImageImpl>())
{
}


SegmScan::CorneaEnfaceImage::~CorneaEnfaceImage() = default;
SegmScan::CorneaEnfaceImage::CorneaEnfaceImage(CorneaEnfaceImage && rhs) = default;
CorneaEnfaceImage & SegmScan::CorneaEnfaceImage::operator=(CorneaEnfaceImage && rhs) = default;


SegmScan::CorneaEnfaceImage::CorneaEnfaceImage(const CorneaEnfaceImage & rhs)
	: d_ptr(make_unique<CorneaEnfaceImageImpl>(*rhs.d_ptr))
{
}


CorneaEnfaceImage & SegmScan::CorneaEnfaceImage::operator=(const CorneaEnfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CorneaEnfaceImage::CorneaEnfaceImageImpl & SegmScan::CorneaEnfaceImage::getImpl(void) const
{
	return *d_ptr;
}
