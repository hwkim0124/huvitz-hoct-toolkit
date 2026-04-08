#pragma once

#ifdef __CPPUTIL_DLL
#define CPPUTIL_DLL_API		__declspec(dllexport)
#else
#define CPPUTIL_DLL_API		__declspec(dllimport)
#endif


