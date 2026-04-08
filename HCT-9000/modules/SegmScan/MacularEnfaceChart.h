#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API MacularEnfaceChart : public OcularEnfaceChart
	{
	public:
		MacularEnfaceChart();
		virtual ~MacularEnfaceChart();

		MacularEnfaceChart(MacularEnfaceChart&& rhs);
		MacularEnfaceChart& operator=(MacularEnfaceChart&& rhs);
		MacularEnfaceChart(const MacularEnfaceChart& rhs);
		MacularEnfaceChart& operator=(const MacularEnfaceChart& rhs);

	public:


	private:
		struct MacularEnfaceChartImpl;
		std::unique_ptr<MacularEnfaceChartImpl> d_ptr;
		MacularEnfaceChartImpl& getImpl(void) const;
	};
}

