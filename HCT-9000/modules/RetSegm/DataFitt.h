#pragma once

#include "RetSegmDef.h"

#include <vector>


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmLayer;


	class RETSEGM_DLL_API DataFitt
	{
	public:
		DataFitt();

	public:
		static bool buildIdealBoundaryOfIOS(SegmLayer& layerIOS, SegmLayer& layerOut, SegmLayer& result);
		static bool buildIdealBoundrayOfIOS(const std::vector<int>& input, std::vector<int>& output);

		static bool interpolateBoundaryByLinearFitting(const std::vector<int>& input, std::vector<int>& output, bool sideFitt = false);
		static bool smoothBoundaryLine(std::vector<int>& input, std::vector<int>& output, float filtSize);

	public:
		static bool createIdealBoundaryOfIOS(Boundary& boundIOS, Boundary& boundOut, Boundary& result);
	};
}

