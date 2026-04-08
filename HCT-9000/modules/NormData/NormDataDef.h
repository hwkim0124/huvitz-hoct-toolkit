#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace NormData
{

}


#ifdef __NORMDATA_DLL
#define NORMDATA_DLL_API		__declspec(dllexport)
#else
#define NORMDATA_DLL_API		__declspec(dllimport)
#endif