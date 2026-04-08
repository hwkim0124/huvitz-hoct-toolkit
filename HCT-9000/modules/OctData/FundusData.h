#pragma once

#include "OctDataDef.h"

#include <string>
#include <memory>
#include <vector>

namespace OctResult {
	class FundusResult;
}


namespace OctData
{
	class OCTDATA_DLL_API FundusData
	{
	public:
		FundusData();
		FundusData(const OctFundusMeasure& desc);
		FundusData(std::unique_ptr<OctResult::FundusResult>&& result);
		virtual ~FundusData();

		FundusData(FundusData&& rhs);
		FundusData& operator=(FundusData&& rhs);
		FundusData(const FundusData& rhs) = delete;
		FundusData& operator=(const FundusData& rhs) = delete;

	public:
		OctResult::FundusResult* getResult(void) const;
		OctFundusMeasure& getDescript(void) const;
		void setDescript(const OctFundusMeasure& desc);

		const OctFundusImage* getFundusImageDescript(int sectIdx, int imageIdx = 0);
		const OctRetinaImage* getRetinaImageDescript(void) const;
		const OctCorneaImage* getCorneaImageDescript(void) const;

		void importFundusResult(std::unique_ptr<OctResult::FundusResult>&& result);

		void clear(void);

	private:
		struct FundusDataImpl;
		std::unique_ptr<FundusDataImpl> d_ptr;
		FundusDataImpl& getImpl(void) const;
	};
}
