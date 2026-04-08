#include "stdafx.h"
#include "OcularReport.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctReport;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct OcularReport::OcularReportImpl
{
	EyeSide side;

	OcularReportImpl() : side(EyeSide::OD)
	{
	}
};


OctReport::OcularReport::OcularReport() :
	d_ptr(make_unique<OcularReportImpl>())
{
}


OctReport::OcularReport::~OcularReport() = default;
OctReport::OcularReport::OcularReport(OcularReport && rhs) = default;
OcularReport & OctReport::OcularReport::operator=(OcularReport && rhs) = default;


void OctReport::OcularReport::setEyeSide(EyeSide side)
{
	d_ptr->side = side;
	return;
}


EyeSide OctReport::OcularReport::getEyeSide(void) const
{
	return d_ptr->side;
}


bool OctReport::OcularReport::isEyeOD(void) const
{
	return getEyeSide() == EyeSide::OD;
}


bool OctReport::OcularReport::updateContents(void)
{
	return true;
}


void OctReport::OcularReport::clearContents(void)
{
	return;
}


OcularReport::OcularReportImpl & OctReport::OcularReport::getImpl(void) const
{
	return *d_ptr;
}