#pragma once

#include "OctGlobalDef.h"

namespace OctGlobal
{
	constexpr int RETINA_SPLIT_CENTER_INIT_X = 320; // 322;
	constexpr int RETINA_SPLIT_CENTER_INIT_Y = 135;// 117; // 155;
	constexpr int RETINA_SPLIT_KERNAL_INIT_WIDTH = 21;
	constexpr int RETINA_SPLIT_KERNAL_INIT_HEIGHT = 24;

	constexpr int RETINA_ROI_INIT_CENTER_X = 326; // 322;
	constexpr int RETINA_ROI_INIT_CENTER_Y = 228; // 240;
	constexpr int RETINA_ROI_INIT_RADIUS = 224; // 120;
	constexpr int RETINA_CENTER_MASK_INIT_SIZE = 20;

	constexpr float RETINA_ENHANCE_CLIP_LIMIT = 1.0f; // 3.8f;
	constexpr int RETINA_ENHANCE_CLIP_SCALAR = 8; // 16;
}