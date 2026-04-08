#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API CrossScanData : public ProtocolData
	{
	public:
		CrossScanData();
		CrossScanData(const OctScanPattern& desc);
		CrossScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		CrossScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~CrossScanData();

		CrossScanData(CrossScanData&& rhs);
		CrossScanData& operator=(CrossScanData&& rhs);
		CrossScanData(const CrossScanData& rhs) = delete;
		CrossScanData& operator=(const CrossScanData& rhs) = delete;

	private:
		struct CrossScanDataImpl;
		std::unique_ptr<CrossScanDataImpl> d_ptr;
		CrossScanDataImpl& getImpl(void) const;
	};
}
