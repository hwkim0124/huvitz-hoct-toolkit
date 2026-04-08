#include "stdafx.h"
#include "ScanPattern.h"

using namespace PatientDB;


ScanPattern::ScanPattern()
{
	clear();
}

ScanPattern::~ScanPattern()
{
}

void ScanPattern::clear(void)
{
	m_index = INVALID_INDEX;
	m_code = 0;
	m_name.clear();
	m_version.clear();
	m_scanType = ScanType::MARCULAR;
	m_status = Status::USE;
	return;
}

void ScanPattern::setIndex(long idx)
{
	m_index = idx;
	return;
}

void ScanPattern::setCode(int code)
{
	m_code = code;
	return;
}

void ScanPattern::setName(wstring name)
{
	m_name = name;
	return;
}

void ScanPattern::setVersion(wstring version)
{
	m_version = version;
	return;
}

void ScanPattern::setScanType(ScanType type)
{
	m_scanType = type;
	return;
}

void ScanPattern::setStatus(Status status)
{
	m_status = status;
	return;
}

