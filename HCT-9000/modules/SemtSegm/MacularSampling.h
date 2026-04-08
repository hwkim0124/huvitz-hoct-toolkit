#pragma once

#include "SemtSegmDef.h"
#include "Sampling.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API MacularSampling :
		public Sampling
	{
	public:
		MacularSampling();
		virtual ~MacularSampling();

		MacularSampling(MacularSampling&& rhs);
		MacularSampling& operator=(MacularSampling&& rhs);
		MacularSampling(const MacularSampling& rhs) = delete;
		MacularSampling& operator=(const MacularSampling& rhs) = delete;

	private:
		struct MacularSamplingImpl;
		std::unique_ptr<MacularSamplingImpl> d_ptr;
		MacularSamplingImpl& getImpl(void) const;
	};
}

