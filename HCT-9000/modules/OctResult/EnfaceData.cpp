#include "stdafx.h"
#include "EnfaceData.h"
#include "EnfaceImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct EnfaceData::EnfaceDataImpl
{
	EnfaceImageList images;
	OctRange range;

	EnfaceDataImpl() 
	{
	}
};


EnfaceData::EnfaceData() :
	d_ptr(make_unique<EnfaceDataImpl>())
{
}


OctResult::EnfaceData::~EnfaceData() = default;
OctResult::EnfaceData::EnfaceData(EnfaceData && rhs) = default;
EnfaceData & OctResult::EnfaceData::operator=(EnfaceData && rhs) = default;


OctResult::EnfaceData::EnfaceData(const EnfaceData & rhs)
	: d_ptr(make_unique<EnfaceDataImpl>(*rhs.d_ptr))
{
}


EnfaceData & OctResult::EnfaceData::operator=(const EnfaceData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::EnfaceData::addImage(EnfaceImage & image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


int OctResult::EnfaceData::addImage(EnfaceImage && image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


EnfaceImage * OctResult::EnfaceData::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	EnfaceImage* p = &d_ptr->images[index];
	return p;
}


int OctResult::EnfaceData::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


void OctResult::EnfaceData::clearAllImages(void)
{
	d_ptr->images.clear();
	return;
}


void OctResult::EnfaceData::setRangeOfEnface(OctRange range)
{
	d_ptr->range = range;
	return;
}


OctRange & OctResult::EnfaceData::getRangeOfEnface(void)
{
	return d_ptr->range;
}


EnfaceData::EnfaceDataImpl & OctResult::EnfaceData::getImpl(void) const
{
	return *d_ptr;
}
