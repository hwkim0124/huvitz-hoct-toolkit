#include "stdafx.h"
#include "SectionImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct SectionImage::SectionImageImpl
{
	vector<uint8_t> buffer;

	CvImage image;
	OctScanImage descript;

	SectionImageImpl() 
	{
	}
};


SectionImage::SectionImage() :
	d_ptr(make_unique<SectionImageImpl>())
{
}


OctResult::SectionImage::SectionImage(OctScanImage desc) :
	d_ptr(make_unique<SectionImageImpl>())
{
	setData(desc.getData(), desc.getWidth(), desc.getHeight(),
		desc.getQuality(), desc.getSignalRatio(), desc.getReferencePoint());
}


OctResult::SectionImage::SectionImage(std::uint8_t * data, int width, int height, float quality, float sigRatio, int refPoint) :
	d_ptr(make_unique<SectionImageImpl>())
{
	setData(data, width, height, quality, sigRatio, refPoint);
}


OctResult::SectionImage::~SectionImage() = default;
OctResult::SectionImage::SectionImage(SectionImage && rhs) = default;
SectionImage & OctResult::SectionImage::operator=(SectionImage && rhs) = default;


OctResult::SectionImage::SectionImage(const SectionImage & rhs)
	: d_ptr(make_unique<SectionImageImpl>(*rhs.d_ptr))
{
}


SectionImage & OctResult::SectionImage::operator=(const SectionImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::SectionImage::setData(const std::uint8_t * data, int width, int height, float quality, float sigRatio, int refPoint)
{
	int size = width * height;
	d_ptr->buffer.assign(data, data + size);

	// Initialize CV image with data in buffer.
	uint8_t* buff = &d_ptr->buffer[0];
	d_ptr->image.fromBitsData(buff, width, height);

	d_ptr->descript.data = buff;
	d_ptr->descript.width = width;
	d_ptr->descript.height = height;

	d_ptr->descript.set(buff, width, height, quality, sigRatio, refPoint);
	return;
}


const std::uint8_t * OctResult::SectionImage::getData(void) const
{
	return d_ptr->descript.data;
}


OctScanImage & OctResult::SectionImage::getDescript(void) const
{
	return d_ptr->descript;
}


bool OctResult::SectionImage::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}


int OctResult::SectionImage::getWidth(void) const
{
	return d_ptr->descript.width;
}


int OctResult::SectionImage::getHeight(void) const
{
	return d_ptr->descript.height;
}


float OctResult::SectionImage::getQualityIndex(void) const
{
	return d_ptr->descript.quality;
}


float OctResult::SectionImage::getSignalRatio(void) const
{
	return d_ptr->descript.sigRatio;
}


int OctResult::SectionImage::getReferencePoint(void) const
{
	return d_ptr->descript.refPoint;
}


void OctResult::SectionImage::setQualityIndex(float value)
{
	d_ptr->descript.quality = value;
	return;
}


void OctResult::SectionImage::setSignalRatio(float value)
{
	d_ptr->descript.sigRatio = value;
	return;
}


void OctResult::SectionImage::setReferencePoint(int value)
{
	d_ptr->descript.refPoint = value;
	return;
}


bool OctResult::SectionImage::exportImage(std::wstring path)
{
	if (!isEmpty() && !path.empty()) {
		return d_ptr->image.saveFile(wtoa(path));
	}
	return false;
}


SectionImage::SectionImageImpl & OctResult::SectionImage::getImpl(void) const
{
	return *d_ptr;
}
