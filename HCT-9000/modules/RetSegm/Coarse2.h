#pragma once

#include "RetSegmDef.h"


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Coarse2
	{
	public:
		Coarse2();
		~Coarse2();

	public:
		static bool makeBoundaryLinesOfRetina(const SegmImage* imgSrc, const SegmImage* imgAsc, const SegmImage* imgDes, SegmLayer* layerInn, SegmLayer* layerOut, float rangeX, bool isDisc, int& discX1, int& discX2, int& retiX1, int& retiX2);
	
	private:
		static bool locateBoundaryEdgesOfRetina(const SegmImage * srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer);
		static bool locateBoundaryEdgesOfRetina2(const SegmImage * srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer, bool isDisc);
		static void removeBoundaryInnerOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& result);
		static void removeBoundaryOuterOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& result);
		static void correctFalseInnerPoints(const SegmImage * srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer);

		static bool replaceFalseBoundaryPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& ideal, float rangeX, bool isDisc);
		static bool expandOuterBoundaryPoints(const SegmImage * ascImg, const std::vector<int>& inner, const std::vector<int>& outer, const std::vector<int>& ideal, std::vector<int>& result1, std::vector<int>& result2);

		static bool checkBoundaryPointsIsValid(const std::vector<int>& points, float minSize = 0.5f);
		static void locateOuterBoundaryOfBackside(const SegmImage* srcImg, const SegmImage* desImg, std::vector<int>& inner, std::vector<int>& outer);
	};
}

