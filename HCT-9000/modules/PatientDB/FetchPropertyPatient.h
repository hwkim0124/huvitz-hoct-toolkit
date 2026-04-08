#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API FetchPropertyPatient
	{
	public:
		FetchPropertyPatient();
		virtual ~FetchPropertyPatient();

		COleDateTime	getUpdateTimeFrom(void);
		COleDateTime	getUpdateTimeTo(void);

		void			clear(void);
		wstring			getPatientCondition(void);

		void			setCreateTimeFrom(COleDateTime time);
		void			setCreateTimeTo(COleDateTime time);
		void			setBirthDateFrom(COleDateTime time);
		void			setBirthDateTo(COleDateTime time);
		void			setUpdateTimeFrom(COleDateTime time);
		void			setUpdateTimeTo(COleDateTime time);
		void			setPatientId(wstring id);
		void			setPatientName(wstring name);
		void			setGender(int gender);
		void			setRaceIndex(long index);
		void			setDescription(wstring desc);
		void			setStatus(Status status);
		void			setMaxCount(long count);

	private:
		// Patient
		COleDateTime	m_createTime1;
		COleDateTime	m_createTime2;
		COleDateTime	m_birthDate1;
		COleDateTime	m_birthDate2;
		COleDateTime	m_updateTime1;
		COleDateTime	m_updateTime2;
		wstring			m_patientId;		// 부분문자열
		wstring			m_patientName;		// 부분문자열
		int				m_gender;
		long			m_raceIndex;
		wstring			m_description;
		Status			m_status;
		long			m_maxCount;
	};
}
