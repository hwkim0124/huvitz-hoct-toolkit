#include "stdafx.h"
#include "FundusFrame.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusFrame::FundusFrameImpl
{
	CvImage frame;
	OctFundusFrame descript;

	FundusFrameImpl()
	{
	}
};


FundusFrame::FundusFrame() :
	d_ptr(make_unique<FundusFrameImpl>())
{
}


OctResult::FundusFrame::FundusFrame(const OctFundusFrame& desc) :
	d_ptr(make_unique<FundusFrameImpl>())
{
	setData(desc.getBuffer(), desc.getWidth(), desc.getHeight());
}


OctResult::FundusFrame::FundusFrame(const std::uint8_t * data, int width, int height) :
	d_ptr(make_unique<FundusFrameImpl>())
{
	setData(data, width, height);
}


OctResult::FundusFrame::~FundusFrame() = default;
OctResult::FundusFrame::FundusFrame(FundusFrame && rhs) = default;
FundusFrame & OctResult::FundusFrame::operator=(FundusFrame && rhs) = default;


OctResult::FundusFrame::FundusFrame(const FundusFrame & rhs)
	: d_ptr(make_unique<FundusFrameImpl>(*rhs.d_ptr))
{
}


FundusFrame & OctResult::FundusFrame::operator=(const FundusFrame & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}



void OctResult::FundusFrame::setData(const std::uint8_t * data, int width, int height)
{
	d_ptr->frame.fromRaw16(data, width, height);
	std::uint8_t* ptr1 = d_ptr->frame.getBitsData();

	d_ptr->descript.setup(ptr1, width, height);
	return;
}


void OctResult::FundusFrame::setData(const OctFundusFrame & desc)
{
	d_ptr->descript = desc;
	d_ptr->frame.fromRGB24(desc._data, desc._width, desc._height);
	d_ptr->descript._data = d_ptr->frame.getBitsData();
	return;
}


const std::uint8_t * OctResult::FundusFrame::getBuffer(void) const
{
	return d_ptr->descript.getBuffer();
}


const OctFundusFrame & OctResult::FundusFrame::getDescript(void) const
{
	auto descCopy = d_ptr->descript;
	descCopy._data = getBuffer();
	descCopy._width = getWidth();
	descCopy._height = getHeight();
	d_ptr->descript = descCopy;
	return d_ptr->descript;
}


bool OctResult::FundusFrame::isEmpty(void) const
{
	return d_ptr->frame.isEmpty();
}


int OctResult::FundusFrame::getWidth(void) const
{
	return d_ptr->frame.getWidth();
}


int OctResult::FundusFrame::getHeight(void) const
{
	return d_ptr->frame.getHeight();
}


bool OctResult::FundusFrame::exportFile(const std::wstring& path)
{
	if (!isEmpty() && !path.empty()) {
		// d_ptr->frame.flipVertical();
		d_ptr->frame.saveFileAsBinary(wtoa(path));
		return true;
	}
	return false;
}


bool OctResult::FundusFrame::importFile(const std::wstring & path)
{
	/*
	if (d_ptr->frame.fromFile(path)) {
		return true;
	}
	*/
	return false;
}


FundusFrame::FundusFrameImpl & OctResult::FundusFrame::getImpl(void) const
{
	return *d_ptr;
}


