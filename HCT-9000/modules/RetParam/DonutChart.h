#pragma once

#include "RetParamDef.h"
#include "EnfaceChart.h"

#include <memory>
#include <vector>


namespace RetParam
{
	class RETPARAM_DLL_API DonutChart : public EnfaceChart
	{
	public:
		DonutChart();
		virtual ~DonutChart();

		DonutChart(DonutChart&& rhs);
		DonutChart& operator=(DonutChart&& rhs);
		DonutChart(const DonutChart& rhs);
		DonutChart& operator=(const DonutChart& rhs);

	public:
		bool updateContent(void) override;

		void setInnerDiameter(float diam);
		void setOuterDiameter(float diam);

		float meanSection(int index) const;
		float meanTotal(void) const;
		float meanSuperior(void) const;
		float meanInferior(void) const;

	protected:
		bool calculateSections(void);

	private:
		struct DonutChartImpl;
		std::unique_ptr<DonutChartImpl> d_ptr;
		DonutChartImpl& getImpl(void) const;
	};
}

