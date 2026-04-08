#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API LineScanData : public ProtocolData
	{
	public:
		LineScanData();
		LineScanData(const OctScanPattern& desc);
		LineScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		LineScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~LineScanData();

		LineScanData(LineScanData&& rhs);
		LineScanData& operator=(LineScanData&& rhs);
		LineScanData(const LineScanData& rhs) = delete;
		LineScanData& operator=(const LineScanData& rhs) = delete;

	private:
		struct LineScanDataImpl;
		std::unique_ptr<LineScanDataImpl> d_ptr;
		LineScanDataImpl& getImpl(void) const;
	};
}

