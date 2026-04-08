#pragma once

#include "OctReportDef.h"


namespace OctReport
{
	class OCTREPORT_DLL_API OcularReport
	{
	public:
		OcularReport();
		virtual ~OcularReport();

		OcularReport(OcularReport&& rhs);
		OcularReport& operator=(OcularReport&& rhs);
		OcularReport(const OcularReport& rhs) = delete;
		OcularReport& operator=(const OcularReport& rhs) = delete;

	public:
		void setEyeSide(EyeSide side);
		EyeSide getEyeSide(void) const;
		bool isEyeOD(void) const;

		virtual bool updateContents(void);
		virtual void clearContents(void);

	private:
		struct OcularReportImpl;
		std::unique_ptr<OcularReportImpl> d_ptr;
		OcularReportImpl& getImpl(void) const;
	};
}

