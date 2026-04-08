#include "stdafx.h"
#include "CorneaBsegm.h"
#include "OcularLayers.h"
#include "CorneaLayers.h"
#include "OcularBsegm.h"

#include "RetSegm2.h"
#include "CppUtil2.h"


using namespace SegmScan;
using namespace RetSegm;
using namespace CppUtil;
using namespace std; 
using namespace cv;


struct CorneaBsegm::CorneaBsegmImpl
{
	vector<float> curvRads;

	vector<float> axialAnteriorRadius;
	vector<float> axialAnteriorRadiusSimK;
	vector<float> axialPosteriorRadius;
	vector<float> tangentialAnteriorRadius;
	vector<float> tangentialPosteriorRadius;
	vector<float> refractivePowerKerato;
	vector<float> refractivePowerAnterior;
	vector<float> refractivePowerPosterior;
	vector<float> refractivePowerTotal;
	vector<float> netMap;
	vector<float> axialTrueNet;
	vector<float> equivalentKeratometer;
	vector<float> elevationAnterior;
	vector<float> elevationPosterior;
	vector<float> height;
	vector<float> pachymetry;
	vector<float> epithelium;

	vector<double> incidenceAngle;

	int corneaTopX;
	int corneaTopY;
	int wideHingeY;
	float rangeX;
	float resolutionX;
	float resolutionY;
	bool isWideCornea = false;
	bool isModelEye = false;
	bool isIncorrectIdx = false;
	CorneaBsegmImpl()
	{
	}
};


CorneaBsegm::CorneaBsegm() :
	d_ptr(make_unique<CorneaBsegmImpl>())
{
	auto layers = make_unique<CorneaLayers>();
	setOcularLayers(std::move(layers));
}


SegmScan::CorneaBsegm::~CorneaBsegm() = default;
SegmScan::CorneaBsegm::CorneaBsegm(CorneaBsegm && rhs) = default;
CorneaBsegm & SegmScan::CorneaBsegm::operator=(CorneaBsegm && rhs) = default;


CorneaBsegm::CorneaBsegmImpl & SegmScan::CorneaBsegm::getImpl(void) const
{
	return *d_ptr;
}


bool SegmScan::CorneaBsegm::performAnalysis(bool meye)
{
	if (source()->isEmpty()) {
		return false;
	}

	float rangeX = getPatternDescript().getScanLength();
	bool isTopo = getPatternDescript().getPatternName() == PatternName::Topography;
	bool inAir = isTopo != true;

	// rangeX = 13.0f;
	getImpl().rangeX = rangeX;
	getImpl().resolutionX = getResolutionX(false);
	getImpl().resolutionY = getResolutionY(inAir);
	getImpl().isWideCornea = rangeX > 9.0f;
	getImpl().isModelEye = meye;

	prepareSample(source(), sample());
	if (!PrepCornea::checkIfValidScanImageOfCornea(sample(), getImageIndex())) {
		goto failed;
	}

	if (!getImpl().isModelEye) {
		getImpl().isModelEye = CoarseCornea2::checkIfModelEyeImage(sample());
	}

	createGradients(sample(), gradients(), ascends(), descends(), getImpl().isModelEye);

	if (!getImpl().isModelEye && getImpl().isWideCornea) {
		int centerX, centerY;
		if (!CoarseCornea::makeupCorneaBorderLines(sample(), ascends(), descends(), layerInn(), layerOut(),
			rangeX, getImpl().corneaTopX, getImpl().corneaTopY, getImpl().wideHingeY, meye)) {
			goto failed;
		}
		// return true;

		elaborateParams();
		if (!elaborateEPI(source(), layerInn(), layerOut(), layerEPI())) {
			goto failed;
		}
		if (!elaborateEND(source(), layerEPI(), layerOut(), layerEND())) {
			goto failed;
		}
		if (!elaborateBOW(source(), layerEPI(), layerEND(), layerBOW())) {
			if (!meye) {
				goto failed;
			}
		}

		centerX = getImpl().corneaTopX;
		centerY = getImpl().corneaTopY;

		if (getPatternDescript().getPatternName() != PatternName::Topography) {
			CoarseCornea2::correctSurfaceReflection(source(), layerEPI(), layerBOW(), centerX, centerY, rangeX);
		}
	}
	else {
		int centerX, centerY;
		if (!CoarseCornea2::makeupCorneaBorderLines(sample(), ascends(), descends(), layerInn(), layerOut(),
			centerX, centerY, rangeX, getImpl().isModelEye)) {
			goto failed;
		}
		// return true;
		getImpl().corneaTopX = centerX;
		getImpl().corneaTopY = centerY;
		elaborateParams();

		centerX = getImpl().corneaTopX;
		centerY = getImpl().corneaTopY;
		CoarseCornea2::elaborateEPI(source(), layerInn(), layerEPI(), centerX, centerY, rangeX);
		CoarseCornea2::elaborateEND(source(), layerEPI(), layerOut(), layerEND(), centerX, centerY, rangeX);
		CoarseCornea2::elaborateBOW(source(), layerEPI(), layerEND(), layerBOW(), centerX, centerY, rangeX);

		if (getPatternDescript().getPatternName() != PatternName::Topography) {
			CoarseCornea2::correctSurfaceReflection(source(), layerEPI(), layerBOW(), centerX, centerY, rangeX);
		}
		else if (getPatternDescript().getPatternName() == PatternName::Topography && !getImpl().isModelEye) {
			
			if (layerEpiForEpi != nullptr) {
				delete layerEpiForEpi;
				layerEpiForEpi = nullptr;
				delete layerBowForEpi;
				layerBowForEpi = nullptr;
			}

			layerEpiForEpi = new OcularLayer(*layerEPI());
			layerBowForEpi = new OcularLayer(*layerBOW());
			CoarseCornea2::correctSurfaceReflection(source(), layerEpiForEpi, layerBowForEpi, centerX, centerY, rangeX);

			if (!CoarseCornea2::correctSurfaceReflectionForTopo(source(), layerEPI(), layerBOW(), centerX, centerY, rangeX)) {
				getImpl().isIncorrectIdx = true;
			}
			else {
				getImpl().isIncorrectIdx = false;
			}
		}
		makeFittingCurvature(rangeX);

	}

	if (getPatternDescript().getPatternName() == PatternName::Topography) {
		calculateCorneaCurvature();
	}

	if (getPatternDescript().getPatternName() == PatternName::Topography && !getImpl().isModelEye) {

		if (layerEpiForEpi != nullptr) {
			delete layerEpiForEpi;
			layerEpiForEpi = nullptr;
			delete layerBowForEpi;
			layerBowForEpi = nullptr;
		}
	}

	auto layers = getCorneaLayers();
	layers->setRegionSize(source()->getWidth(), source()->getHeight());
	setResult(true);
	return true;

failed:
	getCorneaLayers()->initialize(source()->getWidth(), source()->getHeight());
	return false;
}


bool SegmScan::CorneaBsegm::performDewarping(void)
{
	if (source()->isEmpty()) {
		return false;
	}

	float rangeX = getPatternDescript().getScanLength();
	// rangeX = 6.0f;
	getImpl().rangeX = rangeX;
	getImpl().resolutionX = getResolutionX(false);
	getImpl().resolutionY = getResolutionY();
	getImpl().isModelEye = false;

	prepareSample(source(), sample());
	if (!PrepCornea::checkIfValidScanImageOfCornea(sample(), getImageIndex())) {
		goto failed;
	}
	getImpl().isModelEye = CoarseCornea2::checkIfModelEyeImage(sample());

	createGradients(sample(), gradients(), ascends(), descends(), getImpl().isModelEye);
	if (!CoarseCornea::makeupCorneaBorderLines(sample(), ascends(), descends(), layerInn(), layerOut(),
		rangeX, getImpl().corneaTopX, getImpl().corneaTopY, getImpl().wideHingeY)) {
		goto failed;
	}
	
	elaborateParams();
	if (!elaborateEPI(source(), layerInn(), layerOut(), layerEPI())) {
		goto failed;
	}

	applyDewarpingImage();
	setResult(true);
	return true;

failed:
	setResult(false);
	return false;
}


bool SegmScan::CorneaBsegm::applyDewarpingImage(void)
{
	if (source()->isEmpty()) {
		return false;
	}

	int width = source()->getWidth();
	int height = source()->getHeight();
	auto layer = layerInn()->getYs();

	if (layer.size() != width) {
		return false;
	}

	float xRes = getResolutionX();
	float yRes = (float)GlobalSettings::getCorneaScanAxialResolution();

	source()->applyDewarping(layer, xRes, yRes, CORNEA_REFLECTIVE_INDEX);
	return true;
}


void SegmScan::CorneaBsegm::initializeLayers(int width, int height)
{
	if (width == 0) {
		width = getSourceWidth();
	}
	if (height == 0) {
		height = getSourceHeight();
	}

	getCorneaLayers()->initialize(width, height);
	return;
}


void SegmScan::CorneaBsegm::makeFittingCurvature(float rangeX)
{
	auto path = layerEPI()->getYs();
	if (PathCornea::makeFittingCurveOfEPI(path, path)) {
		bool isAxial = false;
		float resX = getImpl().resolutionX;
		float resY = getImpl().resolutionY;
		vector<float> output;

		Coarse::makeCurveRadiusOfCornea(resX, resY, rangeX, path, output, isAxial);
		d_ptr->curvRads = output;
	}
	else {
		d_ptr->curvRads = std::vector<float>(path.size(), 0.0f);
	}
}


bool SegmScan::CorneaBsegm::prepareSample(const OcularImage * imgSrc, OcularImage * imgSample)
{

	PrepCornea::createSampleImage(source(), sample());
	PrepCornea::collectImageStats(sample());
	PrepCornea::collectColumnStats(sample());
	Preprocess::collectImageStats(source());
	Preprocess::collectColumnStats(source());

	auto layers = getCorneaLayers();
	layers->initialize(sample()->getWidth(), sample()->getHeight());
	return true;
}


bool SegmScan::CorneaBsegm::createGradients(const OcularImage * imgSrc, OcularImage * imgGradients, OcularImage * imgAscends, OcularImage * imgDescends, bool isMeye)
{
	if (getImpl().isWideCornea) {
		CoarseCornea::createGradientMapOfCornea(imgSrc, imgAscends, imgDescends, imgGradients);
	}
	else {
		CoarseCornea2::createGradientMaps(imgSrc, imgAscends, imgDescends, imgGradients, isMeye);
	}

	PrepCornea::collectImageStats(imgAscends);
	PrepCornea::collectColumnStats(imgAscends);
	PrepCornea::collectImageStats(imgDescends);
	PrepCornea::collectColumnStats(imgDescends);
	return true;
}


bool SegmScan::CorneaBsegm::elaborateEPI(const OcularImage * imgSrc, OcularLayer * layerInn, OcularLayer * layerOut, OcularLayer * layerEPI)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerInn->resize(width, height);
	layerOut->resize(width, height);

	float rangeX = getImpl().rangeX;
	int hingeY = getImpl().wideHingeY;
	int centerX = getImpl().corneaTopX;
	int centerY = getImpl().corneaTopY;

	vector<int> upper, lower, delta;
	vector<int> path, fitt;
	auto imgCost = costs();

	if (!PathCornea::designLayerContraintsOfEPI(imgSrc, layerInn, rangeX, hingeY, centerX, centerY, upper, lower, delta)) {
		return false;
	}

	if (!PathCornea::createLayerCostMapOfEPI(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!PathCornea::correctReflectiveRegionOfEPI(imgSrc, path, rangeX, hingeY, centerX, centerY)) {
		return false;
	}

	if (!Smooth::smoothLayerEPI(path, width, height, rangeX, layerEPI)) {
		return false;
	}

	if (PathCornea::makeFittingCurveOfEPI(path, path)) {
		bool isAxial = false;
		float resX = getImpl().resolutionX;
		float resY = getImpl().resolutionY;
		vector<float> output;

		Coarse::makeCurveRadiusOfCornea(resX, resY, rangeX, path, output, isAxial);
		d_ptr->curvRads = output;
	}
	else {
		d_ptr->curvRads = std::vector<float>(path.size(), 0.0f);
	}
	return true;
}


bool SegmScan::CorneaBsegm::elaborateEND(const OcularImage * imgSrc, OcularLayer * layerEPI, OcularLayer * layerOut, OcularLayer * layerEND)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerEPI->resize(width, height);
	layerOut->resize(width, height);

	float rangeX = getImpl().rangeX;
	int hingeY = getImpl().wideHingeY;
	int centerX = getImpl().corneaTopX;
	int centerY = getImpl().corneaTopY;

	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!PathCornea::designLayerContraintsOfEND(imgSrc, layerEPI, layerOut, rangeX, hingeY, centerX, centerY, 0, upper, lower, delta)) {
		return false;
	}

	if (!PathCornea::createLayerCostMapOfEND(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (PathCornea::makeFittingCurveOfEND(path, path)) {
		layerOut->initialize(path, width, height);

		if (!PathCornea::designLayerContraintsOfEND(imgSrc, layerEPI, layerOut, rangeX, hingeY, centerX, centerY, 1, upper, lower, delta)) {
			return false;
		}

		if (!PathCornea::createLayerCostMapOfEND(imgSrc, imgCost, upper, lower, delta)) {
			return false;
		}

		if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
			return false;
		}
	}

	if (!Smooth::smoothLayerEND(path, width, height, rangeX, layerEPI, layerEND)) {
		return false;
	}

	return true;
}


bool SegmScan::CorneaBsegm::elaborateBOW(const OcularImage * imgSrc, OcularLayer * layerEPI, OcularLayer * layerEND, OcularLayer * layerBOW)
{
	int width = imgSrc->getWidth();
	int height = imgSrc->getHeight();
	layerEPI->resize(width, height);
	layerEND->resize(width, height);

	float rangeX = getImpl().rangeX;
	int hingeY = getImpl().wideHingeY;
	int centerX = getImpl().corneaTopX;
	int centerY = getImpl().corneaTopY;
	
	vector<int> upper, lower, delta;
	vector<int> path;
	auto imgCost = costs();

	if (!PathCornea::designLayerContraintsOfBOW(imgSrc, layerEPI, layerEND, rangeX, hingeY, centerX, centerY, upper, lower, delta)) {
		return false;
	}

	if (!PathCornea::createLayerCostMapOfBOW(imgSrc, imgCost, upper, lower, delta)) {
		return false;
	}

	if (!OptimalPath::searchPathWithMinCost(imgCost, upper, lower, delta, path)) {
		return false;
	}

	if (!PathCornea::makeFittingCurveOfBOW(path, path, getImpl().corneaTopX, getImpl().corneaTopY)) {
		return false;
	}

	if (!Smooth::smoothLayerBOW(path, width, height, rangeX, layerEPI, layerEND, layerBOW)) {
		return false;
	}
	return true;
}


void SegmScan::CorneaBsegm::elaborateParams(void)
{
	float wRate = (float)source()->getWidth() / SAMPLE_IMAGE_WIDTH;
	float hRate = (float)source()->getHeight() / SAMPLE_IMAGE_HEIGHT;


	getImpl().corneaTopX = (getImpl().corneaTopX >= 0 ? (int)(getImpl().corneaTopX * wRate) : getImpl().corneaTopX);
	getImpl().corneaTopY = (getImpl().corneaTopY >= 0 ? (int)(getImpl().corneaTopY * hRate) : getImpl().corneaTopY);
	getImpl().wideHingeY = (getImpl().wideHingeY >= 0 ? (int)(getImpl().wideHingeY * hRate) : getImpl().wideHingeY);
	return;
}


OcularLayer * SegmScan::CorneaBsegm::layerInn(void) const
{
	return getCorneaLayers()->getInner();
}


OcularLayer * SegmScan::CorneaBsegm::layerOut(void) const
{
	return getCorneaLayers()->getOuter();
}


OcularLayer * SegmScan::CorneaBsegm::layerEPI(void) const
{
	return getCorneaLayers()->getEPI();
}


OcularLayer * SegmScan::CorneaBsegm::layerBOW(void) const
{
	return getCorneaLayers()->getBOW();
}


OcularLayer * SegmScan::CorneaBsegm::layerEND(void) const
{
	return getCorneaLayers()->getEND();
}


bool SegmScan::CorneaBsegm::getIncorrectIdx(void)
{
	return getImpl().isIncorrectIdx;
}

void SegmScan::CorneaBsegm::resetIncorrectIdx(void)
{
	getImpl().isIncorrectIdx = false;
}

bool SegmScan::CorneaBsegm::getThicknessLine(OcularLayerType upper, OcularLayerType lower, std::vector<float_t>& thicks, int width, int height, float upperOffset, float lowerOffset) const
{
	bool result = OcularBsegm::getDistanceLine(upper, lower, thicks, width, height);
	return result;
}


bool SegmScan::CorneaBsegm::getCurvatureRadius(std::vector<float>& radius) const
{
	auto image_w = source()->getWidth();
	auto image_h = source()->getHeight();
	radius = std::vector<float>(image_w, 0.0f);

	if (isEmpty() || image_w <= 0 || image_h <= 0 || d_ptr->curvRads.size() != image_w) {
		return false;
	}

	// CString text, data;
	for (int i = 0; i < image_w; i++) {
		radius[i] = d_ptr->curvRads[i];
		// data.Format(_T("%.2f "), radius[i]);
		// text += data;
	}

	// DebugOut() << "Radius line: ";
	// DebugOut() << wtoa(text);
	return true;
}


bool SegmScan::CorneaBsegm::setCurvatureRadius(const std::vector<float>& rads)
{
	if (isEmpty() || getSourceWidth() != rads.size()) {
		return false;
	}

	d_ptr->curvRads = rads;
	return true;
}


bool SegmScan::CorneaBsegm::calculateCorneaCurvature(void)
{
	float blank = -999.f;
	float rangeX = getImpl().rangeX;
	float resX = getImpl().resolutionX;
	float resY = (float)(GlobalSettings::getCorneaScanAxialResolution() * CORNEA_REFLECTIVE_INDEX);
	int index = getImageIndex();
	int size = getSourceWidth();
	
	d_ptr->axialAnteriorRadius.resize(size, blank);
	d_ptr->axialAnteriorRadiusSimK.resize(size, blank);
	d_ptr->axialPosteriorRadius.resize(size, blank);
	d_ptr->tangentialAnteriorRadius.resize(size, blank);
	d_ptr->tangentialPosteriorRadius.resize(size, blank);
	d_ptr->refractivePowerKerato.resize(size, blank);
	d_ptr->refractivePowerAnterior.resize(size, blank);
	d_ptr->refractivePowerPosterior.resize(size, blank);
	d_ptr->refractivePowerTotal.resize(size, blank);
	d_ptr->netMap.resize(size, blank);
	d_ptr->axialTrueNet.resize(size, blank);
	d_ptr->equivalentKeratometer.resize(size, blank);
	d_ptr->elevationAnterior.resize(size, blank);
	d_ptr->elevationPosterior.resize(size, blank);
	d_ptr->height.resize(size, blank);
	d_ptr->pachymetry.resize(size, blank);
	d_ptr->epithelium.resize(size, blank);
	d_ptr->incidenceAngle.resize(size, blank);

	std::vector<int> epiFoEpiYs;
	std::vector<int> epiFoBowYs;

	if (!getImpl().isModelEye && getPatternDescript().getPatternName() == PatternName::Topography) {
		epiFoEpiYs = layerEpiForEpi->getYs();
		epiFoBowYs = layerBowForEpi->getYs();
	}
	else {
		epiFoEpiYs.clear();
		epiFoBowYs.clear();
	}

	Coarse::calculateCurvatureOfCornea(resX, resY, layerEPI()->getYs(), layerBOW()->getYs(), layerEND()->getYs(), epiFoEpiYs, epiFoBowYs,
		d_ptr->axialAnteriorRadius, d_ptr->axialAnteriorRadiusSimK, d_ptr->axialPosteriorRadius, d_ptr->tangentialAnteriorRadius, d_ptr->tangentialPosteriorRadius,
		d_ptr->refractivePowerKerato, d_ptr->refractivePowerAnterior, d_ptr->refractivePowerPosterior, d_ptr->refractivePowerTotal,
		d_ptr->netMap, d_ptr->axialTrueNet, d_ptr->equivalentKeratometer, d_ptr->elevationAnterior, d_ptr->elevationPosterior,
		d_ptr->height, d_ptr->pachymetry, d_ptr->epithelium, d_ptr->incidenceAngle, index, getImpl().isModelEye);

	return true;
}


bool SegmScan::CorneaBsegm::getCorneaCurvature(std::vector<float>& output, int mapType) const
{
	switch (mapType)
	{
	case 0:
		output = d_ptr->axialAnteriorRadius;
		break;
	case 1:
		output = d_ptr->axialAnteriorRadiusSimK;
		break;
	case 2:
		output = d_ptr->axialPosteriorRadius;
		break;
	case 3:
		output = d_ptr->tangentialAnteriorRadius;
		break;
	case 4:
		output = d_ptr->tangentialPosteriorRadius;
		break;
	case 5:
		output = d_ptr->refractivePowerKerato;
		break;
	case 6:
		output = d_ptr->refractivePowerAnterior;
		break;
	case 7:
		output = d_ptr->refractivePowerPosterior;
		break;
	case 8:
		output = d_ptr->refractivePowerTotal;
		break;
	case 9:
		output = d_ptr->netMap;
		break;
	case 10:
		output = d_ptr->axialTrueNet;
		break;
	case 11:
		output = d_ptr->equivalentKeratometer;
		break;
	case 12:
		output = d_ptr->elevationAnterior;
		break;
	case 13:
		output = d_ptr->elevationPosterior;
		break;
	case 14:
		output = d_ptr->height;
		break;
	case 15:
		output = d_ptr->pachymetry;
		break;
	case 16:
		output = d_ptr->epithelium;
		break;
	default:
		return false;
		break;
	}
	return true;
}