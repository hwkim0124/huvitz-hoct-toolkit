#pragma once

#include "SemtSegmDef.h"
#include "Sampling.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API OpticDiscSampling :
		public Sampling
	{
	public:
		OpticDiscSampling();
		virtual ~OpticDiscSampling();

		OpticDiscSampling(OpticDiscSampling&& rhs);
		OpticDiscSampling& operator=(OpticDiscSampling&& rhs);
		OpticDiscSampling(const OpticDiscSampling& rhs) = delete;
		OpticDiscSampling& operator=(const OpticDiscSampling& rhs) = delete;

	public:

	private:
		struct OpticDiscSamplingImpl;
		std::unique_ptr<OpticDiscSamplingImpl> d_ptr;
		OpticDiscSamplingImpl& getImpl(void) const;
	};
}

