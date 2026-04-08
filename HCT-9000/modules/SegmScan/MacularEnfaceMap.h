#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceMap.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API MacularEnfaceMap : public OcularEnfaceMap
	{
	public:
		MacularEnfaceMap();
		virtual ~MacularEnfaceMap();

		MacularEnfaceMap(MacularEnfaceMap&& rhs);
		MacularEnfaceMap& operator=(MacularEnfaceMap&& rhs);
		MacularEnfaceMap(const MacularEnfaceMap& rhs);
		MacularEnfaceMap& operator=(const MacularEnfaceMap& rhs);

	private:
		struct MacularEnfaceMapImpl;
		std::unique_ptr<MacularEnfaceMapImpl> d_ptr;
		MacularEnfaceMapImpl& getImpl(void) const;
	};
}

