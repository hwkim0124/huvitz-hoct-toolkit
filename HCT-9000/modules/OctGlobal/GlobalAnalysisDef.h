#pragma once

#include "OctGlobalDef.h"

#include <functional>
#include <string>


namespace OctGlobal
{
	enum class CorneaLayerType : unsigned short
	{

	};

	constexpr unsigned short NUMBER_OF_CORNEA_LAYERS = 2;
	constexpr unsigned short NUMBER_OF_RETINA_LAYERS = 8;

	enum class RetinaLayerType : unsigned short
	{
		NONE = 0,
		ILM = 101,
		NFL = 102,
		IOS = 103,
		RPE = 104, 
		CHOROID = 105, 
		INNER, 
		OUTER
	};


	enum class OcularLayerType : unsigned short
	{
		NONE = 0,
		ILM = 101,
		NFL = 102,
		IPL = 103,
		OPL = 104,
		IOS = 105,
		RPE = 106,
		OPR = 107,
		BRM = 108,
		EPI = 201,
		BOW = 202,
		END = 203,
		BASE = 204,		//baseline for disc BMO Imgae
		INNER,
		OUTER
	};


	std::string OCTGLOBAL_DLL_API LayerTypeToString(OcularLayerType type);

	constexpr int LAYER_POINT_INVALID = -1;

	constexpr double RETINA_REFLECTIVE_INDEX = 1.36;
	constexpr double CORNEA_REFLECTIVE_INDEX = 1.376;
	constexpr double CORNEA_KERATO_REFLECTIVE_INDEX = 1.3375;
	constexpr double CORNEA_AQUEOUS_HUMOUR_REFLECTIVE_INDEX = 1.336;

	constexpr double INAIR_REFLECTIVE_INDEX = 1.0;

	constexpr float RETINA_SCAN_PIXEL_RESOLUTION = 3.03f; // 2.95f;
	constexpr float CORNEA_SCAN_PIXEL_RESOLUTION = 3.00f; // 3.03f;
	constexpr float INAIR_SCAN_PIXEL_RESOLUTION = (float)(CORNEA_SCAN_PIXEL_RESOLUTION*CORNEA_REFLECTIVE_INDEX);
	
	constexpr float ONH_DISC_MASK_VALUE = -1.0f;
	constexpr float ONH_CUP_MASK_VALUE = -2.0f;

	constexpr float MACULAR_CENTER_CIRCLE_SIZE = (1.0f * 1.1f);
	constexpr float MACULAR_INNER_CIRCLE_SIZE = (3.0f * 1.1f); // 2.0f;
	constexpr float MACULAR_OUTER_CIRCLE_SIZE = (6.0f * 1.1f); // 3.0f;
	constexpr int RETINA_THICKNESS_CHART_SECTORS = 4;

	constexpr float ONH_DISC_RNFL_CIRCLE_SIZE = (3.45f * 1.05f);
	constexpr float ONH_DISC_RNFL_345_SIZE = (3.45f * 1.1f);
	constexpr float ONH_DISC_RNFL_345_SIZE2 = 3.45f;
	constexpr int ONH_DISC_RNFL_CLOCKS = 12;
	constexpr int ONH_DISC_RNFL_QUADS = 4;

	constexpr float CORNEA_CENTER_CIRCLE_SIZE = 2.0f;
	constexpr float CORNEA_INNER_CIRCLE_SIZE = 5.0f;
	constexpr float CORNEA_OUTER_CIRCLE_SIZE = 6.0f;
	constexpr int CORNEA_THICKNESS_CHART_SECTORS = 8;

	constexpr float CHART_PERCENTILE_1 = (0.01f-0.001f);
	constexpr float CHART_PERCENTILE_5 = (0.05f-0.001f);
	constexpr float CHART_PERCENTILE_95 = (0.95f-0.001f);
	constexpr float CHART_PERCENTILE_99 = (0.99f-0.001f);
	constexpr float CHART_PERCENTILE_100 = (1.00f-0.001f);

	constexpr int TOPO_CALIBRATION_DATA_SIZE = 1024;
	constexpr int TOPO_BSCAN_COUNT = 16;
}