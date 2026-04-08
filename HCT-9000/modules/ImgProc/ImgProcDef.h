#pragma once

#include "OctGlobal2.h"
#include "CppUtil2.h"

using namespace OctGlobal;
using namespace CppUtil;


namespace ImgProc
{
	constexpr int PIXEL_CORRECT_IMAGES_NUM = 5;
};


#ifdef __IMGPROC_DLL
#define IMGPROC_DLL_API		__declspec(dllexport)
#else
#define IMGPROC_DLL_API		__declspec(dllimport)
#endif