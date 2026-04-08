#include "stdafx.h"
#include "MacularSampling.h"

using namespace SemtSegm;
using namespace std;


struct MacularSampling::MacularSamplingImpl
{
	unique_ptr<Sampling> sampling;

	MacularSamplingImpl()
	{
	}
};


MacularSampling::MacularSampling() :
	d_ptr(make_unique<MacularSamplingImpl>())
{
}


SemtSegm::MacularSampling::~MacularSampling() = default;
SemtSegm::MacularSampling::MacularSampling(MacularSampling && rhs) = default;
MacularSampling & SemtSegm::MacularSampling::operator=(MacularSampling && rhs) = default;


MacularSampling::MacularSamplingImpl & SemtSegm::MacularSampling::getImpl(void) const
{
	return *d_ptr;
}


