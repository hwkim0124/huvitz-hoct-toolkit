#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace OctResult {

	constexpr int SCAN_ENFACE_IMAGE_WIDTH = 256;
	constexpr int SCAN_ENFACE_IMAGE_HEIGHT = 256;

	constexpr float SCAN_ENFACE_CLIP_LIMIT = 2.0f; // 4.0f;

	constexpr float FUNDUS_THUMBNAIL_SIZE_RATIO = 0.05f;

	const std::wstring kImageExt = L"jpg";
	const std::wstring kImageExtOld = L"png";
	const std::wstring kBScanImageExt = L"jpg"; // 1.2.0 웹뷰어에서 대응할 시간이 없음. bmp 로 변경해야 함.
	// const std::wstring kBScanImageExt = L"png"; // 1.2.0 웹뷰어에서 대응할 시간이 없음. bmp 로 변경해야 함.
}


#ifdef __OCTRESULT_DLL
#define OCTRESULT_DLL_API		__declspec(dllexport)
#else
#define OCTRESULT_DLL_API		__declspec(dllimport)
#endif