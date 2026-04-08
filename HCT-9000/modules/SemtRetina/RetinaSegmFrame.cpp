#include "stdafx.h"
#include "RetinaSegmFrame.h"
#include "RetinaSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct RetinaSegmFrame::RetinaSegmFrameImpl
{
	OctScanImage image;
	unique_ptr<RetinaSegmenter> segm;

	RetinaSegmFrameImpl()
	{
	}
};


RetinaSegmFrame::RetinaSegmFrame() :
	d_ptr(make_unique<RetinaSegmFrameImpl>())
{
}


SemtRetina::RetinaSegmFrame::~RetinaSegmFrame() = default;
SemtRetina::RetinaSegmFrame::RetinaSegmFrame(RetinaSegmFrame&& rhs) = default;
RetinaSegmFrame& SemtRetina::RetinaSegmFrame::operator=(RetinaSegmFrame&& rhs) = default;

void SemtRetina::RetinaSegmFrame::setBscanImage(const unsigned char* bits, int width, int height, float range, int index)
{
	impl().image.setup(bits, width, height);
	impl().image.setRangeX(range);
	impl().image.setIndex(index);
}

void SemtRetina::RetinaSegmFrame::setBscanImage(OctScanImage image)
{
	impl().image = image;
}

void SemtRetina::RetinaSegmFrame::setBscanIndex(int index)
{
	impl().image.setIndex(index);
}

bool SemtRetina::RetinaSegmFrame::isImageSource(void) const
{
	bool flag = impl().image.isEmpty() == false;
	return flag;
}

int SemtRetina::RetinaSegmFrame::bscanIndex(void) const
{
	auto index = impl().image.getIndex();
	return index;
}

float SemtRetina::RetinaSegmFrame::scanRangeXmm(void) const
{
	auto range = impl().image.getRangeX();
	return range;
}

const OctScanImage& SemtRetina::RetinaSegmFrame::bscanImage(void) const
{
	return impl().image;
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryILM(void) const
{
	auto* p = impl().segm->boundaryILM();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryNFL(void) const
{
	auto* p = impl().segm->boundaryNFL();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryIPL(void) const
{
	auto* p = impl().segm->boundaryIPL();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryOPL(void) const
{
	auto* p = impl().segm->boundaryOPL();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryONL(void) const
{
	auto* p = impl().segm->boundaryONL();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryIOS(void) const
{
	auto* p = impl().segm->boundaryIOS();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryRPE(void) const
{
	auto* p = impl().segm->boundaryRPE();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryBRM(void) const
{
	auto* p = impl().segm->boundaryBRM();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryINN(void) const
{
	auto* p = impl().segm->boundaryINN();
	return p->sourceYs();
}

const std::vector<int>& SemtRetina::RetinaSegmFrame::boundaryOUT(void) const
{
	auto* p = impl().segm->boundaryOUT();
	return p->sourceYs();
}

const int SemtRetina::RetinaSegmFrame::onhDiscMarginMinX(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->discMarginMinX();
	return v;
}

const int SemtRetina::RetinaSegmFrame::onhDiscMarginMaxX(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->discMarginMaxX();
	return v;
}

const int SemtRetina::RetinaSegmFrame::onhCupOpeningMinX(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->cupOpeningMinX();
	return v;
}

const int SemtRetina::RetinaSegmFrame::onhCupOpeningMaxX(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->cupOpeningMaxX();
	return v;
}

const int SemtRetina::RetinaSegmFrame::onhDiscRimVoxelCount(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->discRimVoxelCount();
	return v;
}

const int SemtRetina::RetinaSegmFrame::onhDiscCupVoxelCount(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->discCupVoxelCount();
	return v;
}

const bool SemtRetina::RetinaSegmFrame::isONHDiscRegionDetected(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->isDiscMarginRangeValid();
	return v;
}

const bool SemtRetina::RetinaSegmFrame::isONHCupRegionDetected(void) const
{
	auto* p = impl().segm->onhMorphometrics();
	auto v = p->isCupOpeningRangeValid();
	return v;
}

void SemtRetina::RetinaSegmFrame::setRetinaSegmenter(RetinaSegmenter* segm)
{
	impl().segm = unique_ptr<RetinaSegmenter>(segm);
	return;
}


RetinaSegmenter* SemtRetina::RetinaSegmFrame::retinaSegmenter() const
{
	auto ptr = impl().segm.get();
	return ptr;
}

RetinaSegmFrame::RetinaSegmFrameImpl& SemtRetina::RetinaSegmFrame::impl(void) const
{
	return *d_ptr;
}
