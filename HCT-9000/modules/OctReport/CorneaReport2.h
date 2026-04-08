#pragma once

#include "OctReportDef.h"
#include "ScanReport2.h"


namespace OctResult {
	class EnfaceImage;
}

namespace OctData {
	class ProtocolData;
	class LineScanData;
	class CubeScanData;
}

namespace SegmScan {
	class CorneaEplot;
	class CorneaChart;

	class CorneaThicknessMap;
	class CorneaRadiusMap;
	class CorneaETDRSChart;
}


namespace OctReport
{
	class OCTREPORT_DLL_API CorneaReport2 : public ScanReport2
	{
	public:
		CorneaReport2();
		virtual ~CorneaReport2();

		CorneaReport2(CorneaReport2&& rhs);
		CorneaReport2& operator=(CorneaReport2&& rhs);
		CorneaReport2(const CorneaReport2& rhs) = delete;
		CorneaReport2& operator=(const CorneaReport2& rhs) = delete;

	public:
		bool updateContents(void) override;
		void clearContents(void) override;

		SegmScan::CorneaThicknessMap* getThicknessMap(OcularLayerType upper, OcularLayerType lower);
		SegmScan::CorneaRadiusMap* getCurvatureRadiusMap(void);

		SegmScan::CorneaETDRSChart makeETDRSChart(OcularLayerType upper = OcularLayerType::EPI,
													OcularLayerType lower = OcularLayerType::END,
													float centerX = 0.0f, float centerY = 0.0f);

		// These interfaces had been deprecated. 
		SegmScan::CorneaEplot* getCorneaEplot(OcularLayerType upper, OcularLayerType lower);
		SegmScan::CorneaEplot* getCurvatureRadiusPlot(void);
		SegmScan::CorneaChart makeCorneaChart(OcularLayerType upper, OcularLayerType lower, float centerX = 0.0f, float centerY = 0.0f);

	protected:
		std::unique_ptr<SegmScan::OcularEnfaceMap> createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) override;
	
	private:
		struct CorneaReport2Impl;
		std::unique_ptr<CorneaReport2Impl> d_ptr;
		CorneaReport2Impl& getImpl(void) const;
	};
}



