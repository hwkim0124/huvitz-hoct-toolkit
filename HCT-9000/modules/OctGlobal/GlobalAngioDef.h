#pragma once

#include "OctGlobalDef.h"

#include <cstdint>
#include <string>
#include <functional>


namespace OctGlobal
{
	constexpr int ANGIO_GABOR_FILTER_ORIENTS = 12;
	constexpr float ANGIO_GABOR_FILTER_SIGMA = 3.0f;
	constexpr float ANGIO_GABOR_FILTER_DIVIDER = 1.5f;
	constexpr float ANGIO_GABOR_FILTER_WEIGHT = 0.5f; // 0.35f; // 0.25f;

	constexpr float ANGIO_ENHANCE_PARAM = 1.0f;
	constexpr float ANGIO_BIAS_FIELD_SIGMA = 15.0f;

	constexpr float ANGIO_MOTION_THRESHOLD = 1.5f;
	constexpr float ANGIO_MOTION_OVER_POINTS = 0.5f;
	constexpr float ANGIO_MOTION_DIST_RATIO = 1.15f;

	constexpr int ANGIO_AVERAGE_OFFSET = 3;
};