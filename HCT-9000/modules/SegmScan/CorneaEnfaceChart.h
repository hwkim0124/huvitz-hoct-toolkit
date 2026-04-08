#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaEnfaceChart : public OcularEnfaceChart
	{
	public:
		CorneaEnfaceChart();
		virtual ~CorneaEnfaceChart();

		CorneaEnfaceChart(CorneaEnfaceChart&& rhs);
		CorneaEnfaceChart& operator=(CorneaEnfaceChart&& rhs);
		CorneaEnfaceChart(const CorneaEnfaceChart& rhs);
		CorneaEnfaceChart& operator=(const CorneaEnfaceChart& rhs);

	public:

	private:
		struct CorneaEnfaceChartImpl;
		std::unique_ptr<CorneaEnfaceChartImpl> d_ptr;
		CorneaEnfaceChartImpl& getImpl(void) const;
	};
}

