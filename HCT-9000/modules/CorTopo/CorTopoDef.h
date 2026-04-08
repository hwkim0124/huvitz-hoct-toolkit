#pragma once


#include "OctGlobal2.h"

using namespace OctGlobal;

#include <memory>
#include <string>


namespace CppUtil {
	class CvImage;
}


namespace CorTopo
{
	constexpr int SAMPLE_IMAGE_WIDTH = 256;
	constexpr int SAMPLE_IMAGE_HEIGHT = (768 / 2);

	/*constexpr float REFRACTIVE_INDEX_MEASURE = 1.3425f; 2023.02.17 by PL*/
	constexpr float REFRACTIVE_INDEX_MEASURE = 1.3375f;

	constexpr float VALID_PREVIEW_SNR_MIN = 1.5f;

	constexpr float WIDE_SCAN_RANGE_MIN = 11.0f;

	constexpr int TARGET_REFER_POST_INIT = 256;
};


#ifdef __CORTOPO_DLL
#define CORTOPO_DLL_API		__declspec(dllexport)
#else
#define CORTOPO_DLL_API		__declspec(dllimport)
#endif