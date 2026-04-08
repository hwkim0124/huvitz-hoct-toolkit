#pragma once

#include "SegmScanDef.h"
#include "CorneaEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaETDRSChart : public CorneaEnfaceChart
	{
	public:
		CorneaETDRSChart();
		virtual ~CorneaETDRSChart();

		CorneaETDRSChart(CorneaETDRSChart&& rhs);
		CorneaETDRSChart& operator=(CorneaETDRSChart&& rhs);
		CorneaETDRSChart(const CorneaETDRSChart& rhs);
		CorneaETDRSChart& operator=(const CorneaETDRSChart& rhs);

	public:
		void setSectionSize(float center = CORNEA_CENTER_CIRCLE_SIZE, 
							float inner = CORNEA_INNER_CIRCLE_SIZE, 
							float outer = CORNEA_OUTER_CIRCLE_SIZE);

		float centerThickness(void) const;
		float innerThickness(int section) const;
		float outerThickness(int section) const;
		float averageThickness(void) const;
		float superiorThickness(void) const;
		float inferiorThickness(void) const;

	private:
		struct CorneaETDRSChartImpl;
		std::unique_ptr<CorneaETDRSChartImpl> d_ptr;
		CorneaETDRSChartImpl& getImpl(void) const;
	};
}
