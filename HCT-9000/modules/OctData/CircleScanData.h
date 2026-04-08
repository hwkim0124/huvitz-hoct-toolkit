#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API CircleScanData : public ProtocolData
	{
	public:
		CircleScanData();
		CircleScanData(const OctScanPattern& desc);
		CircleScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		CircleScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~CircleScanData();

		CircleScanData(CircleScanData&& rhs);
		CircleScanData& operator=(CircleScanData&& rhs);
		CircleScanData(const CircleScanData& rhs) = delete;
		CircleScanData& operator=(const CircleScanData& rhs) = delete;

	private:
		struct CircleScanDataImpl;
		std::unique_ptr<CircleScanDataImpl> d_ptr;
		CircleScanDataImpl& getImpl(void) const;
	};
}