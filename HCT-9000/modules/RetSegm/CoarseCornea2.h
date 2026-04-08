#pragma once

#include "RetSegmDef.h"

#include <vector>

namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API CoarseCornea2
	{
	public:
		CoarseCornea2();
		~CoarseCornea2();

	public:
		static bool createGradientMaps(const SegmImage* imgSrc, SegmImage* imgAsc, 
			SegmImage* imgDes = nullptr, SegmImage* imgOut = nullptr, bool isMeye = false);
		static bool checkIfModelEyeImage(const SegmImage* imgSrc);
		static bool makeupCorneaBorderLines(const SegmImage* imgSrc, SegmImage* imgAsc, SegmImage* imgDes,
			SegmLayer* layerInn, SegmLayer* layerOut, int& centerX, int& centerY, float rangeX, bool isMeye);

		static bool correctCenterReflection(const SegmImage* imgSrc, std::vector<int>& layer, int centerX, int centerY, float rangeX);
		static bool correctCenterReflectionForTopo(const SegmImage* imgSrc, std::vector<int>& layer, int centerX, int centerY, float rangeX);
		static bool correctCenterReflectionOfEND(const SegmImage* imgSrc, std::vector<int>& layer, int centerX, int centerY, float rangeX);

		static bool locateCenterReflection(const SegmImage* imgSrc, int centerX, int centerY, float rangeX, int& reflX1, int& reflX2);
		static bool locateCenterReflectionForTopo(const SegmImage* imgSrc, int centerX, int centerY, float rangeX, int& reflX1, int& reflX2);
		static bool locateCenterReflectionOfEND(const SegmImage* imgSrc, int centerX, int centerY, float rangeX, int& reflX1, int& reflX2);

		static bool correctSideRobeEnds(const SegmImage* imgSrc, const std::vector<int>& path, std::vector<int>& layer, 
			int centerX, int centerY, float rangeX);

		static bool correctSurfaceReflection(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerBow, int centerX, int centerY, float rangeX);
		static bool correctSurfaceReflectionForTopo(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerBow, int centerX, int centerY, float rangeX);

		static bool segmentInnerBoundary(const SegmImage* imgSrc, const SegmImage* imgGrad,
			SegmLayer* layerInn, SegmLayer* layerOut, float rangeX, bool isMeye);
		static bool segmentOuterBoundary(const SegmImage* imgSrc, const SegmImage* imgGrad,
			SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX, bool isMeye);
		static bool segmentInnerEnds(const SegmImage* imgSrc, const SegmImage* imgGrad, 
			SegmLayer* layerInn, int centerX, int centerY, float rangeX);
		static bool segmentOuterEnds(const SegmImage* imgSrc, const SegmImage* imgGrad, 
			SegmLayer* layerInn, SegmLayer* layerOut, int centerX, int centerY, float rangeX);

		static bool elaborateEPI(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerEPI, 
			int centerX, int centerY, float rangeX);
		static bool elaborateEND(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerOut,
			SegmLayer* layerEND, int centerX, int centerY, float rangeX);
		static bool elaborateBOW(const SegmImage* imgSrc, SegmLayer* layerEPI, SegmLayer* layerEND,
			SegmLayer* layerBOW, int centerX, int centerY, float rangeX);

		static bool makeFittingCurveOfBOW(std::vector<int>& input, std::vector<int>& output, int centerX, int centerY, float rangeX);

		static bool estimateInnerCoreCurve(SegmLayer* layerInn, std::vector<int>& curve, int& centerX, int& centerY, float rangeX);
		static bool estimateInnerFullCurve(SegmLayer* layerInn, std::vector<int>& curve, int& centerX, int& centerY, float rangeX);
		static bool estimateOuterCoreCurve(SegmLayer* layerOut, std::vector<int>& curve, int centerX, int centerY, float rangeX);

		static bool correctInnerPeripherals(SegmLayer* layerInn, const std::vector<int> curve, int centerX, int centerY, float rangeX);
		static bool correctOuterPeripherals(SegmLayer* layerOut, const std::vector<int> curve, int centerX, int centerY, float rangeX);
	};
}

