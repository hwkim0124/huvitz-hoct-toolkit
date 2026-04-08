#include "stdafx.h"
#include "DatabaseUtil.h"
#include "FetchPropertyFundusResult.h"

using namespace PatientDB;


FetchPropertyFundusResult::FetchPropertyFundusResult()
{
	clear();
}

FetchPropertyFundusResult::~FetchPropertyFundusResult()
{
	clear();
}

void FetchPropertyFundusResult::clear(void)
{
	m_examIndex.clear();
	m_eyeSide = -1;
	m_maxCount = -1;
}

void FetchPropertyFundusResult::addExamIndex(long index)
{
	m_examIndex.push_back(index);
	return;
}

void FetchPropertyFundusResult::setEyeSide(long side)
{
	m_eyeSide = side;
	return;
}

void FetchPropertyFundusResult::setMaxCount(long count)
{
	m_maxCount = count;
	return;
}

wstring FetchPropertyFundusResult::getCondition(void)
{
	wstring s = L"";

	if (!m_examIndex.empty())
	{
		wstring temp = L"";

		for (int i = 0; i < m_examIndex.size(); i++)
		{
			temp += DatabaseUtil::returnOrStingIfNotEmpty(temp);
			temp += L"Exam_ID = " + to_wstring(m_examIndex.at(i));
		}

		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"(" + temp + L")";
	}

//	if (m_examIndex != INVALID_INDEX)
//	{
//		s += "Exam_ID = " + to_string(m_examIndex);
//	}

	if (m_eyeSide != -1)
	{
		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
		s += L"Eye_Side = " + to_wstring((int)m_eyeSide);
	}


	//	if (m_status != STATUS_UNKNOWN)
	//	{
	//		s += DatabaseUtil::returnAndStingIfNotEmpty(s);
	//		s += "Status = " + to_string(m_status);
	//	}

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