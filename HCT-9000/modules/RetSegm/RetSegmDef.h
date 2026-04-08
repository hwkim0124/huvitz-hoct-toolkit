#pragma once

#include "OctGlobal2.h"

using namespace OctGlobal;


namespace RetSegm
{
	constexpr int SAMPLE_IMAGE_WIDTH = 256;
	constexpr int SAMPLE_IMAGE_HEIGHT = (768/2);

	constexpr float SAMPLE_IMAGE_MEAN_MIN = 9.0f; // 15.0f; // 25.0f;
	constexpr float SAMPLE_IMAGE_MEAN_MAX = 75.0f;
	
	constexpr float SAMPLE_PEAK_TO_MEAN_RATIO_MIN = 3.0f; // 4.5f;
	constexpr float SAMPLE_PEAK_TO_MEAN_RATIO_MIN_CORNEA = 2.0f; // 3.0f;

	constexpr float SAMPLE_POOR_PEAK_TO_MEAN_AREA_MIN = 0.50f; // 0.25f;

	constexpr float SCAN_OBJECT_TO_BACKGROUND_RATIO_MIN = 0.75f;
	constexpr float COLUMN_PEAK_MEAN_TO_STDDEV_FACTOR = 1.5f; // 2.0f;
	constexpr float EMPTY_PEAK_COLUMNS_RATIO = 0.35f;


	//	Coarse segmentation of inner and outer boundaries on retina.
	/////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int COARSE_GRADIENT_KERNEL_ROWS = 19; // 27; // 23;
	constexpr int COARSE_GRADIENT_KERNEL_COLS = 5; // 5; // 3;
	constexpr int COARSE_GRADIENT_KERNEL_ROWS2 = 9; // 27; // 23;
	constexpr int COARSE_GRADIENT_KERNEL_COLS2 = 9; // 5; // 3;
	constexpr int COARSE_GRADIENT_KERNEL_ROWS_CORNEA = 7; // 13;
	constexpr int COARSE_GRADIENT_KERNEL_COLS_CORNEA = 5; // 3;
	constexpr int COARSE_GRADIENT_KERNEL_ROWS_CORNEA2 = 5; // 9; // 7; // 13;
	constexpr int COARSE_GRADIENT_KERNEL_COLS_CORNEA2 = 5; // 5; // 5; // 3;

	constexpr float COARSE_BOUNDARY_EDGE_STDDEV_FACTOR = 1.0f;// 2.0f;
	constexpr float COARSE_BOUNDARY_PEAK_STDDEV_FACTOR = 1.0f;// 3.0f;
	constexpr float COARSE_BOUNDARY_EDGE_TO_PEAK_RATIO = 0.25f;
	constexpr int COARSE_BOUNDARY_PEAK_MIN = 45; // 75;
	constexpr int COARSE_BOUNDARY_EDGE_MIN = 45;
	constexpr int COARSE_BOUNDARY_FALL_SIZE = 5;

	constexpr int COARSE_BOUNDARY_PEAK_MIN_CORNEA = 24; // 45;
	constexpr float COARSE_BOUNDARY_EXCEPT_START = 0.45f;
	constexpr float COARSE_BOUNDARY_EXCEPT_CLOSE = 0.55f;

	constexpr float COARSE_BOUNDARY_SEGMENT_SIZE_MIN = 0.05f;
	constexpr float COARSE_BOUNDARY_POINTS_EMPTY_MAX = 0.05f;
	constexpr int COARSE_BOUNDARY_POINTS_Y_RANGE = 35; // 15;

	constexpr int COARSE_OUTER_LINE_Y_OFFSET1 = -5;
	constexpr int COARSE_OUTER_LINE_Y_OFFSET2 = 15;
	constexpr float COARSE_OUTER_LINE_SLACK_MAX = 0.025f;
	constexpr float COARSE_OUTER_LINE_SIZE_MIN = 0.02f;
	constexpr float COARSE_INNER_LINE_EDGE_SIZE = 0.15f;

	constexpr float COARSE_INNER_LINE_FILTER_SIZE = 0.05f;
	constexpr float COARSE_OUTER_LINE_FILTER_SIZE = 0.05f;

	constexpr float COARSE_INNER_POINTS_EMPTY_MAX = 0.05f;
	constexpr float COARSE_OUTER_POINTS_EMPTY_MAX = 0.05f;
	constexpr float COARSE_INNER_POINTS_SLICE_SIZE = 0.05f;
	constexpr float COARSE_OUTER_POINTS_SLICE_SIZE = 0.05f;
	constexpr int COARSE_INNER_POINTS_Y_RANGE = 15;
	constexpr int COARSE_OUTER_POINTS_Y_RANGE = 15;

	constexpr float COARSE_INNER_REMOVE_EMPTY_SIZE = 0.10f;
	constexpr float COARSE_OUTER_REMOVE_EMPTY_SIZE = 0.05f;
	constexpr float COARSE_INNER_REMOVE_GUESS_SIZE = 0.10f;
	constexpr float COARSE_OUTER_REMOVE_GUESS_SIZE = 0.05f;
	constexpr float COARSE_INNER_REMOVE_SLICE_SIZE = 0.02f;
	constexpr float COARSE_OUTER_REMOVE_SLICE_SIZE = 0.03f;

	constexpr float COARSE_INNER_REMOVE_EMPTY_SIZE_CORNEA = 0.025f;  // 0.10f;
	constexpr float COARSE_OUTER_REMOVE_EMPTY_SIZE_CORNEA = 0.10f;
	constexpr float COARSE_INNER_REMOVE_GUESS_SIZE_CORNEA = 0.025f;  // 0.15f;
	constexpr float COARSE_OUTER_REMOVE_GUESS_SIZE_CORNEA = 0.15f;
	constexpr float COARSE_INNER_REMOVE_SLICE_SIZE_CORNEA = 0.025f; // 0.10f;
	constexpr float COARSE_OUTER_REMOVE_SLICE_SIZE_CORNEA = 0.10f;

	constexpr float COARSE_INNER_REMOVE_EMPTY_SIZE_DISK = 0.10f;
	constexpr float COARSE_OUTER_REMOVE_EMPTY_SIZE_DISK = 0.05f;
	constexpr float COARSE_INNER_REMOVE_GUESS_SIZE_DISK = 0.10f;
	constexpr float COARSE_OUTER_REMOVE_GUESS_SIZE_DISK = 0.05f;
	constexpr float COARSE_INNER_REMOVE_SLICE_SIZE_DISK = 0.02f;
	constexpr float COARSE_OUTER_REMOVE_SLICE_SIZE_DISK = 0.05f; // 0.10f;

	constexpr float COARSE_INNER_POINTS_MIN_CORNEA = 0.5f;
	constexpr float COARSE_OUTER_POINTS_MIN_CORNEA = 0.25f;
	constexpr float COARSE_INTER_POINTS_MIN_CORNEA = 0.25f;

	constexpr int COARSE_OUTLIERS_RETRY_MAX_CORNEA = 9;
	constexpr int COARSE_INNER_OUTLIERS_OFFSET_CORNEA = 15;
	constexpr float COARSE_CURVE_PRIMARY_COEFF_MIN_CORNEA = 5.0f;

	constexpr int COARSE_INNER_OUTER_OFFSET_MIN_CORNEA = 45; // 75;
	constexpr int COARSE_INNER_OUTER_OFFSET_MAX_CORNEA = 150; // 300;
	constexpr float COARSE_INNER_OUTER_OVERLAP_START = 0.25f;
	constexpr float COARSE_INNER_OUTER_OVERLAP_CLOSE = 0.75f;
	constexpr float COARSE_INNER_OUTER_OVERLAP_MIN_CORNEA = 0.25f;

	constexpr int COARSE_INNER_INTER_OFFSET_MIN_CORNEA = 3; 
	constexpr int COARSE_INNER_INTER_OFFSET_MAX_CORNEA = 25;
	constexpr float COARSE_INNER_INTER_OVERLAP_START = 0.25f;
	constexpr float COARSE_INNER_INTER_OVERLAP_CLOSE = 0.75f;
	constexpr float COARSE_INNER_INTER_OVERLAP_MIN_CORNEA = 0.25f;

	constexpr int COARSE_INNER_REMOVE_Y_RANGE = 15;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE = 10;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE1 = -7;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE2 = 10;
	constexpr int COARSE_INNER_REMOVE_Y_RANGE_DISK = 25;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE_DISK = 10; // 5;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE_DISK1 = -7; // 5;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE_DISK2 = 15; // 5;

	constexpr int COARSE_INNER_REMOVE_Y_RANGE_CORNEA = 10;
	constexpr int COARSE_INNER_REMOVE_Y_BOUND_INN_CORNEA = 25;
	constexpr int COARSE_INNER_REMOVE_Y_BOUND_OUT_CORNEA = -5;
	constexpr int COARSE_OUTER_REMOVE_Y_RANGE_CORNEA = 10;

	constexpr float COARSE_INNER_SMOOTH_FILTER_SIZE = 0.05f;
	constexpr float COARSE_OUTER_SMOOTH_FILTER_SIZE = 0.05f;

	constexpr int COARSE_INNER_SMOOTH_COUNT_MAX = 5;
	constexpr int COARSE_INNER_SMOOTH_OUTLIER_OFFSET = 35;

	constexpr float COARSE_LINEAR_FITT_DATA_SIZE = 0.025f; // 0.15f; // 0.02f;

	// Optimal Path 
	////////////////////////////////////////////////////////////////////////////
	constexpr int PATH_ILM_UPPER_SPAN = 35;
	constexpr int PATH_ILM_LOWER_SPAN = 10;
	constexpr int PATH_ILM_UPPER_SPAN_DISC = 35; // 10; // 25;
	constexpr int PATH_ILM_LOWER_SPAN_DISC = 75; // 25; // 10;

	constexpr int PATH_ILM_MOVES_MIN = 2;
	constexpr int PATH_ILM_MOVES_MAX = 15; //  25; // 15;
	constexpr int PATH_ILM_MOVES_MIN_DISC = 2; // 3;
	constexpr int PATH_ILM_MOVES_MAX_DISC = 35; // 25; // 15;

	constexpr int PATH_ILM_KERNEL_ROWS = 7; // 5; // 7;
	constexpr int PATH_ILM_KERNEL_COLS = 3; // 5; // 11;
	constexpr float PATH_ILM_STDDEV_TO_GRAY_MIN = 0.0f;
	constexpr float PATH_ILM_STDDEV_TO_GRAY_MAX = 1.5f; // 1.0f; // 2.0f;
	constexpr float PATH_ILM_STDDEV_TO_GRAY_MAX_DISC = 2.0f; // 2.0f; // 2.0f;
	constexpr float PATH_ILM_STDDEV_TO_GRAY_MIN_DISC = 0.0f;

	constexpr int PATH_ILM_CURVE_SIZE = 5; // 7; //  9;
	constexpr int PATH_ILM_CURVE_DEGREE = 2; // 1; //  3;
	constexpr int PATH_ILM_CURVE_SIZE_DISC = 1; //  9;
	constexpr int PATH_ILM_CURVE_DEGREE_DISC = 1; // 1; //  3;


	constexpr int PATH_OPR_UPPER_SPAN = 5; // 5; //  5; // 10;
	constexpr int PATH_OPR_LOWER_SPAN = 10; // 7; // 13; //  25; // 45;
	constexpr int PATH_OPR_UPPER_SPAN_DISC = 5; // 3; // 5; // 10;
	constexpr int PATH_OPR_LOWER_SPAN_DISC = 10; // 15; // 25; // 25; // 25; // 45;

	constexpr int PATH_OPR_MOVES_MIN = 2; // 2; // 3;
	constexpr int PATH_OPR_MOVES_MAX = 5; // 15; // 15;
	constexpr int PATH_OPR_MOVES_MIN_DISC = 2; //  2; // 3;
	constexpr int PATH_OPR_MOVES_MAX_DISC = 9; // 15; // 15;

	constexpr int PATH_OPR_KERNEL_ROWS = 7;// 5;// 7; // 7;
	constexpr int PATH_OPR_KERNEL_COLS = 13; // 13;// 5; // 3;

	constexpr float PATH_OPR_STDDEV_TO_GRAY_MAX = 4.0f;

	constexpr int PATH_OPR_CURVE_SIZE = 1; // 5; // 13; // 9; //  15;
	constexpr int PATH_OPR_CURVE_DEGREE = 1; // 1; //  2; // 3;
	constexpr int PATH_OPR_CURVE_SIZE_DISC = 13; // 5; //  15;
	constexpr int PATH_OPR_CURVE_DEGREE_DISC = 1; //  2; // 3;


	constexpr int PATH_IOS_UPPER_SPAN = 5; // 10; // 10; // 25;
	constexpr int PATH_IOS_LOWER_SPAN = 0; // 3; // 10;
	constexpr int PATH_IOS_UPPER_SPAN_DISC = 10; // 25;
	constexpr int PATH_IOS_LOWER_SPAN_DISC = 0; // 10;

	constexpr int PATH_IOS_MOVES_MIN = 2; // 2;
	constexpr int PATH_IOS_MOVES_MAX = 5; // 15;
	constexpr int PATH_IOS_MOVES_MIN_DISC = 2; // 5; //  2;
	constexpr int PATH_IOS_MOVES_MAX_DISC = 9; // 25; // 15;

	constexpr int PATH_IOS_KERNEL_ROWS = 7; // 7; // 5;
	constexpr int PATH_IOS_KERNEL_COLS = 13; // 5; // 3;
	constexpr float PATH_IOS_STDDEV_TO_GRAY_MAX = 2.5f; // 4.0f; // 2.0f;

	constexpr int PATH_IOS_CURVE_SIZE = 1; // 13; // 9; // 21; //  15;
	constexpr int PATH_IOS_CURVE_DEGREE = 1; // 2; // 3;
	constexpr int PATH_IOS_CURVE_SIZE_DISC = 13; // 9; //  15;
	constexpr int PATH_IOS_CURVE_DEGREE_DISC = 1; //  2; // 3;


	constexpr int PATH_BRM_UPPER_SPAN = 0; // 2; // 0;
	constexpr int PATH_BRM_LOWER_SPAN = 5; // 7; // 5; // 10; // 5; // 7; // 20;
	constexpr int PATH_BRM_UPPER_SPAN_DISC = 2; // 0;
	constexpr int PATH_BRM_LOWER_SPAN_DISC = 7; // 5;// 10; // 5; // 10;

	constexpr int PATH_BRM_MOVES_MIN = 2; // 3;
	constexpr int PATH_BRM_MOVES_MAX = 5;// 15; // 25; // 15; // 5
	constexpr int PATH_BRM_MOVES_MIN_DISC = 2; // 3;
	constexpr int PATH_BRM_MOVES_MAX_DISC = 9;// 15; // 25; // 15; // 5

	constexpr int PATH_BRM_KERNEL_ROWS = 7; // 7; // 7;
	constexpr int PATH_BRM_KERNEL_COLS = 13; // 5; // 3;
	constexpr float PATH_BRM_STDDEV_TO_GRAY_MAX = 2.0f;

	constexpr int PATH_BRM_CURVE_SIZE = 5; // 13; // 9; // 15;
	constexpr int PATH_BRM_CURVE_DEGREE = 1; // 1; // 2;
	constexpr int PATH_BRM_CURVE_SIZE_DISC = 13; // 9; // 15;
	constexpr int PATH_BRM_CURVE_DEGREE_DISC = 1; // 2;


	constexpr int PATH_RPE_UPPER_SPAN = 5; // 15;
	constexpr int PATH_RPE_LOWER_SPAN = 0; // 2; // 3; //  5;
	constexpr int PATH_RPE_UPPER_SPAN_DISC = 5; // 15;
	constexpr int PATH_RPE_LOWER_SPAN_DISC = 0; // 2; // 3; // 2;

	constexpr int PATH_RPE_CURVE_SIZE = 1; // 5; // 9; // 15;
	constexpr int PATH_RPE_CURVE_DEGREE = 1; // 2;
	constexpr int PATH_RPE_CURVE_SIZE_DISC = 13; // 9; // 15;
	constexpr int PATH_RPE_CURVE_DEGREE_DISC = 1; // 2;

	constexpr int PATH_RPE_MOVES_MIN = 2; // 3;
	constexpr int PATH_RPE_MOVES_MAX = 5; // 15; // 25; // 15; // 5
	constexpr int PATH_RPE_MOVES_MIN_DISC = 2; // 3;
	constexpr int PATH_RPE_MOVES_MAX_DISC = 9;// 15; // 25; // 15; // 5


	constexpr int PATH_OPL_UPPER_SPAN = 25; // 15;
	constexpr int PATH_OPL_LOWER_SPAN = 3; // 2; // 3; //  5;
	constexpr int PATH_OPL_UPPER_SPAN_DISC = 25; // 15;
	constexpr int PATH_OPL_LOWER_SPAN_DISC = 3; // 2; // 3; // 2;

	constexpr int PATH_OPL_MOVES_MIN = 2; //  3;
	constexpr int PATH_OPL_MOVES_MAX = 5; // 15; //  25; // 15;// 3;
	constexpr int PATH_OPL_MOVES_MIN_DISC = 2; //  3;
	constexpr int PATH_OPL_MOVES_MAX_DISC = 5;// 25; //  25; // 15;// 3;

	constexpr int PATH_OPL_KERNEL_ROWS = 5; // 5;
	constexpr int PATH_OPL_KERNEL_COLS = 5; // 5;
	constexpr int PATH_OPL_KERNEL_ROWS_DISC = 7; // 5;
	constexpr int PATH_OPL_KERNEL_COLS_DISC = 3; // 5;

	constexpr float PATH_OPL_STDDEV_TO_GRAY_MIN = 0.0f; // 1.0f; // 2.0f;
	constexpr float PATH_OPL_STDDEV_TO_GRAY_MAX = 1.0f; // 2.0f; // 2.0f; // 1.0f; // 2.0f;
	constexpr float PATH_OPL_STDDEV_TO_GRAY_MIN_DISC = 0.0f; // 1.0f; // 2.0f;
	constexpr float PATH_OPL_STDDEV_TO_GRAY_MAX_DISC = 1.0f; // 2.0f; // 1.0f; // 2.0f;

	constexpr int PATH_OPL_CURVE_SIZE = 13; // 5; // 17; // 23; // 9;  // 17; //  25; // 15;
	constexpr int PATH_OPL_CURVE_DEGREE = 1; // 2; // 2;


	constexpr int PATH_IPL_UPPER_SPAN = 25; // 17; //  25; // 15; // 35; // 10; // 15;
	constexpr int PATH_IPL_LOWER_SPAN = 5;
	constexpr int PATH_IPL_UPPER_SPAN_DISC = 17; // 17; // 25; // 13; // 25; // 15;
	constexpr int PATH_IPL_LOWER_SPAN_DISC = 3; // 3; // 5;

	constexpr int PATH_IPL_MOVES_MIN = 2; // 3;
	constexpr int PATH_IPL_MOVES_MAX = 15; // 25; // 15;
	constexpr int PATH_IPL_MOVES_MIN_DISC = 2; //  3;
	constexpr int PATH_IPL_MOVES_MAX_DISC = 25; //  25; // 15;// 3;

	constexpr int PATH_IPL_KERNEL_ROWS = 5; // 9;
	constexpr int PATH_IPL_KERNEL_COLS = 3; // 5; // 13;
	constexpr int PATH_IPL_KERNEL_ROWS_DISC = 7; // 5; // 9;
	constexpr int PATH_IPL_KERNEL_COLS_DISC = 3; // 5; // 13;

	constexpr float PATH_IPL_STDDEV_TO_GRAY_MIN = 0.0f;
	constexpr float PATH_IPL_STDDEV_TO_GRAY_MAX = 2.0f; // 3.0f; // 4.0f;
	constexpr float PATH_IPL_STDDEV_TO_GRAY_MIN_DISC = 0.0f;
	constexpr float PATH_IPL_STDDEV_TO_GRAY_MAX_DISC = 3.0f; // 3.0f; // 4.0f;

	constexpr int PATH_IPL_CURVE_SIZE = 17; // 17; // 5; // 17; // 23; // 9; // 17; //  25;
	constexpr int PATH_IPL_CURVE_DEGREE = 1; // 2; // 1; // 2; // 2;


	constexpr int PATH_NFL_UPPER_SPAN = 35; // 10; // 15;
	constexpr int PATH_NFL_LOWER_SPAN = 5;
	constexpr int PATH_NFL_UPPER_SPAN_DISC = 19; // 17; // 25; // 25; // 15;
	constexpr int PATH_NFL_LOWER_SPAN_DISC = 2; // 3; // 2; // 0;

	constexpr int PATH_NFL_MOVES_MIN = 2; // 3; // 2;
	constexpr int PATH_NFL_MOVES_MAX = 5; // 15; //  15;
	constexpr int PATH_NFL_MOVES_MIN_DISC = 2; // 3; // 2;
	constexpr int PATH_NFL_MOVES_MAX_DISC = 5; // 25; //  15;

	constexpr int PATH_NFL_KERNEL_ROWS = 5;
	constexpr int PATH_NFL_KERNEL_COLS = 3; // 5;
	constexpr int PATH_NFL_KERNEL_ROWS_DISC = 7; // 5;
	constexpr int PATH_NFL_KERNEL_COLS_DISC = 3; // 5;

	constexpr float PATH_NFL_STDDEV_TO_GRAY_MIN = 2.0f; // 3.0f; // 4.0f;
	constexpr float PATH_NFL_STDDEV_TO_GRAY_MAX = 4.0f; // 3.0f; // 4.0f;
	constexpr float PATH_NFL_STDDEV_TO_GRAY_MIN_DISC = 2.0f; // 3.0f; // 4.0f;
	constexpr float PATH_NFL_STDDEV_TO_GRAY_MAX_DISC = 4.0f; // 3.0f; // 3.0f; // 4.0f;

	constexpr int PATH_NFL_CURVE_SIZE = 17; // 13; // 5; // 13; // 9; // 25; //  27;
	constexpr int PATH_NFL_CURVE_DEGREE = 1; // 2; // 2;


	constexpr int LAYER_ILM_UPPER_SPAN = 75; // 5; // 7;
	constexpr int LAYER_ILM_LOWER_SPAN = 15; // 10; // 25; // 15;
	constexpr int LAYER_ILM_UPPER_SPAN_DISC = 25; // 75; // 5; // 7;
	constexpr int LAYER_ILM_LOWER_SPAN_DISC = 55; // 15; // 10; // 25; // 15;

	constexpr int LAYER_ILM_MOVES_MIN = 2; // 3;
	constexpr int LAYER_ILM_MOVES_MAX = 15; //  25; // 15;
	constexpr int LAYER_ILM_MOVES_MIN_DISC = 2; // 3;
	constexpr int LAYER_ILM_MOVES_MAX_DISC = 35; // 25; // 15;

	constexpr int LAYER_ILM_KERNEL_ROWS = 7; // 5; // 9;
	constexpr int LAYER_ILM_KERNEL_COLS = 3; // 5;
	constexpr float LAYER_ILM_STDDEV_TO_GRAY_MAX = 3.0f; // 2.0f; // 1.0f; // 2.0f;
	constexpr float LAYER_ILM_STDDEV_TO_GRAY_MAX_DISC = 3.0f; // 2.0f; // 2.0f;

	constexpr int LAYER_ILM_CURVE_SIZE = 5; // 9; // 5; //  5; // 13; //  15; // 9;
	constexpr int LAYER_ILM_CURVE_DEGREE = 2; // 1; // 2; // 2; // 3;
	constexpr int LAYER_ILM_CURVE_SIZE_DISC = 1; //  5; // 13; //  15; // 9;
	constexpr int LAYER_ILM_CURVE_DEGREE_DISC = 1; // 2; // 3;


	constexpr int LAYER_NFL_UPPER_SPAN = 5;
	constexpr int LAYER_NFL_LOWER_SPAN = 5;
	constexpr int LAYER_NFL_UPPER_SPAN_DISC = 5; // 7;
	constexpr int LAYER_NFL_LOWER_SPAN_DISC = 13; // 5; // 25; // 15;

	constexpr float LAYER_NFL_UPPER_SPAN_BOUND = 0.50f; // 0.25f;
	constexpr float LAYER_NFL_LOWER_SPAN_BOUND = 0.05f; // 0.05f; // 0.85f;
	constexpr float LAYER_NFL_UPPER_SPAN_DISC_BOUND = 0.25f; // 0.50f; // 7;
	constexpr float LAYER_NFL_LOWER_SPAN_DISC_BOUND = 0.85f; // 0.05f; // 5; // 25; // 15;

	constexpr int LAYER_NFL_MOVES_MIN = 2; // 3;
	constexpr int LAYER_NFL_MOVES_MAX = 15; //  25; // 15;
	constexpr int LAYER_NFL_MOVES_MIN_DISC = 2; // 3;
	constexpr int LAYER_NFL_MOVES_MAX_DISC = 35; // 25; // 15;

	constexpr int LAYER_NFL_KERNEL_ROWS = 5; // 9;
	constexpr int LAYER_NFL_KERNEL_COLS = 3; // 5;
	constexpr int LAYER_NFL_KERNEL_ROWS_DISC = 5; // 7; // 9;
	constexpr int LAYER_NFL_KERNEL_COLS_DISC = 3; // 5;
	constexpr float LAYER_NFL_STDDEV_TO_GRAY_MAX = 2.0f; // 1.0f; // 2.0f;
	constexpr float LAYER_NFL_STDDEV_TO_GRAY_MAX_DISC = 2.0f; // 2.0f;

	constexpr float LAYER_NFL_IPL_SPAN_BOUND = 0.33f;
	constexpr float LAYER_NFL_STDDEV_TO_FIRST_PEAK = 2.0f;
	constexpr float LAYER_NFL_RPE_PEAK_RATIO = 0.55f;

	constexpr int LAYER_NFL_CURVE_SIZE = 13; // 17; // 29; // 23; // 15; //  27; // 13; // 15; // 27;
	constexpr int LAYER_NFL_CURVE_DEGREE = 1; // 2; // 2;
	constexpr int LAYER_NFL_CURVE_SIZE_DISC = 1; //  23; // 15; //  27; // 13; // 15; // 27;
	constexpr int LAYER_NFL_CURVE_DEGREE_DISC = 1; // 2;

	
	
	constexpr int LAYER_IPL_UPPER_SPAN = 5;
	constexpr int LAYER_IPL_LOWER_SPAN = 13;
	constexpr int LAYER_IPL_UPPER_SPAN_DISC = 5; // 7;
	constexpr int LAYER_IPL_LOWER_SPAN_DISC = 13; // 5; // 25; // 15;

	constexpr float LAYER_IPL_UPPER_SPAN_BOUND = 0.45f;
	constexpr float LAYER_IPL_LOWER_SPAN_BOUND = 0.25f;
	constexpr float LAYER_IPL_UPPER_SPAN_DISC_BOUND = 0.05f; // 7;
	constexpr float LAYER_IPL_LOWER_SPAN_DISC_BOUND = 0.05f; // 5; // 25; // 15;

	constexpr int LAYER_IPL_MOVES_MIN = 2; // 3;
	constexpr int LAYER_IPL_MOVES_MAX = 15; //  25; // 15;
	constexpr int LAYER_IPL_MOVES_MIN_DISC = 2; // 3;
	constexpr int LAYER_IPL_MOVES_MAX_DISC = 35; // 25; // 15;

	constexpr int LAYER_IPL_CURVE_SIZE = 13; // 23; // 17; // 21;
	constexpr int LAYER_IPL_CURVE_DEGREE = 1; // 2;


	constexpr int LAYER_OPL_UPPER_SPAN = 5;
	constexpr int LAYER_OPL_LOWER_SPAN = 13;
	constexpr int LAYER_OPL_UPPER_SPAN_DISC = 5; // 7;
	constexpr int LAYER_OPL_LOWER_SPAN_DISC = 13; // 5; // 25; // 15;

	constexpr float LAYER_OPL_UPPER_SPAN_BOUND = 0.45f; // 0.25f;
	constexpr float LAYER_OPL_LOWER_SPAN_BOUND = 0.05f; // 0.85f;
	constexpr float LAYER_OPL_UPPER_SPAN_DISC_BOUND = 0.45f; // 0.50f; // 7;
	constexpr float LAYER_OPL_LOWER_SPAN_DISC_BOUND = 0.05f; // 0.05f; // 5; // 25; // 15;

	constexpr int LAYER_OPL_MOVES_MIN = 2; // 3;
	constexpr int LAYER_OPL_MOVES_MAX = 15; //  25; // 15;
	constexpr int LAYER_OPL_MOVES_MIN_DISC = 2; // 3;
	constexpr int LAYER_OPL_MOVES_MAX_DISC = 35; // 25; // 15;

	constexpr int LAYER_OPL_CURVE_SIZE = 13; // 23; // 17; // 21;
	constexpr int LAYER_OPL_CURVE_DEGREE = 1; // 2;

	constexpr int LAYER_IOS_KERNEL_ROWS = 7; // 5;
	constexpr int LAYER_IOS_KERNEL_COLS = 13; // 9; // 5;

	constexpr int LAYER_IOS_UPPER_SPAN = 5; // 7; // 5;
	constexpr int LAYER_IOS_LOWER_SPAN = 2; // 3; // 5;
	constexpr int LAYER_IOS_UPPER_SPAN_DISC = 5; // 5; // 7; // 7;
	constexpr int LAYER_IOS_LOWER_SPAN_DISC = 2; // 3; // 5; // 25; // 15;

	constexpr int LAYER_IOS_MOVES_MIN = 3; // 2;
	constexpr int LAYER_IOS_MOVES_MAX = 9;
	constexpr int LAYER_IOS_MOVES_MIN_DISC = 3;
	constexpr int LAYER_IOS_MOVES_MAX_DISC = 25;

	constexpr int LAYER_IOS_CURVE_SIZE = 1; // 21;
	constexpr int LAYER_IOS_CURVE_DEGREE = 1; // 2;
	constexpr int LAYER_IOS_CURVE_SIZE_DISC = 15; // 21;
	constexpr int LAYER_IOS_CURVE_DEGREE_DISC = 1; // 2;


	constexpr int LAYER_BRM_UPPER_SPAN = 1; // 5;
	constexpr int LAYER_BRM_LOWER_SPAN = 1; // 9; // 3; // 15; // 7;
	constexpr int LAYER_BRM_UPPER_SPAN_DISC = 1; // 5; // 7; // 7;
	constexpr int LAYER_BRM_LOWER_SPAN_DISC = 1; // 9; // 7; // 25; // 15;

	constexpr int LAYER_BRM_MOVES_MIN = 3; // 2;
	constexpr int LAYER_BRM_MOVES_MAX = 9;
	constexpr int LAYER_BRM_MOVES_MIN_DISC = 3;
	constexpr int LAYER_BRM_MOVES_MAX_DISC = 25;

	constexpr int LAYER_BRM_KERNEL_ROWS = 7; // 5;
	constexpr int LAYER_BRM_KERNEL_COLS = 13; // 9; // 7; // 9;// 5;

	constexpr int LAYER_BRM_CURVE_SIZE = 1; // 21;
	constexpr int LAYER_BRM_CURVE_DEGREE = 1; // 2;
	constexpr int LAYER_BRM_CURVE_SIZE_DISC = 15; // 21;
	constexpr int LAYER_BRM_CURVE_DEGREE_DISC = 1; // 2;


	constexpr int LAYER_RPE_UPPER_SPAN = 11; // 5;
	constexpr int LAYER_RPE_LOWER_SPAN = 9; // 7;
	constexpr int LAYER_RPE_UPPER_SPAN_DISC = 11; // 3; // 5; // 7; // 7;
	constexpr int LAYER_RPE_LOWER_SPAN_DISC = 9; // 5; // 25; // 15;

	constexpr int LAYER_RPE_MOVES_MIN = 3; // 2;
	constexpr int LAYER_RPE_MOVES_MAX = 9;
	constexpr int LAYER_RPE_MOVES_MIN_DISC = 3;
	constexpr int LAYER_RPE_MOVES_MAX_DISC = 25;

	constexpr int LAYER_RPE_CURVE_SIZE = 1; // 21;
	constexpr int LAYER_RPE_CURVE_DEGREE = 1; // 2;
	constexpr int LAYER_RPE_CURVE_SIZE_DISC = 15; // 21;
	constexpr int LAYER_RPE_CURVE_DEGREE_DISC = 1; // 2;

	constexpr int LAYER_RPE_KERNEL_ROWS = 5;
	constexpr int LAYER_RPE_KERNEL_COLS = 9;
	constexpr int LAYER_RPE_KERNEL_ROWS_DISC = 5;
	constexpr int LAYER_RPE_KERNEL_COLS_DISC = 7; // 9;

	constexpr int PATH_RPE_KERNEL_ROWS = 7;
	constexpr int PATH_RPE_KERNEL_COLS = 5;

	constexpr float LAYER_IOS_RPE_SPAN_BOUND = 0.25f;
	constexpr float LAYER_RPE_BRM_SPAN_BOUND = 0.25f;

	constexpr int LAYER_EPI_UPPER_SPAN = 5; // 35;
	constexpr int LAYER_EPI_LOWER_SPAN = 20; // 25; // 50;
	constexpr int LAYER_EPI_UPPER_SPAN_CENTER = 0; // 35;
	constexpr int LAYER_EPI_LOWER_SPAN_CENTER = 10; // 25; // 50;
	constexpr int LAYER_EPI_MOVES = 3;

	constexpr float LAYER_EPI_CHANGE_RATIO = 1.5f;
	constexpr int LAYER_EPI_MOVES_MIN = 3;
	constexpr int LAYER_EPI_MOVES_MAX = 7;

	constexpr float LAYER_EPI_STDDEV_TO_GRAY_MAX = 2.5f; // 2.0f;
	constexpr int LAYER_EPI_KERNEL_ROWS = 9; // 13;
	constexpr int LAYER_EPI_KERNEL_COLS = 5; // 5;
	constexpr int LAYER_EPI_KERNEL_ROWS2 = 7; // 13;
	constexpr int LAYER_EPI_KERNEL_COLS2 = 5; // 5;

	constexpr int LAYER_EPI_SMOOTH_CURVE_SIZE = 5; // 11; // 27;
	constexpr int LAYER_EPI_SMOOTH_CURVE_DEGREE = 1; // 2;
	constexpr int LAYER_BOW_SMOOTH_CURVE_SIZE = 9; // 27;
	constexpr int LAYER_BOW_SMOOTH_CURVE_DEGREE = 1; // 2;

	constexpr int LAYER_END_UPPER_SPAN = 5;
	constexpr int LAYER_END_LOWER_SPAN = 150;
	constexpr int LAYER_END_UPPER_SPAN_CENTER = 5;
	constexpr int LAYER_END_LOWER_SPAN_CENTER = 15;
	constexpr int LAYER_END_MOVES = 3;

	constexpr float LAYER_END_CHANGE_RATIO = 1.5f;
	constexpr int LAYER_END_MOVES_MIN = 3;
	constexpr int LAYER_END_MOVES_MAX = 7;

	constexpr float LAYER_END_STDDEV_TO_GRAY_MAX = 2.0f;
	constexpr int LAYER_END_KERNEL_ROWS = 9; // 9;
	constexpr int LAYER_END_KERNEL_COLS = 5;

	constexpr int LAYER_END_SMOOTH_CURVE_SIZE = 9; // 7; // 27;
	constexpr int LAYER_END_SMOOTH_CURVE_DEGREE = 1; // 2;

	constexpr int LAYER_BOW_UPPER_SPAN = -5;
	constexpr int LAYER_BOW_LOWER_SPAN = 30;
	constexpr int LAYER_BOW_MOVES = 3;
	constexpr int LAYER_BOW_KERNEL_ROWS = 3; // 5; // 9;
	constexpr int LAYER_BOW_KERNEL_COLS = 3;

	constexpr float LAYER_BOW_TOP_RANGE_HALF = 0.025f;

	constexpr float PATH_ILM_CHANGE_RATIO = 1.25f;

	constexpr int PATH_COST_MAX = 9999; // 99999;


	// Optic Disk Feature
	/////////////////////////////////////////////////////////////////////////////
	constexpr float FEAT_DISK_SIZE_MIN = 0.12f; // 0.07f;
	constexpr int FEAT_DISK_OUTER_IDEAL_OFFSET = 25;
	constexpr int FEAT_DISK_MARGIN_IDEAL_OFFSET = 10;

	constexpr float FEAT_DISK_CUP_LINE_DEPTH = 150.0f;

	constexpr float FEAT_DISK_EDGE_SIZE_MM = 0.50f;
	constexpr float FEAT_DISK_HOLE_SIZE_MM = 0.36f;
	constexpr float FEAT_DISK_HORZ_SIZE_MM = 0.72f;
	constexpr float FEAT_DISC_DIST_SIZE_MM = 1.80f;

	constexpr int FEAT_DISK_EDGE_OFFSET_Y1 = -5; // -11; // 14; // -16;
	constexpr int FEAT_DISK_EDGE_OFFSET_Y2 = +5;

	// constexpr members are allowed only in literal class. 
	// http://www.codeproject.com/Articles/417719/Constants-and-Constant-Expressions-in-Cplusplus

	// constexpr variables are defined and used to define other objects at compile time, 
	// and in namespace, they seem to have internal linkage, can be included in various modules. 
	// http://www.codeproject.com/Articles/417719/Constants-and-Constant-Expressions-in-Cplusplus

	constexpr float SOURCE_WIDTH_RATIO = 0.5f; // 0.25f;
	constexpr float SOURCE_HEIGHT_RATIO = 0.5f;
	constexpr float RESIZED_WIDTH_RATIO = 0.5f; // 0.25f;
	constexpr float RESIZED_HEIGHT_RATIO = 0.5f;

	constexpr int SAMPLE_WIDTH_MIN = 256;
	constexpr int SAMPLE_HEIGHT_MIN = 96;
	constexpr float SAMPLE_RATIO_MIN = 0.125f;
	constexpr float SAMPLE_WIDTH_RATIO = 0.25f;
	constexpr float SAMPLE_HEIGHT_RATIO = 0.125f;

	constexpr int NOISE_FILTER_SIZE = 5;
	constexpr int NOISE_FILTER_WIDTH = 3;// 3; //  1; //  3;
	constexpr int NOISE_FILTER_HEIGHT = 21; // 15; //  13;


	constexpr float MOIRE_REGION_RATIO = 0.05f;
	constexpr float MOIRE_MARGIN_RATIO = 0.15f;
	constexpr float MOIRE_MARGIN_CORRECT = 0.5f;
	constexpr int MOIRE_CANCELED_REGION_STDDEV = 5;
	constexpr int MOIRE_CANCELED_REGION_MEAN = 10;

	constexpr float IMAGE_MEAN_MIN = 15.0f;
	constexpr float IMAGE_MEAN_MAX = 75.0f;
	constexpr float IMAGE_OBJECT_TO_MEAN_RATIO_MIN = 0.75f;
	constexpr float IMAGE_TOP_REGION_RATIO = 0.10f;
	constexpr float COLUMN_MAX_ON_TOP_REGION_RATIO = 0.15f;
	constexpr int COLUMN_PEAK_MIN_LEVEL = 90;
	constexpr float COLUMN_PEAK_STDDEV_FACTOR = 2.0f;
	constexpr float COLUMN_PEAK_EMPTY_RATIO = 0.35f;

	constexpr int COARSE_GRADIENT_X_ROWS = (int)(48 * SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_GRADIENT_X_COLS = (int)(8 * SAMPLE_WIDTH_RATIO);
	constexpr int COARSE_GRADIENT_X_SIZE = (int)(COARSE_GRADIENT_X_ROWS*COARSE_GRADIENT_X_COLS);
	constexpr int COARSE_GRADIENT_Y_ROWS = (int)(64 * SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_GRADIENT_Y_COLS = (int)(12 * SAMPLE_WIDTH_RATIO);
	constexpr int COARSE_GRADIENT_Y_SIZE = (int)(COARSE_GRADIENT_Y_ROWS*COARSE_GRADIENT_Y_COLS);

	constexpr int COARSE_Y_KERNEL_ROWS = (int)(3);
	constexpr int COARSE_Y_KERNEL_COLS = (int)(15);
	constexpr int COARSE_Y_KERNEL_SIZE = (int)(COARSE_Y_KERNEL_ROWS*COARSE_Y_KERNEL_COLS);

	constexpr float COARSE_PEAK_BASE_RATIO = 0.45f; // 0.65f;
	constexpr int COARSE_PEAK_HILL_SIZE = (int)(16*SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_PEAK_LEVEL_MIN = 35; // 25;
	constexpr int COARSE_PEAK_ILM_LEVEL_MIN = 35;
	constexpr int COARSE_PEAK_RPE_LEVEL_MIN = 20;
	constexpr int COARSE_DISTANCE_TO_PEAK_MAX = (int)(256* SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_DISTANCE_TO_PEAK_MIN = (int)(24* SAMPLE_HEIGHT_RATIO);

	constexpr float COARSE_INNER_SEGMENT_SIZE = 0.05f;
	constexpr float COARSE_OUTER_SEGMENT_SIZE = 0.05f;
	constexpr float COARSE_INNER_SECTION_SIZE = 0.10f;
	constexpr float COARSE_OUTER_SECTION_SIZE = 0.10f;
	constexpr float COARSE_INNER_CHANGE_FACTOR = 1.0f;
	constexpr float COARSE_OUTER_CHANGE_FACTOR = 1.0f;

	constexpr int COARSE_INNER_Y_SPAN = (int)(40 * SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_OUTER_Y_SPAN = (int)(24 * SAMPLE_HEIGHT_RATIO);
	
	constexpr int COARSE_INNER_CHANGE_MIN = (int)(24*SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_INNER_CHANGE_MAX = (int)(120*SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_OUTER_CHANGE_MIN = (int)(16*SAMPLE_HEIGHT_RATIO);
	constexpr int COARSE_OUTER_CHANGE_MAX = (int)(64*SAMPLE_HEIGHT_RATIO);

	constexpr float COARSE_INNER_SMOOTH_SIZE = 0.05f;
	constexpr float COARSE_OUTER_SMOOTH_SIZE = 0.05f;
	constexpr int COARSE_INNER_SMOOTH_COUNT = 3;
	constexpr int COARSE_OUTER_SMOOTH_COUNT = 3;

	constexpr int OPTIMAL_PATH_CHANGE_MAX = 9;
	constexpr int OPTIMAL_PATH_COST_MAX = 99999;

	constexpr int PATH_ILM_UPPER_RANGE = 70; // 50;
	constexpr int PATH_ILM_LOWER_RANGE = 30; // 5;
	constexpr int PATH_IOS_UPPER_RANGE = 30; // 50;
	constexpr int PATH_IOS_LOWER_RANGE = 70; // 5;

	constexpr int PATH_ILM_CHANGE_MAX = 10;
	constexpr int PATH_IOS_CHANGE_MAX = 10;

	constexpr int PATH_ILM_EDGE_SIZE = 6;
	constexpr int PATH_IOS_EDGE_SIZE = 6; // 5;
	constexpr int PATH_RPE_EDGE_SPAN_SIZE = 5;

	constexpr float PATH_ILM_THRESH1 = 0.05f;
	constexpr float PATH_ILM_THRESH2 = 0.55f;
	constexpr float PATH_IOS_THRESH1 = 0.25f; // 0.15f;
	constexpr float PATH_IOS_THRESH2 = 1.00f; // 0.75f;
	constexpr float PATH_RPE_THRESH1 = 0.15f;
	constexpr float PATH_RPE_THRESH2 = 1.00f;



	constexpr int PATH_IOS_UPPER_BOUND_SPAN = 0;
	constexpr int PATH_RPE_UPPER_BOUND_SPAN = 1;

	constexpr int PATH_IOS_LOWER_BOUND_SPAN = 25;
	constexpr int PATH_RPE_LOWER_BOUND_SPAN = 15;

	constexpr float PATH_ILM_DELTA_FACTOR = 1.5f;
	constexpr float PATH_IOS_DELTA_FACTOR = 1.5f;
	constexpr float PATH_RPE_DELTA_FACTOR = 1.5f;


	// constexpr float PATH_ILM_SMOOTH_WINDOW = 0.02f;
	// constexpr float PATH_IOS_SMOOTH_WINDOW = 0.03f;
	constexpr int PATH_ILM_SMOOTH_WIDTH = 12;
	constexpr int PATH_IOS_SMOOTH_WIDTH = 10;
	constexpr int PATH_RPE_SMOOTH_WIDTH = 18;
	constexpr int PATH_ILM_SMOOTH_DEGREE = 3;
	constexpr int PATH_IOS_SMOOTH_DEGREE = 2;
	constexpr int PATH_RPE_SMOOTH_DEGREE = 2;
}



#ifdef __RETSEGM_DLL
#define RETSEGM_DLL_API		__declspec(dllexport)
#else
#define RETSEGM_DLL_API		__declspec(dllimport)
#endif
