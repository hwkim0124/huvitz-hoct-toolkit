#include "stdafx.h"
#include "LayeredBscan.h"
#include "BscanSegmentator.h"

using namespace SemtSegm;
using namespace std;


struct LayeredBscan::LayeredBscanImpl
{
	OctScanImage image;
	unique_ptr<BscanSegmentator> bSegm;

	LayeredBscanImpl()
	{
	}
};


LayeredBscan::LayeredBscan() :
	d_ptr(make_unique<LayeredBscanImpl>())
{
}


SemtSegm::LayeredBscan::~LayeredBscan() = default;
SemtSegm::LayeredBscan::LayeredBscan(LayeredBscan && rhs) = default;
LayeredBscan & SemtSegm::LayeredBscan::operator=(LayeredBscan && rhs) = default;


void SemtSegm::LayeredBscan::setImageSource(const unsigned char * bits, int width, int height, float range, int index)
{
	getImpl().image.setup(bits, width, height);
	getImpl().image.setRangeX(range);
	getImpl().image.setIndex(index);
}

void SemtSegm::LayeredBscan::setImageSource(OctScanImage image)
{
	getImpl().image = image;
}

void SemtSegm::LayeredBscan::setImageIndex(int index)
{
	getImpl().image.setIndex(index);
}


bool SemtSegm::LayeredBscan::isImageSource(void) const
{
	return !getImpl().image.isEmpty();
}

int SemtSegm::LayeredBscan::getImageIndex(void) const
{
	return getImpl().image.getIndex();
}

float SemtSegm::LayeredBscan::getImageRangeX(void) const
{
	return getImpl().image.getRangeX();
}

const OctScanImage & SemtSegm::LayeredBscan::getImageSource(void) const
{
	return getImpl().image;
}

const OcularImage * SemtSegm::LayeredBscan::getImageSample(void) const
{
	return getBscanSegmentator()->getSampling()->sample();
}

const OcularImage * SemtSegm::LayeredBscan::getImageAscent(void) const
{
	return getBscanSegmentator()->getSampling()->ascent();
}

const OcularImage * SemtSegm::LayeredBscan::getImageDescent(void) const
{
	return getBscanSegmentator()->getSampling()->descent();
}

const OcularImage * SemtSegm::LayeredBscan::getImageAverage(void) const
{
	return getBscanSegmentator()->getSampling()->average();;
}

const OcularImage * SemtSegm::LayeredBscan::getInputFlattened(void) const
{
	return getBscanSegmentator()->getClassifier()->getInputImage();
}

const OcularImage * SemtSegm::LayeredBscan::getInputDenoised(void) const
{
	return getBscanSegmentator()->getClassifier()->getInputDenoised();
}

const std::vector<int>& SemtSegm::LayeredBscan::getInnerEdges() const
{
	return getBscanSegmentator()->getBodering()->getInnerEdges();
}

const std::vector<int>& SemtSegm::LayeredBscan::getOuterEdges() const
{
	return getBscanSegmentator()->getBodering()->getOuterEdges();
}

const std::vector<int>& SemtSegm::LayeredBscan::getInnerBound() const
{
	return getBscanSegmentator()->getBodering()->getInnerBound();
}

const std::vector<int>& SemtSegm::LayeredBscan::getOuterBound() const
{
	return getBscanSegmentator()->getBodering()->getOuterBound();
}

const std::vector<int>& SemtSegm::LayeredBscan::getCenterBound() const
{
	return getBscanSegmentator()->getBodering()->getCenterBound();
}

const std::vector<int>& SemtSegm::LayeredBscan::getInnerBorder() const
{
	return getBscanSegmentator()->getBodering()->getInnerBorder();
}

const std::vector<int>& SemtSegm::LayeredBscan::getOuterBorder() const
{
	return getBscanSegmentator()->getBodering()->getOuterBorder();
}

const std::vector<int>& SemtSegm::LayeredBscan::getInnerLayer() const
{
	return getBscanSegmentator()->getLayerInner()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getOuterLayer() const
{
	return getBscanSegmentator()->getLayerOuter()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryILM() const
{
	return getBscanSegmentator()->getLayerILM()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryNFL() const
{
	return getBscanSegmentator()->getLayerNFL()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryOPL() const
{
	return getBscanSegmentator()->getLayerOPL()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryIPL() const
{
	return getBscanSegmentator()->getLayerIPL()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryIOS() const
{
	return getBscanSegmentator()->getLayerIOS()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryRPE() const
{
	return getBscanSegmentator()->getLayerRPE()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryBRM() const
{
	return getBscanSegmentator()->getLayerBRM()->getOptimalPath();
}

const std::vector<int>& SemtSegm::LayeredBscan::getBoundaryOPR() const
{
	return getBscanSegmentator()->getLayerOPR()->getOptimalPath();
}

void SemtSegm::LayeredBscan::setBscanSegmentator(BscanSegmentator * segm)
{
	getImpl().bSegm = unique_ptr<BscanSegmentator>(segm);
}

BscanSegmentator * SemtSegm::LayeredBscan::getBscanSegmentator() const
{
	return getImpl().bSegm.get();
}


LayeredBscan::LayeredBscanImpl & SemtSegm::LayeredBscan::getImpl(void) const
{
	return *d_ptr;
}


