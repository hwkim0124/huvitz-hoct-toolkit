#pragma once


#ifdef __GUICTRL_DLL
#define GUICTRL_DLL_API		__declspec(dllexport)
#else
#define GUICTRL_DLL_API		__declspec(dllimport)
#endif