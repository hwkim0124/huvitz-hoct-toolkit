#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

class PATIENTDB_DLL_API DatabaseUtil
{
public:
	DatabaseUtil();
	virtual ~DatabaseUtil();

public:
	static COleDateTime getDateTime(const unsigned char* str);
	static wstring getDateTimeString(COleDateTime &time);
	static wstring returnAndStingIfNotEmpty(wstring s);
	static wstring returnOrStingIfNotEmpty(wstring s);
//	static wstring convert_string(CString cs);
	static double convert_double(CString cs);
	static COleDateTime calcDateTime(int year, int month, int day);	// 최대일수를 넘어갈 경우 다음달 1일로..
	static COleDateTime getDateMonthAgo(COleDateTime &time);
};

