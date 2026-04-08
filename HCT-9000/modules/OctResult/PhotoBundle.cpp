#include "stdafx.h"
#include "PhotoBundle.h"
#include "PhotoProfile.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PhotoBundle::PhotoBundleImpl
{
	PhotoProfileList sections;

	PhotoBundleImpl()
	{
	}
};


PhotoBundle::PhotoBundle() :
	d_ptr(make_unique<PhotoBundleImpl>())
{
}


OctResult::PhotoBundle::~PhotoBundle() = default;
OctResult::PhotoBundle::PhotoBundle(PhotoBundle && rhs) = default;
PhotoBundle & OctResult::PhotoBundle::operator=(PhotoBundle && rhs) = default;


OctResult::PhotoBundle::PhotoBundle(const PhotoBundle & rhs) :
	d_ptr(make_unique<PhotoBundleImpl>())
{
}


PhotoBundle & OctResult::PhotoBundle::operator=(const PhotoBundle & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::PhotoBundle::addSection(PhotoProfile & section)
{
	d_ptr->sections.push_back(section);
	int size = getSectionCount();
	return size;
}


int OctResult::PhotoBundle::addSection(PhotoProfile && section)
{
	d_ptr->sections.push_back(section);
	int size = getSectionCount();
	return size;
}


PhotoProfile * OctResult::PhotoBundle::getSection(int index)
{
	if (index < 0 || index >= getSectionCount()) {
		return nullptr;
	}

	PhotoProfile* p = &d_ptr->sections[index];
	return p;
}


PhotoProfile * OctResult::PhotoBundle::getSectionLast(void)
{
	int index = getSectionCount() - 1;
	PhotoProfile* p = getSection(index);
	return p;
}


PhotoProfile * OctResult::PhotoBundle::makeSection(int index)
{
	PhotoProfile* p = getSection(index);
	if (p == nullptr) {
		PhotoProfile section;
		addSection(std::move(section));
		p = getSectionLast();
	}
	return p;
}


int OctResult::PhotoBundle::getSectionCount(void) const
{
	return (int)d_ptr->sections.size();
}


void OctResult::PhotoBundle::clearAllSections(void)
{
	d_ptr->sections.clear();
	return;
}


PhotoBundle::PhotoBundleImpl & OctResult::PhotoBundle::getImpl(void) const
{
	return *d_ptr;
}
