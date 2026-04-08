#include "stdafx.h"
#include "Operator.h"

using namespace PatientDB;


Operator::Operator()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_description.clear();
	m_status = Status::USE;
}

Operator::~Operator()
{
}

void Operator::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Operator::setName(wstring name)
{
	m_name = name;
	return;
}

void Operator::setDescription(wstring desc)
{
	m_description = desc;
	return;
}

void Operator::setStatus(Status status)
{
	m_status = status;
	return;
}
