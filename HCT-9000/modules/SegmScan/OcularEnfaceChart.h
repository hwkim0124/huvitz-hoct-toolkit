#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>

namespace RetParam {
	class EnfaceChart;
	class EnfaceTmap;
}


namespace SegmScan
{
	class OcularEnfaceImage;
	class OcularEnfaceMap;

	class SEGMSCAN_DLL_API OcularEnfaceChart
	{
	public:
		OcularEnfaceChart();
		virtual ~OcularEnfaceChart();

		OcularEnfaceChart(OcularEnfaceChart&& rhs);
		OcularEnfaceChart& operator=(OcularEnfaceChart&& rhs);
		OcularEnfaceChart(const OcularEnfaceChart& rhs);
		OcularEnfaceChart& operator=(const OcularEnfaceChart& rhs);

	public:
		EyeSide eyeSide(void) const;
		RetParam::EnfaceChart* enfaceChart(void) const;
		RetParam::EnfaceTmap* enfaceData(void) const;

		void setEyeSide(EyeSide side);
		void setEnfaceChart(RetParam::EnfaceChart* chart);
		void setEnfaceData(RetParam::EnfaceTmap* tmap);
		void setLocation(float xInMM, float yInMM);

		virtual bool updateMetrics(void);
		virtual bool updateMetrics(const std::vector<float>& thicks);
		bool isEmpty(void) const;

	private:
		struct OcularEnfaceChartImpl;
		std::unique_ptr<OcularEnfaceChartImpl> d_ptr;
		OcularEnfaceChartImpl& getImpl(void) const;
	};
}

