#pragma once

#include "OctGlobal2.h"
//#include "GlobalAnalysisDef.h"

using namespace OctGlobal;


class SegmScanDef
{
public:
	SegmScanDef();
	~SegmScanDef();

public:
	enum class EyeSide { OD = 0, OS };
	enum class ScanRegion { Macular = 0, NerveHead, Cornea };
	enum class PatternType { Line = 0, Raster, Cube };
	enum class ScanImageType { Line = 0, Radial, Circle };
	enum class ScanImageSize { WidthMin = 64, WidthMax = 4096, HeightMin = 256, HeightMax = 2048 };

};


namespace SegmScan {

	// enum class EyeSide { Unknown = 0, OD, OS };
	enum class ScanRegion { Macular = 0, OpticDisc, WideRetina, Cornea, AnteriorChamber };
	//enum class PatternType { Preview = 0, Line, Raster, Radial, Circle, Cube };
	enum class BscanType { Line = 0, Radial, Circle, Raster };
	enum class ImageSize { WidthMin = 64, WidthMax = 4096, HeightMin = 256, HeightMax = 2048 };

	enum class ImageType { SOURCE = 0, RESIZED, SAMPLE, SMOOTHED, GRADIENTS, COSTS };
	enum class LayerType { NONE = 0, ILM, IOS, RPE, IIOS, IRPE, INNER, OUTER };

	enum class Section { CENTER = 0, INNER_SUPERIOR, INNER_INFERIOR, INNER_TEMPORAL, INNER_NASAL,  
								OUTER_SUPERIOR, OUTER_INFERIOR, OUTER_TEMPORAL, OUTER_NASAL, 
								SUPERIOR, INFERIOR, NASAL, TEMPORAL, OVERALL };

	enum class ScanType { Line = 0, Radial, Circle, HorzRaster, VertRaster };


	constexpr float AXIAL_RESOLUTION = 3.12f;
	constexpr float LINE_HITTEST_OFFSET = 0.02f;

}

// typedef SegmScan::EyeSide		SegmEyeSide;
typedef SegmScan::ScanRegion	SegmScanRegion;
// typedef SegmScan::PatternType	SegmPatternType;
typedef SegmScan::BscanType		SegmScanType;
typedef SegmScan::ImageSize		SegmImageSize;


#ifdef __SEGMSCAN_DLL
#define SEGMSCAN_DLL_API		__declspec(dllexport)
#else
#define SEGMSCAN_DLL_API		__declspec(dllimport)
#endif

