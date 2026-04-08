#pragma once
#include "BscanSegmentator.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API OpticDiscLayerSegm :
		public BscanSegmentator
	{
	public:
		OpticDiscLayerSegm(LayeredBscan* bscan);
		virtual ~OpticDiscLayerSegm();

		OpticDiscLayerSegm(OpticDiscLayerSegm&& rhs);
		OpticDiscLayerSegm& operator=(OpticDiscLayerSegm&& rhs);
		OpticDiscLayerSegm(const OpticDiscLayerSegm& rhs) = delete;
		OpticDiscLayerSegm& operator=(const OpticDiscLayerSegm& rhs) = delete;

	public:
		bool doSegmentation() override;

	protected:
		Sampling* createSampling() override;

	private:
		struct OpticDiscLayerSegmImpl;
		std::unique_ptr<OpticDiscLayerSegmImpl> d_ptr;
		OpticDiscLayerSegmImpl& getImpl(void) const;
	};
}

