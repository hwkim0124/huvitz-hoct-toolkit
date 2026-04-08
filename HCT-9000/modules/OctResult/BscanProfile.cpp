#include "stdafx.h"
#include "BscanProfile.h"
#include "BscanImage2.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct BscanProfile::BscanProfileImpl
{
	vector<BscanImage2> images;
	OctScanSection descript;

	BscanProfileImpl()
	{
	}
};


BscanProfile::BscanProfile() :
	d_ptr(make_unique<BscanProfileImpl>())
{
}


OctResult::BscanProfile::BscanProfile(OctRoute route) :
	d_ptr(make_unique<BscanProfileImpl>())
{
	d_ptr->descript.route = route;
}


OctResult::BscanProfile::BscanProfile(OctScanSection desc)
{
	d_ptr->descript = desc;
}


OctResult::BscanProfile::~BscanProfile() = default;
OctResult::BscanProfile::BscanProfile(BscanProfile && rhs) = default;
BscanProfile & OctResult::BscanProfile::operator=(BscanProfile && rhs) = default;


OctResult::BscanProfile::BscanProfile(const BscanProfile & rhs)
	: d_ptr(make_unique<BscanProfileImpl>(*rhs.d_ptr))
{
}


BscanProfile & OctResult::BscanProfile::operator=(const BscanProfile & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::BscanProfile::setImage(BscanImage2 & image)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(image);
}


int OctResult::BscanProfile::setImage(BscanImage2 && image)
{
	if (getImageCount() > 0) {
		clearAllImages();
	}
	return addImage(image);
}


int OctResult::BscanProfile::addImage(BscanImage2 & image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


int OctResult::BscanProfile::addImage(BscanImage2 && image)
{
	d_ptr->images.push_back(image);
	int size = getImageCount();
	return size;
}


BscanImage2 * OctResult::BscanProfile::getImage(int index)
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}

	BscanImage2* p = &d_ptr->images[index];
	return p;
}


int OctResult::BscanProfile::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


void OctResult::BscanProfile::clearAllImages(void)
{
	d_ptr->images.clear();
	return;
}


OctScanSection & OctResult::BscanProfile::getDescript(void) const
{
	return d_ptr->descript;
}


void OctResult::BscanProfile::setRouteOfScan(OctRoute route)
{
	d_ptr->descript.route = route;
	return;
}


OctRoute OctResult::BscanProfile::getRouteOfScan(void) const
{
	return d_ptr->descript.route;
}


OctPoint & OctResult::BscanProfile::getStartPoint(void)
{
	return d_ptr->descript.route.getStartPoint();
}


OctPoint & OctResult::BscanProfile::getEndPoint(void)
{
	return d_ptr->descript.route.getEndPoint();
}


bool OctResult::BscanProfile::isCircle(void)
{
	return d_ptr->descript.route.isCircle();
}


bool OctResult::BscanProfile::exportFiles(std::wstring path, std::wstring prefix, std::wstring imageExt)
{
	int size = getImageCount();
	wstring name;

	for (int i = 0; i < size; i++) {
		BscanImage2* image = getImage(i);
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


BscanProfile::BscanProfileImpl & OctResult::BscanProfile::getImpl(void) const
{
	return *d_ptr;
}
