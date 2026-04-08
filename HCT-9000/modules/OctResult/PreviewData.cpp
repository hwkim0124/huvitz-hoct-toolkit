#include "stdafx.h"
#include "PreviewData.h"
#include "SectionBundle.h"
#include "SectionData.h"

#include "CppUtil2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PreviewData::PreviewDataImpl
{
	PreviewDataImpl()
	{
	}
};


PreviewData::PreviewData() :
	d_ptr(make_unique<PreviewDataImpl>())
{
}


OctResult::PreviewData::~PreviewData() = default;
OctResult::PreviewData::PreviewData(PreviewData && rhs) = default;
PreviewData & OctResult::PreviewData::operator=(PreviewData && rhs) = default;


OctResult::PreviewData::PreviewData(const PreviewData & rhs)
	: d_ptr(make_unique<PreviewDataImpl>(*rhs.d_ptr))
{
}


PreviewData & OctResult::PreviewData::operator=(const PreviewData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


PreviewData::PreviewDataImpl & OctResult::PreviewData::getImpl(void) const
{
	return *d_ptr;
}
