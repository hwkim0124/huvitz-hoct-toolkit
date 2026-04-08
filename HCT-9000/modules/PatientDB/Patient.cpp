#include "stdafx.h"
#include "Patient.h"


using namespace PatientDB;


Patient::Patient()
{
	clear();
}

Patient::~Patient()
{
	clear();
}

void Patient::clear(void)
{
	m_index = INVALID_INDEX;
	m_patientID.clear();
	m_fName.clear();
	m_mName.clear();
	m_lName.clear();
	m_gender = Gender::UNKNOWN;
	m_birthDate = COleDateTime();
	m_refractOD = 0.0;
	m_refractOS = 0.0;
	m_raceID = INVALID_INDEX;
	m_physicianIdx = INVALID_INDEX;
	m_operatorIdx = INVALID_INDEX;
	m_description.clear();
	m_createTime = COleDateTime::GetCurrentTime();
	m_updateTime = COleDateTime::GetCurrentTime();
	m_status = Status::USE;
	m_modality.clear();
	m_accessionNumber.clear();

	return;
}

void Patient::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Patient::setPatientID(wstring pid)
{
	m_patientID = pid;
	return;
}

void Patient::setFirstName(wstring fName)
{
	m_fName = fName;
	return;
}

void Patient::setMiddleName(wstring mName)
{
	m_mName = mName;
	return;
}

void Patient::setLastName(wstring lName)
{
	m_lName = lName;
	return;
}

void Patient::setGender(Gender gender)
{
	m_gender = gender;
	return;
}

void Patient::setBirthDate(COleDateTime date)
{
	m_birthDate = date;
	return;
}

void Patient::setRefractOD(double diopt)
{
	m_refractOD = diopt;
	return;
}

void Patient::setRefractOS(double diopt)
{
	m_refractOS = diopt;
	return;
}

void Patient::setRaceID(long rid)
{
	m_raceID = rid;
	return;
}

void Patient::setPhysicianIdx(long idx)
{
	m_physicianIdx = idx;
	return;
}

void Patient::setOperatorIdx(long idx)
{
	m_operatorIdx = idx;
	return;
}

void Patient::setDescription(wstring desc)
{
	m_description = desc;
	return;
}

void Patient::setCreateTime(COleDateTime date)
{
	m_createTime = date;
	return;
}

void Patient::setUpdateTime(COleDateTime date)
{
	m_updateTime = date;
	return;
}

void Patient::setNeedTransfer(int need)
{
	m_needTransfer = need;
	return;
}

void Patient::setStatus(Status status)
{
	m_status = status;
	return;
}

void Patient::setModality(wstring value)
{
	m_modality = value;
}

void Patient::setAccessionNumber(wstring value)
{
	m_accessionNumber = value;
}

void Patient::setStudyDescription(wstring value)
{
	m_studyDescription = value;
}

void Patient::setSeriesDescription(wstring value)
{
	m_seriesDescription = value;
}

wstring Patient::getFullName(wstring sep)
{
	wstring fullName;
	fullName.clear();

	if (!m_fName.empty())
	{
		fullName = m_fName;
	}

	if (!m_mName.empty())
	{
		if (!fullName.empty())
		{
			fullName += sep;
		}
		
		fullName += m_mName;
	}
	 
	if (!m_lName.empty())
	{
		if (!fullName.empty())
		{
			fullName += sep;
		}

		fullName += m_lName;
	}

	return fullName; 
}

wstring Patient::getFullNameForDicom(wstring sep)
{
	wstring fullName;
	fullName.clear();

	if (!m_lName.empty())
	{
		fullName = m_lName;
	}

	if (!m_fName.empty())
	{
		if (!fullName.empty())
		{
			fullName += sep;
		}
		fullName += m_fName;
	}

	if (!m_mName.empty())
	{
		if (!fullName.empty())
		{
			fullName += sep;
		}

		fullName += m_mName;
	}

	return fullName;
}

wstring Patient::getGenderString(void)
{
	wstring ret;

	if (m_gender == Gender::MALE)
	{
		ret = L"M";
	}
	else if (m_gender == Gender::FEMALE)
	{
		ret = L"F";
	}
	else
	{
		ret = L"";
	}

	return ret;
}

int Patient::getAgeOn(COleDateTime when)
{
	int age = when.GetYear() - m_birthDate.GetYear();

	if (m_birthDate.GetMonth() > when.GetMonth())
	{
		age -= 1;
	}
	else if (m_birthDate.GetMonth() == when.GetMonth() && m_birthDate.GetDay() > when.GetDay())
	{
		age -= 1;
	}

	return age;
}