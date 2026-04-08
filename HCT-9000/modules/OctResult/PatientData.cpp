#include "stdafx.h"
#include "PatientData.h"

#include "CppUtil2.h"
#include "OctGlobal2.h"

using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct PatientData::PatientDataImpl
{
	OctPatient data;

	PatientDataImpl()
	{
	}
};


PatientData::PatientData() :
	d_ptr(make_unique<PatientDataImpl>())
{
}


OctResult::PatientData::PatientData(OctPatient data) :
	d_ptr(make_unique<PatientDataImpl>())
{
	d_ptr->data = data;
	return;
}


OctResult::PatientData::~PatientData() = default;
OctResult::PatientData::PatientData(PatientData && rhs) = default;
PatientData & OctResult::PatientData::operator=(PatientData && rhs) = default;


OctResult::PatientData::PatientData(const PatientData & rhs)
	: d_ptr(make_unique<PatientDataImpl>(*rhs.d_ptr))
{
}


PatientData & OctResult::PatientData::operator=(const PatientData & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctResult::PatientData::setPatient(const OctPatient& data)
{
	d_ptr->data = data;
	return;
}


OctPatient & OctResult::PatientData::getPatient(void) const
{
	return d_ptr->data;
}


PatientData::PatientDataImpl & OctResult::PatientData::getImpl(void) const
{
	return *d_ptr;
}
