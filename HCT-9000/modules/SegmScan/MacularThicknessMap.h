#pragma once

#include "SegmScanDef.h"
#include "MacularEnfaceMap.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API MacularThicknessMap : public MacularEnfaceMap
	{
	public:
		MacularThicknessMap();
		virtual ~MacularThicknessMap();

		MacularThicknessMap(MacularThicknessMap&& rhs);
		MacularThicknessMap& operator=(MacularThicknessMap&& rhs);
		MacularThicknessMap(const MacularThicknessMap& rhs);
		MacularThicknessMap& operator=(const MacularThicknessMap& rhs);

	private:
		struct MacularThicknessMapImpl;
		std::unique_ptr<MacularThicknessMapImpl> d_ptr;
		MacularThicknessMapImpl& getImpl(void) const;
	};
}

