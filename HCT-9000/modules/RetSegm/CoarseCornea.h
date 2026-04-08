#pragma once

#include "RetSegmDef.h"
#include "Coarse.h"


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API CoarseCornea : public Coarse
	{
	public:
		CoarseCornea();
		~CoarseCornea();

	public:
		static bool createGradientMapOfCornea(const SegmImage* imgSrc, SegmImage* imgAsc, SegmImage* imgDes = nullptr, SegmImage* imgOut = nullptr);
		static bool createGradientMapOfChamber(const SegmImage* imgSrc, SegmImage* imgAsc, SegmImage* imgDes, SegmImage* imgOut = nullptr);
		static bool makeupCorneaBorderLines(const SegmImage* srcImg, const SegmImage* ascImg, const SegmImage* desImg, SegmLayer* layerInn, SegmLayer* layerOut, 
											float rangeX, int& corneaTopX, int& corneaTopY, int& wideHingeY, bool meye=false);

	public:
		static bool locateCenterReflection(const SegmImage* srcImg, float rangeX, int& reflect_x1, int& reflect_x2);
		static bool locateCenterEdgesOfCornea(const SegmImage* srcImg, const SegmImage* ascImg, std::vector<int>& inner, std::vector<int>& outer, 
												float rangeX, int refelct_x1=-1, int reflect_x2=-1);
		static bool locateCenterEdgesOfCorneaReversed(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX, int& centX, int& centY);

		static bool removeOutliersInCenterEdges(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX);
		static bool removeOutliersInCenterEdgesReversed(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, float rangeX);

		static bool composeInnerCurveByCenterEdges(std::vector<int>& inner, float rangeX, int& centerX, int& centerY);
		static bool composeInnerCurveByCenterEdgesReversed(std::vector<int>& inner, float rangeX, int& centerX, int& centerY);

		static bool locateInnerBorderLine(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, 
											float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY, bool anteriorLens=false);
		static bool locateInnerBorderLineBySides(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer,
											float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY, bool anteriorLens = false);
		static bool locateOuterBorderLine(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, 
											float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY);
		static bool locateOuterBorderLineForWide(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer,
											float rangeX, int hingeY, int reflectX1, int reflectX2, int centerX, int centerY);

		static bool removeOutliersInInnerBorderLine(std::vector<int>& inner, float rangeX, int hingeY, int centerX, int centerY);
		static bool removeFractionsInInnerBorderLine(std::vector<int>& inner, float rangeX, int centerX, int centerY);
		static bool removeFractionsInInnerEdgesLine(std::vector<int>& inner, float rangeX, int centerX, int centerY);
		
		static bool interpolateInnerBorderLine(std::vector<int>& inner, float rangeX, int hingeY, int& centerX, int& centerY);
		static bool interpolateOuterBorderLine(std::vector<int>& inner, std::vector<int>& outer, float rangeX, int hingeY, int& centerX, int& centerY, float dragWeight=0.0f);

		static bool estimateCenterReflection(std::vector<int>& inner, float rangeX, int centerX, int centerY, int& zone_x1, int& zone_x2);
		static bool estimateOuterCurveByInnerCurve(const SegmImage* srcImg, 
			std::vector<int>& inner, std::vector<int>& outer, float rangeX, 
			int centerX, int centerY, bool meye = false);
		static bool detectHingesInWideCornea(const SegmImage* srcImg, std::vector<int>& inner, float rangeX, int centerX, int centerY, int& hingeY);

		static bool removeInnerFractions(std::vector<int>& input, std::vector<int>& output, int& centX, int& centY);
		static bool removeInnerOutliers(std::vector<int>& input, std::vector<int>& output);
		static bool interpolateBoundaryByLinearFitting(std::vector<int>& input, std::vector<int>& output);
	};
}

