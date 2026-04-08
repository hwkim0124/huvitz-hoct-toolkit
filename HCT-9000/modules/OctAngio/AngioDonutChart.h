#pragma once


#include "OctAngioDef.h"
#include "AngioChart.h"

#include <memory>

namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class OCTANGIO_DLL_API AngioDonutChart : public AngioChart
	{
	public:
		AngioDonutChart();
		virtual ~AngioDonutChart();

		AngioDonutChart(AngioDonutChart&& rhs);
		AngioDonutChart& operator=(AngioDonutChart&& rhs);
		AngioDonutChart(const AngioDonutChart& rhs);
		AngioDonutChart& operator=(const AngioDonutChart& rhs);

	public:
		void setSectionDiameters(float inner = 1.0f, float outer = 2.5f);
		bool updateAngioChart(void) override;

		float flowsSection(int index) const;
		float flowsSuperior(void) const;
		float flowsInferior(void) const;
		float flowsTotal(void) const;

		float densitySection(int index) const;
		float densitySuperior(void) const;
		float densityInferior(void) const;
		float densityTotal(void) const;

	protected:

	private:
		struct AngioDonutChartImpl;
		std::unique_ptr<AngioDonutChartImpl> d_ptr;
		AngioDonutChartImpl& getImpl(void) const;
	};
}

