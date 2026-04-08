#pragma once


#include "OctGlobal2.h"

using namespace OctGlobal;

#include <memory>
#include <string>


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	constexpr OcularLayerType DECORR_UPPER_LAYER_TYPE = OcularLayerType::ILM;
	constexpr OcularLayerType DECORR_LOWER_LAYER_TYPE = OcularLayerType::BRM;
	constexpr float DECORR_UPPER_LAYER_OFFSET = 0.0f; 
	constexpr float DECORR_LOWER_LAYER_OFFSET = +240.0f;

	constexpr OcularLayerType VASCULAR_UPPER_LAYER_TYPE = OcularLayerType::NFL;
	constexpr OcularLayerType VASCULAR_LOWER_LAYER_TYPE = OcularLayerType::IPL;
	constexpr float VASCULAR_UPPER_LAYER_OFFSET = -0.0f;
	constexpr float VASCULAR_LOWER_LAYER_OFFSET = +15.0f;

	constexpr OcularLayerType VASCULAR_UPPER_LAYER_TYPE2 = OcularLayerType::NFL;
	constexpr OcularLayerType VASCULAR_LOWER_LAYER_TYPE2 = OcularLayerType::OPL;
	constexpr float VASCULAR_UPPER_LAYER_OFFSET2 = -0.0f;
	constexpr float VASCULAR_LOWER_LAYER_OFFSET2 = +0.0f;

	constexpr OcularLayerType SHADOWED_UPPER_LAYER_TYPE = OcularLayerType::IPL; // OcularLayerType::OPL; // OcularLayerType::OPL;
	constexpr OcularLayerType SHADOWED_LOWER_LAYER_TYPE = OcularLayerType::OPR; // OcularLayerType::BRM;
	constexpr float SHADOWED_UPPER_LAYER_OFFSET = -0.0f;
	constexpr float SHADOWED_LOWER_LAYER_OFFSET = +9.0f; // +0.0f;

	constexpr OcularLayerType SHADOWED_UPPER_LAYER_TYPE2 = OcularLayerType::OPL; // OcularLayerType::IPL; // OcularLayerType::OPL;
	constexpr OcularLayerType SHADOWED_LOWER_LAYER_TYPE2 = OcularLayerType::BRM; // OcularLayerType::BRM;
	constexpr float SHADOWED_UPPER_LAYER_OFFSET2 = -0.0f;
	constexpr float SHADOWED_LOWER_LAYER_OFFSET2 = +0.0f; // +9.0f; // +0.0f;

	constexpr OcularLayerType SUPERFICIAL_UPPER_LAYER_TYPE = OcularLayerType::ILM;
	constexpr OcularLayerType SUPERFICIAL_LOWER_LAYER_TYPE = OcularLayerType::IPL;
	constexpr float SUPERFICIAL_UPPER_LAYER_OFFSET = -0.0f;
	constexpr float SUPERFICIAL_LOWER_LAYER_OFFSET = +0.0f;

	constexpr OcularLayerType NONVASCULAR_UPPER_LAYER_TYPE = OcularLayerType::IOS;
	constexpr OcularLayerType NONVASCULAR_LOWER_LAYER_TYPE = OcularLayerType::RPE;
	constexpr float NONVASCULAR_UPPER_LAYER_OFFSET = -15.0f;
	constexpr float NONVASCULAR_LOWER_LAYER_OFFSET = +25.0f;

	constexpr float DIFFER_NORM_RANGE_MIN = 50.0f;
	constexpr float DIFFER_NORM_RANGE_MAX = 450.0f;
	constexpr float DIFFER_NORM_RANGE_MIN_OUTER = 100.0f;
	constexpr float DIFFER_NORM_RANGE_MAX_OUTER = 900.0f;

	constexpr float DECORR_NORM_RANGE_MIN = 0.025f;
	constexpr float DECORR_NORM_RANGE_MAX = 0.35f;
	constexpr float DECORR_NORM_RANGE_MIN_OUTER = 0.035f;
	constexpr float DECORR_NORM_RANGE_MAX_OUTER = 0.55f;

	constexpr float BSCAN_IMAGE_MEAN = 180.0f;
	constexpr float ANGIO_BSCAN_THRESH_MIN = 180.0f;
	constexpr float ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO = 0.5f; // 1.0f
	constexpr float ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_INNER = 0.5f; // 1.0f;
	constexpr float ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_OUTER = 0.5f; // 1.0f;
	constexpr float ANGIO_BSCAN_THRESH_MEAN_STDEV_RATIO_TOTAL = 0.5f; // 1.0f;

	constexpr int ANGIO_BRM_TO_CHOROID_HEIGHT = 45;
	constexpr int ANGIO_MODEL_EYE_MEAN_MAX = 320; // 180;

	constexpr int FILE_HEADER_SIZE = 256;
	constexpr int FILE_DSIZES_SIZE = 1024;
	constexpr int FILE_DATA_HEIGHT = 768;

	constexpr int ANGIO_PROJ_MASK_AVG_SIZE = 11; // 7; // 11;
	constexpr int ANGIO_PROJ_IMAGE_AVG_SIZE = 11; // 7;
	constexpr float ANGIO_PROJ_MASK_VALUE_MIN = 0.35F; // 0.35f; // 0.15f;

	constexpr float ANGIO_PROJ_DIFFER_NORM_RANGE_MIN = 5.0f; 
	constexpr float ANGIO_PROJ_DIFFER_NORM_RANGE_MAX = 97.5f;
	constexpr float ANGIO_PROJ_DIFFER_NORM_RANGE_MIN2 = 15.0;
	constexpr float ANGIO_PROJ_DIFFER_NORM_RANGE_MAX2 = 99.5f;
	constexpr float ANGIO_PROJ_DIFFER_NORM_SIZE_MIN = 900.0f; // 550.0f; // 1800.0f;
	constexpr float ANGIO_PROJ_DIFFER_NORM_SIZE_MIN2 = 1200.0f; // 700.0f; // 2500.0f;

	constexpr float ANGIO_PROJ_DECORR_NORM_RANGE_MIN = 10.0f;
	constexpr float ANGIO_PROJ_DECORR_NORM_RANGE_MAX = 97.5f;
	constexpr float ANGIO_PROJ_DECORR_NORM_RANGE_MIN2 = 35.0f; // 25.0f;
	constexpr float ANGIO_PROJ_DECORR_NORM_RANGE_MAX2 = 99.0f; // 99.5f;
	constexpr float ANGIO_PROJ_DECORR_NORM_SIZE_MIN = 0.10f; // 0.25f;
	constexpr float ANGIO_PROJ_DECORR_NORM_SIZE_MIN2 = 0.35f; // 0.15f; // 0.35f;

	constexpr int DECORR_IMAGE_HEIGHT = 768;
	constexpr float DECORR_VALUE_MIN = 0.0001f;
	constexpr float DECORR_PROJECT_REMOVE_RANGE_MAX = 6.0f;

	constexpr int MOTION_CORRECTION_VERSION = 101;


	typedef std::vector<std::vector<CppUtil::CvImage>> Amplitudes;
	typedef std::vector<std::vector<CppUtil::CvImage>> Imaginaries;
	typedef std::vector<std::vector<CppUtil::CvImage>> Reals;
	typedef std::vector<std::vector<int>> LayerArrays;
	typedef std::vector<int> LayerPoints;
	typedef std::vector<float> AngioProfile;
};


#ifdef __OCTANGIO_DLL
#define OCTANGIO_DLL_API		__declspec(dllexport)
#else
#define OCTANGIO_DLL_API		__declspec(dllimport)
#endif