#pragma once

#include "OctGlobalDef.h"


namespace OctGlobal
{
	// Retina Tracking
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr float FUNDUS_SCAN_CENTER_OFFSET_X_OD = -1.58f; // -1.58f; // -1.8f;
	constexpr float FUNDUS_SCAN_CENTER_OFFSET_X_OS = +1.58f; //  +1.58f; // +1.8f;
	constexpr float FUNDUS_SCAN_CENTER_OFFSET_Y = 0.0f; // 0.4f;

	constexpr float RETT_CAMERA_PIXEL_RESOL_IN_MM = 0.03f;
	constexpr int RETT_CAMERA_FRAME_WIDTH = 644;
	constexpr int RETT_CAMERA_FRAME_HEIGHT = 480;
	constexpr int RETT_CAMERA_FRAME_CENT_X = RETT_CAMERA_FRAME_WIDTH / 2;
	constexpr int RETT_CAMERA_FRAME_CENT_Y = RETT_CAMERA_FRAME_HEIGHT / 2;

	constexpr float RETT_FEAT_THRESH_TO_REGIST = 0.99f;
	constexpr float RETT_FEAT_THRESH_TO_COMPARE = 0.97f;

	constexpr int RET_TRACK_CAMERA_FRAME_WIDTH = 644;
	constexpr int RET_TRACK_CAMERA_FRAME_HEIGHT = 480;
	constexpr int RET_TRACK_CAMERA_FRAME_CENT_X = RET_TRACK_CAMERA_FRAME_WIDTH / 2;
	constexpr int RET_TRACK_CAMERA_FRAME_CENT_Y = RET_TRACK_CAMERA_FRAME_HEIGHT / 2;
	constexpr float RET_TRACK_CAMERA_PIXEL_RESOL_IN_MM = 0.03f;

	constexpr float RETT_SCAN_CENTER_OFFSET_X_OD = -1.58f; // -1.8f;
	constexpr float RETT_SCAN_CENTER_OFFSET_X_OS = +1.58f;; // +1.8f;
	constexpr float RETT_SCAN_CENTER_OFFSET_Y = 0.0f; // 0.4f;

	constexpr int RET_TRACK_FEATURE_X_SIZE = 121; // 91; // 120;
	constexpr int RET_TRACK_FEATURE_Y_SIZE = 141; //  160;
	constexpr int RET_TRACK_FEATURE_X_SIZE_DISC = 181; // 180;
	constexpr int RET_TRACK_FEATURE_Y_SIZE_DISC = 181; // 160;
	constexpr int RET_TRACK_FEATURE_X_OFFSET = 35; // 40; // 25;
	constexpr int RET_TRACK_FEATURE_Y_OFFSET = -18; // -18; // -25;

	constexpr int RET_TRACK_UPSIDE_X_SIZE = 240;
	constexpr int RET_TRACK_UPSIDE_Y_SIZE = 120;
	constexpr int RET_TRACK_UPSIDE_X_OFFSET = 0;
	constexpr int RET_TRACK_UPSIDE_Y_OFFSET = 50;

	constexpr int RET_TRACK_DOWNSIDE_X_SIZE = 240;
	constexpr int RET_TRACK_DOWNSIDE_Y_SIZE = 120;
	constexpr int RET_TRACK_DOWNSIDE_X_OFFSET = 0;
	constexpr int RET_TRACK_DOWNSIDE_Y_OFFSET = 50;

	constexpr int RETT_TRACK_FEATURE_MEAN_MIN = 50; //  45; // 30;
	constexpr int RETT_TRACK_FEATURE_MEAN_MAX = 150; // 235;
	constexpr float RETT_TRACK_FEATURE_COEFF_VARS_MIN = 0.20f; // 0.15f; // 0.25f; //  0.35f;
	constexpr float RETT_TRACK_FEATURE_COEFF_VARS_MAX = 0.60f;

	constexpr int RETT_TRACK_EYELID_COVERED_MEAN = 200; // 245;
	constexpr float RETT_TRACK_FEATURE_STDEV_MIN = 4.5f;
	constexpr float RETT_TRACK_FEATURE_STDEV_MAX = 75.0f;

	constexpr int RET_TRACK_FRAME_LEVEL_DARK_1 = 45; // 30; // 30; // 30;
	constexpr int RET_TRACK_FRAME_LEVEL_LOW_1 = 70; //  60; // 75; // 55; // 55; //  45; // 75;
	constexpr int RET_TRACK_FRAME_LEVEL_HIGH_1 = 75; // 75; // 80; // 85; // 65; // 110;
	constexpr int RET_TRACK_FRAME_LEVEL_SHINY_1 = 100; // 100; // 110; // 80; // 145;

	constexpr int RET_TRACK_FRAME_LEVEL_DARK_2 = 50; // 30; // 30; // 30;
	constexpr int RET_TRACK_FRAME_LEVEL_LOW_2 = 75; // 55; // 55; //  45; // 75;
	constexpr int RET_TRACK_FRAME_LEVEL_HIGH_2 = 80; // 80; // 85; // 65; // 110;
	constexpr int RET_TRACK_FRAME_LEVEL_SHINY_2 = 105; // 105; // 110; // 80; // 145;


	// Kerato Focus 
	//////////////////////////////////////////////////////////////////////////////////////
	constexpr float KER_SETUP_DIOPTER_1 = -15.0f;
	constexpr float KER_SETUP_DIOPTER_2 = -7.0f;
	constexpr float KER_SETUP_DIOPTER_3 = 0.0f;
	constexpr float KER_SETUP_DIOPTER_4 = +7.0f;
	constexpr float KER_SETUP_DIOPTER_5 = +15.0f;


	constexpr float KER_SETUP_DIAMETER_1 = 6.713f;
	constexpr float KER_SETUP_DIAMETER_2 = 7.287f;
	constexpr float KER_SETUP_DIAMETER_3 = 7.928f;
	constexpr float KER_SETUP_DIAMETER_4 = 8.758f;
	constexpr float KER_SETUP_DIAMETER_5 = 9.509f;

	constexpr int KER_SETUP_DIOPTER_NUM = 5;
	constexpr int KER_SETUP_FOCUS_STEP_NUM = 5;
	constexpr char* KER_SETUP_DATA_FILE_NAME = "kersetup_data.dat";


	// Kerato Alignment 
	//////////////////////////////////////////////////////////////////////////////////////
	/*
	constexpr float KER_ALIGN_TARGET_RANGE_X1 = -9.0f; // -6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_X2 = +9.0f; // +6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Y1 = -9.0f; // -6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Y2 = +9.0f; // +6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_XY = 13.0f; // 6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Z1 = -2.5f; // -2.5f;
	constexpr float KER_ALIGN_TARGET_RANGE_Z2 = +2.5f; // +2.5f;

	constexpr float KER_DIST_PER_ENC_STEP_X1 = 9.0f; // 6.0f;
	constexpr float KER_DIST_PER_ENC_STEP_X2 = 9.0f; // 6.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Y1 = 0.05f; // 50.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Y2 = 0.05f; // 50.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Z1 = 2.0f; // 1.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Z2 = 2.0f; // 1.0f;
	*/

	constexpr float KER_ALIGN_TARGET_RANGE_X1 = -10.0f; // -9.0f; // -6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_X2 = +10.0f; // +6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Y1 = -10.0f; // -6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Y2 = +10.0f; // +6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_XY = 13.0f; // 6.0f;
	constexpr float KER_ALIGN_TARGET_RANGE_Z1 = -0.5f; // -2.5f;
	constexpr float KER_ALIGN_TARGET_RANGE_Z2 = +0.5f; // +2.5f;

	constexpr float KER_DIST_PER_ENC_STEP_X1 = 1.0f; // 9.0f; // 6.0f;
	constexpr float KER_DIST_PER_ENC_STEP_X2 = 1.0f; // 9.0f; // 6.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Y1 = 0.05f; // 50.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Y2 = 0.05f; // 50.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Z1 = 0.1f; // 2.0f; // 1.0f;
	constexpr float KER_DIST_PER_ENC_STEP_Z2 = 0.1f;// 2.0f; // 1.0f;


	// Kerato Focus Level and Z Range
	///////////////////////////////////////////////////////////////////////////////////////
	constexpr int KER_FOCUS_LEVEL_BACK3 = -3;
	constexpr int KER_FOCUS_LEVEL_BACK2 = -2;
	constexpr int KER_FOCUS_LEVEL_BACK1 = -1;
	constexpr int KER_FOCUS_LEVEL_ON = 0;
	constexpr int KER_FOCUS_LEVEL_FRONT1 = 1;
	constexpr int KER_FOCUS_LEVEL_FRONT2 = 2;
	constexpr int KER_FOCUS_LEVEL_FRONT3 = 3;
	constexpr int KER_FOCUS_LEVEL_NONE = 999;

	constexpr float KER_FOCUS_ON_Z_MIN = -1.0f;
	constexpr float KER_FOCUS_ON_Z_MAX = +1.0f;
	constexpr float KER_FOCUS_BACK1_Z_SIZE = 1.0f;
	constexpr float KER_FOCUS_BACK2_Z_SIZE = 1.0f;
	constexpr float KER_FOCUS_BACK3_Z_SIZE = 5.0f;
	constexpr float KER_FOCUS_FRONT1_Z_SIZE = 1.0f;
	constexpr float KER_FOCUS_FRONT2_Z_SIZE = 1.0f;
	constexpr float KER_FOCUS_FRONT3_Z_SIZE = 5.0f;
}