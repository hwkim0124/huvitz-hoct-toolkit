#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerInner : public OptimalLayer
	{
	public:
		LayerInner(BscanSegmentator* pSegm);
		virtual ~LayerInner();

		LayerInner(LayerInner&& rhs);
		LayerInner& operator=(LayerInner&& rhs);
		LayerInner(const LayerInner& rhs) = delete;
		LayerInner& operator=(const LayerInner& rhs) = delete;

	public:
		bool buildOutlinePath();
		bool buildBoundaryPath();
		bool smoothBoundaryPath();

	protected:
		bool designOutlineConstraints();
		bool designBorderConstraints();
		bool createOutlineCostMap();
		bool createBorderCostMap();

	private:
		struct LayerInnerImpl;
		std::unique_ptr<LayerInnerImpl> d_ptr;
		LayerInnerImpl& getImpl(void) const;
	};
}

