#pragma once

#include "OctResultDef.h"

#include <memory>


namespace OctResult
{
	class OCTRESULT_DLL_API PatientData
	{
	public:
		PatientData();
		PatientData(OctPatient data);
		virtual ~PatientData();

		PatientData(PatientData&& rhs);
		PatientData& operator=(PatientData&& rhs);
		PatientData(const PatientData& rhs);
		PatientData& operator=(const PatientData& rhs);

	public:
		void setPatient(const OctPatient& data);
		OctPatient& getPatient(void) const;

	private:
		struct PatientDataImpl;
		std::unique_ptr<PatientDataImpl> d_ptr;
		PatientDataImpl& getImpl(void) const;
	};
}
