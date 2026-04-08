#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerILM : public OptimalLayer
	{
	public:
		LayerILM(BscanSegmentator* pSegm);
		virtual ~LayerILM();

		LayerILM(LayerILM&& rhs);
		LayerILM& operator=(LayerILM&& rhs);
		LayerILM(const LayerILM& rhs) = delete;
		LayerILM& operator=(const LayerILM& rhs) = delete;

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
		struct LayerILMImpl;
		std::unique_ptr<LayerILMImpl> d_ptr;
		LayerILMImpl& getImpl(void) const;
	};
}

