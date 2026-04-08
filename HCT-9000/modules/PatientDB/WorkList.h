#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API Worklist
	{
	public:
		Worklist();
		Worklist(const Worklist &w);
		virtual ~Worklist();
		void clear(void);

	private:
		long			m_index;
		wstring			m_scheduledModality;
		wstring			m_scheduledStation;
		COleDateTime	m_scheduledTime;
		wstring			m_studyInstanceUID;
		wstring			m_requestProcedureID;
		wstring			m_accessionNumber;
		long			m_patientIdx;
		Status			m_status;

	public:
		long			getIndex(void) { return m_index; }
		wstring			getScheduledModality(void) { return m_scheduledModality; }
		wstring			getScheduledStation(void) { return m_scheduledStation; }
		COleDateTime	getScheduledTime(void) { return m_scheduledTime; }
		wstring			getStudyInstanceUID(void) { return m_studyInstanceUID; }
		wstring			getRequestProcedureID(void) { return m_requestProcedureID; }
		wstring			getAccessionNumber(void) { return m_accessionNumber; }
		long			getPatientIdx(void) { return m_patientIdx; }
		Status			getStatus(void) { return m_status; }

		void			setIndex(long idx);
		void			setScheduledModality(wstring modality);
		void			setScheduledStation(wstring station);
		void			setScheduledTime(COleDateTime time);
		void			setStudyInstanceUID(wstring uid);
		void			setRequestProcedureID(wstring id);
		void			setAccessionNumber(wstring number);
		void			setPatientIdx(long idx);
		void			setStatus(Status status);

	};
}


