#include "stdafx.h"
#include "ScanResult.h"
#include "ScanImage.h"
#include "ScanProfile.h"

#include <vector>

#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct ScanResult::ScanResultImpl
{
	vector<ScanImage> images;
	ScanProfile profile;

	ScanResultImpl()
	{
	}
};


ScanResult::ScanResult() :
	d_ptr(make_unique<ScanResultImpl>())
{
}


OctResult::ScanResult::~ScanResult() = default;
OctResult::ScanResult::ScanResult(ScanResult && rhs) = default;
ScanResult & OctResult::ScanResult::operator=(ScanResult && rhs) = default;


OctResult::ScanResult::ScanResult(const ScanResult & rhs)
	: d_ptr(make_unique<ScanResultImpl>(*rhs.d_ptr))
{
}


ScanResult & OctResult::ScanResult::operator=(const ScanResult & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::ScanResult::clear(void)
{
	getImpl().images.clear();
	return;
}


void OctResult::ScanResult::addImage(ScanImage && image)
{
	getImpl().images.push_back(image);
	return;
}


ScanImage * OctResult::ScanResult::image(int index)
{
	if (index < 0 || index >= imageCount()) {
		return nullptr;
	}
	return &getImpl().images[index];
}


int OctResult::ScanResult::imageCount(void) const
{
	return (int)getImpl().images.size();
}


void OctResult::ScanResult::reserveImages(int count)
{
	getImpl().images.reserve(count);
	return;
}


void OctResult::ScanResult::set_profile(ScanProfile && profile)
{
	getImpl().profile = profile;
	return;
}


ScanProfile & OctResult::ScanResult::profile(void) const
{
	return getImpl().profile;
}


ScanResult::ScanResultImpl & OctResult::ScanResult::getImpl(void) const
{
	return *d_ptr;
}
