#include "stdafx.h"
#include "FundusBundle.h"
#include "FundusSection.h"
#include "FundusImage.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusBundle::FundusBundleImpl
{
	std::map<int, unique_ptr<FundusSection>> sections;

	FundusBundleImpl()
	{
	}
};


FundusBundle::FundusBundle() :
	d_ptr(make_unique<FundusBundleImpl>())
{
}


OctResult::FundusBundle::~FundusBundle() = default;
OctResult::FundusBundle::FundusBundle(FundusBundle && rhs) = default;
FundusBundle & OctResult::FundusBundle::operator=(FundusBundle && rhs) = default;

/*
OctResult::FundusBundle::FundusBundle(const FundusBundle & rhs) :
	d_ptr(make_unique<FundusBundleImpl>())
{
}


FundusBundle & OctResult::FundusBundle::operator=(const FundusBundle & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


FundusImage * OctResult::FundusBundle::getSectionImage(int sectIdx, int imageIdx)
{
	FundusSection* pSect = getSection(sectIdx);
	if (pSect != nullptr) {
		return pSect->getImage(imageIdx);
	}
	return nullptr;
}


FundusFrame * OctResult::FundusBundle::getSectionFrame(int sectIdx, int frameIdx)
{
	FundusSection* pSect = getSection(sectIdx);
	if (pSect != nullptr) {
		return pSect->getFrame(frameIdx);
	}
	return nullptr;
}


const OctFundusImage * OctResult::FundusBundle::getSectionImageDescript(int sectIdx, int imageIdx)
{
	FundusImage* pImage = getSectionImage(sectIdx, imageIdx);
	if (pImage != nullptr) {
		return &pImage->getDescript();
	}
	return nullptr;
}


bool OctResult::FundusBundle::addSectionImage(const OctFundusSection & section, const OctFundusImage & image, const OctFundusFrame & frame)
{
	FundusSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	pSect->addImage(image);
	pSect->addFrame(frame);
	return true;
}


bool OctResult::FundusBundle::addSectionImage(const OctFundusSection & section, const OctFundusImage & image)
{
	FundusSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->addImage(image);
}


bool OctResult::FundusBundle::addSectionImage(const OctFundusSection & section, const std::wstring & path)
{
	FundusSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}
	return pSect->importImage(path);
}


bool OctResult::FundusBundle::addSection(const OctFundusSection & section)
{
	int index = section.getIndex();
	if (index >= 0) {
		d_ptr->sections[index] = make_unique<FundusSection>(section);
		return true;
	}
	return false;
}


FundusSection * OctResult::FundusBundle::getSection(int index)
{
	auto iter = d_ptr->sections.find(index);
	if (iter != end(d_ptr->sections)) {
		return iter->second.get();
	}
	return nullptr;
}


FundusSection * OctResult::FundusBundle::getSectionLast(void)
{
	auto iter = rbegin(d_ptr->sections);
	if (iter != rend(d_ptr->sections)) {
		return iter->second.get();
	}
	return nullptr;
}


FundusSection * OctResult::FundusBundle::makeSection(const OctFundusSection & desc)
{
	int index = desc.getIndex();
	FundusSection* p = getSection(index);
	if (p == nullptr) {
		if (addSection(desc)) {
			p = getSection(index);
		}
	}
	return p;
}


int OctResult::FundusBundle::getSectionCount(void) const
{
	return (int)d_ptr->sections.size();
}


std::vector<int> OctResult::FundusBundle::getSectionIndexList(void) const
{
	vector<int> list;
	for (auto iter = d_ptr->sections.begin(); iter != d_ptr->sections.end(); iter++) {
		list.push_back(iter->first);
	}
	return list;
}


void OctResult::FundusBundle::clearAllSections(void)
{
	d_ptr->sections.clear();
	return;
}


FundusBundle::FundusBundleImpl & OctResult::FundusBundle::getImpl(void) const
{
	return *d_ptr;
}
