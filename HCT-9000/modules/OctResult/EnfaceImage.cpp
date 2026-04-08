#include "stdafx.h"
#include "EnfaceImage.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct EnfaceImage::EnfaceImageImpl
{
	OctEnfaceImage descript;
	CvImage image;

	EnfaceImageImpl()
	{
	}
};


EnfaceImage::EnfaceImage() :
	d_ptr(make_unique<EnfaceImageImpl>())
{
}


OctResult::EnfaceImage::EnfaceImage(const OctEnfaceImage & desc) :
	d_ptr(make_unique<EnfaceImageImpl>())
{
	setData(desc);
}


OctResult::EnfaceImage::~EnfaceImage() = default;
OctResult::EnfaceImage::EnfaceImage(EnfaceImage && rhs) = default;
EnfaceImage & OctResult::EnfaceImage::operator=(EnfaceImage && rhs) = default;


OctResult::EnfaceImage::EnfaceImage(const EnfaceImage & rhs)
	: d_ptr(make_unique<EnfaceImageImpl>(*rhs.d_ptr))
{
}


EnfaceImage & OctResult::EnfaceImage::operator=(const EnfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::EnfaceImage::setData(const OctEnfaceImage & desc)
{
	d_ptr->descript = desc;
	d_ptr->image.fromBitsData(desc._data, desc._width, desc._height);
	return;
}


std::uint8_t * OctResult::EnfaceImage::getBuffer(void) const
{
	return d_ptr->image.getBitsData();
}


const OctEnfaceImage& OctResult::EnfaceImage::getDescript(void) const
{
	d_ptr->descript.setup(getBuffer(), getWidth(), getHeight());
	return d_ptr->descript;
}


bool OctResult::EnfaceImage::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}


int OctResult::EnfaceImage::getWidth(void) const
{
	return d_ptr->image.getWidth();
}


int OctResult::EnfaceImage::getHeight(void) const
{
	return d_ptr->image.getHeight();
}


bool OctResult::EnfaceImage::exportFile(const std::wstring& dirPath, const std::wstring& imageName,
										int width, int height)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % kImageExt).str();
	
	bool result = false;
	if (!isEmpty()) {
		CvImage resized;
		if (d_ptr->image.resizeTo(&resized, width, height)) {
			result = resized.saveFile(wtoa(path), true);
		}
	}

	LogD() << "Export enface image, path: " << wtoa(path) << " => " << result;
	return result;
}


bool OctResult::EnfaceImage::importFile(const std::wstring& dirPath, const std::wstring& imageName)
{
	bool ret;

	ret = importFileWithExt(dirPath, imageName, kImageExt);
	if (!ret) {
		ret = importFileWithExt(dirPath, imageName, kImageExtOld);
	}

	return ret;
}

bool OctResult::EnfaceImage::importFileWithExt(const std::wstring& dirPath,
	const std::wstring& imageName, const std::wstring& imageExt)
{
	wstring path = (boost::wformat(L"%s//%s.%s") % dirPath % imageName % imageExt).str();

	bool result = false;
	result = d_ptr->image.fromFile(path);

	LogD() << "Import enface image, path: " << wtoa(path) << " => " << result;
	return result;
}

bool OctResult::EnfaceImage::processImage(void)
{
	if (!isEmpty()) {
		d_ptr->image.equalizeHistogram(SCAN_ENFACE_CLIP_LIMIT);
		//d_ptr->image.resize(256, 256);
		return true;
	}
	return false;
}


EnfaceImage::EnfaceImageImpl & OctResult::EnfaceImage::getImpl(void) const
{
	return *d_ptr;
}
