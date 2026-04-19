#include "stdafx.h"
#include "MacularSegmenter.h"
#include "MacularSegmFrame.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct MacularSegmenter::MacularSegmenterImpl
{
	MacularSegmenterImpl()
	{
	}
};


MacularSegmenter::MacularSegmenter(RetinaSegmFrame* frame) :
	d_ptr(make_unique<MacularSegmenterImpl>()), RetinaSegmenter(frame)
{

}


SemtRetina::MacularSegmenter::~MacularSegmenter() = default;
SemtRetina::MacularSegmenter::MacularSegmenter(MacularSegmenter&& rhs) = default;
MacularSegmenter& SemtRetina::MacularSegmenter::operator=(MacularSegmenter&& rhs) = default;


bool SemtRetina::MacularSegmenter::segment(bool angio)
{
	auto frame = (MacularSegmFrame*)retinaSegmFrame();
	if (!frame->isImageSource()) {
		return false;
	}

	resetAlgorithms();

	auto* resam = bscanResampler();
	auto* crta = retinaSegmCriteria();
	auto& image = frame->bscanImage();
	auto index = frame->bscanIndex();

	if (!resam->runResampling(image, angio)) {
		return false;
	}
	if (!resam->checkRetinaSegmentable()) {
		return false;
	}
	auto* pipe = retinaInferPipeline();
	if (!pipe->inferLayerProbMaps()) {
		return false;
	}

	crta->enableSampleDimensions();
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
	auto* bonl = boundaryONL();
	if (!bonl->detectBoundary()) {
		return false;
	}

	auto* bnfl = boundaryNFL();
	if (!bnfl->detectBoundary()) {
		return false;
	}
	auto* bopl = boundaryOPL();
	if (!bopl->detectBoundary()) {
		return false;
	}
	auto* bipl = boundaryIPL();
	if (!bipl->detectBoundary()) {
		return false;
	}

	auto* bbrm = boundaryBRM();
	if (!bbrm->detectBoundary()) {
		return false;
	}
	auto* bios = boundaryIOS();
	if (!bios->detectBoundary()) {
		return false;
	}
	auto* brpe = boundaryRPE();
	if (!brpe->detectBoundary()) {
		return false;
	}

	crta->enableSourceDimensions();
	band->upscaleToSourceDimensions();

	bios->reconstructLayer();
	brpe->reconstructLayer();
	bbrm->reconstructLayer();

	bnfl->reconstructLayer();
	bipl->reconstructLayer();
	bopl->reconstructLayer();
	bilm->reconstructLayer();
	return true;

	/*


	if (!bilm->refineBoundary()) {
		return false;
	}


	// LogD() << "Boundary detected elapsed: " << CppUtil::ClockTimer::elapsedMsec();

	bnfl->enforceSourceOrder();
	bipl->enforceSourceOrder();
	bopl->enforceSourceOrder();
	bios->enforceSourceOrder();
	brpe->enforceSourceOrder();
	bbrm->enforceSourceOrder();
	*/
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


void SemtRetina::MacularSegmenter::resetAlgorithms(void)
{
	setBscanResampler(new BscanResampler(this));
	setRetinaBandExtractor(new RetinaBandExtractor(this));
	setRetinaInferPipeline(new RetinaInferPipeline(this));
	setRetinaSegmCriteria(new RetinaSegmCriteria(this));
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



MacularSegmenter::MacularSegmenterImpl& SemtRetina::MacularSegmenter::impl(void) const
{
	return *d_ptr;
}


