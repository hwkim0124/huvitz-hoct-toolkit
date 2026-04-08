#pragma once

#include "OctReportDef.h"
#include "OcularReport.h"


namespace OctData {
	class ProtocolData;
	class LineScanData;
	class CubeScanData;
}


namespace SegmScan {
	class OcularEnfaceImage;
	class OcularEnfaceMap;
}


namespace OctReport
{
	class OCTREPORT_DLL_API ScanReport2 : public OcularReport
	{
	public:
		ScanReport2();
		virtual ~ScanReport2();

		ScanReport2(ScanReport2&& rhs);
		ScanReport2& operator=(ScanReport2&& rhs);
		ScanReport2(const ScanReport2& rhs) = delete;
		ScanReport2& operator=(const ScanReport2& rhs) = delete;

	public:
		void setProtocolData(OctData::ProtocolData* data, EyeSide side, bool update = true);
		OctData::ProtocolData* getProtocolData(void) const;
		OctScanPattern* getDescript(void) const;

		bool isEmpty(void) const;

	protected:
		SegmScan::OcularEnfaceImage* prepareEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		SegmScan::OcularEnfaceImage* findEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		bool registEnfaceImage(std::unique_ptr<SegmScan::OcularEnfaceImage> enface);

		SegmScan::OcularEnfaceMap* prepareEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		SegmScan::OcularEnfaceMap* findEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		bool registEnfaceMap(std::unique_ptr<SegmScan::OcularEnfaceMap> tmap);

		virtual std::unique_ptr<SegmScan::OcularEnfaceImage> createEnfaceImage(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f);
		virtual std::unique_ptr<SegmScan::OcularEnfaceMap> createEnfaceMap(OcularLayerType upper, OcularLayerType lower, float upperOffset = 0.0f, float lowerOffset = 0.0f);

	private:
		struct ScanReport2Impl;
		std::unique_ptr<ScanReport2Impl> d_ptr;
		ScanReport2Impl& getImpl(void) const;
	};
}
