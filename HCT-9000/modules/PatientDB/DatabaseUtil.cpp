#include "stdafx.h"
#include "DatabaseUtil.h"

using namespace PatientDB;


DatabaseUtil::DatabaseUtil()
{
}

DatabaseUtil::~DatabaseUtil()
{
}

wstring DatabaseUtil::getDateTimeString(COleDateTime &time)
{
	wstring str = L"";

	if (time.GetStatus() == COleDateTime::valid)
	{
		str = time.Format(L"%Y-%m-%d %H:%M:%S");
	}

	return str;
}



COleDateTime DatabaseUtil::getDateTime(const unsigned char* str)
{
	COleDateTime dt;

	dt.ParseDateTime(CString(str));

	return dt;
}

wstring DatabaseUtil::returnAndStingIfNotEmpty(wstring s)
{
	wstring str;
	str.clear();

	if (s.empty() == false)
	{
		str = L" AND ";
	}

	return str;
}

wstring DatabaseUtil::returnOrStingIfNotEmpty(wstring s)
{
	wstring str = L"";

	if (s.empty() == false)
	{
		str = L" OR ";
	}

	return str;
}

//wstring DatabaseUtil::convert_string(CString cs)
//{
//	CT2CA pszConvertedAnsiString(cs);
//	std::string s(pszConvertedAnsiString);
//
//	return s;
//}

double DatabaseUtil::convert_double(CString cs)
{
	double d = _wtof(cs);

	return d;
}


COleDateTime DatabaseUtil::calcDateTime(int year, int month, int day)
{
	int maxdays[13] =
	{
		0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	// À±³â Àû¿ë
	if ((year % 400 == 0) || (year%4 == 0 && year%100) != 0)
	{
		maxdays[2] = 29;
	}

	if (day > maxdays[month])
	{
		if (month == 12)
		{
			year += 1;
			month = 1;
			day = 1;
		}
		else
		{
			month += 1;
			day = 1;
		}
	}

	return COleDateTime(year, month, day, 0, 0, 0);
}

COleDateTime DatabaseUtil::getDateMonthAgo(COleDateTime &time)
{
	int maxdays[13] =
	{
		0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	int year, month, day;

	year = time.GetYear();
	month = time.GetMonth();
	day = time.GetDay();

	// À±³â Àû¿ë
	if ((year % 400 == 0) || (year % 4 == 0 && year % 100) != 0)
	{
		maxdays[2] = 29;
	}

	if (month == 1)
	{
		year -= 1;
		month = 12;
	}
	else
	{
		month -= 1;
	}

	if (day > maxdays[month])
	{
		day = maxdays[month];
	}

	

	return COleDateTime(year, month, day, 0, 0, 0);
}
