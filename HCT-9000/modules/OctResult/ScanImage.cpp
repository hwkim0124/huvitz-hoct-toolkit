#include "stdafx.h"
#include "ScanImage.h"

#include <vector>
#include <fstream>
#include <cereal\archives\binary.hpp>

#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct ScanImage::ScanImageImpl
{
	vector<unsigned char> data;
	int width;
	int height;
	
	float quality;
	int referPoint;

	OctPath2 path;

	ScanImageImpl()
	{
	}
};


ScanImage::ScanImage() :
	d_ptr(make_unique<ScanImageImpl>())
{
}


OctResult::ScanImage::~ScanImage() = default;
OctResult::ScanImage::ScanImage(ScanImage && rhs) = default;
ScanImage & OctResult::ScanImage::operator=(ScanImage && rhs) = default;


OctResult::ScanImage::ScanImage(const ScanImage & rhs)
	: d_ptr(make_unique<ScanImageImpl>(*rhs.d_ptr))
{
}


ScanImage & OctResult::ScanImage::operator=(const ScanImage & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::ScanImage::copyData(unsigned char * data, int width, int height)
{
	getImpl().data.assign(data, data + width * height);
	return;
}


void OctResult::ScanImage::set_quality(float quality)
{
	getImpl().quality = quality;
	return;
}


void OctResult::ScanImage::set_referPoint(int refPoint)
{
	getImpl().referPoint = refPoint;
	return;
}


void OctResult::ScanImage::set_path(float x1, float y1, float x2, float y2)
{
	getImpl().path.set(x1, y1, x2, y2);
	return;
}


void OctResult::ScanImage::set_path(float x, float y, float radius)
{
	getImpl().path.set(x, y, radius);
	return;
}


void OctResult::ScanImage::clear(void)
{
	getImpl().data.clear();
	getImpl().width = 0;
	getImpl().height = 0;
	set_quality(0.0f);
	set_referPoint(0);
	return;
}


std::uint8_t * OctResult::ScanImage::data(void) const
{
	return &(getImpl().data[0]);
}


int OctResult::ScanImage::width(void) const
{
	return getImpl().width;
}


int OctResult::ScanImage::height(void) const
{
	return getImpl().height;
}


float OctResult::ScanImage::quality(void) const
{
	return getImpl().quality;
}


int OctResult::ScanImage::referPoint(void) const
{
	return getImpl().referPoint;
}


bool OctResult::ScanImage::empty(void) const
{
	return getImpl().data.empty();
}


ScanImage::ScanImageImpl & OctResult::ScanImage::getImpl(void) const
{
	return *d_ptr;
}
