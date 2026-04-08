#include "stdafx.h"
#include "SectionBundle.h"
#include "SectionData.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct SectionBundle::SectionBundleImpl
{
	SectionDataList sections;

	SectionBundleImpl()
	{
	}
};


SectionBundle::SectionBundle() :
	d_ptr(make_unique<SectionBundleImpl>())
{
}


OctResult::SectionBundle::~SectionBundle() = default;
OctResult::SectionBundle::SectionBundle(SectionBundle && rhs) = default;
SectionBundle & OctResult::SectionBundle::operator=(SectionBundle && rhs) = default;


OctResult::SectionBundle::SectionBundle(const SectionBundle & rhs) :
	d_ptr(make_unique<SectionBundleImpl>())
{
}


SectionBundle & OctResult::SectionBundle::operator=(const SectionBundle & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


int OctResult::SectionBundle::addSection(SectionData & section)
{
	d_ptr->sections.push_back(section);
	int size = getSectionCount();
	return size;
}


int OctResult::SectionBundle::addSection(SectionData && section)
{
	d_ptr->sections.push_back(section);
	int size = getSectionCount();
	return size;
}


SectionData * OctResult::SectionBundle::getSection(int index)
{
	if (index < 0 || index >= getSectionCount()) {
		return nullptr;
	}

	SectionData* p = &d_ptr->sections[index];
	return p;
}


SectionData * OctResult::SectionBundle::getSectionLast(void)
{
	int index = getSectionCount() - 1;
	SectionData* p = getSection(index);
	return p;
}


SectionData * OctResult::SectionBundle::makeSection(int index, OctRoute route)
{
	SectionData* p = getSection(index);
	if (p == nullptr) {
		SectionData section(route);
		addSection(std::move(section));
		p = getSectionLast();
	}
	else {
		p->setRouteOfScan(route);
	}
	return p;
}


int OctResult::SectionBundle::getSectionCount(void) const
{
	return (int)d_ptr->sections.size();
}


void OctResult::SectionBundle::clearAllSections(void)
{
	d_ptr->sections.clear();
	return;
}


SectionBundle::SectionBundleImpl & OctResult::SectionBundle::getImpl(void) const
{
	return *d_ptr;
}
