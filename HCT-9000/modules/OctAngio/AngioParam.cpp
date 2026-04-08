#include "stdafx.h"
#include "AngioParam.h"


using namespace OctAngio;
using namespace std;

#include <iterator>
#include <algorithm>
#include <numeric>
#include <vector>

struct AngioParam::AngioParamImpl
{
	bool isVerticalScan = false;
	int overlaps = 3;
	int points = 384;
	int lines = 384;

	float rangeX = 4.5f;
	float rangeY = 4.5f;
	float lineSpace = 0.0f;
	float foveaCx = 0.0f;
	float foveaCy = 0.0f;

	OcularLayerType slabUpperType = OcularLayerType::NFL;
	OcularLayerType slabLowerType = OcularLayerType::OPL;
	float slabUpperOffset = 0.0f;
	float slabLowerOffset = 0.0f;

	bool axialAlign = true;
	bool lateralAlign = false;
	bool pixelAveraging = true;
	int pixelAverageSize = 2;
	bool decorrCircular = false;
	bool differOutput = true;
	bool decorrOutput = false;
	bool reflectCorrect = false;

	float decorrThresh = 0.0f;
	int intensThresh1 = 0;
	int intensThresh2 = 0;

	float decorrNormThresh1 = 0.0f;
	float decorrNormThresh2 = 0.0f;
	float differNormThresh1 = 0.0f;
	float differNormThresh2 = 0.0f;

	bool layersSelected = true;
	bool postProcessing = false;
	bool normProjection = true;
	bool projectArtifactRemoval = false;
	bool motionCorrect = false;
	bool vascularLayers = false;
	bool contrastEnhance = false;
	float contrastClipLimit = 4.0f;

	float biasSigma = 0.0f;
	float normDropOff = 0.0f;
	float noiseReductionRate = 0.25f;

	AngioParamImpl() {

	};
};


AngioParam::AngioParam()
	: d_ptr(make_unique<AngioParamImpl>())
{
}


AngioParam::~AngioParam()
{
	// Destructor should be defined for unique_ptr to delete AngioDecorrImpl as an incomplete type.
}


OctAngio::AngioParam::AngioParam(AngioParam && rhs) = default;
AngioParam & OctAngio::AngioParam::operator=(AngioParam && rhs) = default;


OctAngio::AngioParam::AngioParam(const AngioParam & rhs)
	: d_ptr(make_unique<AngioParamImpl>(*rhs.d_ptr))
{
}


AngioParam & OctAngio::AngioParam::operator=(const AngioParam & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}

auto OctAngio::AngioParam::isVerticalScan(void) -> bool &
{
	return getImpl().isVerticalScan;
}

auto OctAngio::AngioParam::numberOfOverlaps(void) -> int &
{
	return getImpl().overlaps;
}

auto OctAngio::AngioParam::numberOfPoints(void) -> int &
{
	return getImpl().points;
}

auto OctAngio::AngioParam::numberOfLines(void) -> int &
{
	return getImpl().lines;
}

auto OctAngio::AngioParam::scanRangeX(void) -> float &
{
	return getImpl().rangeX;
}

auto OctAngio::AngioParam::scanRangeY(void) -> float &
{
	return getImpl().rangeY;
}

auto OctAngio::AngioParam::scanLineSpace(void) -> float &
{
	return getImpl().lineSpace;
}

auto OctAngio::AngioParam::foveaCenterX(void) -> float &
{
	return getImpl().foveaCx;
}

auto OctAngio::AngioParam::foveaCenterY(void) -> float &
{
	return getImpl().foveaCy;
}

auto OctAngio::AngioParam::lowerLayerType(void) -> OctGlobal::OcularLayerType &
{
	return getImpl().slabLowerType;
}

auto OctAngio::AngioParam::upperLayerType(void) -> OctGlobal::OcularLayerType &
{
	return getImpl().slabUpperType;
}

auto OctAngio::AngioParam::lowerLayerOffset(void) -> float &
{
	return getImpl().slabLowerOffset;
}

auto OctAngio::AngioParam::upperLayerOffset(void) -> float &
{
	return getImpl().slabUpperOffset;
}

auto OctAngio::AngioParam::useAxialAlign(void) -> bool &
{
	return getImpl().axialAlign;
}

auto OctAngio::AngioParam::useLateralAlign(void) -> bool &
{
	return getImpl().lateralAlign;
}

auto OctAngio::AngioParam::usePixelAveraging(void) -> bool &
{
	return getImpl().pixelAveraging;
}

auto OctAngio::AngioParam::pixelAverageSize(void) -> int &
{
	return getImpl().pixelAverageSize;
}

auto OctAngio::AngioParam::useDecorrCircular(void) -> bool &
{
	return getImpl().decorrCircular;
}

auto OctAngio::AngioParam::useDifferOutput(void) -> bool &
{
	return getImpl().differOutput;
}

auto OctAngio::AngioParam::useDecorrOutput(void) -> bool &
{
	return getImpl().decorrOutput;
}

auto OctAngio::AngioParam::useReflectCorrection(void) -> bool &
{
	return getImpl().reflectCorrect;
}

auto OctAngio::AngioParam::decorrLowerThreshold(void) -> float &
{
	return getImpl().decorrThresh;
}

auto OctAngio::AngioParam::intensLowerThreshold(void) -> int &
{
	return getImpl().intensThresh1;
}

auto OctAngio::AngioParam::intensUpperthreshold(void) -> int &
{
	return getImpl().intensThresh2;
}

auto OctAngio::AngioParam::decorrNormLowerThreshold(void) -> float &
{
	return getImpl().decorrNormThresh1;
}

auto OctAngio::AngioParam::decorrNormUpperThreshold(void) -> float &
{
	return getImpl().decorrNormThresh2;
}

auto OctAngio::AngioParam::differNormLowerThreshold(void) -> float &
{
	return getImpl().differNormThresh1;
}

auto OctAngio::AngioParam::differNormUpperThreshold(void) -> float &
{
	return getImpl().differNormThresh2;
}

auto OctAngio::AngioParam::useLayersSelected(void) -> bool &
{
	return getImpl().layersSelected;
}

auto OctAngio::AngioParam::usePostProcessing(void) -> bool &
{
	return getImpl().postProcessing;
}

auto OctAngio::AngioParam::useNormProjection(void) -> bool &
{
	return getImpl().normProjection;
}

auto OctAngio::AngioParam::useProjectArtifactRemoval(void) -> bool &
{
	return getImpl().projectArtifactRemoval;
}

auto OctAngio::AngioParam::useMotionCorrection(void) -> bool &
{
	return getImpl().motionCorrect;
}

auto OctAngio::AngioParam::useVascularLayers(void) -> bool &
{
	return getImpl().vascularLayers;
}

auto OctAngio::AngioParam::useContrastEnhance(void) -> bool &
{
	return getImpl().contrastEnhance;
}


AngioParam::AngioParamImpl & OctAngio::AngioParam::getImpl(void) const
{
	return *d_ptr;
}

auto OctAngio::AngioParam::contrastClipLimit(void) -> float &
{
	return getImpl().contrastClipLimit;
}

auto OctAngio::AngioParam::biasFieldSigma(void) -> float &
{
	return getImpl().biasSigma;
}

auto OctAngio::AngioParam::normalizeDropOff(void) -> float &
{
	return getImpl().normDropOff;
}

auto OctAngio::AngioParam::noiseReductionRate(void) -> float &
{
	return getImpl().noiseReductionRate;
}
