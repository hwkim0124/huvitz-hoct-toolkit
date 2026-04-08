#pragma once


#include "OctAngioDef.h"
#include "AngioChart.h"

#include <memory>

namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class OCTANGIO_DLL_API AngioEtdrsChart : public AngioChart
	{
	public:
		AngioEtdrsChart();
		virtual ~AngioEtdrsChart();

		AngioEtdrsChart(AngioEtdrsChart&& rhs);
		AngioEtdrsChart& operator=(AngioEtdrsChart&& rhs);
		AngioEtdrsChart(const AngioEtdrsChart& rhs);
		AngioEtdrsChart& operator=(const AngioEtdrsChart& rhs);

	public:
		void setSectionDiameters(float inner = 1.5f, float outer = 2.5f);
		bool updateAngioChart(void) override;

		float flowsCenter(void) const;
		float flowsSection(int index) const;
		float flowsTotal(void) const;
		float flowsSuperior(void) const;
		float flowsInferior(void) const;

		float densityCenter(void) const;
		float densitySection(int index) const;
		float densityTotal(void) const;
		float densitySuperior(void) const;
		float densityInferior(void) const;

	protected:

	private:
		struct AngioEtdrsChartImpl;
		std::unique_ptr<AngioEtdrsChartImpl> d_ptr;
		AngioEtdrsChartImpl& getImpl(void) const;
	};
}

