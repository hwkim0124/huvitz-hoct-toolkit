#include "stdafx.h"
#include "DiagnosisGroup.h"

using namespace PatientDB;


DiagnosisGroup::DiagnosisGroup()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_displayOrder = 0;
	m_status = Status::USE;
}

DiagnosisGroup::~DiagnosisGroup()
{

}

void DiagnosisGroup::setIndex(long idx)
{
	m_index = idx;
	return;
}

void DiagnosisGroup::setName(wstring name)
{
	m_name = name;
	return;
}

void DiagnosisGroup::setDisplayOrder(int order)
{
	m_displayOrder = order;
	return;
}

void DiagnosisGroup::setStatus(Status status)
{
	m_status = status;
	return;
}
