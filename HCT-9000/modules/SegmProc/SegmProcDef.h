#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;

namespace SegmProc
{

};


#ifdef __SEGMPROC_DLL
#define SEGMPROC_DLL_API		__declspec(dllexport)
#else
#define SEGMPROC_DLL_API		__declspec(dllimport)
#endif
