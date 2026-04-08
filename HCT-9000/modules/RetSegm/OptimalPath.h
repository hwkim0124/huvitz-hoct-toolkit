#pragma once

#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API OptimalPath
	{
	public:
		OptimalPath();

	public:
		static bool designLayerContraintsOfEPI(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool designLayerContraintsOfEND(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);

		static bool designLayerContraintsOfILM(const SegmImage* imgSrc, SegmLayer* layerILM, SegmLayer* layerNFL, 
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designLayerContraintsOfNFL(const SegmImage* imgSrc, SegmLayer* layerILM, SegmLayer* layerNFL, SegmLayer* layerIPL, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designLayerContraintsOfRPE(const SegmImage* imgSrc, SegmLayer* layerIOS, SegmLayer* layerBRM, SegmLayer* layerRPE, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc = false, int disc1 = -1, int disc2 = -1);

		static bool designLayerContraintsOfIPL(const SegmImage* imgSrc, SegmLayer* layerNFL, SegmLayer* layerIPL, SegmLayer* layerOPL, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designLayerContraintsOfOPL(const SegmImage* imgSrc, SegmLayer* layerIPL, SegmLayer* layerOPL, SegmLayer* layerRPE, SegmLayer* layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);

		static bool designLayerContraintsOfIOS(const SegmImage* imgSrc, SegmLayer* layerOPL, SegmLayer* layerIOS, SegmLayer* layerRPE, SegmLayer* layerBRM,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designLayerContraintsOfBRM(const SegmImage* imgSrc, SegmLayer* layerIOS, SegmLayer* layerRPE, SegmLayer* layerBRM,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);

		static bool createLayerCostMapOfEPI(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createLayerCostMapOfEND(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);


		static bool createLayerCostMapOfILM(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isSample = true);
		static bool createLayerCostMapOfNFL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);
		static bool createLayerCostMapOfIPL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);
		static bool createLayerCostMapOfOPL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);
		static bool createLayerCostMapOfRPE(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);
		static bool createLayerCostMapOfIOS(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);
		static bool createLayerCostMapOfBRM(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, bool isDisk = false);


		static bool designPathConstraintsOfILM(const SegmImage * imgSrc, SegmLayer * layerInn, SegmLayer * layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												int reti1, int reti2,
												bool isDisk = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfOut(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfOPR(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc=false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfIOS(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOut, SegmLayer * layerOPR,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfBRM(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer * layerOPR,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfRPE(const SegmImage * imgSrc, SegmLayer * layerIOS, SegmLayer * layerOPR, SegmLayer * layerBRM,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);

		static bool designPathConstraintsOfOPL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIOS,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfIPL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerOPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfNFL(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfNFL2(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool designPathConstraintsOfNFL3(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false, int disc1 = -1, int disc2 = -1, bool retry = false);
		static bool designPathConstraintsOfNFL4(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, float rangeX,
												bool isDisc = false, int disc1 = -1, int disc2 = -1, bool retry = false);
		static bool designPathConstraintsOfNFL5(const SegmImage * imgSrc, SegmLayer * layerILM, SegmLayer * layerIPL,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, float rangeX,
												bool isDisc = false, int disc1 = -1, int disc2 = -1, bool retry = false);

		static bool createPathCostMapOfILM(const SegmImage * imgSrc, SegmImage* imgCost, 
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
												bool isDisc = false);
		static bool createPathCostMapOfOut(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createPathCostMapOfIOS(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createPathCostMapOfRPE(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createPathCostMapOfOPR(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);
		static bool createPathCostMapOfBRM(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta);

		static bool createPathCostMapOfOPL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool createPathCostMapOfIPL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc = false, int disc1 = -1, int disc2 = -1);
		static bool createPathCostMapOfNFL(const SegmImage * imgSrc, SegmImage* imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, 
												bool isDisc = false, int disc1 = -1, int disc2 = -1);

		static bool searchPathWithMinCost(const SegmImage * imgCost,
												std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta, std::vector<int>& output);

	public:
		static bool findBoundaryToILM(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut, SegmImage* imgCost, SegmLayer* layerILM);
		static bool findBoundaryToIOS(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut, SegmImage* imgCost, SegmLayer* layerIOS);

	private:
		static bool imposePathConstraintsForILM(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
													std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
													int upperRange, int lowerRange, int moveSpan);
		static bool imposePathConstraintsForIOS(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut,
													std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& delta,
													int upperRange, int lowerRange, int moveSpan);

		static bool createPathCostMap(const SegmImage* srcImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta,
													float thresh1, float thresh2, int edgeSpan, bool ascent, SegmImage* costImg);
		static bool searchPathMinCost(const SegmImage* costImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, SegmLayer* layerOut);

	public:
		static bool findBoundaryILM(const ImageMat& srcImg, Boundary& innerBound, Boundary& outerBound, ImageMat& costImg, Boundary& result);
		static bool findBoundaryIOS(const ImageMat& srcImg, Boundary& initUpper, Boundary& initLower, ImageMat& costImg, Boundary& result);
		static bool findBoundaryToRPE(const ImageMat& srcImg, Boundary& innerBound, ImageMat& costImg, Boundary& boundRPE);

	private:
		static bool imposePathConstraintsForILM(const ImageMat& srcImg, Boundary& innerBound, Boundary& outerBound, 
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& span,
														int upperRange, int lowerRange, int moveSpan);
		static bool imposePathConstraintsForIOS(const ImageMat& srcImg, Boundary& innerBound, Boundary& outerBound,
														std::vector<int>& upper, std::vector<int>& lower, std::vector<int>& span,
														int upperRange, int lowerRange, int moveSpan);


		static std::vector<int> makePathChangeContraints(const Boundary& inBound, float factor=1.0f);
		static std::vector<int> adjustPathUpperBound(const ImageMat& srcImg, Boundary& inBound, int span);
		static std::vector<int> adjustPathLowerBound(const ImageMat& srcImg, Boundary& inBound, int span);

		static bool createPathCostMap(const ImageMat& srcImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, 
										float thresh1, float thresh2, int edgeSpan, bool ascent, ImageMat& costImg);
		static bool searchPathMinCost(const ImageMat& costImg, std::vector<int> upper, std::vector<int> lower, std::vector<int> delta, Boundary& outBound);
	};
}

