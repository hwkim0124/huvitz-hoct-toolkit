#include "stdafx.h"
#include "SurfaceImage.h"
#include "CppUtil2.h"
#include "PatternOutput.h"

using namespace SegmScan;
using namespace CppUtil;
using namespace std;


struct SurfaceImage::SurfaceImageImpl
{
	CvImage srcImage;
	CvImage dispImage;
	float width;
	float height;
	PatternOutput* output;

	SurfaceImageImpl() {
	}
};


SurfaceImage::SurfaceImage() :
	d_ptr(make_unique<SurfaceImageImpl>())
{
}


SegmScan::SurfaceImage::SurfaceImage(float width, float height) :
	d_ptr(make_unique<SurfaceImageImpl>())
{
	setSurfaceRegion(width, height);
}


SegmScan::SurfaceImage::SurfaceImage(const std::wstring & path, float width, float height) :
	d_ptr(make_unique<SurfaceImageImpl>())
{
	loadFile(path);
	setSurfaceRegion(width, height);
}


SegmScan::SurfaceImage::SurfaceImage(const unsigned char * pBits, int rows, int cols, float width, float height) :
	d_ptr(make_unique<SurfaceImageImpl>())
{
	loadBitsData(pBits, cols, rows);
	setSurfaceRegion(width, height);
}


SegmScan::SurfaceImage::~SurfaceImage() = default;
SegmScan::SurfaceImage::SurfaceImage(SurfaceImage && rhs) = default;
SurfaceImage & SegmScan::SurfaceImage::operator=(SurfaceImage && rhs) = default;


SegmScan::SurfaceImage::SurfaceImage(const SurfaceImage & rhs)
	: d_ptr(make_unique<SurfaceImageImpl>(*rhs.d_ptr))
{
}


SurfaceImage & SegmScan::SurfaceImage::operator=(const SurfaceImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


SurfaceImage::SurfaceImageImpl & SegmScan::SurfaceImage::getImpl(void) const
{
	return *d_ptr;
}


void SegmScan::SurfaceImage::setPatternOutput(PatternOutput * output)
{
	d_ptr->output = output;
	return;
}


PatternOutput * SegmScan::SurfaceImage::getPatternOutput(void) const
{
	return d_ptr->output;
}


Gdiplus::Bitmap * SegmScan::SurfaceImage::getBitmap(void) const
{
	return getImpl().dispImage.getBitmap();
}


bool SegmScan::SurfaceImage::loadFile(const std::wstring & path)
{
	return getImpl().srcImage.fromFile(path) ;
}


bool SegmScan::SurfaceImage::loadBitsData(const unsigned char * pBits, int width, int height, int padding)
{
	return getImpl().srcImage.fromBitsData(pBits, width, height, padding) ;
}


bool SegmScan::SurfaceImage::isEmpty(void) const
{
	return getImpl().srcImage.isEmpty();
}


void SegmScan::SurfaceImage::setSurfaceRegion(float width, float height)
{
	getImpl().width = width;
	getImpl().height = height;
	return;
}


void SegmScan::SurfaceImage::setDisplaySize(int wpix, int hpix)
{
	if (isEmpty() != true) {
		getImpl().srcImage.resizeTo(&getImpl().dispImage, wpix, hpix);
	}
	return;
}


int SegmScan::SurfaceImage::getWidth(void) const
{
	return getImpl().dispImage.getWidth();
}


int SegmScan::SurfaceImage::getHeight(void) const
{
	return getImpl().dispImage.getHeight();
}


float SegmScan::SurfaceImage::getRegionWidth(void) const
{
	return getImpl().width;
}


float SegmScan::SurfaceImage::getRegionHeight(void) const
{
	return getImpl().height;
}


BscanResult * SegmScan::SurfaceImage::getBscanResultFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr) {
		return false;
	}

	int index = p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
	if (index >= 0) {
		BscanResult* p = getPatternOutput()->getResult(index);
		return p;
	}
	return nullptr;
}


BscanImage * SegmScan::SurfaceImage::getBscanImageFromPosition(int posX, int posY) const
{
	BscanResult* p = getBscanResultFromPosition(posX, posY);
	if (p != nullptr) {
		return p->getImage();
	}
	return nullptr;
}


int SegmScan::SurfaceImage::getBscanIndexFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr) {
		return false;
	}

	return p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
}


bool SegmScan::SurfaceImage::getBscanStartPosition(int resultIdx, int * posX, int * posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr) {
		return false;
	}

	return p->getBscanStartPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


bool SegmScan::SurfaceImage::getBscanEndPosition(int resultIdx, int * posX, int * posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr) {
		return false;
	}

	return p->getBscanEndPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


float SegmScan::SurfaceImage::getSurfaceWidth(void) const
{
	return getImpl().width;
}


float SegmScan::SurfaceImage::getSurfaceHeight(void) const
{
	return getImpl().height;
}


