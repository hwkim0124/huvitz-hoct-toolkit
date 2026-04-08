#pragma once
#include <string>
#include "PatientDBDef.h"

using namespace std;

namespace PatientDB
{
	class PATIENTDB_DLL_API FetchPropertyFundusResult
	{
	public:
		FetchPropertyFundusResult();
		virtual ~FetchPropertyFundusResult();

		void			clear(void);
		void			addExamIndex(long index);
		void			setEyeSide(long side);
		void			setMaxCount(long count);
		wstring			getCondition(void);

	private:
		vector <long>	m_examIndex;
		long			m_eyeSide;
		long			m_maxCount;
	};
}