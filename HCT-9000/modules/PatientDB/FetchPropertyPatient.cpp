#include "stdafx.h"
#include "DatabaseUtil.h"
#include "FetchPropertyPatient.h"

using namespace PatientDB;


FetchPropertyPatient::FetchPropertyPatient()
{
	clear();
}

FetchPropertyPatient::~FetchPropertyPatient()
{
	clear();
}

void FetchPropertyPatient::clear(void)
{
	m_createTime1 = COleDateTime();
	m_createTime1.SetStatus(COleDateTime::invalid);
	m_createTime2 = COleDateTime();
	m_createTime2.SetStatus(COleDateTime::invalid);
	m_birthDate1 = COleDateTime();
	m_birthDate1.SetStatus(COleDateTime::invalid);
	m_birthDate2 = COleDateTime();
	m_birthDate2.SetStatus(COleDateTime::invalid);
	m_updateTime1 = COleDateTime();
	m_updateTime1.SetStatus(COleDateTime::invalid);
	m_updateTime2 = COleDateTime();
	m_updateTime2.SetStatus(COleDateTime::invalid);
	m_patientId.clear();
	m_patientName.clear();
	m_gender = -1;
	m_raceIndex = INVALID_INDEX;
	m_status = Status::UNKNOWN;
	m_maxCount = -1;
}

COleDateTime FetchPropertyPatient::getUpdateTimeFrom(void)
{
	return m_updateTime1;
}

COleDateTime FetchPropertyPatient::getUpdateTimeTo(void)
{
	return m_updateTime2;
}

void FetchPropertyPatient::setCreateTimeFrom(COleDateTime time)
{
	m_createTime1.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 0, 0, 0);
	return;
}

void FetchPropertyPatient::setCreateTimeTo(COleDateTime time)
{
	m_createTime2.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 23, 59, 59);
	return;
}

void FetchPropertyPatient::setBirthDateFrom(COleDateTime time)
{
	m_birthDate1.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 0, 0, 0);
	return;
}

void FetchPropertyPatient::setBirthDateTo(COleDateTime time)
{
	m_birthDate2.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 23, 59, 59);
	return;
}

void FetchPropertyPatient::setUpdateTimeFrom(COleDateTime time)
{
	m_updateTime1.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 0, 0, 0);
	return;
}

void FetchPropertyPatient::setUpdateTimeTo(COleDateTime time)
{
	m_updateTime2.SetDateTime(time.GetYear(), time.GetMonth(), time.GetDay(), 23, 59, 59);
	return;
}

void FetchPropertyPatient::setPatientId(std::wstring id)
{
	m_patientId = id;
	return;
}

void FetchPropertyPatient::setPatientName(std::wstring name)
{
	m_patientName = name;
	return;
}

void FetchPropertyPatient::setGender(int gender)
{
	m_gender = gender;
	return;
}

void FetchPropertyPatient::setRaceIndex(long index)
{
	m_raceIndex = index;
	return;
}

void FetchPropertyPatient::setDescription(wstring desc)
{
	m_description = desc;
	return;
}

void FetchPropertyPatient::setStatus(Status status)
{
	m_status = status;
	return;
}

void FetchPropertyPatient::setMaxCount(long count)
{
	m_maxCount = count;
	return;
}

wstring FetchPropertyPatient::getPatientCondition(void)
{
	wstring s = L"";

	if (m_createTime1.GetStatus() == COleDateTime::valid)
	{
		s += L"Create_Time >= '" + DatabaseUtil::getDateTimeString(m_createTime1) + L"'";
	}

	if (m_createTime2.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Create_Time <= '" + DatabaseUtil::getDateTimeString(m_createTime2) + L"'";
	}

	if (m_birthDate1.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Birth_Date >= '" + DatabaseUtil::getDateTimeString(m_birthDate1) + L"'";
	}

	if (m_birthDate2.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Birth_Date <= '" + DatabaseUtil::getDateTimeString(m_birthDate2) + L"'";
	}

	if (m_updateTime1.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Update_Time >= '" + DatabaseUtil::getDateTimeString(m_updateTime1) + L"'";
	}

	if (m_updateTime2.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Update_Time <= '" + DatabaseUtil::getDateTimeString(m_updateTime2) + L"'";
	}

	if (m_patientId.empty() == false)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Patient_IDS like '%" + m_patientId + L"%'";
	}

	if (m_patientName.empty() == false)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"(First_Name like '%" + m_patientName + L"%'";
		s += L" OR ";
		s += L"Middle_Name like '%" + m_patientName + L"%'";
		s += L" OR ";
		s += L"Last_Name like '%" + m_patientName + L"%')";
	}

	if (m_gender != -1)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Gender = " + to_wstring((int)m_gender);
	}

	if (m_raceIndex != INVALID_INDEX)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Race_ID = " + to_wstring(m_raceIndex);
	}

	if (m_description.empty() == false)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Description like '%" + m_description + L"%'";
	}

	if (m_status != Status::UNKNOWN)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Status = " + to_wstring((int)m_status);
	}

	if (s.empty() == false)
	{
		s = L" WHERE " + s;
	}

	if (m_maxCount > 0)
	{
		s += L" LIMIT " + to_wstring(m_maxCount);
	}

	return s;
}