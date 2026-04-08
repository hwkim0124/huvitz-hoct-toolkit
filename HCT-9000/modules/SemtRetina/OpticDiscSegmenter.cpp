#include "stdafx.h"
#include "OpticDiscSegmenter.h"
#include "MacularSegmFrame.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct OpticDiscSegmenter::OpticDiscSegmenterImpl
{
	OpticDiscSegmenterImpl()
	{
	}
};


OpticDiscSegmenter::OpticDiscSegmenter(RetinaSegmFrame* frame) :
	d_ptr(make_unique<OpticDiscSegmenterImpl>()), RetinaSegmenter(frame)
{

}


SemtRetina::OpticDiscSegmenter::~OpticDiscSegmenter() = default;
SemtRetina::OpticDiscSegmenter::OpticDiscSegmenter(OpticDiscSegmenter&& rhs) = default;
OpticDiscSegmenter& SemtRetina::OpticDiscSegmenter::operator=(OpticDiscSegmenter&& rhs) = default;


bool SemtRetina::OpticDiscSegmenter::segment(void)
{
	auto frame = (MacularSegmFrame*)retinaSegmFrame();
	if (!frame->isImageSource()) {
		return false;
	}

	resetAlgorithms();

	auto* resam = bscanResampler();
	auto& image = frame->bscanImage();
	auto index = frame->bscanIndex();

	if (!resam->runResampling(image, MODEL_INPUT_WIDTH, MODEL_INPUT_HEIGHT)) {
		return false;
	}
	if (!resam->checkRetinaSegmentable()) {
		return false;
	}

	auto* pipe = retinaInferPipeline();
	if (!pipe->inferLayerProbMaps()) {
		return false;
	}

	auto* band = retinaBandExtractor();
	if (!band->estimateHorizontalBounds()) {
		return false;
	}

	if (!band->detectInnerRetinaBoundary()) {
		return false;
	}
	if (!band->detectOuterRetinaBoundary()) {
		return false;
	}
	if (!band->detectOpticNerveHeadRegion()) {
		return false;
	}

	auto* binn = boundaryINN();
	if (!binn->detectBoundary()) {
		return false;
	}

	auto* bout = boundaryOUT();
	if (!bout->detectBoundary()) {
		return false;
	}

	auto* bilm = boundaryILM();
	if (!bilm->detectBoundary()) {
		return false;
	}

	auto* brpe = boundaryRPE();
	if (!brpe->detectBoundary()) {
		return false;
	}

	auto* bbrm = boundaryBRM();
	if (!bbrm->detectBoundary()) {
		return false;
	}

	auto* bonl = boundaryONL();
	if (!bonl->detectBoundary()) {
		return false;
	}

	auto* bios = boundaryIOS();
	if (!bios->detectBoundary()) {
		return false;
	}

	auto* bopl = boundaryOPL();
	if (!bopl->detectBoundary()) {
		return false;
	}

	auto* bnfl = boundaryNFL();
	if (!bnfl->detectBoundary()) {
		return false;
	}

	auto* bipl = boundaryIPL();
	if (!bipl->detectBoundary()) {
		return false;
	}

	if (!bilm->refineBoundary()) {
		return false;
	}
	if (!brpe->refineBoundary()) {
		return false;
	}

	bnfl->enforceSourceOrder();
	bipl->enforceSourceOrder();
	bopl->enforceSourceOrder();
	bios->enforceSourceOrder();
	brpe->enforceSourceOrder();
	bbrm->enforceSourceOrder();

	if (band->isNerveHeadRangeValid()) {
		auto* onhm = onhMorphometrics();
		onhm->accumulateDiscRimVoxels();
		onhm->accumulateDiscCupVoxels();
		/*
		if (!bbrm->refineBoundary()) {
			return false;
		}
		*/
	}
	return true;
}


void SemtRetina::OpticDiscSegmenter::resetAlgorithms(void)
{
	setBscanResampler(new BscanResampler());
	setRetinaBandExtractor(new RetinaBandExtractor(this));
	setRetinaInferPipeline(new RetinaInferPipeline(this));
	setONHMorphometrics(new ONHMorphometrics(this));

	setBoundaryILM(new BoundaryILM(this));
	setBoundaryNFL(new BoundaryNFL(this));
	setBoundaryIPL(new BoundaryIPL(this));
	setBoundaryOPL(new BoundaryOPL(this));
	setBoundaryONL(new BoundaryONL(this));
	setBoundaryIOS(new BoundaryIOS(this));
	setBoundaryRPE(new BoundaryRPE(this));
	setBoundaryBRM(new BoundaryBRM(this));

	setBoundaryINN(new BoundaryINN(this));
	setBoundaryOUT(new BoundaryOUT(this));
	return;
}



OpticDiscSegmenter::OpticDiscSegmenterImpl& SemtRetina::OpticDiscSegmenter::impl(void) const
{
	return *d_ptr;
}


