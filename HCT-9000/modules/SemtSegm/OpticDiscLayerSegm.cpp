#include "stdafx.h"
#include "OpticDiscLayerSegm.h"
#include "OpticDiscSampling.h"
#include "OpticDiscBscan.h"


using namespace SemtSegm;
using namespace std;


struct OpticDiscLayerSegm::OpticDiscLayerSegmImpl
{
	OpticDiscLayerSegmImpl()
	{
	}
};


OpticDiscLayerSegm::OpticDiscLayerSegm(LayeredBscan* bscan) :
	d_ptr(make_unique<OpticDiscLayerSegmImpl>()), BscanSegmentator(bscan)
{
}


SemtSegm::OpticDiscLayerSegm::~OpticDiscLayerSegm() = default;
SemtSegm::OpticDiscLayerSegm::OpticDiscLayerSegm(OpticDiscLayerSegm && rhs) = default;
OpticDiscLayerSegm & SemtSegm::OpticDiscLayerSegm::operator=(OpticDiscLayerSegm && rhs) = default;


bool SemtSegm::OpticDiscLayerSegm::doSegmentation()
{
	auto pScan = (OpticDiscBscan*)getLayeredBscan();
	if (!pScan->isImageSource()) {
		return false;
	}

	resetAlgorithms();

	auto* pSample = (OpticDiscSampling*)getSampling();
	auto& image = pScan->getImageSource();
	auto index = pScan->getImageIndex();

	if (!pSample->produceSample(image)) {
		return false;
	}
	if (!pSample->checkIfValidSample(index)) {
		return false;
	}
	if (!pSample->createGradientMaps()) {
		return false;
	}

	auto* pBorder = (Bordering*)getBodering();
	if (!pBorder->establishOcularBound()) {
		return false;
	}
	if (!pBorder->locateOutlinePoints()) {
		return false;
	}

	if (!pBorder->buildInnerBoundary()) {
		return false;
	}
	
	if (!pBorder->buildOuterBoundary()) {
		return false;
	}

	auto* pClassify = (Classifier*)getClassifier();
	if (!pClassify->prepareInput()) {
		return false;
	}
	if (!pClassify->classifyPixels(false)) {
		return false;
	}

	int image_w = getImageSource().getWidth();
	int image_h = getImageSource().getHeight();
	int classify_w = pClassify->SOURCE_WIDTH;
	int classify_h = pClassify->SOURCE_HEIGHT;
	int input_w = pClassify->INPUT_WIDTH;
	int input_h = pClassify->INPUT_HEIGHT;
	auto shifts = pClassify->getAlignShifts();
	
	getLayerInner()->setRegionDimension(classify_w, classify_h);
	getLayerOuter()->setRegionDimension(classify_w, classify_h);

	getLayerInner()->applyVeritcalShifts(shifts);
	getLayerOuter()->applyVeritcalShifts(shifts);

	getLayerILM()->setRegionDimension(input_w, input_h);
	getLayerNFL()->setRegionDimension(input_w, input_h);
	getLayerIPL()->setRegionDimension(input_w, input_h);
	getLayerOPL()->setRegionDimension(input_w, input_h);
	getLayerIOS()->setRegionDimension(input_w, input_h);
	getLayerRPE()->setRegionDimension(input_w, input_h);
	getLayerBRM()->setRegionDimension(input_w, input_h);
	getLayerOPR()->setRegionDimension(input_w, input_h);

	getLayerILM()->buildFlattenedPath();
	getLayerOPL()->buildFlattenedPath();
	getLayerIPL()->buildFlattenedPath();
	getLayerNFL()->buildFlattenedPath();
	getLayerIOS()->buildFlattenedPath();
	getLayerBRM()->buildFlattenedPath();
	getLayerRPE()->buildFlattenedPath();
	getLayerOPR()->buildFlattenedPath();
	
	getLayerIPL()->alterFlattenedPath();
	getLayerIOS()->alterFlattenedPath();
	getLayerILM()->alterFlattenedPath();

	getLayerILM()->applyVeritcalShifts(shifts, true);
	getLayerOPL()->applyVeritcalShifts(shifts, true);
	getLayerIPL()->applyVeritcalShifts(shifts, true);
	getLayerNFL()->applyVeritcalShifts(shifts, true);
	getLayerIOS()->applyVeritcalShifts(shifts, true);
	getLayerBRM()->applyVeritcalShifts(shifts, true);
	getLayerRPE()->applyVeritcalShifts(shifts, true);
	getLayerOPR()->applyVeritcalShifts(shifts, true);

	getLayerInner()->applyVeritcalShifts(shifts, true);
	getLayerOuter()->applyVeritcalShifts(shifts, true);

	getLayerILM()->setRegionDimension(image_w, image_h, false);
	getLayerNFL()->setRegionDimension(image_w, image_h, false);
	getLayerIPL()->setRegionDimension(image_w, image_h, false);
	getLayerOPL()->setRegionDimension(image_w, image_h, false);
	getLayerIOS()->setRegionDimension(image_w, image_h, false);
	getLayerRPE()->setRegionDimension(image_w, image_h, false);
	getLayerBRM()->setRegionDimension(image_w, image_h, false);
	getLayerOPR()->setRegionDimension(image_w, image_h, false);

	getBodering()->inflateOpticDiscRegion();

	getLayerILM()->buildBoundaryLayer();
	getLayerNFL()->buildBoundaryLayer();
	getLayerOPL()->buildBoundaryLayer();
	getLayerIPL()->buildBoundaryLayer();

	getLayerIOS()->buildBoundaryLayer();
	getLayerRPE()->buildBoundaryLayer();
	getLayerBRM()->buildBoundaryLayer();
	getLayerOPR()->buildBoundaryLayer();

	getMeasuring()->findOpticDiscSideParams();
	getMeasuring()->findOpticDiscCupParams();
	return true;
}


Sampling * SemtSegm::OpticDiscLayerSegm::createSampling() 
{
	return new OpticDiscSampling();
}


OpticDiscLayerSegm::OpticDiscLayerSegmImpl & SemtSegm::OpticDiscLayerSegm::getImpl(void) const
{
	return *d_ptr;
}

