#pragma once

#include "RetSegmDef.h"

#include <vector>

namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Smooth
	{
	public:
		Smooth();
		virtual ~Smooth();

	public:
		static bool smoothLayerEPI(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer* layerEPI);
		static bool smoothLayerEND(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer* layerEPI, SegmLayer* layerEND);
		static bool smoothLayerBOW(const std::vector<int>& path, int width, int height, float rangeX, SegmLayer* layerEPI, SegmLayer* layerEND, SegmLayer* layerBOW);

		static bool smoothLayerILM(const std::vector<int>& path, int width, int height, SegmLayer* layerNFL, SegmLayer* layerILM,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerNFL(const std::vector<int>& path, int width, int height, SegmLayer* layerILM, SegmLayer* layerOPL, SegmLayer* layerNFL, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerRPE(const std::vector<int>& path, int width, int height, SegmLayer* layerNFL, SegmLayer* layerBRM, SegmLayer* layerRPE, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);

		static bool smoothLayerIPL(const std::vector<int>& path, int width, int height, SegmLayer* layerNFL, SegmLayer* layerOPL, SegmLayer* layerIPL, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerOPL(const std::vector<int>& path, int width, int height, SegmLayer* layerIPL, SegmLayer* layerRPE, SegmLayer* layerOPL, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerIOS(const std::vector<int>& path, int width, int height, SegmLayer* layerOPL, SegmLayer* layerRPE, SegmLayer* layerIOS, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerBRM(const std::vector<int>& path, int width, int height, SegmLayer* layerRPE, SegmLayer* layerBRM, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothLayerBRM2(SegmLayer * layerOut, SegmLayer* layerBRM);

		static bool smoothCurveILM(const std::vector<int>& path, int width, int height, SegmLayer* layerILM, 
									bool isDisc=false, int discX1=-1, int discX2=-1);
		static bool smoothCurveOPR(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, SegmLayer* layerOPR, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);

		static bool smoothCurveIOS(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer* layerIOS,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothCurveBRM(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, SegmLayer* layerCHR,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothCurveRPE(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer* layerRPE,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);

		static bool smoothCurveOPL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer* layerOPL, 
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothCurveIPL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer* layerIPL,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);
		static bool smoothCurveNFL(const std::vector<int>& path, int width, int height, const std::vector<int>& inner, const std::vector<int>& outer, SegmLayer* layerNFL,
									bool isDisc = false, int discX1 = -1, int discX2 = -1);

		static int selectPointsNFL(const SegmImage* imgSrc, SegmLayer* layerILM, SegmLayer* layerIPL, SegmLayer* layerNFL);
	};
}


