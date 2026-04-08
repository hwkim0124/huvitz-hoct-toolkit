#pragma once

#include "OctGlobalDef.h"


namespace OctGlobal
{
	// Default Pattern preview, enface settings. 
	///////////////////////////////////////////////////////////////////////////////////////////
	constexpr int PATTERN_PREVIEW_LATERAL_POINTS = 1024; // 512;
	// constexpr int PATTERN_PREVIEW_AXIAL_POINTS = 512;
	constexpr int PATTERN_PREVIEW_ASCAN_POINTS = PATTERN_PREVIEW_LATERAL_POINTS;

	constexpr int PATTERN_PREVIEW_IMAGE_WIDTH = PATTERN_PREVIEW_LATERAL_POINTS;
	// constexpr int PATTERN_PREVIEW_IMAGE_HEIGHT = PATTERN_PREVIEW_AXIAL_POINTS; // within fft output line size. 

	constexpr float PATTERN_ENFACE_RANGE_X = 6.0f;
	constexpr float PATTERN_ENFACE_RANGE_Y = 6.0f;
	constexpr int PATTERN_ENFACE_ASCAN_POINTS = 256;
	constexpr int PATTERN_ENFACE_BSCAN_LINES = 64; // 96; // 128;

	constexpr int PATTERN_ENFACE_IMAGE_WIDTH = PATTERN_ENFACE_ASCAN_POINTS;
	constexpr int PATTERN_ENFACE_IMAGE_HEIGHT = PATTERN_ENFACE_BSCAN_LINES;

	constexpr int PATTERN_PREVIEW_AVERAGE_SIZE = 5;
	constexpr int PATTERN_PREVIEW_HISTORY_SIZE = 10;


	// Available Pattern Options.
	////////////////////////////////////////////////////////////////////////////////////////////
	constexpr float PATTERN_BSCAN_RANGE1 = 6.0f;
	constexpr float PATTERN_BSCAN_RANGE2 = 9.0f;
	constexpr float PATTERN_BSCAN_RANGE3 = 12.0f;
	constexpr float PATTERN_BSCAN_RANGE4 = 3.45f;		// Disc circle radius.
	constexpr float PATTERN_BSCAN_RANGE5 = 8.0f;		// Topography radius.
	constexpr int PATTERN_NUM_BSCAN_RANGES = 4;

	constexpr int PATTERN_CSCAN_OVERLAPS1 = 1;
	constexpr int PATTERN_CSCAN_OVERLAPS2 = 3;
	constexpr int PATTERN_CSCAN_OVERLAPS3 = 5;
	constexpr int PATTERN_CSCAN_OVERLAPS4 = 7;

	constexpr int PATTERN_BSCAN_OVERLAPS1 = 1;
	constexpr int PATTERN_BSCAN_OVERLAPS2 = 5;
	constexpr int PATTERN_BSCAN_OVERLAPS3 = 10;
	constexpr int PATTERN_BSCAN_OVERLAPS4 = 15;
	constexpr int PATTERN_BSCAN_OVERLAPS5 = 30;
	constexpr int PATTERN_BSCAN_OVERLAPS6 = 45;
	constexpr int PATTERN_NUM_BSCAN_OVERLAPS = 6;

	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO1 = 2;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO2 = 3;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO3 = 4;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO4 = 5;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO5 = 6;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO6 = 7;
	constexpr int PATTERN_BSCAN_OVERLAPS_ANGIO7 = 8;

	constexpr float PATTERN_BSCAN_SPACE1 = 0.15f;
	constexpr float PATTERN_BSCAN_SPACE2 = 0.20f;
	constexpr float PATTERN_BSCAN_SPACE3 = 0.25f;
	constexpr float PATTERN_BSCAN_SPACE4 = 0.30f;
	constexpr float PATTERN_BSCAN_SPACE5 = 0.35f;
	constexpr int PATTERN_NUM_BSCAN_SPACES = 5;

	constexpr int PATTERN_ASCAN_POINTS1 = 1024;
	constexpr int PATTERN_ASCAN_POINTS2 = 512;
	constexpr int PATTERN_ASCAN_POINTS3 = 384;
	constexpr int PATTERN_ASCAN_POINTS4 = 256;
	constexpr int PATTERN_NUM_ASCAN_POINTS = 4;

	constexpr int PATTERN_ASCAN_POINTS1_ANGIO = 512;
	constexpr int PATTERN_ASCAN_POINTS2_ANGIO = 384;
	constexpr int PATTERN_ASCAN_POINTS3_ANGIO = 304;
	constexpr int PATTERN_ASCAN_POINTS4_ANGIO = 256;
	constexpr int PATTERN_ASCAN_POINTS5_ANGIO = 224;
	constexpr int PATTERN_NUM_ASCAN_POINTS_ANGIO = 5;

	constexpr int PATTERN_BSCAN_LINES1 = 256;
	constexpr int PATTERN_BSCAN_LINES2 = 128;
	constexpr int PATTERN_BSCAN_LINES3 = 96;
	constexpr int PATTERN_BSCAN_LINES4 = 64;
	constexpr int PATTERN_NUM_BSCAN_LINES = 4;

	constexpr int PATTERN_BSCAN_ANGIO_LINES1 = 512;
	constexpr int PATTERN_BSCAN_ANGIO_LINES2 = 384;
	constexpr int PATTERN_BSCAN_ANGIO_LINES3 = 304;
	constexpr int PATTERN_BSCAN_ANGIO_LINES4 = 256;
	constexpr int PATTERN_BSCAN_ANGIO_LINES5 = 224;
	constexpr int PATTERN_BSCAN_ANGIO_LINES6 = 1;

	constexpr int PATTERN_LINE_LINES = 1;
	constexpr int PATTERN_CROSS_LINES = 10;
	constexpr int PATTERN_RADIAL_LINES1 = 12;
	constexpr int PATTERN_RADIAL_LINES2 = 18;
	constexpr int PATTERN_RADIAL_LINES3 = 24;
	constexpr int PATTERN_RASTER_LINES1 = 25; 
	constexpr int PATTERN_RASTER_LINES2 = 13;
	constexpr int PATTERN_RASTER_LINES3 = 9;
	constexpr int PATTERN_RASTER_LINES4 = 7;

	constexpr int PATTERN_TOPOGRAPHY_LINES1 = 16;
	constexpr int PATTERN_TOPOGRAPHY_LINES2 = 32;
	constexpr int PATTERN_TOPOGRAPHY_LINES3 = 64;

	constexpr int PATTERN_DIRECTION_X_Y = 0;
	constexpr int PATTERN_DIRECTION_Y_X = 1;


	// Pattern description Code
	/////////////////////////////////////////////////////////////////////////////////////////////
	constexpr int PATTERN_MACULAR_POINT_CODE = 0x0100;
	constexpr int PATTERN_MACULAR_LINE_CODE = 0x0101;
	constexpr int PATTERN_MACULAR_CROSS_CODE = 0x0102;
	constexpr int PATTERN_MACULAR_RADIAL_CODE = 0x0103;
	constexpr int PATTERN_MACULAR_3D_CODE = 0x0104;
	constexpr int PATTERN_MACULAR_RASTER_CODE = 0x0105;
	constexpr int PATTERN_MACULAR_CIRCLE_CODE = 0x0106;
	constexpr int PATTERN_MACULAR_ANGIO_CODE = 0x0107;

	constexpr int PATTERN_DISC_POINT_CODE = 0x0200;
	//constexpr int PATTERN_DISC_LINE_CODE = 0x0201;
	constexpr int PATTERN_DISC_RADIAL_CODE = 0x0203;
	constexpr int PATTERN_DISC_3D_CODE = 0x0204;
	constexpr int PATTERN_DISC_CIRCLE_CODE = 0x0205;
	constexpr int PATTERN_DISC_RASTER_CODE = 0x0206;
	constexpr int PATTERN_DISC_ANGIO_CODE = 0x0207;

	constexpr int PATTERN_ANTERIOR_POINT_CODE = 0x0300;
	constexpr int PATTERN_ANTERIOR_LINE_CODE = 0x0301;
	constexpr int PATTERN_ANTERIOR_RADIAL_CODE = 0x0303;
	constexpr int PATTERN_ANTERIOR_3D_CODE = 0x0304;
	constexpr int PATTERN_ANTERIOR_ANGIO_CODE = 0x0305;
	constexpr int PATTERN_ANTERIOR_WIDE_CODE = 0x0306;
	constexpr int PATTERN_ANTERIOR_AL_CODE = 0x0307;
	constexpr int PATTERN_ANTERIOR_LT_CODE = 0x0308;

	constexpr int PATTERN_CALIBRATION_POINT_CODE = 0x0401;

	constexpr int PATTERN_MACULAR_DISC_CODE = 0x0500;

	constexpr int PATTERN_TOPOGRAPHY_CODE = 0x0600;

	// Pattern description Name
	/////////////////////////////////////////////////////////////////////////////////////////////
	constexpr char* PATTERN_MACULAR_POINT_NAME = "Macular Point";
	constexpr char* PATTERN_MACULAR_LINE_NAME = "Macular Line";
	constexpr char* PATTERN_MACULAR_CROSS_NAME = "Macular Cross";
	constexpr char* PATTERN_MACULAR_RADIAL_NAME = "Macular Radial";
	constexpr char* PATTERN_MACULAR_3D_NAME = "Macular 3D";
	constexpr char* PATTERN_MACULAR_RASTER_NAME = "Macular Raster";
	constexpr char* PATTERN_MACULAR_CIRCLE_NAME = "Macular Circle";
	constexpr char* PATTERN_MACULAR_ANGIO_NAME = "Macular Angio";

	//constexpr int PATTERN_DISC_LINE_CODE = 0x0201;
	constexpr char* PATTERN_DISC_POINT_NAME = "Disc Point";
	constexpr char* PATTERN_DISC_RADIAL_NAME = "Disc Radial";
	constexpr char* PATTERN_DISC_3D_NAME = "Disc 3D";
	constexpr char* PATTERN_DISC_CIRCLE_NAME = "Disc Circle";
	constexpr char* PATTERN_DISC_RASTER_NAME = "Disc Raster";
	constexpr char* PATTERN_DISC_ANGIO_NAME = "Disc Angio";

	constexpr char* PATTERN_ANTERIOR_POINT_NAME = "Anterior Point";
	constexpr char* PATTERN_ANTERIOR_LINE_NAME = "Anterior Line";
	constexpr char* PATTERN_ANTERIOR_RADIAL_NAME = "Anterior Radial";
	constexpr char* PATTERN_ANTERIOR_3D_NAME = "Anterior 3D";
	constexpr char* PATTERN_ANTERIOR_ANGIO_NAME = "Anterior Angio";
	constexpr char* PATTERN_ANTERIOR_WIDE_NAME = "Anterior Full";
	constexpr char* PATTERN_ANTERIOR_AL_NAME = "Axial Length";
	constexpr char* PATTERN_ANTERIOR_LT_NAME = "Lens Thickness";
	constexpr char* PATTERN_CALIBRATION_POINT_NAME = "Calibration Point";

	constexpr char* PATTERN_MACULAR_DISC_NAME = "Macular Wide";

	constexpr char* PATTERN_TOPOGRAPHY_NAME = "Topography";

	constexpr float PATTERN_DISC_CIRCLE_RNAGE_X = 3.45f;
	constexpr float PATTERN_DISC_CIRCLE_RNAGE_Y = 3.45f;
	constexpr float PATTERN_DISC_CIRCLE_RNAGE_COMP = 1.1f;

	// Internal pattern types. 
	//////////////////////////////////////////////////////////////////////////////////////////////
	enum class LineTraceType {
		Line = 0,
		Radial,
		Circle,
		HorzRaster,
		VertRaster,
		HorzRasterFast,
		VertRasterFast
	};

	enum class PatternDomain {
		Unknown = 0,
		Macular,
		OpticDisc,
		Cornea, 
		Mirror
	};

	enum class PatternType {
		Unknown = 0,
		Point,
		HorzLine,
		VertLine,
		Circle,
		Cross,
		Radial,
		HorzRaster,
		VertRaster,
		HorzCube, 
		VertCube
	};

	enum class PatternName {
		Unknown = 0,
		CalibrationPoint,
		MacularPoint,
		MacularLine,
		MacularCross,
		MacularRadial,
		MacularRaster,
		MacularCircle,
		Macular3D,
		MacularAngio,
		DiscPoint,
		DiscLine,
		DiscRadial,
		DiscCircle,
		DiscRaster,
		Disc3D, 
		DiscAngio,
		AnteriorPoint,
		AnteriorLine,
		AnteriorRadial,
		Anterior3D,
		AnteriorAngio,
		MacularDisc,
		AnteriorFull,
		Topography,
		AxialLength,
		LensThickness,
		BioWithTopo,
	};

}