#pragma once

#include "OctReportDef.h"
#include "ScanReport2.h"
#include "MacularReport2.h"

namespace OctResult {
	class EnfaceImage;
}

namespace OctData {
	class ProtocolData;
	class LineScanData;
	class CubeScanData;
}

namespace SegmScan {
	class DiscEshot;
	class DiscEplot;
	class DiscChart;

	class DiscThicknessMap;
	class DiscEnfaceImage;
	class DiscClockChart;
}


namespace OctReport
{
	class OCTREPORT_DLL_API DiscReport2 : public MacularReport2
	{
	public:
		DiscReport2();
		virtual ~DiscReport2();

		DiscReport2(DiscReport2&& rhs);
		DiscReport2& operator=(DiscReport2&& rhs);
		DiscReport2(const DiscReport2& rhs) = delete;
		DiscReport2& operator=(const DiscReport2& rhs) = delete;

	public:
		bool isNerveHeadCup(void) const;
		bool isNerveHeadDisc(void) const;
		bool isNerveHeadCenter(void) const;

		int getNerveHeadCenterLineIndex(void) const;
		int getNerveHeadCenterLateralPos(void) const;
		float getNerveHeadCenterX(void) const;
		float getNerveHeadCenterY(void) const;

		float getDiscArea(void) const;
		float getDiscVolume(void) const;
		float getCupArea(void) const;
		float getCupVolume(void) const;
		float getRimArea(void) const;

		float getCupDiscHorzRatio(void) const;
		float getCupDiscVertRatio(void) const;
		float getCupDiscAreaRatio(void) const;

		bool updateContents(void) override;
		void clearContents(void) override;

		SegmScan::DiscClockChart makeDiscClockChart(OcularLayerType upper = OcularLayerType::ILM, 
													OcularLayerType lower = OcularLayerType::NFL, float upperOffset = 0.0f, float lowerOffset = 0.0f,
													float centerX = 0.0f, float centerY = 0.0f, float outerDiam = ONH_DISC_RNFL_CIRCLE_SIZE);

		SegmScan::DiscEnfaceImage* getEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f);
		SegmScan::DiscThicknessMap* getThicknessMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);

		// These interfaces had been deprecated. 
		SegmScan::DiscEshot* getDiscEshot(OcularLayerType upper, OcularLayerType lower);
		SegmScan::DiscEplot* getDiscEplot(OcularLayerType upper, OcularLayerType lower);
		SegmScan::DiscChart makeDiscChart(OcularLayerType upper, OcularLayerType lower, 
			float centerX = 0.0f, float centerY = 0.0f, float outerDiam = ONH_DISC_RNFL_CIRCLE_SIZE);

	protected:
		void locateCupDiscCenter(void);
		void calculateCupDiscArea(void);
		void calculateCupDiscVolume(void);
		void calculateCupDiscRatio(void);

		std::unique_ptr<SegmScan::OcularEnfaceImage> createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) override;
		std::unique_ptr<SegmScan::OcularEnfaceMap> createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f) override;

	private:
		struct DiscReport2Impl;
		std::unique_ptr<DiscReport2Impl> d_ptr;
		DiscReport2Impl& getImpl(void) const;
	};
}

