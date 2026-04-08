#include "stdafx.h"
#include "OpticDiscSampling.h"

using namespace SemtSegm;
using namespace std;


struct OpticDiscSampling::OpticDiscSamplingImpl
{
	unique_ptr<Sampling> sampling;

	OpticDiscSamplingImpl()
	{
	}
};


OpticDiscSampling::OpticDiscSampling() :
	d_ptr(make_unique<OpticDiscSamplingImpl>())
{
}


SemtSegm::OpticDiscSampling::~OpticDiscSampling() = default;
SemtSegm::OpticDiscSampling::OpticDiscSampling(OpticDiscSampling && rhs) = default;
OpticDiscSampling & SemtSegm::OpticDiscSampling::operator=(OpticDiscSampling && rhs) = default;


OpticDiscSampling::OpticDiscSamplingImpl & SemtSegm::OpticDiscSampling::getImpl(void) const
{
	return *d_ptr;
}


