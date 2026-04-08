#pragma once

#include "SegmScanDef.h"
#include "ScanReport.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class MacularImage;
	class MacularPlot;
	class SurfaceImage;


	class SEGMSCAN_DLL_API MacularReport : public ScanReport
	{
	public:
		MacularReport();
		virtual ~MacularReport();
		 
		MacularReport(MacularReport&& rhs);
		MacularReport& operator=(MacularReport&& rhs);
		MacularReport(const MacularReport& rhs);
		MacularReport& operator=(const MacularReport& rhs);

	private:
		struct MacularReportImpl;
		std::unique_ptr<MacularReportImpl> d_ptr;
		MacularReportImpl& getImpl(void) const;

	public:
		virtual void clearAllContents(void);

		int createMacularImage(int width, int height, LayerType upper = LayerType::ILM, LayerType lower = LayerType::IOS);
		int createMacularPlot(int width, int height, LayerType upper = LayerType::ILM, LayerType lower = LayerType::IOS);
		MacularImage* getMacularImage(int index = 0) const;
		MacularPlot* getMacularPlot(int index = 0) const;

		int getMacularImageCount(void) const;
		int getMacularPlotCount(void) const;
		void removeAllMacularImages(void);
		void removeAllMacularPlots(void);
	};
}
