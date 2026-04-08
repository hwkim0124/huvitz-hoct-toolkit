#include "stdafx.h"
#include "RetinaSegmenter.h"
#include "RetinaSegmFrame.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaSegmenter::RetinaSegmenterImpl
{
	const RetinaSegmFrame* frame = nullptr;

	unique_ptr<BscanResampler> resampler;
	unique_ptr<RetinaBandExtractor> extractor;
	unique_ptr<RetinaInferPipeline> pipeline;
	unique_ptr<ONHMorphometrics> onhMetrics;

	unique_ptr<BoundaryILM> boundILM;
	unique_ptr<BoundaryNFL> boundNFL;
	unique_ptr<BoundaryIPL> boundIPL;
	unique_ptr<BoundaryOPL> boundOPL;
	unique_ptr<BoundaryONL> boundONL;
	unique_ptr<BoundaryIOS> boundIOS;
	unique_ptr<BoundaryRPE> boundRPE;
	unique_ptr<BoundaryBRM> boundBRM;

	unique_ptr<BoundaryINN> boundINN;
	unique_ptr<BoundaryOUT> boundOUT;

	RetinaSegmenterImpl()
	{
	}
};


RetinaSegmenter::RetinaSegmenter(RetinaSegmFrame* frame) :
	d_ptr(make_unique<RetinaSegmenterImpl>())
{
	impl().frame = frame;
}


SemtRetina::RetinaSegmenter::~RetinaSegmenter() = default;
SemtRetina::RetinaSegmenter::RetinaSegmenter(RetinaSegmenter&& rhs) = default;
RetinaSegmenter& SemtRetina::RetinaSegmenter::operator=(RetinaSegmenter&& rhs) = default;


const RetinaSegmFrame* SemtRetina::RetinaSegmenter::retinaSegmFrame(void) const
{
	auto* ptr = impl().frame;
	return ptr;
}

BscanResampler* SemtRetina::RetinaSegmenter::bscanResampler(void) const
{
	return impl().resampler.get();
}

RetinaBandExtractor* SemtRetina::RetinaSegmenter::retinaBandExtractor(void) const
{
	return impl().extractor.get();
}

RetinaInferPipeline* SemtRetina::RetinaSegmenter::retinaInferPipeline(void) const
{
	return impl().pipeline.get();
}

ONHMorphometrics* SemtRetina::RetinaSegmenter::onhMorphometrics(void) const
{
	return impl().onhMetrics.get();
}

BoundaryILM* SemtRetina::RetinaSegmenter::boundaryILM(void) const
{
	return impl().boundILM.get();
}

BoundaryNFL* SemtRetina::RetinaSegmenter::boundaryNFL(void) const
{
	return impl().boundNFL.get();
}

BoundaryIPL* SemtRetina::RetinaSegmenter::boundaryIPL(void) const
{
	return impl().boundIPL.get();
}

BoundaryOPL* SemtRetina::RetinaSegmenter::boundaryOPL(void) const
{
	return impl().boundOPL.get();
}

BoundaryONL* SemtRetina::RetinaSegmenter::boundaryONL(void) const
{
	return impl().boundONL.get();
}

BoundaryIOS* SemtRetina::RetinaSegmenter::boundaryIOS(void) const
{
	return impl().boundIOS.get();
}

BoundaryRPE* SemtRetina::RetinaSegmenter::boundaryRPE(void) const
{
	return impl().boundRPE.get();
}

BoundaryBRM* SemtRetina::RetinaSegmenter::boundaryBRM(void) const
{
	return impl().boundBRM.get();
}

BoundaryINN* SemtRetina::RetinaSegmenter::boundaryINN(void) const
{
	return impl().boundINN.get();
}

BoundaryOUT* SemtRetina::RetinaSegmenter::boundaryOUT(void) const
{
	return impl().boundOUT.get();
}

void SemtRetina::RetinaSegmenter::setBscanResampler(BscanResampler* p)
{
	impl().resampler = move(unique_ptr<BscanResampler>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setRetinaBandExtractor(RetinaBandExtractor* p)
{
	impl().extractor = move(unique_ptr<RetinaBandExtractor>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setRetinaInferPipeline(RetinaInferPipeline* p)
{
	impl().pipeline = move(unique_ptr<RetinaInferPipeline>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setONHMorphometrics(ONHMorphometrics* p)
{
	impl().onhMetrics = move(unique_ptr<ONHMorphometrics>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryILM(BoundaryILM* p)
{
	impl().boundILM = move(unique_ptr<BoundaryILM>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryNFL(BoundaryNFL* p)
{
	impl().boundNFL = move(unique_ptr<BoundaryNFL>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryIPL(BoundaryIPL* p)
{
	impl().boundIPL = move(unique_ptr<BoundaryIPL>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryOPL(BoundaryOPL* p)
{
	impl().boundOPL = move(unique_ptr<BoundaryOPL>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryONL(BoundaryONL* p)
{
	impl().boundONL = move(unique_ptr<BoundaryONL>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryIOS(BoundaryIOS* p)
{
	impl().boundIOS = move(unique_ptr<BoundaryIOS>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryRPE(BoundaryRPE* p)
{
	impl().boundRPE = move(unique_ptr<BoundaryRPE>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryBRM(BoundaryBRM* p)
{
	impl().boundBRM = move(unique_ptr<BoundaryBRM>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryINN(BoundaryINN* p)
{
	impl().boundINN = move(unique_ptr<BoundaryINN>(p));
	return;
}

void SemtRetina::RetinaSegmenter::setBoundaryOUT(BoundaryOUT* p)
{
	impl().boundOUT = move(unique_ptr<BoundaryOUT>(p));
	return;
}

RetinaSegmenter::RetinaSegmenterImpl& SemtRetina::RetinaSegmenter::impl(void) const
{
	return *d_ptr;
}


