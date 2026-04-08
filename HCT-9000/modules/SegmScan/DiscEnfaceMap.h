#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceMap.h"

#include <memory>
#include <vector>

namespace SegmScan
{
	class SEGMSCAN_DLL_API DiscEnfaceMap : public OcularEnfaceMap
	{
	public:
		DiscEnfaceMap();
		virtual ~DiscEnfaceMap();

		DiscEnfaceMap(DiscEnfaceMap&& rhs);
		DiscEnfaceMap& operator=(DiscEnfaceMap&& rhs);
		DiscEnfaceMap(const DiscEnfaceMap& rhs);
		DiscEnfaceMap& operator=(const DiscEnfaceMap& rhs);

	private:
		struct DiscEnfaceMapImpl;
		std::unique_ptr<DiscEnfaceMapImpl> d_ptr;
		DiscEnfaceMapImpl& getImpl(void) const;
	};
}

