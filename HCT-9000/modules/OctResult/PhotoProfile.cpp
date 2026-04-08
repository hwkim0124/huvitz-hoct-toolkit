#include "stdafx.h"
#include "PhotoProfile.h"
#include "PhotoImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PhotoProfile::PhotoProfileImpl
{
	PhotoImageList photos;

	PhotoProfileImpl()
	{
	}
};


PhotoProfile::PhotoProfile() :
	d_ptr(make_unique<PhotoProfileImpl>())
{
}


OctResult::PhotoProfile::~PhotoProfile() = default;
OctResult::PhotoProfile::PhotoProfile(PhotoProfile && rhs) = default;
PhotoProfile & OctResult::PhotoProfile::operator=(PhotoProfile && rhs) = default;


OctResult::PhotoProfile::PhotoProfile(const PhotoProfile & rhs)
	: d_ptr(make_unique<PhotoProfileImpl>(*rhs.d_ptr))
{
}


PhotoProfile & OctResult::PhotoProfile::operator=(const PhotoProfile & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::PhotoProfile::addImage(PhotoImage & image)
{
	d_ptr->photos.push_back(image);
	int size = getImageCount();
	return size;
}


int OctResult::PhotoProfile::addImage(PhotoImage && image)
{
	d_ptr->photos.push_back(image);
	int size = getImageCount();
	return size;
}


PhotoImage * OctResult::PhotoProfile::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	PhotoImage* p = &d_ptr->photos[index];
	return p;
}


int OctResult::PhotoProfile::getImageCount(void) const
{
	return (int)d_ptr->photos.size();
}


void OctResult::PhotoProfile::clearImagePhotos(void)
{
	d_ptr->photos.clear();
	return;
}


PhotoProfile::PhotoProfileImpl & OctResult::PhotoProfile::getImpl(void) const
{
	return *d_ptr;
}
