#include "stdafx.h"
#include "FundusPhoto.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusPhoto::FundusPhotoImpl
{
	FundusPhotoImpl()
	{
	}
};


FundusPhoto::FundusPhoto() : 
	d_ptr(make_unique<FundusPhotoImpl>())
{
}


OctResult::FundusPhoto::~FundusPhoto() = default;
OctResult::FundusPhoto::FundusPhoto(FundusPhoto && rhs) = default;
FundusPhoto & OctResult::FundusPhoto::operator=(FundusPhoto && rhs) = default;


OctResult::FundusPhoto::FundusPhoto(const FundusPhoto & rhs)
	: d_ptr(make_unique<FundusPhotoImpl>(*rhs.d_ptr))
{
}


FundusPhoto & OctResult::FundusPhoto::operator=(const FundusPhoto & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


FundusPhoto::FundusPhotoImpl & OctResult::FundusPhoto::getImpl(void) const
{
	return *d_ptr;
}
