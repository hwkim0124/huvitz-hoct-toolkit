#include "stdafx.h"
#include "BscanImage2.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct BscanImage2::BscanImage2Impl
{
	CvImage image;
	OctScanImage descript;
	wstring filename;

	bool vertReversed = true; 

	BscanImage2Impl()
	{
	}
};


BscanImage2::BscanImage2() :
	d_ptr(make_unique<BscanImage2Impl>())
{
}


OctResult::BscanImage2::BscanImage2(const OctScanImage& desc) :
	d_ptr(make_unique<BscanImage2Impl>())
{
	setData(desc.getData(), desc.getWidth(), desc.getHeight(),
		desc.getQuality(), desc.getSignalRatio(), desc.getReferencePoint(), 
		desc.isReversed());
}


OctResult::BscanImage2::BscanImage2(std::uint8_t * data, int width, int height, float quality, float sigRatio, int refPoint) :
	d_ptr(make_unique<BscanImage2Impl>())
{
	setData(data, width, height, quality, sigRatio, refPoint);
}


OctResult::BscanImage2::~BscanImage2() = default;
OctResult::BscanImage2::BscanImage2(BscanImage2 && rhs) = default;
BscanImage2 & OctResult::BscanImage2::operator=(BscanImage2 && rhs) = default;


OctResult::BscanImage2::BscanImage2(const BscanImage2 & rhs)
	: d_ptr(make_unique<BscanImage2Impl>(*rhs.d_ptr))
{
}


BscanImage2 & OctResult::BscanImage2::operator=(const BscanImage2 & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::BscanImage2::setData(const std::uint8_t * data, int width, int height, float quality, float sigRatio, int refPoint, bool reverse)
{
	d_ptr->image.fromBitsData(data, width, height);
	// d_ptr->image.flipHorizontal();

	if (reverse) {
		d_ptr->image.flipHorizontal();
	}

	std::uint8_t* buff = d_ptr->image.getBitsData();

	d_ptr->descript.setup(buff, width, height, quality, sigRatio, refPoint);
	return;
}


void OctResult::BscanImage2::setData(const OctScanImage & desc)
{
	d_ptr->image.fromBitsData(desc._data, desc._width, desc._height);

	if (desc.isReversed()) {
		d_ptr->image.flipHorizontal();
	}

	d_ptr->descript = desc;
	d_ptr->descript.setReversed(false);

	getImpl().descript._data = d_ptr->image.getBitsData();
	return;
}


bool OctResult::BscanImage2::setImage(CppUtil::CvImage * image)
{
	if (image && !image->isEmpty()) {
		int width = image->getWidth();
		int height = image->getHeight();

		if (getWidth() == width && getHeight() == height) {
			d_ptr->image.fromBitsData(image->getBitsData(), width, height);
			return true;
		}
	}
	return false;
}


std::uint8_t * OctResult::BscanImage2::getBuffer(void) const
{
	return d_ptr->image.getBitsData();
}


const OctScanImage & OctResult::BscanImage2::getDescript(void) const
{
	auto descCopy = d_ptr->descript;
	descCopy._data = getBuffer();
	descCopy._width = getWidth();
	descCopy._height = getHeight();
	d_ptr->descript = descCopy;
	return d_ptr->descript;
}


CppUtil::CvImage & OctResult::BscanImage2::getImage(void) const
{
	return d_ptr->image;
}


std::wstring OctResult::BscanImage2::getFileName(bool path) const
{
	if (!path) {
		auto path = d_ptr->filename;
		auto npos = path.find_last_of('/');
		if (npos != wstring::npos) {
			return path.substr(npos + 1);
		}
		npos = path.find_last_of('\\');
		if (npos != wstring::npos) {
			return path.substr(npos + 1);
		}
	}
	return d_ptr->filename;
}


bool OctResult::BscanImage2::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}


int OctResult::BscanImage2::getWidth(void) const
{
	return d_ptr->image.getWidth();
}


int OctResult::BscanImage2::getHeight(void) const
{
	return d_ptr->image.getHeight();
}


float OctResult::BscanImage2::getQualityIndex(void) const
{
	return d_ptr->descript._quality;
}


float OctResult::BscanImage2::getSignalRatio(void) const
{
	return d_ptr->descript._sigRatio;
}


int OctResult::BscanImage2::getReferencePoint(void) const
{
	return d_ptr->descript._refPoint;
}


void OctResult::BscanImage2::setQualityIndex(float value)
{
	d_ptr->descript._quality = value;
	return;
}


void OctResult::BscanImage2::setSignalRatio(float value)
{
	d_ptr->descript._sigRatio = value;
	return;
}


void OctResult::BscanImage2::setReferencePoint(int value)
{
	d_ptr->descript._refPoint = value;
	return;
}


bool OctResult::BscanImage2::updateFile(void)
{
	if (!d_ptr->filename.empty()) {
		auto path = d_ptr->filename;
		if (d_ptr->image.saveFile(wtoa(path), false)) {
			return true;
		}
	}
	return false;
}


bool OctResult::BscanImage2::exportFile(const std::wstring& path)
{
	if (!isEmpty() && !path.empty()) {
		if (d_ptr->image.saveFile(wtoa(path), getImpl().vertReversed)) {
			d_ptr->filename = path;
			return true;
		}
	}
	return false;
}


bool OctResult::BscanImage2::importFile(const std::wstring & path)
{
	if (d_ptr->image.fromFile(path)) {
		d_ptr->filename = path;
		return true;
	}
	return false;
}


void OctResult::BscanImage2::flipVert(void)
{
	if (!isEmpty()) {
		d_ptr->image.flipVertical();
		d_ptr->vertReversed = !d_ptr->vertReversed;
	}
	return;
}


BscanImage2::BscanImage2Impl & OctResult::BscanImage2::getImpl(void) const
{
	return *d_ptr;
}
