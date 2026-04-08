#include "stdafx.h"
#include "FundusReport.h"


#include "CppUtil2.h"
#include "OctResult2.h"
#include "OctData2.h"


using namespace OctReport;
using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusReport::FundusReportImpl
{
	FundusData* data;

	FundusReportImpl() : data(nullptr)
	{
	}
};


FundusReport::FundusReport() :
	d_ptr(make_unique<FundusReportImpl>())
{
}


OctReport::FundusReport::~FundusReport() = default;
OctReport::FundusReport::FundusReport(FundusReport && rhs) = default;
FundusReport & OctReport::FundusReport::operator=(FundusReport && rhs) = default;


void OctReport::FundusReport::setFundusData(OctData::FundusData * data)
{
	d_ptr->data = data;
	return;
}


OctData::FundusData * OctReport::FundusReport::getFundusData(void) const
{
	return d_ptr->data;
}


bool OctReport::FundusReport::isEmpty(void) const
{
	return (d_ptr->data == nullptr);
}


FundusReport::FundusReportImpl & OctReport::FundusReport::getImpl(void) const
{
	return *d_ptr;
}