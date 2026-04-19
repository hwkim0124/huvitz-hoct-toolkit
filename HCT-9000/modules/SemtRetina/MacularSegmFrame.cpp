#include "stdafx.h"
#include "MacularSegmFrame.h"
#include "MacularSegmenter.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtRetina;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct MacularSegmFrame::MacularSegmFrameImpl
{
	MacularSegmFrameImpl()
	{
	}
};


MacularSegmFrame::MacularSegmFrame() :
	d_ptr(make_unique<MacularSegmFrameImpl>())
{
}


SemtRetina::MacularSegmFrame::~MacularSegmFrame() = default;
SemtRetina::MacularSegmFrame::MacularSegmFrame(MacularSegmFrame&& rhs) = default;
MacularSegmFrame& SemtRetina::MacularSegmFrame::operator=(MacularSegmFrame&& rhs) = default;


bool SemtRetina::MacularSegmFrame::segmentMacularLayers(bool angio)
{
	resetRetinaSegmenter();

	auto* segm = macularSegmenter();
	if (!segm->segment(angio)) {
		return false;
	}
	return true;
}


void SemtRetina::MacularSegmFrame::resetRetinaSegmenter()
{
	setRetinaSegmenter(new MacularSegmenter(this));
	return;
}


MacularSegmenter* SemtRetina::MacularSegmFrame::macularSegmenter() const
{
	auto ptr = (MacularSegmenter*)retinaSegmenter();
	return ptr;
}


MacularSegmFrame::MacularSegmFrameImpl& SemtRetina::MacularSegmFrame::impl(void) const
{
	return *d_ptr;
}
