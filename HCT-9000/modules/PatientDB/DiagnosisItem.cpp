#include "stdafx.h"
#include "DiagnosisItem.h"

using namespace PatientDB;


DiagnosisItem::DiagnosisItem()
{
	m_index = INVALID_INDEX;
	m_name.clear();
	m_groupIdx = INVALID_INDEX;
	m_displayOrder = 0;
	m_status = Status::USE;
}

DiagnosisItem::~DiagnosisItem()
{
}

void DiagnosisItem::setIndex(long idx)
{
	m_index = idx;
	return;
}

void DiagnosisItem::setName(wstring name)
{
	m_name = name;
	return;
}

void DiagnosisItem::setGroupIdx(long idx)
{
	m_groupIdx = idx;
	return;
}

void DiagnosisItem::setDisplayOrder(int order)
{
	m_displayOrder = order;
	return;
}

void DiagnosisItem::setStatus(Status status)
{
	m_status = status;
	return;
}
