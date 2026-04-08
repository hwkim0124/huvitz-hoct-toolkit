#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{

	class PATIENTDB_DLL_API Device
	{
	public:
		Device();
		virtual ~Device();

	private:
		long		m_index;
		wstring		m_name;
		wstring		m_description;
		wstring		m_ipAddress;
		int			m_port;
		Status		m_status;

	public:
		long		getIndex(void) { return m_index; }
		wstring		getName(void) { return m_name; }
		wstring		getDescription(void) { return m_description; }
		wstring		getIpAddress(void) { return m_ipAddress; }
		int			getPort(void) { return m_port; }
		Status		getStatus(void) { return m_status; }

		void		setIndex(long idx);
		void		setName(wstring name);
		void		setDescription(wstring desc);
		void		setIpAddress(wstring ipAddress);
		void		setPort(int port);
		void		setStatus(Status status);
	};
}
