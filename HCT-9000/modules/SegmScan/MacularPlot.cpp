#include "stdafx.h"
#include "MacularPlot.h"
#include "RetParam2.h"
#include "CppUtil2.h"
#include "PatternOutput.h"


using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct MacularPlot::MacularPlotImpl
{
	EnfacePlot plot;
	CvImage image;
	PatternOutput* output;

	MacularPlotImpl(PatternOutput* p) : output(p) {

	}
};


MacularPlot::MacularPlot(PatternOutput* output) :
	d_ptr(make_unique<MacularPlotImpl>(output))
{
}


SegmScan::MacularPlot::~MacularPlot() = default;
SegmScan::MacularPlot::MacularPlot(MacularPlot && rhs) = default;
MacularPlot & SegmScan::MacularPlot::operator=(MacularPlot && rhs) = default;


SegmScan::MacularPlot::MacularPlot(const MacularPlot & rhs)
	: d_ptr(make_unique<MacularPlotImpl>(*rhs.d_ptr))
{
}


MacularPlot & SegmScan::MacularPlot::operator=(const MacularPlot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


CppUtil::CvImage * SegmScan::MacularPlot::getImage(void) const
{
	return &(d_ptr->image);
}


RetParam::EnfacePlot * SegmScan::MacularPlot::getPlot(void) const
{
	return &(d_ptr->plot);
}


void SegmScan::MacularPlot::setPatternOutput(PatternOutput * output)
{
	d_ptr->output = output;
	return;
}


PatternOutput * SegmScan::MacularPlot::getPatternOutput(void) const
{
	return d_ptr->output;
}


bool SegmScan::MacularPlot::update(int width, int height, LayerType upper, LayerType lower)
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	auto bscans = p->getSegmImages();
	auto uppers = p->getSegmLayers(upper);
	auto lowers = p->getSegmLayers(lower);
	auto wScan = p->getDescript()->getWidth();
	auto hScan = p->getDescript()->getHeight();

	if (getPlot()->initialize(bscans, uppers, lowers, wScan, hScan) == false) {
		return false;
	}

	return getPlot()->drawImage(getImage(), width, height);
}


Gdiplus::Bitmap * SegmScan::MacularPlot::getBitmap(void) const
{
	return getImage()->getBitmap();
}


std::vector<float> SegmScan::MacularPlot::copyData(void) const
{
	return getImage()->copyDataInFloats();
}


float SegmScan::MacularPlot::getAt(int x, int y) const
{
	return getImage()->getAtInFloat(x, y);
}


int SegmScan::MacularPlot::getWidth(void) const
{
	return getImage()->getWidth();
}


int SegmScan::MacularPlot::getHeight(void) const
{
	return getImage()->getHeight();
}


bool SegmScan::MacularPlot::isEmpty(void) const
{
	return d_ptr->image.isEmpty();
}


BscanResult * SegmScan::MacularPlot::getBscanResultFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	int index = p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
	if (index >= 0) {
		BscanResult* p = getPatternOutput()->getResult(index);
		return p;
	}
	return nullptr;
}


BscanImage * SegmScan::MacularPlot::getBscanImageFromPosition(int posX, int posY) const
{
	BscanResult* p = getBscanResultFromPosition(posX, posY);
	if (p != nullptr) {
		return p->getImage();
	}
	return nullptr;
}


int SegmScan::MacularPlot::getBscanIndexFromPosition(int posX, int posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanIndexFromPosition(posX, posY, getWidth(), getHeight());
}


bool SegmScan::MacularPlot::getBscanStartPosition(int resultIdx, int * posX, int * posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanStartPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


bool SegmScan::MacularPlot::getBscanEndPosition(int resultIdx, int * posX, int * posY) const
{
	PatternOutput* p = getPatternOutput();
	if (p == nullptr || p->getDescript()->isCubePattern() == false) {
		return false;
	}

	return p->getBscanStartPosition(resultIdx, getWidth(), getHeight(), posX, posY);
}


float SegmScan::MacularPlot::getSectionThickness(Section sect) const
{
	float thick = 0.0f;

	switch (sect) {
	case Section::CENTER:
		thick = getPlot()->getMeanCenter();
		break;
	case Section::INNER_SUPERIOR:
		thick = getPlot()->getMeanInner(0);
		break;
	case Section::INNER_INFERIOR:
		thick = getPlot()->getMeanInner(2);
		break;
	case Section::INNER_TEMPORAL:
		thick = getPlot()->getMeanInner(1);
		break;
	case Section::INNER_NASAL:
		thick = getPlot()->getMeanInner(3);
		break;
	case Section::OUTER_SUPERIOR:
		thick = getPlot()->getMeanOuter(0);
		break;
	case Section::OUTER_INFERIOR:
		thick = getPlot()->getMeanOuter(2);
		break;
	case Section::OUTER_TEMPORAL:
		thick = getPlot()->getMeanOuter(1);
		break;
	case Section::OUTER_NASAL:
		thick = getPlot()->getMeanOuter(3);
		break;
	}

	PatternOutput* p = getPatternOutput();
	if (p != nullptr) {
		thick = thick * p->getDescript()->getAxialResolution();
	}
	return thick;
}


float SegmScan::MacularPlot::getAverageThickness(Section sect) const
{
	float thick = 0.0f;

	switch (sect) {
	case Section::OVERALL:
		thick = getPlot()->getMeanCenter();
		break;
	case Section::SUPERIOR:
		thick = getPlot()->getMeanSuperior();
		break;
	case Section::INFERIOR:
		thick = getPlot()->getMeanInferior();
		break;
	}

	PatternOutput* p = getPatternOutput();
	if (p != nullptr) {
		thick = thick * p->getDescript()->getAxialResolution();
	}
	return thick;
}