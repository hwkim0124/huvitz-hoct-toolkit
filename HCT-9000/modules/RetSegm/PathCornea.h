#pragma once


#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API PathCornea
	{
	public:
		PathCornea();
		~PathCornea();

	public:
		static bool designLayerContraintsOfEPI(const SegmImage* imgSrc, SegmLayer* layerInn, 
												float rangeX, int hingeY, int centerX, int centerY,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool designLayerContraintsOfEND(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
												float rangeX, int hingeY, int centerX, int centerY, int times,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool designLayerContraintsOfBOW(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
												float rangeX, int hingeY, int centerX, int centerY,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);

		static bool createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfEPI2(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfBOW(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);

		static bool makeFittingCurveOfEPI(std::vector<int>& input, std::vector<int>& output);
		static bool makeFittingCurveOfEND(std::vector<int>& input, std::vector<int>& output);
		static bool makeFittingCurveOfBOW(std::vector<int>& input, std::vector<int>& output, int& centX, int& centY);

		static bool correctReflectiveRegionOfEPI(const SegmImage * imgSrc, std::vector<int>& input, float rangeX, int hingeY, int centerX, int centerY);

		static void makeCenterRegionOfEPI(std::vector<int>& input, int& centX1, int& centX2, float rangeX = 6.0f);
		static bool replaceCenterRegionOfEPI(std::vector<int>& input, std::vector<int>& output);
	};
}

