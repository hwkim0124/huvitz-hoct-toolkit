#include "stdafx.h"
#include "PhotoImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PhotoImage::PhotoImageImpl
{
	PhotoImageImpl()
	{
	}
};


PhotoImage::PhotoImage() :
	d_ptr(make_unique<PhotoImageImpl>())
{
}


OctResult::PhotoImage::~PhotoImage() = default;
OctResult::PhotoImage::PhotoImage(PhotoImage && rhs) = default;
PhotoImage & OctResult::PhotoImage::operator=(PhotoImage && rhs) = default;


OctResult::PhotoImage::PhotoImage(const PhotoImage & rhs)
	: d_ptr(make_unique<PhotoImageImpl>(*rhs.d_ptr))
{
}


PhotoImage & OctResult::PhotoImage::operator=(const PhotoImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


PhotoImage::PhotoImageImpl & OctResult::PhotoImage::getImpl(void) const
{
	return *d_ptr;
}
