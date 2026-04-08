#include "stdafx.h"
#include "RetinaImage.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct RetinaImage::RetinaImageImpl
{
	OctRetinaImage descript;
	CvImage image;

	RetinaImageImpl()
	{
	}
};


RetinaImage::RetinaImage() :
	d_ptr(make_unique<RetinaImageImpl>())
{
}


OctResult::RetinaImage::RetinaImage(const OctRetinaImage & desc) :
	d_ptr(make_unique<RetinaImageImpl>())
{
	setData(desc);
}


OctResult::RetinaImage::~RetinaImage() = default;
OctResult::RetinaImage::RetinaImage(RetinaImage && rhs) = default;
RetinaImage & OctResult::RetinaImage::operator=(RetinaImage && rhs) = default;


OctResult::RetinaImage::RetinaImage(const RetinaImage & rhs)
	: d_ptr(make_unique<RetinaImageImpl>(*rhs.d_ptr))
{
}


RetinaImage & OctResult::RetinaImage::operator=(const RetinaImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



void OctResult::RetinaImage::setData(const OctRetinaImage & desc)
{
	d_ptr->descript = desc;

	d_ptr->image.fromBitsData(desc._data, desc._width, desc._height);
	getImpl().descript._data = d_ptr->image.getBitsData();
	return;
}


const OctRetinaImage & OctResult::RetinaImage::getDescript(void) const
{
	auto descCopy = d_ptr->descript;
	descCopy._data = getBuffer();
	descCopy._width = getWidth();
	descCopy._height = getHeight();
	d_ptr->descript = descCopy;
	return d_ptr->descript;
}


std::uint8_t * OctResult::RetinaImage::getBuffer(void) const
{
	return d_ptr->image.getBitsData();
}


bool OctResult::RetinaImage::isEmpty(void) const
{
	return getImpl().image.isEmpty();
}


int OctResult::RetinaImage::getWidth(void) const
{
	return getImpl().image.getWidth();
}


int OctResult::RetinaImage::getHeight(void) const
{
	return getImpl().image.getHeight();
}


bool OctResult::RetinaImage::exportFile(std::wstring dirPath, std::wstring imageName)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % kImageExt).str();

	bool result = false;
	if (!isEmpty()) {
		result = d_ptr->image.saveFile(wtoa(path), false);
	}

	LogD() << "Export retina image, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::RetinaImage::importFile(std::wstring dirPath, std::wstring imageName)
{
	bool ret;

	ret = importFileWithExt(dirPath, imageName, kImageExt);
	if (!ret) {
		ret = importFileWithExt(dirPath, imageName, kImageExtOld);
	}

	return ret;
}

bool OctResult::RetinaImage::importFileWithExt(std::wstring dirPath, std::wstring imageName, std::wstring imageExt)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % imageExt).str();

	bool result = false;
	result = d_ptr->image.fromFile(path);

	LogD() << "Import retina image, path: " << wtoa(path) << " => " << result;
	return result;
}


RetinaImage::RetinaImageImpl & OctResult::RetinaImage::getImpl(void) const
{
	return *d_ptr;
}


