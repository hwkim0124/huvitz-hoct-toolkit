#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerBRM : public OptimalLayer
	{
	public:
		LayerBRM(BscanSegmentator* pSegm);
		virtual ~LayerBRM();

		LayerBRM(LayerBRM&& rhs);
		LayerBRM& operator=(LayerBRM&& rhs);
		LayerBRM(const LayerBRM& rhs) = delete;
		LayerBRM& operator=(const LayerBRM& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool buildBoundaryLayer();

	protected:
		bool designFlattenedConstraints();
		bool createFlattenedCostMap();
		bool smoothFlattenedPath();

	private:
		struct LayerBRMImpl;
		std::unique_ptr<LayerBRMImpl> d_ptr;
		LayerBRMImpl& getImpl(void) const;

	};
}

