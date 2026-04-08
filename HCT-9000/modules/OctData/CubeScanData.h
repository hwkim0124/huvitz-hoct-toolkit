#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API CubeScanData : public ProtocolData
	{
	public:
		CubeScanData();
		CubeScanData(const OctScanPattern& desc);
		CubeScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		CubeScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~CubeScanData();

		CubeScanData(CubeScanData&& rhs);
		CubeScanData& operator=(CubeScanData&& rhs);
		CubeScanData(const CubeScanData& rhs) = delete;
		CubeScanData& operator=(const CubeScanData& rhs) = delete;

	private:
		struct CubeScanDataImpl;
		std::unique_ptr<CubeScanDataImpl> d_ptr;
		CubeScanDataImpl& getImpl(void) const;
	};
}

