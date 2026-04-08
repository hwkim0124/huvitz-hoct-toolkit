#include "stdafx.h"
#include "CorneaLayerSegm.h"

using namespace SemtSegm;
using namespace std;


struct CorneaLayerSegm::CorneaLayerSegmImpl
{
	CorneaLayerSegmImpl()
	{
	}
};


CorneaLayerSegm::CorneaLayerSegm(LayeredBscan* bscan) :
	d_ptr(make_unique<CorneaLayerSegmImpl>()), BscanSegmentator(bscan)
{
}


SemtSegm::CorneaLayerSegm::~CorneaLayerSegm() = default;
SemtSegm::CorneaLayerSegm::CorneaLayerSegm(CorneaLayerSegm && rhs) = default;
CorneaLayerSegm & SemtSegm::CorneaLayerSegm::operator=(CorneaLayerSegm && rhs) = default;


CorneaLayerSegm::CorneaLayerSegmImpl & SemtSegm::CorneaLayerSegm::getImpl(void) const
{
	return *d_ptr;
}

