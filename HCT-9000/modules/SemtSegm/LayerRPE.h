#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerRPE : public OptimalLayer
	{
	public:
		LayerRPE(BscanSegmentator* pSegm);
		virtual ~LayerRPE();

		LayerRPE(LayerRPE&& rhs);
		LayerRPE& operator=(LayerRPE&& rhs);
		LayerRPE(const LayerRPE& rhs) = delete;
		LayerRPE& operator=(const LayerRPE& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool buildBoundaryLayer();

	protected:
		bool designFlattenedConstraints();
		bool createFlattenedCostMap();
		bool smoothFlattenedPath();

	private:
		struct LayerRPEImpl;
		std::unique_ptr<LayerRPEImpl> d_ptr;
		LayerRPEImpl& getImpl(void) const;
	};
}

