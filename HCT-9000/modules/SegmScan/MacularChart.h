#pragma once

#include "SegmScanDef.h"
#include "MacularETDRSChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class MacularEplot;

	class SEGMSCAN_DLL_API MacularChart : public MacularETDRSChart
	{
	public:
		MacularChart();
		virtual ~MacularChart();

		MacularChart(MacularChart&& rhs);
		MacularChart& operator=(MacularChart&& rhs);
		MacularChart(const MacularChart& rhs);
		MacularChart& operator=(const MacularChart& rhs);

	public:

	private:
		struct MacularChartImpl;
		std::unique_ptr<MacularChartImpl> d_ptr;
		MacularChartImpl& getImpl(void) const;
	};
}
