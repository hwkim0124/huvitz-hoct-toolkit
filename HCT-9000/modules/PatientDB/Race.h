#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API Race
	{
	public:
		Race();
		virtual ~Race();

	private:
		long		m_index;
		wstring		m_name;
		int			m_displayOrder;
		Status		m_status;

	public:
		long		getIndex(void) { return m_index; }
		wstring		getName(void) { return m_name; }
		int			getDisplayOrder(void) { return m_displayOrder; }
		Status		getStatus(void) { return m_status; }

		void		setIndex(long idx);
		void		setName(wstring name);
		void		setDisplayOrder(int order);
		void		setStatus(Status status);

		static bool disporder_asc(Race &data1, Race &data2) {
			return (data2.m_displayOrder > data1.m_displayOrder);
		}

		static bool disporder_desc(Race &data1, Race &data2) {
			return (data1.m_displayOrder > data2.m_displayOrder);
		}
	};
}