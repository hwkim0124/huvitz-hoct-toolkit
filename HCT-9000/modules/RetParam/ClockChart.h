#pragma once

#include "RetParamDef.h"
#include "EnfaceChart.h"

#include <memory>
#include <vector>


namespace RetParam
{
	class RETPARAM_DLL_API ClockChart : public EnfaceChart
	{
	public:
		ClockChart();
		virtual ~ClockChart();

		ClockChart(ClockChart&& rhs);
		ClockChart& operator=(ClockChart&& rhs);
		ClockChart(const ClockChart& rhs);
		ClockChart& operator=(const ClockChart& rhs);

	public:
		void setOuterDiameter(float diam);
		bool updateContent(void) override;
		bool updateContent(const std::vector<float>& thicks) override;

		float meanClock(int index) const;
		float meanQuadrant(int index) const;

		const std::vector<float> getGraphTSNIT(int size) const;

	protected:
		bool calculateSections(void);
		bool calculateRnflThickness(void);
		bool calculateRnflThickness(const std::vector<float>& thicks);

		std::vector<float> makeGraphTSNIT(int size, EyeSide side, bool fromTemple = true);
		bool updateClockSections(const std::vector<float>& graph);

	private:
		struct ClockChartImpl;
		std::unique_ptr<ClockChartImpl> d_ptr;
		ClockChartImpl& getImpl(void) const;
	};
}

