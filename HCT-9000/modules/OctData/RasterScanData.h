#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API RasterScanData : public ProtocolData
	{
	public:
		RasterScanData();
		RasterScanData(const OctScanPattern& desc);
		RasterScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		RasterScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~RasterScanData();

		RasterScanData(RasterScanData&& rhs);
		RasterScanData& operator=(RasterScanData&& rhs);
		RasterScanData(const RasterScanData& rhs) = delete;
		RasterScanData& operator=(const RasterScanData& rhs) = delete;

	private:
		struct RasterScanDataImpl;
		std::unique_ptr<RasterScanDataImpl> d_ptr;
		RasterScanDataImpl& getImpl(void) const;
	};
}