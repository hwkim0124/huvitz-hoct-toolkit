#include "stdafx.h"
#include "OpticDiscBscan.h"
#include "OpticDiscLayerSegm.h"

using namespace SemtSegm;
using namespace std;


struct OpticDiscBscan::OpticDiscBscanImpl
{
	// unique_ptr<OpticDiscLayerSegm> bSegm;

	OpticDiscBscanImpl()
	{
	}
};


OpticDiscBscan::OpticDiscBscan() :
	d_ptr(make_unique<OpticDiscBscanImpl>())
{
}


SemtSegm::OpticDiscBscan::~OpticDiscBscan() = default;
SemtSegm::OpticDiscBscan::OpticDiscBscan(OpticDiscBscan && rhs) = default;
OpticDiscBscan & SemtSegm::OpticDiscBscan::operator=(OpticDiscBscan && rhs) = default;


bool SemtSegm::OpticDiscBscan::performSegmentation()
{
	resetBscanSegmentator();

	if (!getBscanSegmentator()->doSegmentation()) {
		return false;
	}

	return true;
}

int SemtSegm::OpticDiscBscan::getOpticDiscX1() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticDiscX1();
}

int SemtSegm::OpticDiscBscan::getOpticDiscX2() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticDiscX2();
}

int SemtSegm::OpticDiscBscan::getOpticDiscPixels() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticDiscPixels();
}

int SemtSegm::OpticDiscBscan::getOpticCupX1() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticCupX1();
}

int SemtSegm::OpticDiscBscan::getOpticCupX2() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticCupX2();
}

int SemtSegm::OpticDiscBscan::getOpticCupPixels() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->getOpticCupPixels();
}

bool SemtSegm::OpticDiscBscan::isOpticDiscRegion() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->isOpticDiscRegion();
}

bool SemtSegm::OpticDiscBscan::isOpticCupRegion() const
{
	auto* pSegm = getBscanSegmentator();
	return pSegm->getMeasuring()->isOpticCupRegion();
}


void SemtSegm::OpticDiscBscan::resetBscanSegmentator()
{
	// std::make_unique already returns a pure rvalue.
	// getImpl().bSegm = make_unique<OpticDiscLayerSegm>(this);
	setBscanSegmentator(new OpticDiscLayerSegm(this));
	return;
}


OpticDiscBscan::OpticDiscBscanImpl & SemtSegm::OpticDiscBscan::getImpl(void) const
{
	return *d_ptr;
}

