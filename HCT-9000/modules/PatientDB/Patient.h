#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API Patient
	{
	public:
		Patient();
		virtual ~Patient();

	private:
		long			m_index;
		wstring			m_patientID;
		wstring			m_fName;
		wstring			m_mName;
		wstring			m_lName;
		Gender			m_gender;
		COleDateTime	m_birthDate;
		double			m_refractOD;
		double			m_refractOS;
		long			m_raceID;
		long			m_physicianIdx;
		long			m_operatorIdx;
		wstring			m_description;
		COleDateTime	m_createTime;
		COleDateTime	m_updateTime;
		int				m_needTransfer;
		Status			m_status;
		wstring			m_modality;
		wstring			m_accessionNumber;
		wstring			m_studyDescription;
		wstring			m_seriesDescription;

	public:
		void			clear();

		long			getIndex(void) { return m_index; }
		wstring			getPatientId(void) { return m_patientID; }
		wstring			getFirstName(void) { return m_fName; }
		wstring			getMiddleName(void) { return m_mName; }
		wstring			getLastName(void) { return m_lName; }
		Gender			getGender(void) { return m_gender; }
		COleDateTime	getBirthDate(void) { return m_birthDate; }
		double			getRefractOD(void) { return m_refractOD; }
		double			getRefractOS(void) { return m_refractOS; }
		long			getRaceID(void) { return m_raceID; }
		long			getPhysicianIdx(void) { return m_physicianIdx; }
		long			getOperatorIdx(void) { return m_operatorIdx; }
		wstring			getDescription(void) { return m_description; }
		COleDateTime	getCreateTime(void) { return m_createTime; }
		COleDateTime	getUpdateTime(void) { return m_updateTime; }
		int				getNeedTransfer(void) { return m_needTransfer; }
		Status			getStatus(void) { return m_status; }
		wstring			getModality() { return m_modality; }
		wstring			getAccessionNumber() { return m_accessionNumber; }
		wstring			getStudyDescription() { return m_studyDescription; }
		wstring			getSeriesDescription() { return m_seriesDescription; }

		void			setIndex(long idx);
		void			setPatientID(wstring pid);
		void			setFirstName(wstring fName);
		void			setMiddleName(wstring mName);
		void			setLastName(wstring lName);
		void			setGender(Gender gender);
		void			setBirthDate(COleDateTime date);
		void			setRefractOD(double diopt);
		void			setRefractOS(double diopt);
		void			setRaceID(long rid);
		void			setPhysicianIdx(long idx);
		void			setOperatorIdx(long idx);
		void			setDescription(wstring desc);
		void			setCreateTime(COleDateTime date);
		void			setUpdateTime(COleDateTime date);
		void			setNeedTransfer(int need);
		void			setStatus(Status status);
		void			setModality(wstring value);
		void			setAccessionNumber(wstring value);
		void			setStudyDescription(wstring value);
		void			setSeriesDescription(wstring value);

		wstring			getFullName(wstring sep = L" ");
		wstring			getFullNameForDicom(wstring sep = L" ");
		wstring			getGenderString(void);
		int				getAgeOn(COleDateTime when);
	};
}
