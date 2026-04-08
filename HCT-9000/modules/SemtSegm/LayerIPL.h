#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerIPL : public OptimalLayer
	{
	public:
		LayerIPL(BscanSegmentator* pSegm);
		virtual ~LayerIPL();

		LayerIPL(LayerIPL&& rhs);
		LayerIPL& operator=(LayerIPL&& rhs);
		LayerIPL(const LayerIPL& rhs) = delete;
		LayerIPL& operator=(const LayerIPL& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool alterFlattenedPath();
		bool buildBoundaryLayer();

	protected:
		bool designFlattenedConstraints();
		bool designFlattenedConstraints2();
		bool designBoundaryConstraints();
		bool createFlattenedCostMap();
		bool createFlattenedCostMap2();
		bool createBoundaryCostMap();
		bool smoothFlattenedPath();
		bool smoothBoundaryPath();

	private:
		struct LayerIPLImpl;
		std::unique_ptr<LayerIPLImpl> d_ptr;
		LayerIPLImpl& getImpl(void) const;
	};
}
