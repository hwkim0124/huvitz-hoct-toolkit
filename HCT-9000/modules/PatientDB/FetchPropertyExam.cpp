#include "stdafx.h"
#include "DatabaseUtil.h"
#include "FetchPropertyExam.h"

using namespace PatientDB;


FetchPropertyExam::FetchPropertyExam()
{
	clear();
}

FetchPropertyExam::~FetchPropertyExam()
{

}

void FetchPropertyExam::clear()
{
	m_examDate1 = COleDateTime();
	m_examDate1.SetStatus(COleDateTime::invalid);
	m_examDate2 = COleDateTime();
	m_examDate2.SetStatus(COleDateTime::invalid);
	m_patientIndex.clear();
	m_physicianIndex.clear();
	m_operatorIndex.clear();
	m_status = Status::UNKNOWN;
	m_maxCount = -1;
}

COleDateTime FetchPropertyExam::getExamDateFrom(void)
{
	return m_examDate1;
}

COleDateTime FetchPropertyExam::getExamDateTo(void)
{
	return m_examDate2;
}

void FetchPropertyExam::setExamDateFrom(COleDateTime time)
{
	m_examDate1 = time;
	return;
}

void FetchPropertyExam::setExamDateTo(COleDateTime time)
{
	m_examDate2 = time;
	return;
}

void FetchPropertyExam::addPatientIndex(long index)
{
	m_patientIndex.push_back(index);
	return;
}

void FetchPropertyExam::addPhysicianIndex(long index)
{
	m_physicianIndex.push_back(index);
	return;
}

void FetchPropertyExam::addOperatorIndex(long index)
{
	m_operatorIndex.push_back(index);
	return;
}

void FetchPropertyExam::setStatus(Status status)
{
	m_status = status;
	return;
}

void FetchPropertyExam::setMaxCount(long count)
{
	m_maxCount = count;
	return;
}

wstring FetchPropertyExam::getExamCondition(void)
{
	wstring s = L"";

	if (m_examDate1.GetStatus() == COleDateTime::valid)
	{
		s += L"Exam_Date >= '" + DatabaseUtil::getDateTimeString(m_examDate1) + L"'";
	}

	if (m_examDate2.GetStatus() == COleDateTime::valid)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Exam_Date <= '" + DatabaseUtil::getDateTimeString(m_examDate2) + L"'";
	}

	if (!m_patientIndex.empty())
	{
		wstring temp = L"";
		
		for (int i = 0; i < m_patientIndex.size(); i++)
		{
			temp += DatabaseUtil::returnOrStingIfNotEmpty(temp);
			temp += L"Patient_ID = " + to_wstring(m_patientIndex.at(i));
		}

		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"(" + temp + L")";
	}


//	for (int i = 0; i < m_physicianIndex.size(); i++)
//	{
//		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
//		s += "Physician_ID = " + to_string(m_physicianIndex.at(i));
//	}
//
//	for (int i = 0; i < m_operatorIndex.size(); i++)
//	{
//		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
//		s += "Operator_ID = " + to_string(m_operatorIndex.at(i));
//	}

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