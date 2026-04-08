#pragma once

#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Smooth2
	{
	public:
		Smooth2();
		~Smooth2();

	public:
		static bool smoothBoundaryINN(const std::vector<int> path, std::vector<int>& layer);
		static bool smoothBoundaryOUT(const std::vector<int> path, const std::vector<int> inner, std::vector<int>& layer);

		static bool smoothLayerEPI(const std::vector<int>& path, std::vector<int>& layer);
		static bool smoothLayerEND(const std::vector<int>& path, SegmLayer* layerEPI, std::vector<int>& layer);
		static bool smoothLayerEND2(const std::vector<int>& path, SegmLayer* layerEPI, std::vector<int>& layer);
		static bool smoothLayerBOW(const std::vector<int>& path, SegmLayer* layerEPI, SegmLayer* layerBOW, std::vector<int>& layer);
	};
}

