#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API DiscEnfaceChart : public OcularEnfaceChart
	{
	public:
		DiscEnfaceChart();
		virtual ~DiscEnfaceChart();

		DiscEnfaceChart(DiscEnfaceChart&& rhs);
		DiscEnfaceChart& operator=(DiscEnfaceChart&& rhs);
		DiscEnfaceChart(const DiscEnfaceChart& rhs);
		DiscEnfaceChart& operator=(const DiscEnfaceChart& rhs);

	public:

	private:
		struct DiscEnfaceChartImpl;
		std::unique_ptr<DiscEnfaceChartImpl> d_ptr;
		DiscEnfaceChartImpl& getImpl(void) const;
	};
}

