#include "stdafx.h"
#include "SectionData.h"
#include "SectionImage.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct SectionData::SectionDataImpl
{
	SectionImageList images;
	OctScanSection descript;

	SectionDataImpl()
	{
	}
};


SectionData::SectionData() :
	d_ptr(make_unique<SectionDataImpl>())
{
}


OctResult::SectionData::SectionData(OctRoute route) :
	d_ptr(make_unique<SectionDataImpl>())
{
	d_ptr->descript.route = route;
}


OctResult::SectionData::SectionData(OctScanSection desc)
{
	d_ptr->descript = desc;
}


OctResult::SectionData::~SectionData() = default;
OctResult::SectionData::SectionData(SectionData && rhs) = default;
SectionData & OctResult::SectionData::operator=(SectionData && rhs) = default;


OctResult::SectionData::SectionData(const SectionData & rhs)
	: d_ptr(make_unique<SectionDataImpl>(*rhs.d_ptr))
{
}


SectionData & OctResult::SectionData::operator=(const SectionData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::SectionData::setImage(SectionImage & image)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(image);
}


int OctResult::SectionData::setImage(SectionImage && image)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(image);
}


int OctResult::SectionData::addImage(SectionImage & image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


int OctResult::SectionData::addImage(SectionImage && image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


SectionImage * OctResult::SectionData::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	SectionImage* p = &d_ptr->images[index];
	return p;
}


int OctResult::SectionData::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


void OctResult::SectionData::clearAllImages(void)
{
	d_ptr->images.clear();
	return;
}


OctScanSection & OctResult::SectionData::getDescript(void) const
{
	return d_ptr->descript;
}


void OctResult::SectionData::setRouteOfScan(OctRoute route)
{
	d_ptr->descript.route = route;
	return;
}


OctRoute OctResult::SectionData::getRouteOfScan(void) const
{
	return d_ptr->descript.route;
}


OctPoint & OctResult::SectionData::getStartPoint(void)
{
	return d_ptr->descript.route.getStartPoint();
}


OctPoint & OctResult::SectionData::getEndPoint(void)
{
	return d_ptr->descript.route.getEndPoint();
}


bool OctResult::SectionData::isCircle(void)
{
	return d_ptr->descript.route.isCircle();
}


bool OctResult::SectionData::exportFiles(std::wstring path, std::wstring prefix, std::wstring imageExt)
{
	int size = getImageCount();
	wstring name;

	for (int i = 0; i < size; i++) {
		SectionImage* image = getImage(i);
		if (image != nullptr) {
			if (size == 1) {
				name = (boost::wformat(L"%s//%s.%s") % path % prefix % imageExt).str();
			}
			else {
				name = (boost::wformat(L"%s//%s_%02d.%s") % path % prefix % i % imageExt).str();
			}

			if (!image->exportImage(name)) {
				return false;
			}
		}
	}
	return true;
}


SectionData::SectionDataImpl & OctResult::SectionData::getImpl(void) const
{
	return *d_ptr;
}
