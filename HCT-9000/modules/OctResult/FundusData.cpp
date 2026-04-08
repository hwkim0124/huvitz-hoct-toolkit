#include "stdafx.h"
#include "FundusData.h"
#include "FundusPhoto.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusData::FundusDataImpl
{
	FundusPhotoList photos;

	OctRoute route;

	FundusDataImpl()
	{
	}
};


FundusData::FundusData() :
	d_ptr(make_unique<FundusDataImpl>())
{
}


OctResult::FundusData::~FundusData() = default;
OctResult::FundusData::FundusData(FundusData && rhs) = default;
FundusData & OctResult::FundusData::operator=(FundusData && rhs) = default;


OctResult::FundusData::FundusData(const FundusData & rhs)
	: d_ptr(make_unique<FundusDataImpl>(*rhs.d_ptr))
{
}


FundusData & OctResult::FundusData::operator=(const FundusData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::FundusData::addPhoto(FundusPhoto & image)
{
	d_ptr->photos.push_back(image);
	int size = getPhotoCount();
	return size;
}


int OctResult::FundusData::addPhoto(FundusPhoto && image)
{
	d_ptr->photos.push_back(image);
	int size = getPhotoCount();
	return size;
}


FundusPhoto * OctResult::FundusData::getPhoto(int index)
{
	if (index < 0 || index >= getPhotoCount()) {
		return nullptr;
	}

	FundusPhoto* p = &d_ptr->photos[index];
	return p;
}


int OctResult::FundusData::getPhotoCount(void) const
{
	return (int)d_ptr->photos.size();
}


void OctResult::FundusData::clearAllPhotos(void)
{
	d_ptr->photos.clear();
	return;
}


FundusData::FundusDataImpl & OctResult::FundusData::getImpl(void) const
{
	return *d_ptr;
}
