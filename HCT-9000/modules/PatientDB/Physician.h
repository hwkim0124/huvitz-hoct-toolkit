#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API Physician
	{
	public:
		Physician();
		virtual ~Physician();

	private:
		long		m_index;
		wstring		m_name;
		wstring		m_description;
		Status		m_status;

	public:
		long		getIndex(void) { return m_index; }
		wstring		getName(void) { return m_name; }
		wstring		getDescription(void) { return m_description; }
		Status		getStatus(void) { return m_status; }

		void		setIndex(long idx);
		void		setName(wstring name);
		void		setDescription(wstring desc);
		void		setStatus(Status status);
	};
}