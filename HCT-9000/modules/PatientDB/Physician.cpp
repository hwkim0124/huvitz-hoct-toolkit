#include "stdafx.h"
#include "Physician.h"

using namespace PatientDB;


Physician::Physician()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_description.clear();
	m_status = Status::USE;
}

Physician::~Physician()
{
}

void Physician::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Physician::setName(wstring name)
{
	m_name = name;
	return;
}

void Physician::setDescription(wstring desc)
{
	m_description = desc;
	return;
}

void Physician::setStatus(Status status)
{
	m_status = status;
	return;
}
