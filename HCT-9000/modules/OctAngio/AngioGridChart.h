#pragma once

#include "OctAngioDef.h"
#include "AngioChart.h"

#include <memory>

namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class OCTANGIO_DLL_API AngioGridChart : public AngioChart
	{
	public:
		AngioGridChart();
		virtual ~AngioGridChart();

		AngioGridChart(AngioGridChart&& rhs);
		AngioGridChart& operator=(AngioGridChart&& rhs);
		AngioGridChart(const AngioGridChart& rhs);
		AngioGridChart& operator=(const AngioGridChart& rhs);

	public:
		void setSectionSize(float size = 1.0f);
		bool updateAngioChart(void) override;

		float flowsSection(int index) const;
		float densitySection(int index) const;

	private:
		struct AngioGridChartImpl;
		std::unique_ptr<AngioGridChartImpl> d_ptr;
		AngioGridChartImpl& getImpl(void) const;
	};
}

