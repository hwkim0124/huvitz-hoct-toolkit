#pragma once

#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerONL : public OptimalLayer
	{
	public:
		LayerONL(BscanSegmentator* pSegm);
		virtual ~LayerONL();

		LayerONL(LayerONL&& rhs);
		LayerONL& operator=(LayerONL&& rhs);
		LayerONL(const LayerONL& rhs) = delete;
		LayerONL& operator=(const LayerONL& rhs) = delete;

	public:
		bool buildOutlinePath();
		bool smoothOutlnePath();
		bool filterOuterPoints();

	protected:
		bool designOutlineConstraints();
		bool createOutlineCostMap();

	private:
		struct LayerONLImpl;
		std::unique_ptr<LayerONLImpl> d_ptr;
		LayerONLImpl& getImpl(void) const;
	};
}

