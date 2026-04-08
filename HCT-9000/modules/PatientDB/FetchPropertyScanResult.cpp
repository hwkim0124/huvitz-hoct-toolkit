#include "stdafx.h"
#include "DatabaseUtil.h"
#include "FetchPropertyScanResult.h"

using namespace PatientDB;


FetchPropertyScanResult::FetchPropertyScanResult()
{
	clear();
}

FetchPropertyScanResult::~FetchPropertyScanResult()
{
	clear();
}

void FetchPropertyScanResult::clear(void)
{
	m_examIndex.clear();
	m_eyeSide = -1;
	m_maxCount = -1;
}

void FetchPropertyScanResult::addExamIndex(long index)
{
	m_examIndex.push_back(index);
	return;
}

void FetchPropertyScanResult::setEyeSide(int side)
{
	m_eyeSide = side;
	return;
}

void FetchPropertyScanResult::setMaxCount(long count)
{
	m_maxCount = count;
	return;
}

wstring FetchPropertyScanResult::getCondition(void)
{
	wstring ret = L"";

	if (!m_examIndex.empty())
	{
		wstring temp;

		for (int i = 0; i < m_examIndex.size(); i++)
		{
			temp += DatabaseUtil::returnOrStingIfNotEmpty(temp);
			temp += L"Exam_ID = " + to_wstring(m_examIndex.at(i));
		}

		ret += DatabaseUtil::returnAndStingIfNotEmpty(ret);
		ret += L"(" + temp + L")";
	}

//	if (m_examIndex != INVALID_INDEX)
//	{
//		s += "Exam_ID = " + to_string(m_examIndex);
//	}

	if (m_eyeSide != -1)
	{
		ret += DatabaseUtil::returnAndStingIfNotEmpty(ret);
		ret += L"Eye_Side = " + to_wstring((int)m_eyeSide);
	}


//	if (m_status != STATUS_UNKNOWN)
//	{
//		ret += DatabaseUtil::returnAndStingIfNotEmpty(ret);
//		ret += "Status = " + to_string(m_status);
//	}

	if (ret.empty() == false)
	{
		ret = L" WHERE " + ret;
	}

	if (m_maxCount > 0)
	{
		ret += L" LIMIT " + to_wstring(m_maxCount);
	}

	return ret;
}