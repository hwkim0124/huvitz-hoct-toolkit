#include "stdafx.h"
#include "MacularLayerSegm.h"

using namespace SemtSegm;
using namespace std;


struct MacularLayerSegm::MacularLayerSegmImpl
{
	MacularLayerSegmImpl()
	{
	}
};


MacularLayerSegm::MacularLayerSegm(LayeredBscan* bscan) :
	d_ptr(make_unique<MacularLayerSegmImpl>()), BscanSegmentator(bscan)
{
}


SemtSegm::MacularLayerSegm::~MacularLayerSegm() = default;
SemtSegm::MacularLayerSegm::MacularLayerSegm(MacularLayerSegm && rhs) = default;
MacularLayerSegm & SemtSegm::MacularLayerSegm::operator=(MacularLayerSegm && rhs) = default;


MacularLayerSegm::MacularLayerSegmImpl & SemtSegm::MacularLayerSegm::getImpl(void) const
{
	return *d_ptr;
}

