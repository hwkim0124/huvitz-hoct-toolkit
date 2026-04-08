#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API FetchPropertyScanResult
	{
	public:
		FetchPropertyScanResult();
		virtual ~FetchPropertyScanResult();

		void			clear(void);
		void			addExamIndex(long index);
		void			setEyeSide(int side);
		void			setMaxCount(long count);
		wstring			getCondition(void);

	private:
		vector <long>	m_examIndex;
		int				m_eyeSide;
		long			m_maxCount;
	};
}
