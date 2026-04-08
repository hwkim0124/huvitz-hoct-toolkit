#include "stdafx.h"
#include "Angiography.h"
#include "Loader.h"

#include "OctData2.h"
#include "OctResult2.h"
#include "OctReport2.h"
#include "OctAngio2.h"
#include "SegmProc2.h"
#include "CppUtil2.h"
#include "SigChain2.h"
#include "SigProc2.h"

#include <vector>

using namespace OctSystem;
using namespace OctData;
using namespace OctResult;
using namespace OctReport;
using namespace SegmProc;
using namespace std;
using namespace CppUtil;
using namespace SigChain;
using namespace SigProc;
using namespace OctAngio;


struct Angiography::AngiographyImpl
{
	ProtocolData* data;
	vector<vector<int>> upperLayers;
	vector<vector<int>> lowerLayers;
	vector<vector<int>> upperLayers2;
	vector<vector<int>> lowerLayers2;
	vector<vector<int>> upperLayers3;
	vector<vector<int>> lowerLayers3;

	bool useVasularLayers;
	bool useMotionCorrect;
	bool useBiasFieldCorrect;
	float noiseReductionRate;

	std::string importDirPath = "";

	int currentIdx = 0;
	vector<OctAngio::Angiogram2> angiogram = vector<OctAngio::Angiogram2>(8);

	AngiographyImpl() : data(nullptr), useVasularLayers(false), useMotionCorrect(true), 
						useBiasFieldCorrect(true), noiseReductionRate(0.25f) {

	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Angiography::AngiographyImpl> Angiography::d_ptr(new AngiographyImpl());


Angiography::Angiography()
{
}


Angiography::~Angiography()
{
}


bool OctSystem::Angiography::initiateAngiogram(OctData::ProtocolData * data, bool prepare)
{
	getImpl().data = data;

	if (!isValidProtocol()) {
		return false;
	}

	auto& desc = data->getDescript();
	if (!initiateAngiogram(desc, prepare)) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::initiateAngiogram(const OctScanPattern & desc, bool prepare)
{
	int lines = desc.getNumberOfScanLines();
	int points = desc.getNumberOfScanPoints();
	int repeats = desc.getScanOverlaps();
	bool vertical = !desc.isHorizontal();

	getAngiogram().resetLayout(lines, points, repeats, vertical);
	getAngiogram().resetScanRange(desc.getScanRangeX(), desc.getScanRangeY());

	if (!getAngiogram().loadDataBuffer()) {
		return false;
	}

	loadLayerSegmentsToAngiogram();

	if (prepare) {
		getAngiogram().prepareFlowData(true, true);
	}
	return true;
}


bool OctSystem::Angiography::initiateAngiogramByDataFile(OctData::ProtocolData * data, const std::string dirPath, const std::string fileName)
{
	getImpl().data = data;
	
	if (!isValidProtocol()) {
		return false;
	}

	auto& desc = data->getDescript();
	if (!initiateAngiogramByDataFile(desc, dirPath, fileName)) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::initiateAngiogramByDataFile(const OctScanPattern & desc, const std::string dirPath, const std::string fileName)
{
	int lines = desc.getNumberOfScanLines();
	int points = desc.getNumberOfScanPoints();
	int repeats = desc.getScanOverlaps();
	bool vertical = !desc.isHorizontal();

	getAngiogram().setImportPath(dirPath);
	getAngiogram().resetLayout(lines, points, repeats, vertical);
	getAngiogram().resetScanRange(desc.getScanRangeX(), desc.getScanRangeY());

	if (!getAngiogram().loadDataFiles(dirPath, fileName)) {
		return false;
	}

	loadLayerSegmentsToAngiogram();
	getAngiogram().prepareFlowData(true, false);
	return true;
}


bool OctSystem::Angiography::initiateAngiogramByDataFile(int lines, int points, int repeats, bool vertical, const std::string dirPath, const std::string fileName)
{
	getAngiogram().setImportPath(dirPath);
	getAngiogram().resetLayout(lines, points, repeats, vertical);

	if (!getAngiogram().loadDataFiles(dirPath, fileName)) {
		return false;
	}

	loadLayerSegmentsToAngiogram();
	getAngiogram().prepareFlowData(true, false);
	return true;
}


bool OctSystem::Angiography::initiateAngiogramByDataFile2(const std::string dirPath, const std::string fileName)
{
	getAngiogram().setImportPath(dirPath);
	if (!getAngiogram().loadDataFileVer2(dirPath, fileName)) {
		return false;
	}

	loadLayerSegmentsToAngiogram();
	getAngiogram().prepareFlowData(false, false);
	return true;
}


bool OctSystem::Angiography::initiateAngiogramByImageFiles(int lines, int points, int repeats, bool vertical, const std::string dirPath)
{
	getAngiogram().setImportPath(dirPath);
	getAngiogram().resetLayout(lines, points, repeats, vertical);

	if (!getAngiogram().loadDataImages(dirPath)) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::prepareAngiogram(bool alignAxial, bool alignLateral, bool realign)
{
	getAngiogram().useAlignAxial() = alignAxial;
	getAngiogram().useAlignLateral() = alignLateral;

	loadLayerSegmentsToAngiogram();

	if (!getAngiogram().prepareFlowData(true, realign)) {
		return false;
	}
	return true;
}

bool OctSystem::Angiography::generateMotionData(void)
{
	if (!getAngiogram().estimateMotionVectors()) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::updateAngiogram(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, 
											float rangeX, float rangeY, float centerX, float centerY, bool isDisc)
{
	if (!getAngiogram().isAmplitduesValid()) {
		return false;
	}

	getAngiogram().useVascularLayers() = getImpl().useVasularLayers;
	getAngiogram().useMotionCorrection() = getImpl().useMotionCorrect;
	getAngiogram().useBiasFieldCorrection() = getImpl().useBiasFieldCorrect;
	getAngiogram().noiseReductionRate() = getImpl().noiseReductionRate;

	getAngiogram().resetSlabRange(upper, lower, upperOffset, lowerOffset);
	getAngiogram().resetScanRange(rangeX, rangeY, centerX, centerY);
	getAngiogram().generateProjection(true);
	return true;
}


bool OctSystem::Angiography::updateAngiogram2(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, float rangeX, float rangeY, float centerX, float centerY, bool isDisc)
{
	getAngiogram().useVascularLayers() = getImpl().useVasularLayers;
	getAngiogram().useMotionCorrection() = getImpl().useMotionCorrect;
	getAngiogram().useBiasFieldCorrection() = getImpl().useBiasFieldCorrect;
	getAngiogram().noiseReductionRate() = getImpl().noiseReductionRate;

	getAngiogram().resetSlabRange(upper, lower, upperOffset, lowerOffset);
	getAngiogram().resetScanRange(rangeX, rangeY, centerX, centerY);
	getAngiogram().generateProjection(false);
	return true;
}


bool OctSystem::Angiography::importDataFile(const std::string dirPath, const std::string fileName)
{
	if (!getAngiogram().loadDataFiles(dirPath, fileName)) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::exportDataFile(const std::string dirPath, const std::string fileName)
{
	if (!getAngiogram().isAmplitduesValid()) {
		LogD() << "Invalid amplitudues, exporting angio data file failed!";
		return false;
	}

	if (!getAngiogram().saveDataFiles(dirPath)) {
		return false;
	}
	return true;
}


bool OctSystem::Angiography::importDataFile2(const std::string dirPath, const std::string fileName)
{
	if (!getAngiogram().loadDataFileVer2(dirPath, fileName)) {
		return false;
	}

	return true;
}


bool OctSystem::Angiography::exportDataFile2(const std::string dirPath, const std::string fileName)
{
	if (!getAngiogram().isDecorrelationsValid()) {
		LogD() << "Invalid decorrelations, exporting angio data file failed!";
		return false;
	}

	if (!getAngiogram().saveDataFileVer2(dirPath)) {
		return false;
	}

	return true;
}


std::string OctSystem::Angiography::getImportDirPath(void)
{
	return getAngiogram().importPath();
}


bool OctSystem::Angiography::loadLayerSegmentsToAngiogram(void)
{
	if (isValidProtocol()) {
		AngioLayers layers;
		if (!importLayerSegmentsFromProtocolData(layers)) {
			return false;
		}
		getAngiogram().setLayerSegments(move(layers));
	}
	else {
		getAngiogram().loadLayerSegments();
	}

	return true;
}


CppUtil::CvImage OctSystem::Angiography::getAngioImage(int width, int height, float clipLimit)
{
	auto image = getAngiogram().createAnigoImage();

	if (!image.isEmpty()) {
		/*
		if (clipLimit <= 0.0f) {
			clipLimit = GlobalSettings::angioEnhanceParam();
		}
		*/

		if (clipLimit > 0.0f) {
			image.equalizeHistogram(clipLimit);
		}

		if (width != 0 && height != 0) {
			if (width != image.getWidth() || height != image.getHeight()) {
				image.resize(width, height);
			}
		}
	}

	return image;
}


CppUtil::CvImage OctSystem::Angiography::getOffsetImage(int width, int height)
{
	auto image = getAngiogram().createOffsetImage();

	if (!image.isEmpty()) {
		if (width != 0 && height != 0) {
			if (width != image.getWidth() || height != image.getHeight()) {
				image.resize(width, height);
			}
		}
	}

	return image;
}


CppUtil::CvImage OctSystem::Angiography::getScanImage(int lineIdx, int width, int height, float clipLimit)
{
	auto image = getAngiogram().createScanImage(lineIdx);

	if (!image.isEmpty()) {
		if (clipLimit > 0.0f) {
			image.equalizeHistogram(clipLimit);
		}

		if (width != 0 && height != 0) {
			if (width != image.getWidth() || height != image.getHeight()) {
				image.resize(width, height);
			}
		}
	}

	return image;
}


CppUtil::CvImage OctSystem::Angiography::getDecorrImage(int lineIdx, int width, int height, float clipLimit)
{
	auto image = getAngiogram().createDecorrImage(lineIdx);

	if (!image.isEmpty()) {
		if (width != 0 && height != 0) {
			if (width != image.getWidth() || height != image.getHeight()) {
				image.resize(width, height);
			}
		}
	}

	return image;
}


OctAngio::AngioEtdrsChart OctSystem::Angiography::createEtdrsChart(float threshold, float centerX, float centerY, float innerDiam, float outerDiam)
{
	auto chart = OctAngio::AngioEtdrsChart();
	auto image = getAngiogram().createAnigoImage();

	if (image.isEmpty()) {
		return chart;
	}

	auto& angio = getAngiogram();
	chart.setupAngioChart(image, angio.scanRangeX(), angio.scanRangeY(), angio.imageWidth(), angio.imageHeight());
	chart.setCenterPosition(centerX, centerY);
	chart.setSectionDiameters(innerDiam, outerDiam);
	chart.setThreshold(threshold);
	chart.updateAngioChart();

	return chart;
}


OctAngio::AngioDonutChart OctSystem::Angiography::createDonutChart(float threshold, float centerX, float centerY, float innerDiam, float outerDiam)
{
	auto chart = OctAngio::AngioDonutChart();
	auto image = getAngiogram().createAnigoImage();

	if (image.isEmpty()) {
		return chart;
	}

	auto& angio = getAngiogram();
	chart.setupAngioChart(image, angio.scanRangeX(), angio.scanRangeY(), angio.imageWidth(), angio.imageHeight());
	chart.setCenterPosition(centerX, centerY);
	chart.setSectionDiameters(innerDiam, outerDiam);
	chart.setThreshold(threshold);
	chart.updateAngioChart();

	return chart;
}


OctAngio::AngioGridChart OctSystem::Angiography::createGridChart(float threshold, float centerX, float centerY, float gridSize)
{
	auto chart = OctAngio::AngioGridChart();
	auto image = getAngiogram().createAnigoImage();

	if (image.isEmpty()) {
		return chart;
	}

	auto& angio = getAngiogram();
	chart.setupAngioChart(image, angio.scanRangeX(), angio.scanRangeY(), angio.imageWidth(), angio.imageHeight());
	chart.setCenterPosition(centerX, centerY);
	chart.setSectionSize(gridSize);
	chart.setThreshold(threshold);
	chart.updateAngioChart();

	return chart;
}


bool OctSystem::Angiography::isAllDataLoaded(void)
{
	if (!getAngiogram().isAmplitduesValid()) {
		return false;
	}
	/*
	if (!AngioSetup::checkIfAllDataLoaded()) {
		LogD() << "Anigo amplitudes not loaded";
		return false;
	}
	*/
	return true;
}


void OctSystem::Angiography::useVascularLayers(bool flag)
{
	getImpl().useVasularLayers = flag;
	return;
}


void OctSystem::Angiography::useMotionCorrection(bool flag)
{
	getImpl().useMotionCorrect = flag;
	return;
}


void OctSystem::Angiography::useBiasFieldCorrection(bool flag)
{
	getImpl().useBiasFieldCorrect = flag;
	return;
}


void OctSystem::Angiography::setNoiseReductionRate(float rate)
{
	getImpl().noiseReductionRate = min(max(rate, 0.0f), 10.0f);
	return;
}


bool OctSystem::Angiography::isValidProtocol(void)
{
	auto data = getImpl().data;
	if (data == nullptr) {
		// LogD() << "Angio protocol data is not valid!";
		return false;
	}
	/*
	if (data->countBscanSections() != AngioSetup::numberOfAngioLines()) {
		LogD() << "Angio protocol data invalid, dsize: " << data->countBscanSections() << ", lines: " << AngioSetup::numberOfAngioLines();
		return false;
	}
	*/
	return true;
}


bool OctSystem::Angiography::importLayerSegmentsFromProtocolData(AngioLayers& layers)
{
	if (!isValidProtocol()) {
		return false;
	}

	auto& desc = getImpl().data->getDescript();
	int lines = desc.getNumberOfScanLines();
	int repeats = desc.getScanOverlaps();
	int points = desc.getNumberOfScanPoints();
	int isize = lines * repeats;

	LogD() << "Import layers from protocol data, lines: " << lines << ", repeats: " << repeats << ", points: " << points;
	layers.setupAngioLayers(lines, points, repeats);

	for (int i = 0; i < lines; i++) {
		auto bscan = getImpl().data->getBscanData(i);
		auto height = bscan->getImageHeight();

		if (bscan && bscan->isBsegmResult()) {
			for (int j = 0; j < NUMBER_OF_RETINA_LAYERS; j++) {
				auto k = static_cast<int>(OcularLayerType::ILM) + j;
				auto type = static_cast<OcularLayerType>(k);
				/*
				if (type == OcularLayerType::OPR) {
					continue;
				}
				*/

				auto v = bscan->getLayerPoints(type);
				if (v.size() != points) {
					if (type == OcularLayerType::OPR) {
						auto v = bscan->getLayerPoints(OcularLayerType::BRM);
					}
					if (v.size() != points) {
						LogD() << "Angio layer segment result empty, at line: " << i;
						break;
					}
				}
				layers.setLayerPoints(i, type, v);
			}
		}
		else {
			LogD() << "Angio segment result empty, at line: " << i;
			continue;
		}
	}

	layers.testLayerSegments();
	return true;
}


bool OctSystem::Angiography::importLayerSegments(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	if (!isValidProtocol()) {
		return false;
	}

	auto& desc = getImpl().data->getDescript();
	int nLines = desc.getNumberOfScanLines();
	int nRepeats = desc.getScanOverlaps();
	int nPoints = desc.getNumberOfScanPoints();
	int isize = nLines * nRepeats;

	LogD() << "Angio layer segment files importing, lines: " << nLines << ", repeats: " << nRepeats << ", points: " << nPoints;
	LogD() << "Upper layer: " << LayerTypeToString(upper) << ", offset: " << upperOffset;
	LogD() << "Lower layer: " << LayerTypeToString(lower) << ", offset: " << lowerOffset;

	getImpl().upperLayers = vector<vector<int>>(nLines);
	getImpl().lowerLayers = vector<vector<int>>(nLines);
	getImpl().upperLayers2 = vector<vector<int>>(nLines);
	getImpl().lowerLayers2 = vector<vector<int>>(nLines);
	getImpl().upperLayers3 = vector<vector<int>>(nLines);
	getImpl().lowerLayers3 = vector<vector<int>>(nLines);

	for (int i = 0; i < nLines; i++) {
		auto bscan = getImpl().data->getBscanData(i);
		auto height = bscan->getImageHeight();
		vector<int> layer1;
		vector<int> layer2;

		// Following sections could have empty layers. 
		if (bscan && bscan->isBsegmResult()) {
			layer1 = bscan->getLayerPoints(upper);
			layer2 = bscan->getLayerPoints(lower);
		}
		else {
			LogD() << "Angio layer segment result empty, at line: " << i;
			continue;
		}

		if (layer1.size() != nPoints || layer2.size() != nPoints) {
			// layer1 = vector<int>(nPoints, -1);
			// layer2 = vector<int>(nPoints, -1);
			LogD() << "Angio layer segment size not matched, at line: " << i << ", size: " << layer1.size();
		}
		else {
			if (getImpl().useVasularLayers)
			{
				if (upper == OcularLayerType::ILM && lower == OcularLayerType::IPL) {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 0; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 5; });

					auto layerE = bscan->getLayerPoints(OcularLayerType::IOS);
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (upper == OcularLayerType::NFL && lower == OcularLayerType::IPL) {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 1; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 5; });

					auto layerE = bscan->getLayerPoints(OcularLayerType::IOS);
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (upper == OcularLayerType::IPL && lower == OcularLayerType::OPL) {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 5; });
					transform(layer1.begin(), layer1.end(), layer2.begin(), [&](int x) { return x + 23; });

					auto layerE = bscan->getLayerPoints(OcularLayerType::IOS);
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (upper == OcularLayerType::IPL && lower == OcularLayerType::IPL) {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 5; });
					transform(layer1.begin(), layer1.end(), layer2.begin(), [&](int x) { return x + 23; });

					auto layerE = bscan->getLayerPoints(OcularLayerType::IOS);
					transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
					transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });
				}
				else if (upper == OcularLayerType::IPL && lower == OcularLayerType::BRM) {
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + 23; });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + 0; });
				}
				else if (upper == OcularLayerType::BRM && lower == OcularLayerType::BRM) {
					transform(layer1.begin(), layer1.end(), layer2.begin(), [&](int x) { return min(x + 5, height - 1); });
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return min(x + 15, height - 1); });
				}
			}
			else {
				if (upperOffset != 0.0f) {
					int offset = (int)(upperOffset / GlobalSettings::getRetinaScanAxialResolution());
					transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return min(max(x + offset, 0), height-1); });
				}
				if (lowerOffset != 0.0f) {
					int offset = (int)(lowerOffset / GlobalSettings::getRetinaScanAxialResolution());
					transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return min(max(x + offset, 0), height - 1); });
				}
				if (upperOffset != 0.0f || lowerOffset != 0.0f) {
					transform(begin(layer2), end(layer2), begin(layer1), begin(layer2), [](int e1, int e2) { return max(e1, e2); });
				}
			}

			getImpl().upperLayers[i] = layer1;
			getImpl().lowerLayers[i] = layer2;

			if (i == 0) {
				if (layer1.size() > 5) {
					auto& v = layer1;
					LogD() << "Uppers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
				}
				else {
					LogD() << "Uppers: empty";
				}
				if (layer2.size() > 5) {
					auto& v = layer2;
					LogD() << "Lowers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
				}
				else {
					LogD() << "Lowers: empty";
				}
			}

			{
				OcularLayerType upper = OcularLayerType::NFL;
				OcularLayerType lower = OcularLayerType::IPL;
				float upperOffset = 0.0f;
				float lowerOffset = 15.0f;

				if (i == 0) {
					LogD() << "Angio layer segment files importing, lines: " << nLines << ", repeats: " << nRepeats << ", points: " << nPoints;
					LogD() << "Upper layer: " << LayerTypeToString(upper) << ", offset: " << upperOffset;
					LogD() << "Lower layer: " << LayerTypeToString(lower) << ", offset: " << lowerOffset;
				}

				auto layer1 = bscan->getLayerPoints(upper);
				auto layer2 = bscan->getLayerPoints(lower);
				int offset1 = (int)(upperOffset / GlobalSettings::getRetinaScanAxialResolution());
				int offset2 = (int)(lowerOffset / GlobalSettings::getRetinaScanAxialResolution());

				transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + offset1; });
				transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + offset2; });

				auto layerE = bscan->getLayerPoints(OcularLayerType::OPL);
				transform(begin(layer1), end(layer1), begin(layerE), begin(layer1), [](int e1, int e2) { return min(e1, e2); });
				transform(begin(layer2), end(layer2), begin(layerE), begin(layer2), [](int e1, int e2) { return min(e1, e2); });

				getImpl().upperLayers2[i] = layer1;
				getImpl().lowerLayers2[i] = layer2;

				if (i == 0) {
					if (layer1.size() > 5) {
						auto& v = layer1;
						LogD() << "Uppers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
					}
					else {
						LogD() << "Uppers: empty";
					}
					if (layer2.size() > 5) {
						auto& v = layer2;
						LogD() << "Lowers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
					}
					else {
						LogD() << "Lowers: empty";
					}
				}
			}

			{
				OcularLayerType upper = OcularLayerType::IOS;
				OcularLayerType lower = OcularLayerType::BRM;
				float upperOffset = 0.0f;
				float lowerOffset = 0.0f;

				if (i == 0) {
					LogD() << "Angio layer segment files importing, lines: " << nLines << ", repeats: " << nRepeats << ", points: " << nPoints;
					LogD() << "Upper layer: " << LayerTypeToString(upper) << ", offset: " << upperOffset;
					LogD() << "Lower layer: " << LayerTypeToString(lower) << ", offset: " << lowerOffset;
				}

				auto layer1 = bscan->getLayerPoints(upper);
				auto layer2 = bscan->getLayerPoints(lower);
				int offset1 = (int)(upperOffset / GlobalSettings::getRetinaScanAxialResolution());
				int offset2 = (int)(lowerOffset / GlobalSettings::getRetinaScanAxialResolution());

				transform(layer1.begin(), layer1.end(), layer1.begin(), [&](int x) { return x + offset1; });
				transform(layer2.begin(), layer2.end(), layer2.begin(), [&](int x) { return x + offset2; });

				getImpl().upperLayers3[i] = layer1;
				getImpl().lowerLayers3[i] = layer2;

				if (i == 0) {
					if (layer1.size() > 5) {
						auto& v = layer1;
						LogD() << "Uppers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
					}
					else {
						LogD() << "Uppers: empty";
					}
					if (layer2.size() > 5) {
						auto& v = layer2;
						LogD() << "Lowers: " << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ", " << v[4];
					}
					else {
						LogD() << "Lowers: empty";
					}
				}
			}
		}
	}
	return true;
}


void OctSystem::Angiography::clearAngiography(void)
{
	getImpl().importDirPath = "";
	getImpl().useVasularLayers = false;
	getImpl().useBiasFieldCorrect = true;
	getImpl().useMotionCorrect = true;
	getImpl().upperLayers.clear();
	getImpl().lowerLayers.clear();
	getImpl().currentIdx = 0;
	getImpl().angiogram = vector<OctAngio::Angiogram2>(8);
	getImpl().data = nullptr;
	return;
}


void OctSystem::Angiography::setCurrentAngiogram(int index)
{
	int idx = min(max(index, 0), (int)(getImpl().angiogram.size() - 1));
	getImpl().currentIdx = idx;
	return;
}


OctAngio::Angiogram2 & OctSystem::Angiography::getAngiogram(void)
{
	int idx = getImpl().currentIdx;
	return getImpl().angiogram[idx];
}


Angiography::AngiographyImpl & OctSystem::Angiography::getImpl(void)
{
	return *d_ptr;
}