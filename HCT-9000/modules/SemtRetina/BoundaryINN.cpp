#include "stdafx.h"
#include "BoundaryINN.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryINN::BoundaryINNImpl
{
	BoundaryINNImpl()
	{
	}
};


BoundaryINN::BoundaryINN(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryINNImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryINN::~BoundaryINN() = default;
SemtRetina::BoundaryINN::BoundaryINN(BoundaryINN&& rhs) = default;
BoundaryINN& SemtRetina::BoundaryINN::operator=(BoundaryINN&& rhs) = default;


bool SemtRetina::BoundaryINN::detectBoundary(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto inns = band->innerYsFull();

	this->sampleYs() = inns;

	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryINN::resizeToMatchSource(void)
{
	auto* segm = retinaSegmenter();
	auto* resa = segm->bscanResampler();

	auto src_w = resa->sampleWidth();
	auto src_h = resa->sampleHeight();
	auto dst_w = resa->sourceWidth();
	auto dst_h = resa->sourceHeight();

	auto path = this->sampleYs();
	auto outs = std::vector<int>(dst_w, -1);
	if (!resizeBoundaryPath(path, src_w, src_h, dst_w, dst_h, outs)) {
		return false;
	}

	this->sourceYs() = outs;
	return true;
}

BoundaryINN::BoundaryINNImpl& SemtRetina::BoundaryINN::impl(void) const
{
	return *d_ptr;
}