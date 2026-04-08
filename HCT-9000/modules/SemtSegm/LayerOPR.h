#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerOPR : public OptimalLayer
	{
	public:
		LayerOPR(BscanSegmentator* pSegm);
		virtual ~LayerOPR();

		LayerOPR(LayerOPR&& rhs);
		LayerOPR& operator=(LayerOPR&& rhs);
		LayerOPR(const LayerOPR& rhs) = delete;
		LayerOPR& operator=(const LayerOPR& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool buildBoundaryLayer();

	protected:

	private:
		struct LayerOPRImpl;
		std::unique_ptr<LayerOPRImpl> d_ptr;
		LayerOPRImpl& getImpl(void) const;
	};
}
