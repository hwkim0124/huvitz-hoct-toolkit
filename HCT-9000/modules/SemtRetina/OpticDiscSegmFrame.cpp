#include "stdafx.h"
#include "OpticDiscSegmFrame.h"
#include "OpticDiscSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct OpticDiscSegmFrame::OpticDiscSegmFrameImpl
{
	OpticDiscSegmFrameImpl()
	{
	}
};


OpticDiscSegmFrame::OpticDiscSegmFrame() :
	d_ptr(make_unique<OpticDiscSegmFrameImpl>())
{
}


SemtRetina::OpticDiscSegmFrame::~OpticDiscSegmFrame() = default;
SemtRetina::OpticDiscSegmFrame::OpticDiscSegmFrame(OpticDiscSegmFrame&& rhs) = default;
OpticDiscSegmFrame& SemtRetina::OpticDiscSegmFrame::operator=(OpticDiscSegmFrame&& rhs) = default;


bool SemtRetina::OpticDiscSegmFrame::segmentMacularLayers(bool angio)
{
	resetRetinaSegmenter();

	auto* segm = opticDiscSegmenter();
	if (!segm->segment(angio)) {
		return false;
	}
	return true;
}


void SemtRetina::OpticDiscSegmFrame::resetRetinaSegmenter()
{
	setRetinaSegmenter(new OpticDiscSegmenter(this));
	return;
}


OpticDiscSegmenter* SemtRetina::OpticDiscSegmFrame::opticDiscSegmenter() const
{
	auto ptr = (OpticDiscSegmenter*)retinaSegmenter();
	return ptr;
}


OpticDiscSegmFrame::OpticDiscSegmFrameImpl& SemtRetina::OpticDiscSegmFrame::impl(void) const
{
	return *d_ptr;
}
