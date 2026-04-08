#include "stdafx.h"
#include "BscanBundle.h"
#include "BscanSection.h"
#include "BscanImage2.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct BscanBundle::BscanBundleImpl
{
	std::map<int, unique_ptr<BscanSection>> sections;
	OctScanPattern descript;

	BscanBundleImpl()
	{
	}
};


BscanBundle::BscanBundle() :
	d_ptr(make_unique<BscanBundleImpl>())
{
}


OctResult::BscanBundle::BscanBundle(const OctScanPattern & desc) :
	d_ptr(make_unique<BscanBundleImpl>())
{
	d_ptr->descript = desc;
}


OctResult::BscanBundle::~BscanBundle() = default;
OctResult::BscanBundle::BscanBundle(BscanBundle && rhs) = default;
BscanBundle & OctResult::BscanBundle::operator=(BscanBundle && rhs) = default;

/*
OctResult::BscanBundle::BscanBundle(const BscanBundle & rhs) :
	d_ptr(make_unique<BscanBundleImpl>())
{
}


BscanBundle & OctResult::BscanBundle::operator=(const BscanBundle & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}
*/


void OctResult::BscanBundle::setDescript(const OctScanPattern & desc)
{
	getImpl().descript = desc;
	return;
}


OctScanPattern & OctResult::BscanBundle::getDescript(void) const
{
	return getImpl().descript;
}


BscanImage2 * OctResult::BscanBundle::getSectionImage(int sectIdx, int imageIdx)
{
	BscanSection* pSect = getSection(sectIdx);
	if (pSect != nullptr) {
		return pSect->getImage(imageIdx);
	}
	return nullptr;
}


int OctResult::BscanBundle::getSectionImageCount(int sectIdx)
{
	BscanSection* pSect = getSection(sectIdx);
	if (pSect != nullptr) {
		return pSect->getImageCount();
	}
	return 0;
}


const OctScanImage * OctResult::BscanBundle::getSectionImageDescript(int sectIdx, int imageIdx)
{
	BscanImage2* pImage = getSectionImage(sectIdx, imageIdx);
	if (pImage != nullptr) {
		return &pImage->getDescript();
	}
	return nullptr;
}


bool OctResult::BscanBundle::addSectionImage(const OctScanSection & section, const OctScanImage & image, int sectSize)
{
	BscanSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	if (pSect->getImageCount() >= sectSize) {
		pSect->removeImage(0);
	}
	return pSect->addImage(image);
}


bool OctResult::BscanBundle::addSectionImage(const OctScanSection & section, const OctScanImage & image)
{
	BscanSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->addImage(image);
}


bool OctResult::BscanBundle::addSectionImage(const OctScanSection & section, const std::wstring& path)
{
	BscanSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->importImage(path, true);
}


bool OctResult::BscanBundle::setSectionImage(const OctScanSection & section, const OctScanImage & image)
{
	BscanSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->setImage(image);
}


bool OctResult::BscanBundle::setSectionImage(const OctScanSection & section, const std::wstring & path)
{
	BscanSection* pSect = makeSection(section);
	if (pSect == nullptr) {
		return false;
	}

	return pSect->importImage(path, false);
}


bool OctResult::BscanBundle::addSection(const OctScanSection & sect)
{
	int index = sect.getIndex();
	if (index >= 0) {
		d_ptr->sections[index] = make_unique<BscanSection>(sect);
		return true;
	}
	return false;
}


BscanSection * OctResult::BscanBundle::getSection(int index)
{
	auto iter = d_ptr->sections.find(index);
	if (iter != end(d_ptr->sections)) {
		return iter->second.get();
	}
	return nullptr;
}


BscanSection * OctResult::BscanBundle::getSectionLast(void)
{
	auto iter = rbegin(d_ptr->sections);
	if (iter != rend(d_ptr->sections)) {
		return iter->second.get();
	}
	return nullptr;
}


BscanSection * OctResult::BscanBundle::makeSection(const OctScanSection & desc)
{
	int index = desc.getIndex();
	BscanSection* p = getSection(index);
	if (p == nullptr) {
		if (addSection(desc)) {
			p = getSection(index);
		}
	}
	return p;
}


int OctResult::BscanBundle::getSectionCount(void) const
{
	return (int)d_ptr->sections.size();
}


std::vector<int> OctResult::BscanBundle::getSectionIndexList(void) const
{
	vector<int> list;
	for (auto iter = d_ptr->sections.begin(); iter != d_ptr->sections.end(); iter++) {
		list.push_back(iter->first);
	}
	return list;
}


void OctResult::BscanBundle::clearAllSections(void)
{
	d_ptr->sections.clear();
	return;
}


BscanBundle::BscanBundleImpl & OctResult::BscanBundle::getImpl(void) const
{
	return *d_ptr;
}
