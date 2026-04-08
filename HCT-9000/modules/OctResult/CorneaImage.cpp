#include "stdafx.h"
#include "CorneaImage.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct CorneaImage::CorneaImageImpl
{
	OctCorneaImage descript;
	CvImage image;

	CorneaImageImpl()
	{
	}
};


CorneaImage::CorneaImage() :
	d_ptr(make_unique<CorneaImageImpl>())
{
}


OctResult::CorneaImage::CorneaImage(const OctCorneaImage & desc) :
	d_ptr(make_unique<CorneaImageImpl>())
{
	setData(desc);
}

OctResult::CorneaImage::~CorneaImage() = default;
OctResult::CorneaImage::CorneaImage(CorneaImage && rhs) = default;
CorneaImage & OctResult::CorneaImage::operator=(CorneaImage && rhs) = default;


OctResult::CorneaImage::CorneaImage(const CorneaImage & rhs)
	: d_ptr(make_unique<CorneaImageImpl>(*rhs.d_ptr))
{
}


CorneaImage & OctResult::CorneaImage::operator=(const CorneaImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::CorneaImage::setData(const OctCorneaImage & desc)
{
	d_ptr->descript = desc;

	d_ptr->image.fromBitsData(desc._data, desc._width, desc._height);
	getImpl().descript._data = d_ptr->image.getBitsData();
	return;
}


const OctCorneaImage & OctResult::CorneaImage::getDescript(void) const
{
	auto descCopy = d_ptr->descript;
	descCopy._data = getBuffer();
	descCopy._width = getWidth();
	descCopy._height = getHeight();
	d_ptr->descript = descCopy;
	return d_ptr->descript;
}


std::uint8_t * OctResult::CorneaImage::getBuffer(void) const
{
	return d_ptr->image.getBitsData();
}


bool OctResult::CorneaImage::isEmpty(void) const
{
	return getImpl().image.isEmpty();
}


int OctResult::CorneaImage::getWidth(void) const
{
	return getImpl().image.getWidth();
}


int OctResult::CorneaImage::getHeight(void) const
{
	return getImpl().image.getHeight();
}


bool OctResult::CorneaImage::exportFile(std::wstring dirPath, std::wstring imageName)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % kImageExt).str();

	bool result = false;
	if (!isEmpty()) {
		result = d_ptr->image.saveFile(wtoa(path), false);
	}

	LogD() << "Export cornea image, path: " << wtoa(path) << " => " << result;
	return false;
}


bool OctResult::CorneaImage::importFile(std::wstring dirPath, std::wstring imageName)
{
	bool ret;

	ret = importFileWithExt(dirPath, imageName, kImageExt);
	if (!ret) {
		ret = importFileWithExt(dirPath, imageName, kImageExtOld);
	}

	return ret;
}

bool OctResult::CorneaImage::importFileWithExt(std::wstring dirPath, std::wstring imageName,
	std::wstring imageExt)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % imageExt).str();

	bool result = false;
	result = d_ptr->image.fromFile(path);

	LogD() << "Import cornea image, path: " << wtoa(path) << " => " << result;
	return result;
}

CorneaImage::CorneaImageImpl & OctResult::CorneaImage::getImpl(void) const
{
	return *d_ptr;
}

