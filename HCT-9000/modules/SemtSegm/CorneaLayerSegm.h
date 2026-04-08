#pragma once
#include "BscanSegmentator.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API CorneaLayerSegm :
		public BscanSegmentator
	{
	public:
		CorneaLayerSegm(LayeredBscan* bscan);
		virtual ~CorneaLayerSegm();

		CorneaLayerSegm(CorneaLayerSegm&& rhs);
		CorneaLayerSegm& operator=(CorneaLayerSegm&& rhs);
		CorneaLayerSegm(const CorneaLayerSegm& rhs) = delete;
		CorneaLayerSegm& operator=(const CorneaLayerSegm& rhs) = delete;

	private:
		struct CorneaLayerSegmImpl;
		std::unique_ptr<CorneaLayerSegmImpl> d_ptr;
		CorneaLayerSegmImpl& getImpl(void) const;
	};
}

