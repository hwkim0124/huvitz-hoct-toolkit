#include "stdafx.h"
#include "Analysis.h"

#include "OctData2.h"
#include "OctResult2.h"
#include "OctReport2.h"
#include "SegmProc2.h"
#include "CppUtil2.h"


using namespace OctSystem;
using namespace OctData;
using namespace OctResult;
using namespace OctReport;
using namespace SegmProc;
using namespace std;
using namespace CppUtil;


struct Analysis::AnalysisImpl
{
	ProtocolDataset protocolSet ;
	FundusDataset fundusSet;
	OcularReportSet reportSet;

	AnalysisImpl() {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Analysis::AnalysisImpl> Analysis::d_ptr(new AnalysisImpl());


Analysis::Analysis()
{
}


Analysis::~Analysis()
{
}


OctData::ProtocolData* OctSystem::Analysis::insertProtocolData(std::shared_ptr<OctResult::ScannerResult> result, bool segment, bool clear)
{
	if (clear) {
		clearAllScanData();
	}

	auto descript = result->getDescript();
	auto pattern = result->getDescript().getPattern();

	ProtocolData* data;
	if (pattern.isCubeScan()) {
		data = obtainCubeScanData();
	}
	else if (pattern.isCircleScan()) {
		data = obtainCircleScanData();
	}
	else if (pattern.isCrossScan()) {
		data = obtainCrossScanData();
	}
	else if (pattern.isRadialScan()) {
		data = obtainRadialScanData();
	}
	else if (pattern.isRasterScan()) {
		data = obtainRasterScanData();
	}
	else {
		data = obtainLineScanData();
	}

	data->importScannerResult(result);

	if (segment) {
		doSegmentation(data);
	}
	return data;
}


bool OctSystem::Analysis::checkIfProtocolDataExist(int index)
{
	return (getProtocolData(index) != nullptr);
}


OctData::ProtocolData * OctSystem::Analysis::getProtocolData(int index)
{
	return getProtocolSet().getProtocolData(index);
}


LineScanData * OctSystem::Analysis::obtainLineScanData(int index)
{
	return getProtocolSet().getOrCreateLineScanData(index) ;
}


CubeScanData * OctSystem::Analysis::obtainCubeScanData(int index)
{
	return getProtocolSet().getOrCreateCubeScanData(index);
}


OctData::CircleScanData * OctSystem::Analysis::obtainCircleScanData(int index)
{
	return getProtocolSet().getOrCreateCircleScanData(index);
}


OctData::CrossScanData * OctSystem::Analysis::obtainCrossScanData(int index)
{
	return getProtocolSet().getOrCreateCrossScanData(index);
}


OctData::RadialScanData * OctSystem::Analysis::obtainRadialScanData(int index)
{
	return getProtocolSet().getOrCreateRadialScanData(index);
}


OctData::RasterScanData * OctSystem::Analysis::obtainRasterScanData(int index)
{
	return getProtocolSet().getOrCreateRasterScanData(index);
}


void OctSystem::Analysis::clearAllScanData(void)
{
	getProtocolSet().clear();
	return;
}


bool OctSystem::Analysis::checkIfFundusDataExist(int index)
{
	return (getFundusData(index) != nullptr);
}


OctData::FundusData * OctSystem::Analysis::getFundusData(int index)
{
	return getFundusSet().getFundusData(index);
}


OctData::FundusData * OctSystem::Analysis::obtainFundusData(int index)
{
	return getFundusSet().getOrCreateFundusData(index);
}


void OctSystem::Analysis::clearAllFundusData(void)
{
	getFundusSet().clear();
	return;
}


bool OctSystem::Analysis::doSegmentation(ProtocolData * data, bool save, std::wstring exportPath)
{
	if (data == nullptr) {
		return false;
	}

	CppUtil::ClockTimer::start();

	if (data->getDescript().isCorneaScan()) {
		CorneaSession sess;
		sess.loadScanData(data);
		sess.processScanData();
	}
	else {
		if (data->getDescript().isMacularScan()) {
			MacularSession sess;
			sess.loadScanData(data);
			sess.processScanData();
		}
		else {
			DiscSession sess;
			sess.loadScanData(data);
			sess.processScanData();
		}
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "SegmSession process elapsed: " << msec;

	if (save) {
		data->exportBsegmResults(exportPath);
	}
	return true;
}


bool OctSystem::Analysis::doDewarpingOfCorneaScan(OctData::ProtocolData * data, bool save)
{
	if (data == nullptr) {
		return false;
	}

	if (!data->getDescript().isCorneaScan()) {
		return false;
	}

	CppUtil::ClockTimer::start();

	CorneaSession sess;
	sess.loadScanData(data);
	sess.processDewarping();

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Dewarping process elapsed: " << msec;

	if (save) {
		data->updatePatternImages();
		data->updatePreviewImages();
	}

	return true;
}


OctReport::MacularReport2 * OctSystem::Analysis::obtainMacularReport(int index)
{
	return getReports().getOrCreateMacularReport(index);
}


OctReport::DiscReport2 * OctSystem::Analysis::obtainDiscReport(int index)
{
	return getReports().getOrCreateDiscReport(index);
}


OctReport::CorneaReport2 * OctSystem::Analysis::obtainCorneaReport(int index)
{
	return getReports().getOrCreateCorneaReport(index);
}


OctReport::FundusReport * OctSystem::Analysis::obtainFundusReport(int index)
{
	return getReports().getOrCreateFundusReport(index);
}


void OctSystem::Analysis::clearAllReports(void)
{
	getReports().clear();
	return;
}


Analysis::AnalysisImpl & OctSystem::Analysis::getImpl(void)
{
	return *d_ptr;
}


OctData::ProtocolDataset & OctSystem::Analysis::getProtocolSet(void)
{
	return d_ptr->protocolSet;
}


OctData::FundusDataset & OctSystem::Analysis::getFundusSet(void)
{
	return d_ptr->fundusSet;
}


OctReport::OcularReportSet & OctSystem::Analysis::getReports(void)
{
	return d_ptr->reportSet;
}

bool OctSystem::Analysis::getCorneaLayerAndCurvature(ProtocolData* data, OcularLayerType layer, std::vector<std::vector<int>>& points, std::vector<std::vector<float>>& curves)
{
	if (data == nullptr) {
		return false;
	}

	CorneaSession sess;
	if (data->getDescript().isCorneaScan()) {
		sess.loadScanData(data);
		sess.processScanData();
	}
	else {
		return false;
	}

	for (int i = 0; i < 16; i++) {
		auto bscan = data->getBscanData(i);
		points[i] = bscan->getLayerPoints(layer);
		curves[i] = bscan->getCorneaCurvature(1); //AxialSimK
	}

	return true;
}