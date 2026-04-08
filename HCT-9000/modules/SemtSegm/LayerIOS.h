#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerIOS : public OptimalLayer
	{
	public:
		LayerIOS(BscanSegmentator* pSegm);
		virtual ~LayerIOS();

		LayerIOS(LayerIOS&& rhs);
		LayerIOS& operator=(LayerIOS&& rhs);
		LayerIOS(const LayerIOS& rhs) = delete;
		LayerIOS& operator=(const LayerIOS& rhs) = delete;

	public:
		bool buildFlattenedPath();
		bool alterFlattenedPath();
		bool buildBoundaryLayer();

	protected:
		bool designFlattenedConstraints();
		bool designFlattenedConstraints2();
		bool createFlattenedCostMap();
		bool smoothFlattenedPath();

	private:
		struct LayerIOSImpl;
		std::unique_ptr<LayerIOSImpl> d_ptr;
		LayerIOSImpl& getImpl(void) const;
	};
}

