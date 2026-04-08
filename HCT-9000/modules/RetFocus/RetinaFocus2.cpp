#include "stdafx.h"
#include "RetinaFocus2.h"
#include "RetinaFrame.h"

using namespace RetFocus;
using namespace std;


struct RetinaFocus2::RetinaFocus2Impl
{
	RetinaFrame retFrame;

	RetinaFocus2Impl()
	{
	}
};


RetinaFocus2::RetinaFocus2()
	: d_ptr(make_unique<RetinaFocus2Impl>())
{
}


RetFocus::RetinaFocus2::~RetinaFocus2() = default;
RetFocus::RetinaFocus2::RetinaFocus2(RetinaFocus2 && rhs) = default;
RetinaFocus2 & RetFocus::RetinaFocus2::operator=(RetinaFocus2 && rhs) = default;

RetinaFrame & RetFocus::RetinaFocus2::getRetinaFrame(void) const
{
	return d_ptr->retFrame;
}


void RetFocus::RetinaFocus2::setRetinaFrame(RetinaFrame && image)
{
	d_ptr->retFrame = std::move(image);
	return;
}


bool RetFocus::RetinaFocus2::isSplitFocus(void) const
{
	return d_ptr->retFrame.isSplitFocus();
}


bool RetFocus::RetinaFocus2::isUpperSplit(void) const
{
	return d_ptr->retFrame.isUpperSplit();
}


bool RetFocus::RetinaFocus2::isLowerSplit(void) const
{
	return d_ptr->retFrame.isLowerSplit();
}


float RetFocus::RetinaFocus2::getSplitOffset(void) const
{
	return d_ptr->retFrame.getSplitOffset();
}


bool RetFocus::RetinaFocus2::loadImage(const unsigned char * bits, int width, int height, int padding)
{
	bool res = d_ptr->retFrame.loadData(bits, width, height, padding);
	return res;
}


RetinaFocus2::RetinaFocus2Impl & RetFocus::RetinaFocus2::getImpl(void)
{
	return *d_ptr;
}

