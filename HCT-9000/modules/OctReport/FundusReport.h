#pragma once

#include "OctReportDef.h"
#include "OcularReport.h"


namespace OctData {
	class FundusData;
}



namespace OctReport
{
	class OCTREPORT_DLL_API FundusReport : public OcularReport
	{
	public:
		FundusReport();
		virtual ~FundusReport();

		FundusReport(FundusReport&& rhs);
		FundusReport& operator=(FundusReport&& rhs);
		FundusReport(const FundusReport& rhs) = delete;
		FundusReport& operator=(const FundusReport& rhs) = delete;

	public:
		void setFundusData(OctData::FundusData* data);
		OctData::FundusData* getFundusData(void) const;
		bool isEmpty(void) const;


	private:
		struct FundusReportImpl;
		std::unique_ptr<FundusReportImpl> d_ptr;
		FundusReportImpl& getImpl(void) const;
	};
}

