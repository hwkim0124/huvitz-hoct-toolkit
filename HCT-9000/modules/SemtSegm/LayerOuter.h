#pragma once


#include "SemtSegmDef.h"
#include "OptimalLayer.h"


namespace SemtSegm
{
	class SEMTSEGM_DLL_API LayerOuter : public OptimalLayer
	{
	public:
		LayerOuter(BscanSegmentator* pSegm);
		virtual ~LayerOuter();

		LayerOuter(LayerOuter&& rhs);
		LayerOuter& operator=(LayerOuter&& rhs);
		LayerOuter(const LayerOuter& rhs) = delete;
		LayerOuter& operator=(const LayerOuter& rhs) = delete;

	public:
		bool buildOutlinePath();
		bool buildBoundaryPath();
		bool alterBoundaryPath(bool withDisc);

		bool smoothOutlnePath();
		bool smoothBorderPath();
		bool smoothBorderPathWithDisc();

	protected:
		bool designOutlineConstraints();
		bool designBorderConstraints();
		bool designBorderConstraints2();

		bool createOutlineCostMap();
		bool createBorderCostMap();

		bool makeupOutlineInterpolation(std::vector<int>& result) ;

	private:
		struct LayerOuterImpl;
		std::unique_ptr<LayerOuterImpl> d_ptr;
		LayerOuterImpl& getImpl(void) const;
	};
}
