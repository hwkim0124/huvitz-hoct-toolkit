#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;

namespace OctData {

	const float FOVEA_CENTER_LINE_START = 0.20f;
	const float FOVEA_CENTER_LINE_CLOSE = 0.80f;
	const float FOVEA_CENTER_XPOS_START = 0.20f;
	const float FOVEA_CENTER_XPOS_CLOSE = 0.80f;
	const float FOVEA_CENTER_DISC_WIDTH = 0.15f;

	const float FOVEA_PERI_MARGIN_X_IN_MM = 1.0f;
	const float FOVEA_PERI_MARGIN_Y_IN_MM = 0.5f;
	const float FOVEA_PARA_INNER_RADIUS_IN_MM = 0.5f;
	const float FOVEA_PARA_OUTER_RADIUS_IN_MM = 1.5f;
	const float FOVEA_CENTER_DISC_RADIUS_IN_MM = 1.2f;
	const float FOVEA_CENTER_DISC_NERVE_LIMIT = 45.0f;

	const float FOVEA_CENTER_DISC_DIFF_RATIO_MIN = 0.25f;
	const float FOVEA_CENTER_DISC_DIFF_RATIO_MAX = 0.75f;
	const float FOVEA_DISC_DEPTH_DIFF_MAX = 2.0f;
	const float FOVEA_DISC_BOTTOM_OFFSET = 15.0f;
	const float FOVEA_DISC_BOTTOM_DIFF_MAX = 450.0f;
	const float FOVEA_DISC_AREA_DIFF_MAX = 1.5f;

	const float FOVEA_OPTIC_DISC_RANGE_X_WIDE = 11.0f;
}


#ifdef __OCTREPORT_DLL
#define OCTREPORT_DLL_API		__declspec(dllexport)
#else
#define OCTREPORT_DLL_API		__declspec(dllimport)
#endif
