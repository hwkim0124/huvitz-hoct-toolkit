#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API Exam
	{
	public:
		Exam();
		Exam(const Exam &e);
		virtual ~Exam();
		void clear(void);

	private:
		long			m_index;
		COleDateTime	m_examDate;
		long			m_patientIdx;
		wstring			m_diagnosis;
		wstring			m_comment;
		wstring			m_filePath;
		Status			m_status;
		wstring			m_modality;
		wstring			m_scheduledDate;
		wstring			m_scheduledTime;
		wstring			m_studyInstanceUID;


	public:
		long			getIndex(void) { return m_index; }
		COleDateTime	getExamDate(void) { return m_examDate; }
		long			getPatientIdx(void) { return m_patientIdx; }
		wstring			getDiagnosis(void) { return m_diagnosis; }
		wstring			getComment(void) { return m_comment; }
		wstring			getFilePath(void) { return m_filePath; }
		Status			getStatus(void) { return m_status; }
		wstring			getModality(void) { return m_modality; }
		wstring			getScheduledDate(void) { return m_scheduledDate; }
		wstring			getScheduledTime(void) { return m_scheduledTime; }
		wstring			getStudyInstanceUID(void) { return m_studyInstanceUID; }

		void			setIndex(long idx);
		void			setExamDate(COleDateTime time);
		void			setPatientIdx(long idx);
		void			setDiagnosis(wstring diagnosis);
		void			setComment(wstring comment);
		void			setFilePath(wstring filePath);
		void			setStatus(Status status);
		void			setModality(wstring modality);
		void			setScheduledDate(wstring date);
		void			setScheduledTime(wstring time);
		void			setStudyInstanceUID(wstring uid);
	};
}