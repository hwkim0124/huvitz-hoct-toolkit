#pragma once

#include "OctDataDef.h"
#include "ProtocolData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API RadialScanData : public ProtocolData
	{
	public:
		RadialScanData();
		RadialScanData(const OctScanPattern& desc);
		RadialScanData(std::unique_ptr<OctResult::ScannerResult>&& result);
		RadialScanData(std::shared_ptr<OctResult::ScannerResult> result);
		virtual ~RadialScanData();

		RadialScanData(RadialScanData&& rhs);
		RadialScanData& operator=(RadialScanData&& rhs);
		RadialScanData(const RadialScanData& rhs) = delete;
		RadialScanData& operator=(const RadialScanData& rhs) = delete;

	private:
		struct RadialScanDataImpl;
		std::unique_ptr<RadialScanDataImpl> d_ptr;
		RadialScanDataImpl& getImpl(void) const;
	};
}