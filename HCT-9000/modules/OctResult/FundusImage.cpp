#include "stdafx.h"
#include "FundusImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusImage::FundusImageImpl
{
	CvImage image;
	OctFundusImage descript;

	FundusImageImpl()
	{
	}
};


FundusImage::FundusImage() :
	d_ptr(make_unique<FundusImageImpl>())
{
}


OctResult::FundusImage::FundusImage(const OctFundusImage& desc) :
	d_ptr(make_unique<FundusImageImpl>())
{
	setData(desc.getBuffer(), desc.getWidth(), desc.getHeight());
}


OctResult::FundusImage::FundusImage(const std::uint8_t * data, int width, int height) :
	d_ptr(make_unique<FundusImageImpl>())
{
	setData(data, width, height);
}


OctResult::FundusImage::~FundusImage() = default;
OctResult::FundusImage::FundusImage(FundusImage && rhs) = default;
FundusImage & OctResult::FundusImage::operator=(FundusImage && rhs) = default;


OctResult::FundusImage::FundusImage(const FundusImage & rhs)
	: d_ptr(make_unique<FundusImageImpl>(*rhs.d_ptr))
{
}


FundusImage & OctResult::FundusImage::operator=(const FundusImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



void OctResult::FundusImage::setData(const std::uint8_t * data, int width, int height)
{
	d_ptr->image.fromRGB24(data, width, height);
	std::uint8_t* ptr1 = d_ptr->image.getBitsData();

	d_ptr->descript.setup(ptr1, width, height);
	return;
}


void OctResult::FundusImage::setData(const OctFundusImage & desc)
{
	d_ptr->descript = desc;
	d_ptr->image.fromRGB24(desc._data, desc._width, desc._height);
	d_ptr->descript._data = d_ptr->image.getBitsData();
	return;
}


const std::uint8_t * OctResult::FundusImage::getBuffer(void) const
{
	return d_ptr->image.getBitsData();
}


const OctFundusImage & OctResult::FundusImage::getDescript(void) const
{
	auto descCopy = d_ptr->descript;
	descCopy._data = getBuffer();
	descCopy._width = getWidth();
	descCopy._height = getHeight();
	d_ptr->descript = descCopy;
	return d_ptr->descript;
}


bool OctResult::FundusImage::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}


int OctResult::FundusImage::getWidth(void) const
{
	return d_ptr->image.getWidth();
}


int OctResult::FundusImage::getHeight(void) const
{
	return d_ptr->image.getHeight();
}


bool OctResult::FundusImage::exportFile(unsigned int imgQuality, const std::wstring& path)
{
	if (!isEmpty() && !path.empty()) {
		// Initialize CV image with data in buffer.
		// uint8_t* buff = &d_ptr->buffer[0];
		// d_ptr->image.fromRGB24(buff, getWidth(), getHeight());
		d_ptr->image.saveFile(wtoa(path), true, imgQuality);
		return true;
	}
	return false;
}


bool OctResult::FundusImage::exportThumbnail(const std::wstring& path, bool removeReflectionLight, float ratio)
{
	if (!isEmpty() && !path.empty()) {
		int thumb_w = (int)(getWidth() * ratio);
		int thumb_h = (int)(getHeight() * ratio);

		CvImage resized;
		if (removeReflectionLight) {
			d_ptr->image.applyRemoveReflectionLight(
				GlobalSettings::removeReflectionLight1(),
				GlobalSettings::removeReflectionLight2(),
				GlobalSettings::removeReflectionLight3()
			);
		}
		if (d_ptr->image.resizeTo(&resized, thumb_w, thumb_h)) {
			resized.saveFile(wtoa(path), true);
			return true;
		}
	}
	return false;
}


bool OctResult::FundusImage::importFile(const std::wstring & path)
{
	if (d_ptr->image.fromFile(path, false)) {
		return true;
	}
	return false;
}


FundusImage::FundusImageImpl & OctResult::FundusImage::getImpl(void) const
{
	return *d_ptr;
}


