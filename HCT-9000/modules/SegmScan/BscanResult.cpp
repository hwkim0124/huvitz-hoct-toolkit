#include "stdafx.h"
#include "BscanResult.h"
#include "BscanImage.h"
#include "ScanRange.h"

using namespace SegmScan;
using namespace std;


struct BscanResult::BscanResultImpl
{
	BscanImageVect images;
	ScanRange range;
};


BscanResult::BscanResult() :
	d_ptr(make_unique<BscanResultImpl>())
{
}


SegmScan::BscanResult::BscanResult(ScanRange & range) :
	d_ptr(make_unique<BscanResultImpl>())
{
	setRange(range);
}


SegmScan::BscanResult::BscanResult(const std::wstring& path) :
	d_ptr(make_unique<BscanResultImpl>())
{
	addImage(path);
}


SegmScan::BscanResult::BscanResult(BscanImage & image) :
	d_ptr(make_unique<BscanResultImpl>())
{
	addImage(image);
}


SegmScan::BscanResult::BscanResult(ScanRange & range, const std::wstring & path) :
	d_ptr(make_unique<BscanResultImpl>())
{
	setRange(range);
	addImage(path);
}


SegmScan::BscanResult::~BscanResult() = default;
SegmScan::BscanResult::BscanResult(BscanResult && rhs) = default;
BscanResult & SegmScan::BscanResult::operator=(BscanResult && rhs) = default;


SegmScan::BscanResult::BscanResult(const BscanResult & rhs)
	: d_ptr(make_unique<BscanResultImpl>(*rhs.d_ptr))
{
}


BscanResult & SegmScan::BscanResult::operator=(const BscanResult & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


ScanRange * SegmScan::BscanResult::getRange(void) const
{
	return &(d_ptr->range);
}


void SegmScan::BscanResult::setRange(ScanRange & range)
{
	d_ptr->range = std::move(range);
	return;
}


void SegmScan::BscanResult::addImage(BscanImage & image)
{
	d_ptr->images.push_back(std::move(image));
	return;
}


void SegmScan::BscanResult::addImage(BscanImage && image)
{
	d_ptr->images.push_back(image);
	return;
}


void SegmScan::BscanResult::addImage(const std::wstring & path)
{
	d_ptr->images.push_back({ path });
	return;
}


BscanImage * SegmScan::BscanResult::getImage(int index) const
{
	if (index < 0 || index >= getImageCount()) {
		return nullptr;
	}
	return &(d_ptr->images[index]);
}


int SegmScan::BscanResult::getImageCount(void) const
{
	return (int)d_ptr->images.size();
}


bool SegmScan::BscanResult::isEmpty(void) const
{
	if (getImageCount() == 0 || getImage()->isEmpty()) {
		return true;
	}
	return false;
}