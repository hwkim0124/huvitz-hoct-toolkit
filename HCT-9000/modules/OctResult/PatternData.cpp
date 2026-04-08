#include "stdafx.h"
#include "PatternData.h"
#include "SectionData.h"

#include <boost/format.hpp>

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PatternData::PatternDataImpl
{
	OctScanPattern descript;

	PatternDataImpl() 
	{
	}
};


PatternData::PatternData() :
	d_ptr(make_unique<PatternDataImpl>())
{
}


OctResult::PatternData::~PatternData() = default;
OctResult::PatternData::PatternData(PatternData && rhs) = default;
PatternData & OctResult::PatternData::operator=(PatternData && rhs) = default;


OctResult::PatternData::PatternData(const PatternData & rhs)
	: d_ptr(make_unique<PatternDataImpl>(*rhs.d_ptr))
{
}


PatternData & OctResult::PatternData::operator=(const PatternData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::PatternData::setDescript(OctScanPattern desc)
{
	d_ptr->descript = desc;
	return;
}


OctScanPattern & OctResult::PatternData::getDescript(void) const
{
	return d_ptr->descript;
}


bool OctResult::PatternData::exportFiles(std::wstring path)
{
	int size = getSectionCount();
	wstring name;

	for (int i = 0; i < size; i++) {
		SectionData* sect = getSection(i);
		if (sect != nullptr) {
			name = (boost::wformat(L"%03d") % i).str();
			if (!sect->exportFiles(path, name)) {
				return false;
			}
		}
	}
	return true;
}


PatternData::PatternDataImpl & OctResult::PatternData::getImpl(void) const
{
	return *d_ptr;
}
