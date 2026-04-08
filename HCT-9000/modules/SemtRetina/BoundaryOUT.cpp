#include "stdafx.h"
#include "BoundaryOUT.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct BoundaryOUT::BoundaryOUTImpl
{
	BoundaryOUTImpl()
	{
	}
};


BoundaryOUT::BoundaryOUT(RetinaSegmenter* segm) :
	d_ptr(make_unique<BoundaryOUTImpl>()), RetinaBoundary(segm)
{
}


SemtRetina::BoundaryOUT::~BoundaryOUT() = default;
SemtRetina::BoundaryOUT::BoundaryOUT(BoundaryOUT&& rhs) = default;
BoundaryOUT& SemtRetina::BoundaryOUT::operator=(BoundaryOUT&& rhs) = default;


bool SemtRetina::BoundaryOUT::detectBoundary(void)
{
	auto* segm = retinaSegmenter();
	auto* band = segm->retinaBandExtractor();
	auto outs = band->outerYsFull();

	this->sampleYs() = outs;

	if (!resizeToMatchSource()) {
		return false;
	}
	return true;
}


bool SemtRetina::BoundaryOUT::resizeToMatchSource(void)
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

BoundaryOUT::BoundaryOUTImpl& SemtRetina::BoundaryOUT::impl(void) const
{
	return *d_ptr;
}