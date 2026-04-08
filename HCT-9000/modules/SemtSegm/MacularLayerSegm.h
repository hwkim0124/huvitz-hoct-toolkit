#pragma once
#include "BscanSegmentator.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API MacularLayerSegm :
		public BscanSegmentator
	{
	public:
		MacularLayerSegm(LayeredBscan* bscan);
		virtual ~MacularLayerSegm();

		MacularLayerSegm(MacularLayerSegm&& rhs);
		MacularLayerSegm& operator=(MacularLayerSegm&& rhs);
		MacularLayerSegm(const MacularLayerSegm& rhs) = delete;
		MacularLayerSegm& operator=(const MacularLayerSegm& rhs) = delete;

	private:
		struct MacularLayerSegmImpl;
		std::unique_ptr<MacularLayerSegmImpl> d_ptr;
		MacularLayerSegmImpl& getImpl(void) const;
	};
}

