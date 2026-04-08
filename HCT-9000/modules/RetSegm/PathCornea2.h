#pragma once


#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API PathCornea2
	{
	public:
		PathCornea2();
		~PathCornea2();

	public:
		static bool designLayerConstraintsOfInner(const SegmImage* imgSrc, const SegmImage* imgGrad,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta, bool isMeye);
		static bool designLayerConstraintsOfOuter(const SegmImage* imgSrc, const SegmImage* imgGrad,
			SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta, bool isMeye);
		static bool designLayerConstraintsOfInnerEnds(const SegmImage* imgSrc, SegmLayer* layerInn,
			int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta);
		static bool designLayerConstraintsOfOuterEnds(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
			int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta);

		static bool designLayerConstraintsOfEPI(const SegmImage* imgSrc, SegmLayer* layerInn, 
			int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta);
		static bool designLayerConstraintsOfEND(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerOut, 
			int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta);
		static bool designLayerConstraintsOfBOW(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerEND,
			int centerX, int centerY, float rangeX,
			std::vector<int> &upper, std::vector<int> &lower, std::vector<int>& delta);

		static bool createLayerCostMapOfInner(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfOuter(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfInnerEnds(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfOuterEnds(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);

		static bool createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfBOW(const SegmImage * imgSrc, SegmImage* imgCost,
			std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
	};
}

