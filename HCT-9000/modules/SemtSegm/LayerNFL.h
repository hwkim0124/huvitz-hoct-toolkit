#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerNFL : public OptimalLayer
	{
	public:
		LayerNFL(BscanSegmentator* pSegm);
		virtual ~LayerNFL();

		LayerNFL(LayerNFL&& rhs);
		LayerNFL& operator=(LayerNFL&& rhs);
		LayerNFL(const LayerNFL& rhs) = delete;
		LayerNFL& operator=(const LayerNFL& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool buildBoundaryLayer();

	protected:
		bool designFlattenedConstraints();
		bool designBoundaryConstraints();
		bool createFlattenedCostMap();
		bool createBoundaryCostMap();
		bool smoothFlattenedPath();
		bool smoothBoundaryPath();

	private:
		struct LayerNFLImpl;
		std::unique_ptr<LayerNFLImpl> d_ptr;
		LayerNFLImpl& getImpl(void) const;
	};
}

