#pragma once

#include "SegmScanDef.h"
#include "DiscClockChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class DiscEplot;

	class SEGMSCAN_DLL_API DiscChart : public DiscClockChart
	{
	public:
		DiscChart();
		virtual ~DiscChart();

		DiscChart(DiscChart&& rhs);
		DiscChart& operator=(DiscChart&& rhs);
		DiscChart(const DiscChart& rhs);
		DiscChart& operator=(const DiscChart& rhs);

	public:

	private:
		struct DiscChartImpl;
		std::unique_ptr<DiscChartImpl> d_ptr;
		DiscChartImpl& getImpl(void) const;
	};
}
