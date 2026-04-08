#pragma once


namespace OctGlobal {


}


#ifdef __OCTGLOBAL_DLL
#define OCTGLOBAL_DLL_API		__declspec(dllexport)
#else
#define OCTGLOBAL_DLL_API		__declspec(dllimport)
#endif