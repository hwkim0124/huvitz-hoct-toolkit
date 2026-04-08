#pragma once


#include "SegmScanDef.h"
#include "MacularEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{

	class SEGMSCAN_DLL_API MacularETDRSChart : public MacularEnfaceChart
	{
	public:
		MacularETDRSChart();
		virtual ~MacularETDRSChart();

		MacularETDRSChart(MacularETDRSChart&& rhs);
		MacularETDRSChart& operator=(MacularETDRSChart&& rhs);
		MacularETDRSChart(const MacularETDRSChart& rhs);
		MacularETDRSChart& operator=(const MacularETDRSChart& rhs);

	public:
		void setSectionSize(float center = MACULAR_CENTER_CIRCLE_SIZE, 
							float inner = MACULAR_INNER_CIRCLE_SIZE, 
							float outer = MACULAR_OUTER_CIRCLE_SIZE);

		float centerThickness(void) const;
		float innerThickness(int section) const;
		float outerThickness(int section) const;
		float hexagonalThickness(int section) const;

		float minCenterThickness(void) const;
		float maxCenterThickness(void) const;
		float centerThicknessPercentile(void) const;
		float innerThicknessPercentile(int section) const;
		float outerThicknessPercentile(int section) const;
		float hexagonalThicknessPercentile(int section) const;

		float averageThickness(void) const;
		float superiorThickness(void) const;
		float inferiorThickness(void) const;

	private:
		struct MacularETDRSChartImpl;
		std::unique_ptr<MacularETDRSChartImpl> d_ptr;
		MacularETDRSChartImpl& getImpl(void) const;
	};
}

