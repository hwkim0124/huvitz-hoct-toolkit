#include "stdafx.h"
#include "Angiogram2.h"
#include "AngioData.h"
#include "AngioFile.h"
#include "AngioLayout.h"
#include "AngioLayers.h"
#include "AngioDecorr.h"
#include "AngioMotion.h"
#include "AngioPost.h"
#include "CppUtil2.h"
#include "SigChain2.h"

#include <numeric>
#include <algorithm>

using namespace CppUtil;
using namespace OctAngio;
using namespace SigChain;
using namespace std;
using namespace cv;


struct Angiogram2::Angiogram2Impl
{
	AngioData data;
	AngioFile file;
	AngioLayout layout;
	AngioLayers layers;
	AngioLayers layers2;
	AngioLayers layers3;

	AngioDecorr decorr;
	AngioDecorr decorr2;
	AngioDecorr decorr3;
	AngioMotion motion;
	AngioPost post;

	vector<unsigned char> imageBits;

	bool useAlignAxial = true;
	bool useAlignLateral = false;
	bool useDecorrCircular = false;
	bool useBiasFieldCorrect = true;

	bool useLayersSelected = true;
	bool useVascularLayers = false;
	bool useMotionCorrect = true;
	bool usePostProcessing = true;
	bool useNormProjection = true;
	bool useDecorrOutput = false;
	bool useDifferOutput = false; 

	bool useProjectionRemoval = true;
	bool useReflectCorrect = true;
	bool useProjectionStats = false;

	float upperThreshold = 0.0f;
	float lowerThreshold = 0.0f;
	float decorrUpperThreshold = 0.0f;
	float decorrLowerThreshold = 0.0f;
	float differUpperThreshold = 0.0f;
	float differLowerThreshold = 0.0f;

	float decorrBaseThreshold = 0.0f; // 0.05f;
	float normalizeDropOff = 0.0f;
	float biasFieldSigma = 15.0f;
	float noiseReductionRate = 0.5f;

	int numberOfOverlaps = 0;
	bool usePixelAverage = false;
	int pixelAverageOffset = 3; // 2;

	int avgOffsetInnFlows = 0;
	int avgOffsetOutFlows = 0;
	float thresholdRatioInnFlows = 1.0f;
	float thresholdRatioOutFlows = 1.0f;

	int avgOffsetVascMask = ANGIO_PROJ_MASK_AVG_SIZE;
	int avgOffsetProjMask = ANGIO_PROJ_MASK_AVG_SIZE;
	float weightProjMask = 1.0f;
	int avgOffsetInnProj = 9;
	int avgOffsetOutProj = 9;

	bool isModelEye = false;

	std::string importPath;

	Angiogram2Impl() {
	};
};



Angiogram2::Angiogram2()
	: d_ptr(make_unique<Angiogram2Impl>())
{
}


Angiogram2::~Angiogram2()
{
	// Destructor should be defined for unique_ptr to delete Angiogram2Impl as an incomplete type.
}


OctAngio::Angiogram2::Angiogram2(Angiogram2 && rhs) = default;
Angiogram2 & OctAngio::Angiogram2::operator=(Angiogram2 && rhs) = default;


void OctAngio::Angiogram2::resetLayout(int lines, int points, int repeats, bool vertical)
{
	Layout().setupLayout(lines, points, repeats, vertical);
	Layers().setupAngioLayers(lines, points, repeats);
	Layers2().setupAngioLayers(lines, points, repeats);
	Layers3().setupAngioLayers(lines, points, repeats);
	return;
}


void OctAngio::Angiogram2::resetScanRange(float rangeX, float rangeY, float centerX, float centerY, bool isDisc)
{
	Layout().setupRange(rangeX, rangeY, centerX, centerY, isDisc);
	return;
}


void OctAngio::Angiogram2::resetSlabRange(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset)
{
	Layers().setupSlabRange(upper, lower, upperOffset, lowerOffset);
	return;
}


std::string OctAngio::Angiogram2::importPath(void)
{
	return getImpl().importPath;
}


void OctAngio::Angiogram2::setImportPath(std::string path)
{
	getImpl().importPath = path;
	return;
}


bool OctAngio::Angiogram2::loadDataBuffer(void)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();

	if (Data().fetchAmplitudesFromBuffer(lines, points, repeats)) {
		return true;
	}
	return false;
}


bool OctAngio::Angiogram2::loadDataFiles(const std::string dirPath, const std::string fileName)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();
	auto vertical = Layout().isVerticalScan();

	std::wstring path = atow(dirPath);
	if (!SystemFunc::isDirectory(path)) {
		LogE() << "Import directory to load angio data not found!, path: " << dirPath;
		return false;
	}

	CppUtil::ClockTimer::start();
	LogD() << "Loading angiogram data, lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", vertical: " << vertical;

	if (!Data().importAmplitudesFromDataFiles(lines, points, repeats, dirPath, fileName)) {
		return false;
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angiogram data loaded, elapsed: " << msec;
	return true;
}


bool OctAngio::Angiogram2::loadDataFileVer2(const std::string dirPath, const std::string fileName)
{
	std::wstring path = atow(dirPath);
	if (!SystemFunc::isDirectory(path)) {
		LogE() << "Import directory to load angio data not found!, path: " << dirPath;
		return false;
	}

	CppUtil::ClockTimer::start();
	LogD() << "Loading angio data file2...";

	if (!AngioFile::loadAngioDataFile(dirPath, fileName, Layout(), Decorr(), Motion())) {
		return false;
	}
	else {
		auto lines = Layout().numberOfLines();
		auto points = Layout().numberOfPoints();
		auto repeats = Layout().numberOfRepeats();
		auto vertical = Layout().isVerticalScan();
		resetLayout(lines, points, repeats, vertical);
		LogD() << "lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", vertical: " << vertical;
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angiogram data loaded, elapsed: " << msec;
	return true;
}



bool OctAngio::Angiogram2::loadDataImages(const std::string dirPath)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();

	CppUtil::ClockTimer::start();
	LogD() << "Loading angiogram images, lines: " << lines << ", points: " << points << ", repeats: " << repeats;

	if (!Data().importAmplitudesFromImageFiles(lines, points, repeats, dirPath)) {
		return false;
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angiogram images loaded, elapsed: " << msec;
	return true;
}


bool OctAngio::Angiogram2::saveDataFiles(const std::string dirPath, const std::string fileName)
{
	int lines = Layout().numberOfLines();
	int repeats = Layout().numberOfRepeats();

	std::wstring path = atow(dirPath);
	if (!SystemFunc::isDirectory(path)) {
		LogE() << "Export directory not found for angio data!, path: " << dirPath;
		return false;
	}

	if (Data().exportAmplitudesToDataFiles(lines, repeats, dirPath, fileName)) {
		return true;
	}
	return false;
}


bool OctAngio::Angiogram2::saveDataFileVer2(const std::string dirPath, const std::string fileName)
{
	std::wstring path = atow(dirPath);
	if (!SystemFunc::isDirectory(path)) {
		LogE() << "Export directory not found for angio data!, path: " << dirPath;
		return false;
	}

	if (AngioFile::saveAngioDataFile(dirPath, fileName, Layout(), Decorr(), Motion())) {
		return true;
	}
	
	return false;
}


bool OctAngio::Angiogram2::isAmplitduesValid(void)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();

	if (Data().checkIfAmplitudesLoaded(lines, points, repeats)) {
		return true;
	}
	return false;
}


bool OctAngio::Angiogram2::isDecorrelationsValid(void)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();

	if (Decorr().checkIfDecorrelationsLoaded(lines, points, repeats)) {
		return true;
	}
	return false;
}


bool OctAngio::Angiogram2::isAngioImage(void)
{
	return imageBits() != nullptr;
}


bool OctAngio::Angiogram2::isFoveaAvascularZone(void)
{
	if (Layout().isDiscScan()) {
		return false;
	}

	if (Layout().scanRangeX() > 6.0f || Layout().scanRangeY() > 6.0f) {
		return false;
	}

	if (Layers().isOuterRetinaFlows()) {
		return false;
	}

	return true;
}

bool OctAngio::Angiogram2::isModelEyeImage(void)
{
	return getImpl().isModelEye;
}


bool OctAngio::Angiogram2::loadLayerSegments(const std::string dirPath)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();
	bool vascular = useVascularLayers();

	auto path = dirPath;
	if (path.empty()) {
		path = getImpl().importPath;
	}

	if (!Layers().loadLayerSegmentFiles(path)) {
		return false;
	}

	return true;
}



bool OctAngio::Angiogram2::loadLayerSegments(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset, const std::string dirPath)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();
	bool vascular = useVascularLayers();

	auto path = dirPath;
	if (path.empty()) {
		path = getImpl().importPath;
	}

	if (!Layers().importLayersFromSegmentFiles(upper, lower, upperOffset, lowerOffset, path, vascular)) {
		return false;
	}

	if (!Layers2().importLayersFromSegmentFiles(OcularLayerType::NFL, OcularLayerType::IPL, 0.0f, 15.0f, path)) {
		return false;
	}

	if (!Layers3().importLayersFromSegmentFiles(OcularLayerType::IOS, OcularLayerType::BRM, 0.0f, 0.0f, path)) {
		return false;
	}
	return true;
}


void OctAngio::Angiogram2::setLayerSegments(AngioLayers&& layers)
{
	getImpl().layers = move(layers);
	return;
}


void OctAngio::Angiogram2::setLayerSegments(OcularLayerType upper, OcularLayerType lower, 
											LayerArrays& upperLayers, LayerArrays& lowerLayers,
											LayerArrays& upperLayers2, LayerArrays& lowerLayers2,
											LayerArrays& upperLayers3, LayerArrays& lowerLayers3)
{
	Layers().setUpperLayers(upperLayers);
	Layers().setLowerLayers(lowerLayers);
	Layers().upperLayerType() = upper;
	Layers().lowerLayerType() = lower;

	Layers2().setUpperLayers(upperLayers2);
	Layers2().setLowerLayers(lowerLayers2);
	Layers2().upperLayerType() = OcularLayerType::NFL;
	Layers2().lowerLayerType() = OcularLayerType::IPL;

	Layers3().setUpperLayers(upperLayers3);
	Layers3().setLowerLayers(lowerLayers3);
	Layers3().upperLayerType() = OcularLayerType::IOS;
	Layers3().lowerLayerType() = OcularLayerType::BRM;
	return;
}


bool OctAngio::Angiogram2::prepareFlowData(bool recalc, bool align, bool motion)
{
	int lines = Layout().numberOfLines();
	int repeats = Layout().numberOfRepeats();
	int points = Layout().numberOfPoints();

	CppUtil::ClockTimer::start();
	LogD() << "Preparing angio data, lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", recalc: " << recalc << ", align: " << align << ", motion: " << motion;

	if (recalc) {
		if (!produceFlowSignals(align, motion)) {
			return false;
		}
	}

	if (!buildProjectionMasks()) {
		return false;
	}

	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angiogram data prepared, elapsed: " << msec;
	return true;
}


bool OctAngio::Angiogram2::estimateMotionVectors(void)
{
	LogD() << "Estimating motion vectors...";
	performMotionRegistration();
	return true;
}


void OctAngio::Angiogram2::performBscansAlignment(void)
{
	int lines = Layout().numberOfLines();
	int repeats = Layout().numberOfRepeats();

	if (useAlignAxial() || useAlignLateral()) {
		Data().alignRepeatedAmplitudes(lines, repeats, useAlignAxial(), useAlignLateral(), Layers().upperLayers(), Layers().lowerLayers());
	}
	return;
}

void OctAngio::Angiogram2::performMotionRegistration(void)
{
	int lines = Layout().numberOfLines();
	int repeats = Layout().numberOfRepeats();

	std::vector<float> xList, yList, eccList;
	if (Data().computeSequentialDisplacements(lines, repeats, xList, yList, eccList)) {
		Motion().setBscanTranslations(xList, yList, eccList);
	}
	return;
}

bool OctAngio::Angiogram2::produceFlowSignals(bool align, bool motion)
{
	if (!isAmplitduesValid()) {
		return false;
	}

	if (align) {
		performBscansAlignment();
	}

	if (motion) {
		performMotionRegistration();
	}

	if (!Decorr().estimateThreshold(Layout(), Data(), Layers())) {
		return false;
	}

	int overlaps = numberOfOverlaps();
	int repeats = Layout().numberOfRepeats();

	int avgOffset1 = avgOffsetInnFlows();
	int avgOffset2 = avgOffsetOutFlows();
	float threshRatio1 = thresholdRatioInnFlows();
	float threshRatio2 = thresholdRatioOutFlows();

	overlaps = (overlaps == 0 ? repeats : overlaps);
	if (!Decorr().calculateSignals(Layout(), Data(), Layers(), overlaps, avgOffset1, avgOffset2, threshRatio1, threshRatio2)) {
		return false;
	}
	return true;
}

bool OctAngio::Angiogram2::buildProjectionMasks(void)
{
	// Projection mask for vascular structure. (NFL~IPL)
	auto uppersV = Layers().getUpperLayersOfVasculature();
	auto lowersV = Layers().getLowerLayersOfVasculature();
	auto outerFlow = false;
	auto vascAvgOffs = avgOffsetVascularMask();

	LogD() << "Building projection masks for vascular structure";
		
	if (!Decorr().updateProjectionMask(Layout(), uppersV, lowersV, outerFlow, vascAvgOffs)) {
		return false;
	}

	auto deco_angio = vector<float>(Decorr().decorrAngiogram());
	auto diff_angio = vector<float>(Decorr().differAngiogram());

	Decorr2().decorrAngiogram() = vector<float>(deco_angio);
	Decorr2().differAngiogram() = vector<float>(diff_angio);
	Post().createProjectionMask(Layout(), Decorr2().decorrAngiogram(), Decorr2().decorrProjectionMask(), false);
	Post().createProjectionMask(Layout(), Decorr2().differAngiogram(), Decorr2().differProjectionMask(), false);

	// Projection mask for artifact removal. (ILM~OPL)
	auto uppersD = Layers().getUpperLayersOfVasculature2();
	auto lowersD = Layers().getLowerLayersOfVasculature2();
	outerFlow = false;
	auto maskAvgOffs = avgOffsetProjectionMask();

	if (!Decorr().updateProjectionMask(Layout(), uppersD, lowersD, outerFlow, maskAvgOffs)) {
		return false;
	}

	Decorr3().decorrAngiogram() = vector<float>(deco_angio);
	Decorr3().differAngiogram() = vector<float>(diff_angio);
	Post().createProjectionMask(Layout(), Decorr3().decorrAngiogram(), Decorr3().decorrProjectionMask(), false);
	Post().createProjectionMask(Layout(), Decorr3().differAngiogram(), Decorr3().differProjectionMask(), false);

	auto diff_mean = std::accumulate(diff_angio.cbegin(), diff_angio.cend(), 0.0f) / diff_angio.size();
	getImpl().isModelEye = (diff_mean <= ANGIO_MODEL_EYE_MEAN_MAX ? true : false);
	LogD() << "Angio diff mean: " << diff_mean << ", model_eye: " << getImpl().isModelEye;

	/*
	{
		int w = Layout().getWidth();
		int h = Layout().getHeight();
		auto v = Decorr2().differAngiogram();
		auto max_val = *max_element(v.begin(), v.end());
		auto min_val = *min_element(v.begin(), v.end());
		max_val = min(max_val, min_val + 4500);
		auto range = (max_val - min_val);

		CvImage image;
		auto size = w * h;
		auto grays = vector<unsigned char>(size);
		for (int r = 0; r < h; r++) {
			for (int c = 0; c < w; c++) {
				auto index = r * w + c;
				auto dcval = (v[index] - min_val) / range;
				grays[index] = (unsigned char)min(max((int)(dcval * 255.0f), 0), 255);
			}
		}
		image.fromBitsData(&grays[0], w, h);
		image.saveFile("vess01.png");
	}
	*/

	auto& vessDiff = Decorr2().differProjectionMask();
	auto& vessDeco = Decorr2().decorrProjectionMask();
	auto& maskDiff = Decorr3().differProjectionMask();
	auto& maskDeco = Decorr3().decorrProjectionMask();
	bool vertical = Layout().isVerticalScan();

	bool blank = false;
	blank |= all_of(vessDiff.begin(), vessDiff.end(), [](float i) { return i <= 0; });
	blank |= all_of(vessDeco.begin(), vessDeco.end(), [](float i) { return i <= 0; });
	blank |= all_of(maskDeco.begin(), maskDeco.end(), [](float i) { return i <= 0; });
	blank |= all_of(maskDiff.begin(), maskDiff.end(), [](float i) { return i <= 0; });
		
	if (!blank) {
		Post().removeProjectionStripeNoise2(Layout(), vessDiff, vertical);
		Post().removeProjectionStripeNoise2(Layout(), vessDeco, vertical);
		Post().removeProjectionStripeNoise2(Layout(), maskDeco, vertical);
		Post().removeProjectionStripeNoise2(Layout(), maskDiff, vertical);

		Motion().rotateVerticalScan(Layout(), true, vessDiff);
		Motion().rotateVerticalScan(Layout(), true, vessDeco);

		int w = Layout().getWidth();
		int h = Layout().getHeight();
		
		/*
		{
			int w = Layout().getWidth();
			int h = Layout().getHeight();
			auto v = Decorr2().differProjectionMask();
			auto max_val = *max_element(v.begin(), v.end());
			auto min_val = *min_element(v.begin(), v.end());
			max_val = min(max_val, min_val + 4500);
			auto range = (max_val - min_val);

			CvImage image;
			auto size = w * h;
			auto grays = vector<unsigned char>(size);
			for (int r = 0; r < h; r++) {
				for (int c = 0; c < w; c++) {
					auto index = r * w + c;
					auto dcval = (v[index] - min_val) / range;
					grays[index] = (unsigned char)min(max((int)(dcval * 255.0f), 0), 255);
				}
			}
			image.fromBitsData(&grays[0], w, h);
			image.saveFile("vess02.png");
		}
		*/

		Post().performVesselProcessing(w, h, vessDiff);
		Post().performVesselProcessing(w, h, vessDeco);
		Post().performVesselProcessing(w, h, maskDiff);
		Post().performVesselProcessing(w, h, maskDeco);
	}
	else {
		LogD() << "Blank mask for vascular structure";
	}
	return true;
}


bool OctAngio::Angiogram2::buildProjectionImages(bool calcStats)
{
	bool usePAR = useProjectionArtifactRemoval();
	auto& maskDecorr = Decorr3().decorrProjectionMask();
	auto& maskDiffer = Decorr3().differProjectionMask();

	int avgOffsetInn = avgOffsetInnProjection();
	int avgOffsetOut = avgOffsetOutProjection();
	float maskWeight = weightProjectionMask();
	bool modelEye = isModelEyeImage();

	if (!Decorr().updateProjectionProfiles(Layout(), Layers(), calcStats, usePAR, modelEye, maskDecorr, maskDiffer, avgOffsetInn, avgOffsetOut, maskWeight)) {
		return false;
	}

	if (calcStats) {
		if (!Decorr().calculateBscanImageStats(Layout())) {
			return false;
		}

		if (!Decorr().calculateProjectionStats(modelEye)) {
			return false;
		}
	}
	return true;
}

bool OctAngio::Angiogram2::processProjectionImages(void)
{
	int width = Data().dataWidth();
	int height = Data().dataHeight();

	int angioWidth = Layout().getWidth();
	int angioHeight = Layout().getHeight();
	bool outerflows = Layers().isOuterRetinaFlows();

	float noiseRate = getImpl().noiseReductionRate;
	Post().applyNoiseReduction(Layout(), Decorr().differAngiogram(), noiseRate);
	Post().applyNoiseReduction(Layout(), Decorr().decorrAngiogram(), noiseRate);

	if (usePostProcessing()) {
		if (!outerflows) {
			if (useBiasFieldCorrection()) {
				Motion().correctBiasField(Layout(), Decorr().differAngiogram(), biasFieldSigma(), isFoveaAvascularZone());
				Motion().correctBiasField(Layout(), Decorr().decorrAngiogram(), biasFieldSigma(), isFoveaAvascularZone());
			}
		}
	}

	if (usePostProcessing()) { // useMotionCorrection()) {
		auto& dcs = Decorr().decorrAngiogram();
		auto& dfs = Decorr().differAngiogram();
		auto dvs = Decorr2().decorrAngiogram();

		bool blank = false;
		blank |= all_of(dcs.begin(), dcs.end(), [](float i) { return i <= 0; });
		blank |= all_of(dfs.begin(), dfs.end(), [](float i) { return i <= 0; });
		blank |= all_of(dvs.begin(), dvs.end(), [](float i) { return i <= 0; });

		if (!blank) {
			Motion().rotateVerticalScan(Layout(), true, dcs);
			Motion().rotateVerticalScan(Layout(), true, dfs);

			bool postproc = true;// (Layers().isOuterRetinaFlows() ? false : true);
			bool regist = (useMotionCorrection() && postproc);

			Motion().correctMotionArtifacts2(Layout(), dcs, dfs, dvs, regist);
			Motion().rotateVerticalScan(Layout(), false, dcs);
			Motion().rotateVerticalScan(Layout(), false, dfs);

			if (postproc) {
				std::vector<std::thread> workers;
				for (int t = 0; t < 2; t++) {
					workers.push_back(std::thread([t, angioWidth, angioHeight, &dcs, &dfs, outerflows, this]() {
						if (t == 0) {
							Post().performPostProcessing(angioWidth, angioHeight, dcs);
						}
						else {
							Post().performPostProcessing(angioWidth, angioHeight, dfs, outerflows);
						}
					}));
				}
				std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
					t.join();
				});
			}
		}
	}
	return true;
}

bool OctAngio::Angiogram2::processProjectionImages2(void)
{
	int width = Data().dataWidth();
	int height = Data().dataHeight();

	int angioWidth = Layout().getWidth();
	int angioHeight = Layout().getHeight();
	bool outerflows = Layers().isOuterRetinaFlows();
	auto vertical = Layout().isVerticalScan();
	bool modelEye = isModelEyeImage();

	if (usePostProcessing() || true) { 
		auto& dcs = Decorr().decorrAngiogram();
		auto& dfs = Decorr().differAngiogram();

		Post().removeProjectionStripeNoise2(Layout(), dcs, vertical);
		Post().removeProjectionStripeNoise2(Layout(), dfs, vertical);
		 
		bool postproc = usePostProcessing();
		bool motion = useMotionCorrection();
		bool bias = useBiasFieldCorrection();

		if (postproc) {
			bool blank = false;
			blank |= all_of(dcs.begin(), dcs.end(), [](float i) { return i <= 0; });
			blank |= all_of(dfs.begin(), dfs.end(), [](float i) { return i <= 0; });

			if (!blank) {
				if (motion) {
					auto& vessDiff = Decorr2().differProjectionMask();
					Motion().computeMotionShifts(Layout(), vessDiff);
					Motion().rotateVerticalScan(Layout(), false, vessDiff);

					Motion().rotateVerticalScan(Layout(), true, dcs);
					Motion().rotateVerticalScan(Layout(), true, dfs);
					Motion().applyMotionCorrection(Layout(), dcs);
					Motion().applyMotionCorrection(Layout(), dfs);
					Motion().rotateVerticalScan(Layout(), false, dcs);
					Motion().rotateVerticalScan(Layout(), false, dfs);
				}
				if (bias) {
					Post().applyBiasFieldCorrection(Layout(), Decorr().differAngiogram(), isFoveaAvascularZone());
					Post().applyBiasFieldCorrection(Layout(), Decorr().decorrAngiogram(), isFoveaAvascularZone());
				}

				std::vector<std::thread> workers;
				for (int t = 0; t < 2; t++) {
					workers.push_back(std::thread([t, angioWidth, angioHeight, &dcs, &dfs, outerflows, this]() {
						if (t == 0) {
							Post().performPostProcessing(angioWidth, angioHeight, dcs, outerflows);
						}
						else {
							Post().performPostProcessing(angioWidth, angioHeight, dfs, outerflows);
						}
						}));
				}
				std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
					t.join();
					});
			}
		}
	}
	return true;
}


bool OctAngio::Angiogram2::normalizeProjectionImages(void)
{
	int width = Data().dataWidth();
	int height = Data().dataHeight();
	int angioWidth = Layout().getWidth();
	int angioHeight = Layout().getHeight();

	float decorrMin = decorrLowerThreshold();
	float decorrMax = decorrUpperThreshold();
	float differMin = differLowerThreshold();
	float differMax = differUpperThreshold();
	float decorrSize = 0.0f;
	float differSize = 0.0f;

	bool outerflows = Layers().isOuterRetinaFlows();
	bool modelEye = isModelEyeImage();

	if (decorrMin == 0.0f && decorrMax == 0.0f) {
		decorrMin = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DECORR_NORM_RANGE_MIN2 : ANGIO_PROJ_DECORR_NORM_RANGE_MIN;
		decorrMax = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DECORR_NORM_RANGE_MAX2 : ANGIO_PROJ_DECORR_NORM_RANGE_MAX;
		decorrSize = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DECORR_NORM_SIZE_MIN2 : ANGIO_PROJ_DECORR_NORM_SIZE_MIN;
	}
	else {
		modelEye = false;
	}
	if (differMin == 0.0f && differMax == 0.0f) {
		differMin = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DIFFER_NORM_RANGE_MIN2 : ANGIO_PROJ_DIFFER_NORM_RANGE_MIN;
		differMax = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DIFFER_NORM_RANGE_MAX2 : ANGIO_PROJ_DIFFER_NORM_RANGE_MAX;
		differSize = Layers().isOuterRetinaFlows() ? ANGIO_PROJ_DIFFER_NORM_SIZE_MIN2 : ANGIO_PROJ_DIFFER_NORM_SIZE_MIN;
	}
	else {
		modelEye = false;
	}

	Decorr().normalizeDecorrAngiogram(Layout(), Layers(), decorrMin, decorrMax, decorrSize, modelEye);
	Decorr().normalizeDifferAngiogram(Layout(), Layers(), differMin, differMax, differSize, modelEye);

	bool decorrout = useDecorrOutput();
	if (GlobalSettings::isUserModeSettings()) {
		if (Layers().isOuterRetinaFlows()) {
			decorrout = true;
		}
		else {
			decorrout = false;
		}
	}

	buildImageBitmap(decorrout, outerflows, modelEye);
	return true;
}

bool OctAngio::Angiogram2::generateProjection(bool calcStats)
{
	int lines = Layout().numberOfLines();
	int points = Layout().numberOfPoints();
	int repeats = Layout().numberOfRepeats();
	auto& data = Data().getAmplitudes();
	bool reflectCorrect = useReflectionCorrection();

	if (GlobalSettings::isUserModeSettings()) {
		biasFieldSigma() = GlobalSettings::angioBiasFieldSigma();
		Post().garborFilerOrients() = GlobalSettings::angioFilterOrients();
		Post().garborFilterDivider() = GlobalSettings::angioFilterDivider();
		Post().garborFilterSigma() = GlobalSettings::angioFilterSigma();
		Post().garborFilterWeight() = GlobalSettings::angioFilterWeight();
		calcStats = false;
	}

	CppUtil::ClockTimer::start();
	LogD() << "Generating angio projection, lines: " << lines << ", points: " << points << ", repeats: " << repeats << ", calc_stats: " << calcStats;

	if (!buildProjectionImages(calcStats)) {
		return false;
	}
	if (!processProjectionImages2()) {
		return false;
	}
	if (!normalizeProjectionImages()) {
		return false;
	}
	
	double msec = CppUtil::ClockTimer::elapsedMsec();
	LogD() << "Angiogram projection updated, elapsed: " << msec;
	return true;
}


bool OctAngio::Angiogram2::buildImageBitmap(bool decorr, bool outerFlows, bool modelEye)
{
	int width = imageWidth();
	int height = imageHeight();
	int size = width * height;
	vector<float> output;

	// Select source of angio. 
	if (decorr) {
		output = Decorr().decorrAngiogram();
	}
	else {
		output = Decorr().differAngiogram();
	}

	// Convert into grayscale image. 
	auto grays = vector<unsigned char>(size);
	int index;
	float dcval = 0.0f;
	for (int r = 0; r < height; r++) {
		for (int c = 0; c < width; c++) {
			index = r * width + c;
			dcval = output[index];
			grays[index] = (unsigned char)min(max((int)(dcval * 255.0f), 0), 255);
		}
	}

	// Apply equalization as default only for superficial angio. 
	if (!modelEye) {
		if (!outerFlows) {
			CvImage image;
			image.fromBitsData(&grays[0], width, height);
			image.equalizeHistogram(1.0f);
			grays = image.copyDataInBytes();
		}
	}

	getImpl().imageBits = grays;
	return true;
}


const unsigned char * OctAngio::Angiogram2::imageBits(void)
{
	if (getImpl().imageBits.size() <= 0) {
		return nullptr;
	}
	return &(getImpl().imageBits[0]);
}


int OctAngio::Angiogram2::imageWidth(void) 
{
	return (Layout().isVerticalScan() ? Layout().numberOfLines() : Layout().numberOfPoints());
}


int OctAngio::Angiogram2::imageHeight(void) 
{
	return (Layout().isVerticalScan() ? Layout().numberOfPoints() : Layout().numberOfLines());
}


CppUtil::CvImage OctAngio::Angiogram2::createAnigoImage(void)
{
	int width = imageWidth();
	int height = imageHeight();
	int size = width * height;

	auto bits = imageBits();
	if (!bits) {
		return CppUtil::CvImage();
	}

	CvImage image;
	image.fromBitsData(&bits[0], width, height);
	return image;
}


CppUtil::CvImage OctAngio::Angiogram2::createOffsetImage(void)
{
	int width = imageWidth();
	int height = imageHeight();
	int size = width * height;

	auto& offsets = Decorr().decorrAxialOffsets();
	if (offsets.size() != size) {
		return CppUtil::CvImage();
	}

	CvImage image;
	image.fromFloat32((const unsigned char*)&offsets[0], width, height);
	return image;
}


CppUtil::CvImage OctAngio::Angiogram2::createDecorrImage(int lineIdx)
{
	return Decorr().getDecorrelationImage(lineIdx);
}


CppUtil::CvImage OctAngio::Angiogram2::createScanImage(int lineIdx)
{
	int width = scanImageWidth();
	int height = scanImageHeight();
	int size = width * height;

	auto bits = getScanImageBits(lineIdx, 0);
	if (!bits) {
		return CppUtil::CvImage();
	}

	CvImage image;
	image.fromBitsData(&bits[0], width, height);
	return image;
}


std::vector<float> OctAngio::Angiogram2::getDecorrelationsOnHorzLines(bool axialMax)
{
	int width = imageWidth();
	int height = imageHeight();

	auto& data = (axialMax ? Decorr().decorrProjectionMax() : Decorr().decorrAngiogram());
	if (data.empty() || data.size() != width * height) {
		return std::vector<float>();
	}

	auto decors = vector<float>(height);
	for (int r = 0; r < height; r++) {
		decors[r] = std::accumulate(begin(data) + r*width, data.begin() + (r + 1)*width, 0.0f) / width;
	}
	return decors;
}


std::vector<float> OctAngio::Angiogram2::getDecorrelationsOnVertLines(bool axialMax)
{
	int width = imageWidth();
	int height = imageHeight();

	auto image = Decorr().decorrAngiogramImage();
	image.rotate90();

	auto& data = image.copyDataInFloats();
	if (data.empty() || data.size() != width * height) {
		return std::vector<float>();
	}

	auto decors = vector<float>(width);
	for (int r = 0; r < width; r++) {
		decors[r] = std::accumulate(begin(data) + r*height, data.begin() + (r + 1)*height, 0.0f) / height;
	}
	return decors;
}


std::vector<int> OctAngio::Angiogram2::getUpperLayerPoints(int imageIdx)
{
	if (!isAngioImage()) {
		return std::vector<int>();
	}

	auto layers = Layers().getUpperLayersOfSlab();
	if (layers.size() > imageIdx) {
		return layers[imageIdx];
	}
	return std::vector<int>();
}


std::vector<int> OctAngio::Angiogram2::getLowerLayerPoints(int imageIdx)
{
	if (!isAngioImage()) {
		return std::vector<int>();
	}

	auto layers = Layers().getLowerLayersOfSlab();
	if (layers.size() > imageIdx) {
		return layers[imageIdx];
	}
	return std::vector<int>();
}


int OctAngio::Angiogram2::numberOfDecorrImages(void)
{
	return (int)Data().getAmplitudes().size();
}


int OctAngio::Angiogram2::numberOfOverlapImages(void)
{
	if (numberOfDecorrImages() > 0) {
		return (int)Data().getAmplitudes()[0].size();
	}
	return 0;
}


float * OctAngio::Angiogram2::getDecorrImageBits(int lineIdx)
{
	return Decorr().getDecorrelationData(lineIdx);
}


unsigned char* OctAngio::Angiogram2::getScanImageBits(int lineIdx, int repeatIdx)
{
	return Data().getGrayscaledDataBits(lineIdx, repeatIdx);
}

int OctAngio::Angiogram2::scanImageWidth(void)
{
	return Data().dataWidth();
}

int OctAngio::Angiogram2::scanImageHeight(void)
{
	return Data().dataHeight();
}

float OctAngio::Angiogram2::scanRangeX(void)
{
	return Layout().scanRangeX();
}

float OctAngio::Angiogram2::scanRangeY(void)
{
	return Layout().scanRangeY();
}


bool & OctAngio::Angiogram2::useAlignAxial(void)
{
	return getImpl().useAlignAxial;
}

bool & OctAngio::Angiogram2::useAlignLateral(void)
{
	return getImpl().useAlignLateral;
}

bool & OctAngio::Angiogram2::useDecorrCircular(void)
{
	return getImpl().useDecorrCircular;
}

bool & OctAngio::Angiogram2::useLayersSelected(void)
{
	return getImpl().useLayersSelected;
}

bool & OctAngio::Angiogram2::useVascularLayers(void)
{
	return getImpl().useVascularLayers;
}

bool & OctAngio::Angiogram2::useMotionCorrection(void)
{
	return getImpl().useMotionCorrect;
}

bool & OctAngio::Angiogram2::useBiasFieldCorrection(void)
{
	return getImpl().useBiasFieldCorrect;
}

bool & OctAngio::Angiogram2::usePostProcessing(void)
{
	return getImpl().usePostProcessing;
}

bool & OctAngio::Angiogram2::useNormProjection(void)
{
	return getImpl().useNormProjection;
}

bool & OctAngio::Angiogram2::useDecorrOutput(void)
{
	return getImpl().useDecorrOutput;
}

bool & OctAngio::Angiogram2::useDifferOutput(void)
{
	return getImpl().useDifferOutput;
}

bool & OctAngio::Angiogram2::useProjectionArtifactRemoval(void)
{
	return getImpl().useProjectionRemoval;
}

bool & OctAngio::Angiogram2::useReflectionCorrection(void)
{
	return getImpl().useReflectCorrect;
}

bool & OctAngio::Angiogram2::useProjectionStatistics(void)
{
	return getImpl().useProjectionStats;
}

float & OctAngio::Angiogram2::intensityUpperThreshold(void)
{
	return getImpl().upperThreshold;
}

float & OctAngio::Angiogram2::intensityLowerThreshold(void)
{
	return getImpl().lowerThreshold;
}

float & OctAngio::Angiogram2::decorrLowerThreshold(void)
{
	return getImpl().decorrLowerThreshold;
}

float & OctAngio::Angiogram2::decorrUpperThreshold(void)
{
	return getImpl().decorrUpperThreshold;
}

float & OctAngio::Angiogram2::differLowerThreshold(void)
{
	return getImpl().differLowerThreshold;
}

float & OctAngio::Angiogram2::differUpperThreshold(void)
{
	return getImpl().differUpperThreshold;
}

float & OctAngio::Angiogram2::decorrBaseThreshold(void)
{
	return getImpl().decorrBaseThreshold;
}

float & OctAngio::Angiogram2::biasFieldSigma(void)
{
	return getImpl().biasFieldSigma;
}

float & OctAngio::Angiogram2::noiseReductionRate(void)
{
	return getImpl().noiseReductionRate;
}

float & OctAngio::Angiogram2::normalizeDropOff(void)
{
	return getImpl().normalizeDropOff;
}

int & OctAngio::Angiogram2::numberOfOverlaps(void)
{
	return getImpl().numberOfOverlaps;
}

bool & OctAngio::Angiogram2::usePixelAverage(void)
{
	return getImpl().usePixelAverage;
}

int & OctAngio::Angiogram2::pixelAverageOffset(void)
{
	return getImpl().pixelAverageOffset;
}

int& OctAngio::Angiogram2::avgOffsetInnFlows(void)
{
	return getImpl().avgOffsetInnFlows;
}

int& OctAngio::Angiogram2::avgOffsetOutFlows(void)
{
	return getImpl().avgOffsetOutFlows;
}

float& OctAngio::Angiogram2::thresholdRatioInnFlows(void)
{
	return getImpl().thresholdRatioInnFlows;
}

float& OctAngio::Angiogram2::thresholdRatioOutFlows(void)
{
	return getImpl().thresholdRatioOutFlows;
}

int& OctAngio::Angiogram2::avgOffsetVascularMask(void)
{
	return getImpl().avgOffsetVascMask;
}

int& OctAngio::Angiogram2::avgOffsetProjectionMask(void)
{
	return getImpl().avgOffsetProjMask;
}

float& OctAngio::Angiogram2::weightProjectionMask(void)
{
	return getImpl().weightProjMask;
}

int& OctAngio::Angiogram2::avgOffsetInnProjection(void)
{
	return getImpl().avgOffsetInnProj;
}

int& OctAngio::Angiogram2::avgOffsetOutProjection(void)
{
	return getImpl().avgOffsetOutProj;
}

OctAngio::AngioLayout & OctAngio::Angiogram2::Layout(void)
{
	return getImpl().layout;
}


OctAngio::AngioData & OctAngio::Angiogram2::Data(void)
{
	return getImpl().data;
}

OctAngio::AngioLayers & OctAngio::Angiogram2::Layers(void)
{
	return getImpl().layers;
}

OctAngio::AngioLayers & OctAngio::Angiogram2::Layers2(void)
{
	return getImpl().layers2;
}

OctAngio::AngioLayers & OctAngio::Angiogram2::Layers3(void)
{
	return getImpl().layers3;
}

OctAngio::AngioDecorr & OctAngio::Angiogram2::Decorr(void)
{
	return getImpl().decorr;
}

OctAngio::AngioDecorr & OctAngio::Angiogram2::Decorr2(void)
{
	return getImpl().decorr2;
}

OctAngio::AngioDecorr & OctAngio::Angiogram2::Decorr3(void)
{
	return getImpl().decorr3;
}

OctAngio::AngioMotion & OctAngio::Angiogram2::Motion(void)
{
	return getImpl().motion;
}


OctAngio::AngioPost & OctAngio::Angiogram2::Post(void)
{
	return getImpl().post;
}

Angiogram2::Angiogram2Impl & OctAngio::Angiogram2::getImpl(void) const
{
	return *d_ptr;
}
