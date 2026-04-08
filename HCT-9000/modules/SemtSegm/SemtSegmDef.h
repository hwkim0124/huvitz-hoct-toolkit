#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace RetSegm {
	class SegmLayer;
	class SegmImage;
}

namespace SemtSegm
{
	using OcularImage = RetSegm::SegmImage;
	using OcularLayer = RetSegm::SegmLayer;

	constexpr int SAMPLE_WIDTH = 256;
	constexpr int SAMPLE_HEIGHT = (768 / 2);

	constexpr float VALID_IMAGE_MEAN_MIN = 9.0f; // 15.0f; // 25.0f;
	constexpr float VALID_IMAGE_MEAN_MAX = 75.0f;

	constexpr float VALID_PEAK_TO_MEAN_RATIO_MIN = 3.0f; // 4.5f;
	constexpr float VALID_PEAK_TO_MEAN_RATIO_MIN_CORNEA = 2.0f; // 3.0f;
	constexpr float VALID_POOR_SNR_RATIO_MIN = 0.50f; // 0.25f;
}


#ifdef __SEMTSEGM_DLL
#define SEMTSEGM_DLL_API		__declspec(dllexport)
#else
#define SEMTSEGM_DLL_API		__declspec(dllimport)
#endif