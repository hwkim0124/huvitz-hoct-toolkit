#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctData {
	class ProtocolData;
	class ProtocolDataset;

	class LineScanData;
	class CubeScanData;
	class CircleScanData;
	class CrossScanData;
	class RadialScanData;
	class RasterScanData;

	class FundusData;
	class FundusDataset;
}


namespace OctResult {
	class ScannerResult;
	class FundusResult;
}


namespace OctReport {
	class OcularReport;
	class OcularReportSet;

	class MacularReport2;
	class DiscReport2;
	class CorneaReport2;

	class FundusReport;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API Analysis
	{
	public:
		Analysis();
		virtual ~Analysis();

	public:
		static OctData::ProtocolData* insertProtocolData(std::shared_ptr<OctResult::ScannerResult> result, bool segment = false, bool clear = true);

		static bool checkIfProtocolDataExist(int index = 0);
		static OctData::ProtocolData* getProtocolData(int index = 0);
		static OctData::LineScanData* obtainLineScanData(int index = 0);
		static OctData::CubeScanData* obtainCubeScanData(int index = 0);
		static OctData::CircleScanData* obtainCircleScanData(int index = 0);
		static OctData::CrossScanData* obtainCrossScanData(int index = 0);
		static OctData::RadialScanData* obtainRadialScanData(int index = 0);
		static OctData::RasterScanData* obtainRasterScanData(int index = 0);
		static void clearAllScanData(void);

		static bool checkIfFundusDataExist(int index = 0);
		static OctData::FundusData* getFundusData(int index = 0);
		static OctData::FundusData* obtainFundusData(int index = 0);
		static void clearAllFundusData(void);

		static bool doSegmentation(OctData::ProtocolData* data, bool save = false, std::wstring exportPath = L"./export");
		static bool doDewarpingOfCorneaScan(OctData::ProtocolData* data, bool save = false);

		static OctReport::MacularReport2* obtainMacularReport(int index = 0);
		static OctReport::DiscReport2* obtainDiscReport(int index = 0);
		static OctReport::CorneaReport2* obtainCorneaReport(int index = 0);

		static OctReport::FundusReport* obtainFundusReport(int index = 0);
		static void clearAllReports(void);
		static bool getCorneaLayerAndCurvature(OctData::ProtocolData * data, OcularLayerType layer, std::vector<std::vector<int>>& points, std::vector<std::vector<float>>& curves);
	
	protected:
		static OctData::ProtocolDataset& getProtocolSet(void);
		static OctData::FundusDataset& getFundusSet(void);
		static OctReport::OcularReportSet& getReports(void);

	private:
		struct AnalysisImpl;
		static std::unique_ptr<AnalysisImpl> d_ptr;
		static AnalysisImpl& getImpl(void);

	};
}
