#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>


namespace RetParam {
	class EnfaceDraw;
	class EnfacePlot;
}


namespace Gdiplus {
	class Bitmap;
}


namespace SegmScan
{
	class PatternOutput;
	class SurfaceImage;


	class SEGMSCAN_DLL_API ScanReport
	{
	public:
		ScanReport();
		virtual ~ScanReport();

		ScanReport(ScanReport&& rhs);
		ScanReport& operator=(ScanReport&& rhs);
		ScanReport(const ScanReport& rhs);
		ScanReport& operator=(const ScanReport& rhs);

	private:
		struct ScanReportImpl;
		std::unique_ptr<ScanReportImpl> d_ptr;
		ScanReportImpl& getImpl(void) const;

	protected:
		int insertEnfaceDraw(RetParam::EnfaceDraw& draw);
		int insertEnfacePlot(RetParam::EnfacePlot& plot);
		RetParam::EnfaceDraw* getEnfaceDraw(int index) const;
		RetParam::EnfacePlot* getEnfacePlot(int index) const;
		int getEnfaceDrawCount(void) const;
		int getEnfacePlotCount(void) const;

	public:
		void setPatternOutput(PatternOutput* output, bool clear = true);
		PatternOutput* getPatternOutput(void) const;

		virtual void clearAllContents(void);

		int createCorneaIRimage(float width, float height, int wpix=0, int hpix=0, const wchar_t* path = nullptr);
		int createRetinaIRimage(float width, float height, int wpix=0, int hpix=0, const wchar_t* path = nullptr);
		int createRetinaOCTimage(float width, float height, int wpix=0, int hpix=0, const wchar_t* path = nullptr);
		SurfaceImage* getCorneaIRimage(int index = 0) const;
		SurfaceImage* getRetinaIRimage(int index = 0) const;
		SurfaceImage* getRetinaOCTimage(int index = 0) const;

		int getCorneaIRimageCount(void) const;
		int getRetinaIRimageCount(void) const;
		int getRetinaOCTimageCount(void) const;
		void removeAllCorneaIRimages(void) ;
		void removeAllRetinaIRimages(void) ;
		void removeAllRetinaOCTimages(void) ;

	};
}
