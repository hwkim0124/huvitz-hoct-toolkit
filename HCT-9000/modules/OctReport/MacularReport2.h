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
	class MacularEshot;
	class MacularEplot;
	class MacularChart;

	class GCCThicknessChart;
	class MacularETDRSChart;
	class MacularEnfaceImage;
	class MacularThicknessMap;
}


namespace OctReport
{
	class OCTREPORT_DLL_API MacularReport2 : public ScanReport2
	{
	public:
		MacularReport2();
		virtual ~MacularReport2();

		MacularReport2(MacularReport2&& rhs);
		MacularReport2& operator=(MacularReport2&& rhs);
		MacularReport2(const MacularReport2& rhs) = delete;
		MacularReport2& operator=(const MacularReport2& rhs) = delete;

	public:
		bool isFoveaCenter(void) const;
		float getFoveaCenterX(void) const;
		float getFoveaCenterY(void) const;
		float getFoveaCenterThickness(void) const;

		int getFoveaCenterLineIndex(void) const;
		int getFoveaCenterLateralPos(void) const;

		bool updateContents(void) override;
		void clearContents(void) override;

		SegmScan::GCCThicknessChart makeGCCThicknessChart(OcularLayerType upper = OcularLayerType::ILM, 
														  OcularLayerType lower = OcularLayerType::IPL, 
														  float upperOffset = 0.0f, float lowerOffset = 0.0f,
														  float centerX = 0.0f, float centerY = 0.0f);
		SegmScan::MacularETDRSChart makeETDRSChart(OcularLayerType upper = OcularLayerType::ILM,
													OcularLayerType lower = OcularLayerType::RPE, 
													float upperOffset = 0.0f, float lowerOffset = 0.0f,
													float centerX = 0.0f, float centerY = 0.0f);

		SegmScan::MacularEnfaceImage* getEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		SegmScan::MacularThicknessMap* getThicknessMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);

		// These interfaces had been deprecated. 
		SegmScan::MacularEshot* getMacularEshot(OcularLayerType upper, OcularLayerType lower);
		SegmScan::MacularEplot* getMacularEplot(OcularLayerType upper, OcularLayerType lower);
		SegmScan::MacularChart makeMacularChart(OcularLayerType upper, OcularLayerType lower, float centerX = 0.0f, float centerY = 0.0f);


	protected:
		void locateFoveaCenter(void);
		void determineFoveaCenterInMacularScan(float rangeX, float rangeY, int numLines, int numPoints, EyeSide side);
		void detectFoveaCenterInMacularScan(float rangeX, float rangeY, int numLines, int numPoints);
		void detectFoveaCenterInWideScan(float rangeX, float rangeY, int numLines, int numPoints);

		std::unique_ptr<SegmScan::OcularEnfaceImage> createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) override;
		std::unique_ptr<SegmScan::OcularEnfaceMap> createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) override;

	private:
		struct MacularReport2Impl;
		std::unique_ptr<MacularReport2Impl> d_ptr;
		MacularReport2Impl& getImpl(void) const;
	};
}

