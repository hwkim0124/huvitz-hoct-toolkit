#pragma once


#include "OctGlobal2.h"

using namespace OctGlobal;


namespace RetParam {

	constexpr int ENFACE_BSCANS_MIN = 50;

	constexpr float DISC_CENTER_RADIUS = (1.0f / 2.0f);
	constexpr float DISC_INNER_RADIUS = (3.0f / 2.0f);
	constexpr float DISC_OUTER_RADIUS = (5.0f / 2.0f);

}


#ifdef __RETPARAM_DLL
#define RETPARAM_DLL_API		__declspec(dllexport)
#else
#define RETPARAM_DLL_API		__declspec(dllimport)
#endif