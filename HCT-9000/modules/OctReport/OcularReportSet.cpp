#include "stdafx.h"
#include "OcularReportSet.h"
#include "MacularReport2.h"
#include "DiscReport2.h"
#include "CorneaReport2.h"
#include "FundusReport.h"

#include <map>

#include "CppUtil2.h"

using namespace OctReport;
using namespace CppUtil;
using namespace std;


struct OcularReportSet::OcularReportSetImpl
{
	std::map<int, unique_ptr<OcularReport>> dataset;

	OcularReportSetImpl()
	{
	}
};


OcularReportSet::OcularReportSet() :
	d_ptr(make_unique<OcularReportSetImpl>())
{
}


OctReport::OcularReportSet::~OcularReportSet() = default;
OctReport::OcularReportSet::OcularReportSet(OcularReportSet && rhs) = default;
OcularReportSet & OctReport::OcularReportSet::operator=(OcularReportSet && rhs) = default;


MacularReport2 * OctReport::OcularReportSet::getOrCreateMacularReport(int index)
{
	auto data = static_cast<MacularReport2*>(emplaceOcularReport<MacularReport2>(index));
	return data;
}


DiscReport2 * OctReport::OcularReportSet::getOrCreateDiscReport(int index)
{
	auto data = static_cast<DiscReport2*>(emplaceOcularReport<DiscReport2>(index));
	return data;
}


CorneaReport2 * OctReport::OcularReportSet::getOrCreateCorneaReport(int index)
{
	auto data = static_cast<CorneaReport2*>(emplaceOcularReport<CorneaReport2>(index));
	return data;
}


FundusReport * OctReport::OcularReportSet::getOrCreateFundusReport(int index)
{
	auto data = static_cast<FundusReport*>(emplaceOcularReport<FundusReport>(index));
	return data;
}


void OctReport::OcularReportSet::clear(void)
{
	d_ptr->dataset.clear();
	return;
}


OcularReportSet::OcularReportSetImpl & OctReport::OcularReportSet::getImpl(void) const
{
	return *d_ptr;
}