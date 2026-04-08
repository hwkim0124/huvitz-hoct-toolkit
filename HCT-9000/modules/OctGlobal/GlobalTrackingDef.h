#pragma once

#include "OctGlobalDef.h"

namespace OctGlobal
{
    // Retina Tracking
    //////////////////////////////////////////////////////////////////////////////////////
    constexpr int RETINA_TRACK_FRAME_WIDTH = 644;
    constexpr int RETINA_TRACK_FRAME_HEIGHT = 480; 
    constexpr int RETINA_TRACK_FRAME_CENT_X = RETINA_TRACK_FRAME_WIDTH / 2;
    constexpr int RETINA_TRACK_FRAME_CENT_Y = RETINA_TRACK_FRAME_HEIGHT / 2;
    constexpr float RETINA_TRACK_FRAME_PIXEL_RESOL_IN_MM = 0.03f;

    constexpr int RETINA_TRACK_PATCH_WIDTH = 120; // 100;
    constexpr int RETINA_TRACK_PATCH_HEIGHT = 160; 
    constexpr int RETINA_TRACK_PATCH_X_OFFSET = 60;
    constexpr int RETINA_TRACK_PATCH_Y_OFFSET = (RETINA_TRACK_PATCH_HEIGHT/2);

    constexpr int RETINA_TRACK_ROI_WIDTH = 160;
	constexpr int RETINA_TRACK_ROI_WIDTH_DISC = 240;
    constexpr int RETINA_TRACK_ROI_HEIGHT = 200; 
	constexpr int RETINA_TRACK_ROI_X_OFFSET = 35; // 30; // 20;
    constexpr int RETINA_TRACK_ROI_Y_OFFSET = (RETINA_TRACK_ROI_HEIGHT/2);

    constexpr int RETINA_TRACK_BBOX_WIDTH = 111;
	constexpr int RETINA_TRACK_BBOX_HEIGHT = 111; // 121; // 110;
    constexpr int RETINA_TRACK_OBJECT_MEAN_MIN = 60; //  75;
    constexpr int RETINA_TRACK_OBJECT_MEAN_MAX = 180;
    constexpr int RETINA_TRACK_OBJECT_STDEV_MIN = 15;

    constexpr int RETINA_TRACK_UPSIDE_X_SIZE = 200;
    constexpr int RETINA_TRACK_UPSIDE_Y_SIZE = 120;
    constexpr int RETINA_TRACK_UPSIDE_X_OFFSET = 0;
    constexpr int RETINA_TRACK_UPSIDE_Y_OFFSET = 60;

    constexpr int RETINA_TRACK_DOWNSIDE_X_SIZE = 200;
    constexpr int RETINA_TRACK_DOWNSIDE_Y_SIZE = 120;
    constexpr int RETINA_TRACK_DOWNSIDE_X_OFFSET = 0;
    constexpr int RETINA_TRACK_DOWNSIDE_Y_OFFSET = 60;

    constexpr float RETINA_TRACK_UPSIDE_COVERED_MEAN = 200;
    constexpr float RETINA_TRACK_DOWNSIDE_COVERED_MEAN = 200;

    constexpr int RETINA_TRACK_FRAME_LEVEL_DARK = 60;
    constexpr int RETINA_TRACK_FRAME_LEVEL_LOW = 85;
    constexpr int RETINA_TRACK_FRAME_LEVEL_HIGH = 100;
    constexpr int RETINA_TRACK_FRAME_LEVEL_SHINY = 125;

    constexpr float RETINA_TRACK_SCAN_CENTER_OFFSET_X_OD = -1.58f;
    constexpr float RETINA_TRACK_SCAN_CENTER_OFFSET_X_OS = +1.58f;
    constexpr float RETINA_TRACK_SCAN_CENTER_OFFSET_Y = 0.0f;

    constexpr float RETINA_TRACK_TARGET_SCORE_MIN = 0.85f;
	constexpr float RETINA_TRACK_TARGET_DIST_MAX = 64.0f; // 32.0f;

	constexpr float RETINA_TRACK_TARGET_MOVED_X = 0.5f; // 2.0f;
	constexpr float RETINA_TRACK_TARGET_MOVED_Y = 0.5f; // 1.0f;
	constexpr float RETINA_TRACK_TARGET_MOVED_X2 = (RETINA_TRACK_TARGET_MOVED_X*2.0f);
	constexpr float RETINA_TRACK_TARGET_MOVED_Y2 = (RETINA_TRACK_TARGET_MOVED_Y*2.0f);
}
