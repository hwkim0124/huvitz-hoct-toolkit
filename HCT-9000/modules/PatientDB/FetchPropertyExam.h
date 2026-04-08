#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{

	class PATIENTDB_DLL_API FetchPropertyExam
	{
	public:
		FetchPropertyExam();
		virtual ~FetchPropertyExam();
		void			clear(void);

		COleDateTime	getExamDateFrom(void);
		COleDateTime	getExamDateTo(void);

		void			setExamDateFrom(COleDateTime dt);
		void			setExamDateTo(COleDateTime dt);
		void			addPatientIndex(long index);
		void			addPhysicianIndex(long index);
		void			addOperatorIndex(long index);
		void			setStatus(Status status);
		void			setMaxCount(long count);

		wstring			getExamCondition(void);

	private:
		// Exam
		COleDateTime	m_examDate1;
		COleDateTime	m_examDate2;
		vector <long>	m_patientIndex;
		vector <long>	m_physicianIndex;
		vector <long>	m_operatorIndex;
		Status			m_status;
		long			m_maxCount;
	};

}