#pragma once

#include "RetSegmDef.h"

#include <vector>

namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Feature
	{
	public:
		Feature();
		virtual ~Feature();

	public:
		static bool estimateOpticDiskMargin(const SegmImage * imgSrc, const SegmImage * imgAsc, const std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& ideal, int& disc1, int& disc2, float rangeX=6.0f);
		static bool estimateOpticDiskMargin(const std::vector<int>& input, const std::vector<int>& ideal, int& disc1, int& disc2);
		static bool calculateDiscSidePixels(const std::vector<int>& inner, const std::vector<int>& outer, int& disc1, int& disc2, int& pixels);
		static bool calculateCupDepthPixels(const std::vector<int>& inner, const std::vector<int>& outer, int disc1, int disc2, int& cup1, int& cup2, int& pixels);
	};
}

