#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API ScanPattern
	{
	public:
		ScanPattern();
		virtual ~ScanPattern();
		void clear(void);

	private:
		long		m_index;
		int			m_code;
		wstring		m_name;
		wstring		m_version;

		ScanType	m_scanType;
		Status		m_status;

	public:
		long		getIndex(void) { return m_index; }
		int			getCode(void) { return m_code; }
		wstring		getName(void) { return m_name; }
		wstring		getVersion(void) { return m_version; }
		ScanType	getScanType(void) { return m_scanType; }
		Status		getStatus(void) { return m_status; }

		void		setIndex(long idx);
		void		setCode(int code);
		void		setName(wstring name);
		void		setVersion(wstring version);
		void		setScanType(ScanType type);
		void		setStatus(Status status);
	};
}
