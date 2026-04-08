#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerOPL : public OptimalLayer
	{
	public:
		LayerOPL(BscanSegmentator* pSegm);
		virtual ~LayerOPL();

		LayerOPL(LayerOPL&& rhs);
		LayerOPL& operator=(LayerOPL&& rhs);
		LayerOPL(const LayerOPL& rhs) = delete;
		LayerOPL& operator=(const LayerOPL& rhs) = delete;

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
		struct LayerOPLImpl;
		std::unique_ptr<LayerOPLImpl> d_ptr;
		LayerOPLImpl& getImpl(void) const;
	};
}

