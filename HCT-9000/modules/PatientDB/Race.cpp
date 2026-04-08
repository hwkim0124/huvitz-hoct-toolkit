#include "stdafx.h"
#include "Race.h"

using namespace PatientDB;


Race::Race()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_displayOrder = 0;
	m_status = Status::USE;
}

Race::~Race()
{
}

void Race::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Race::setName(wstring name)
{
	m_name = name;
	return;
}

void Race::setDisplayOrder(int order)
{
	m_displayOrder = order;
	return;
}

void Race::setStatus(Status status)
{
	m_status = status;
	return;
}