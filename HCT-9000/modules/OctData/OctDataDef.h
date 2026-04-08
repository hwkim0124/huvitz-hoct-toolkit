#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;

namespace OctData {

}


#ifdef __OCTDATA_DLL
#define OCTDATA_DLL_API		__declspec(dllexport)
#else
#define OCTDATA_DLL_API		__declspec(dllimport)
#endif