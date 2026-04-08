#include "stdafx.h"
#include "MacularImage.h"
#include "RetParam2.h"
#include "CppUtil2.h"
#include "PatternOutput.h"


using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularImage::MacularImageImpl
{
	EnfaceDraw draw;
	CvImage image;
	PatternOutput* output;

	MacularImageImpl(PatternOutput* p) : output(p) {

	}
};


MacularImage::MacularImage(PatternOutput* output) : 
	d_ptr(make_unique<MacularImageImpl>(output))
{
}


SegmScan::MacularImage::~MacularImage() = default;
SegmScan::MacularImage::MacularImage(MacularImage && rhs) = default;
MacularImage & SegmScan::MacularImage::operator=(MacularImage && rhs) = default;


SegmScan::MacularImage::MacularImage(const MacularImage & rhs)
	: d_ptr(make_unique<MacularImageImpl>(*rhs.d_ptr))
{
}


MacularImage & SegmScan::MacularImage::operator=(const MacularImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CppUtil::CvImage * SegmScan::MacularImage::getImage(void) const
{
	return &(d_ptr->image);
}


RetParam::EnfaceDraw * SegmScan::MacularImage::getDraw(void) const
{
	return &(d_ptr->draw);
}


void SegmScan::MacularImage::setPatternOutput(PatternOutput * output)
{
	d_ptr->output = output;
	return;
}


PatternOutput * SegmScan::MacularImage::getPatternOutput(void) const
{
	return d_ptr->output;
}


bool SegmScan::MacularImage::update(int width, int height, LayerType upper, LayerType lower)
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	auto bscans = p->getSegmImages();
	auto uppers = p->getSegmLayers(upper);
	auto lowers = p->getSegmLayers(lower);
	auto wScan = p->getDescript()->getWidth();
	auto hScan = p->getDescript()->getHeight();

	if (getDraw()->initialize(bscans, uppers, lowers, wScan, hScan) == false) {
		return false;
	}

	return getDraw()->drawImage(getImage(), width, height);
}


Gdiplus::Bitmap * SegmScan::MacularImage::getBitmap(void) const
{
	return getImage()->getBitmap();
}


std::vector<unsigned char> SegmScan::MacularImage::copyData(void) const
{
	return getImage()->copyDataInBytes();
}


unsigned char SegmScan::MacularImage::getAt(int x, int y) const
{
	return getImage()->getAtInByte(x, y);
}


BscanResult * SegmScan::MacularImage::getBscanResultFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	int index = p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
	if (index >= 0) {
		BscanResult* p = getPatternOutput()->getResult(index);
		return p;
	}
	return nullptr;
}


BscanImage * SegmScan::MacularImage::getBscanImageFromPosition(int posX, int posY) const
{
	BscanResult* p = getBscanResultFromPosition(posX, posY);
	if (p != nullptr) {
		return p->getImage();
	}
	return nullptr;
}


int SegmScan::MacularImage::getBscanIndexFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
}


bool SegmScan::MacularImage::getBscanStartPosition(int resultIdx, int* posX, int* posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanStartPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


bool SegmScan::MacularImage::getBscanEndPosition(int resultIdx, int* posX, int* posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanEndPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


int SegmScan::MacularImage::getWidth(void) const
{
	return getImage()->getWidth();
}


int SegmScan::MacularImage::getHeight(void) const
{
	return getImage()->getHeight();
}


bool SegmScan::MacularImage::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}
