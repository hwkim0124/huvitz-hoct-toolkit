#include "stdafx.h"
#include "LayerOPR.h"
#include "BscanSegmentator.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace SemtSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;

#include <algorithm>


struct LayerOPR::LayerOPRImpl
{
	LayerOPRImpl()
	{
	}
};


LayerOPR::LayerOPR(BscanSegmentator* pSegm) :
	d_ptr(make_unique<LayerOPRImpl>()), OptimalLayer(pSegm)
{
}


SemtSegm::LayerOPR::~LayerOPR() = default;
SemtSegm::LayerOPR::LayerOPR(LayerOPR && rhs) = default;
LayerOPR & SemtSegm::LayerOPR::operator=(LayerOPR && rhs) = default;


bool SemtSegm::LayerOPR::buildFlattenedPath()
{
	auto* pSegm = getSegmentator();

	const auto& layer = pSegm->getLayerBRM()->getOptimalPath();

	getOptimalPath() = layer;
	return true;
}

bool SemtSegm::LayerOPR::buildBoundaryLayer()
{
	auto* pSegm = getSegmentator();
	auto* pSample = pSegm->getSampling();

	int width = pSample->getSourceWidth();
	int height = pSample->getSourceHeight();
	float rangeX = pSegm->getImageRangeX();

	const int FILTER_SIZE = 13;
	const int CURVE_DEGREE = 1;

	auto& path = getOptimalPath();
	int size = (int)path.size();
	if (size <= 0) {
		return false;
	}

	const auto& inner = pSegm->getLayerRPE()->getOptimalPath();

	float scale = (1.0f / pSample->getSampleWidthRatio());
	int filter = (int)(scale * FILTER_SIZE);
	auto outs = path;

	outs = CppUtil::SgFilter::smoothInts(outs, filter, CURVE_DEGREE);
	std::transform(cbegin(outs), cend(outs), cbegin(inner), begin(outs), [=](int elem1, int elem2) { return max(elem1, elem2); });
	transform(begin(outs), end(outs), begin(outs), [=](int elm) { return min(max(elm, 0), height - 1); });

	path = outs;
	return true;
}

LayerOPR::LayerOPRImpl & SemtSegm::LayerOPR::getImpl(void) const
{
	return *d_ptr;
}
