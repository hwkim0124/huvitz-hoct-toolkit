#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API DiagnosisItem
	{
	public:
		DiagnosisItem();
		virtual ~DiagnosisItem();

	private:
		long		m_index;
		wstring		m_name;
		long		m_groupIdx;
		int			m_displayOrder;
		Status		m_status;

	public:
		long		getIndex(void) { return m_index; }
		wstring		getName(void) { return m_name; }
		long		getGroupIdx(void) { return m_groupIdx; }
		int			getDisplayOrder(void) { return m_displayOrder; }
		Status		getStatus(void) { return m_status; }

		void		setIndex(long idx);
		void		setName(wstring name);
		void		setGroupIdx(long idx);
		void		setDisplayOrder(int order);
		void		setStatus(Status status);
	};
}