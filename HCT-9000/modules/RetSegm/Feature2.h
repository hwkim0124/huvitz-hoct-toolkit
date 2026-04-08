#pragma once

#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Feature2
	{
	public:
		Feature2();
		virtual ~Feature2();

	public:
		static bool detectBoundaryPointsRegion(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int& regX1, int& regX2);
		static bool detectOpticDiscRegion(const std::vector<int>& outer, float rangeX, int& discX1, int& discX2);
		static bool estimateOpticDiscMargin(const SegmImage * ascImg, const std::vector<int>& inner, std::vector<int>& outer, float rangeX, int& discX1, int& discX2);
		static bool estimateOpticDiscBounds(const SegmImage * ascImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX, int& discX1, int& discX2);
		static bool adjustOpticDiscBounds(const std::vector<int>& inner, std::vector<int>& outer, float rangeX, int& discX1, int& discX2);

		static bool makeupIdealFittingBoundary(const std::vector<int>& line, std::vector<int>& ideal, float fittSize = 0.10f);
		static bool makeupIdealOuterBoundary(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int discX1, int discX2, std::vector<int>& ideal);
		static bool makeupIdealOuterBoundary2(const std::vector<int>& outer, float rangeX, std::vector<int>& ideal);

		static bool makeupIdealInnerBoundary(const std::vector<int>& inner, const std::vector<int>& outer, float rangeX, int discX1, int discX2, std::vector<int>& ideal);
	};
}

