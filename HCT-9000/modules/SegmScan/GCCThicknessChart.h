#pragma once

#include "SegmScanDef.h"
#include "MacularEnfaceChart.h"

#include <memory>
#include <vector>


namespace SegmScan
{

	class SEGMSCAN_DLL_API GCCThicknessChart : public MacularEnfaceChart
	{
	public:
		GCCThicknessChart();
		virtual ~GCCThicknessChart();

		GCCThicknessChart(GCCThicknessChart&& rhs);
		GCCThicknessChart& operator=(GCCThicknessChart&& rhs);
		GCCThicknessChart(const GCCThicknessChart& rhs);
		GCCThicknessChart& operator=(const GCCThicknessChart& rhs);

	public:
		void setSectionSize(float inner, float outer);

		float sectionThickness(int section) const;
		float averageThickness(void) const;
		float superiorThickness(void) const;
		float inferiorThickness(void) const;

	private:
		struct GCCThicknessChartImpl;
		std::unique_ptr<GCCThicknessChartImpl> d_ptr;
		GCCThicknessChartImpl& getImpl(void) const;
	};
}
