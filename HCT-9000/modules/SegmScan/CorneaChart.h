#pragma once

#include "SegmScanDef.h"
#include "CorneaETDRSChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaChart : public CorneaETDRSChart
	{
	public:
		CorneaChart();
		virtual ~CorneaChart();

		CorneaChart(CorneaChart&& rhs);
		CorneaChart& operator=(CorneaChart&& rhs);
		CorneaChart(const CorneaChart& rhs);
		CorneaChart& operator=(const CorneaChart& rhs);

	public:

	private:
		struct CorneaChartImpl;
		std::unique_ptr<CorneaChartImpl> d_ptr;
		CorneaChartImpl& getImpl(void) const;
	};
}
