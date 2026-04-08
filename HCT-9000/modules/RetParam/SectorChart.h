#pragma once

#include "RetParamDef.h"
#include "EnfaceChart.h"

#include <memory>
#include <vector>


namespace RetParam
{
	class RETPARAM_DLL_API SectorChart : public EnfaceChart
	{
	public:
		SectorChart();
		virtual ~SectorChart();

		SectorChart(SectorChart&& rhs);
		SectorChart& operator=(SectorChart&& rhs);
		SectorChart(const SectorChart& rhs);
		SectorChart& operator=(const SectorChart& rhs);

	public:
		void setCenterDiameter(float diam);
		void setInnerDiameter(float diam);
		void setOuterDiameter(float diam);
		void setCorneaThickness(bool flag);

		bool updateContent(void) override;

		float meanCenter(void) const;
		float meanInnerSector(int index) const;
		float meanOuterSector(int index) const;
		float meanHexagonalSector(int index) const;

		float minCenter(void) const;
		float maxCenter(void) const;
		
		float meanTotal(void) const;
		float meanSuperior(void) const;
		float meanInferior(void) const;

	protected:
		bool calculateRetinaThickness(void);
		bool calculateCorneaThickness(void);

	private:
		struct SectorChartImpl;
		std::unique_ptr<SectorChartImpl> d_ptr;
		SectorChartImpl& getImpl(void) const;
	};
}

