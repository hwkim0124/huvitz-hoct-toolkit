#pragma once

#include "RetSegmDef.h"


namespace RetSegm
{
	class ImageMat;
	class Boundary;

	class SegmImage;
	class SegmLayer;

	class RETSEGM_DLL_API Coarse
	{
	public:
		Coarse();

	public:
		static bool makeBoundaryLinesOfRetina(const SegmImage* imgSrc, const SegmImage* imgAsc, SegmLayer* layerInn, SegmLayer* layerOut, bool isDisk = false);
		static bool locateBoundaryEdgesOfRetina(const SegmImage* imgSrc, const SegmImage* imgAsc, std::vector<int>& inner, std::vector<int>& outer);

		static void removeInnerBoundaryOutliers(std::vector<int>& inner, std::vector<int>& outer);
		static void removeOuterBoundaryOutliers(std::vector<int>& inner, std::vector<int>& outer);
		static bool replaceFalseInnerPoints(const SegmImage* imgSrc, std::vector<int>& inner, std::vector<int>& outer);

		static bool removeInnerLineFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk = false);
		static bool removeOuterLineFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk = false);
		static bool removeInnerLineOutliers(const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& output);
		static bool removeOuterLineOutliers(std::vector<int>& inner, std::vector<int>& outer, bool isDisc = false);

		static bool interpolateBoundaryByLinearFitting(const std::vector<int>& input, std::vector<int>& output, bool sideFitt = false);
		static bool interpolateBoundaryByCompare(std::vector<int>& input, std::vector<int>& output, bool upper = true);
		static bool smoothBoundaryLine(std::vector<int>& input, std::vector<int>& output, float filtSize);

		static bool createGradientMapOfRetina(const SegmImage* imgSrc, SegmImage* imgAsc, SegmImage* imgDes, SegmImage* imgOut, bool isDisk = false);
		static bool createGradientMapOfCornea(const SegmImage* imgSrc, SegmImage* imgAsc, SegmImage* imgDes, SegmImage* imgOut, bool isDisk = false);

		static bool createGraidentMap(const SegmImage* imgSrc, SegmImage* imgOut, int rows, int cols, bool descent);
		static bool createAverageMap(const SegmImage* imgSrc, SegmImage* imgOut, int rows, int cols);

		static bool makeInnerBoundaryLineOfCornea(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut);
		static bool makeOuterBoundaryLineOfCornea(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut);

		static bool locateInnerBoundaryOfCornea(const SegmImage* imgSrc, std::vector<int>& inner, std::vector<int>& outer);
		static bool locateOuterBoundaryOfCornea(const SegmImage* imgSrc, std::vector<int>& inner, std::vector<int>& outer);


		static bool correctFalseOuterPoints(std::vector<int>& inner, std::vector<int>& outer);
		static bool removeInnerFractionsOfCornea(std::vector<int>& input, std::vector<int>& output);
		static bool removeInnerFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk = false);
		static bool removeOuterFractions(std::vector<int>& input, std::vector<int>& output, bool isDisk = false);
		static bool removeInnerOutliers(std::vector<int>& input, std::vector<int>& output);

		static bool removeInnerOutliersOfCornea(std::vector<int>& input, std::vector<int>& output);

		static bool makeFittingCurveOfCornea(std::vector<int>& input, std::vector<int>& output);
		static bool makeCurveRadiusOfCornea(float resolutionX, float resolutionY, float rangeX, std::vector<int>& input, std::vector<float>& output, bool isAxial);
		static bool makeBowmanLayerOfCornea(const SegmImage* imgSrc, const std::vector<int>& inner, const std::vector<int>& outer, std::vector<int>& output);

		static bool checkIfPointsAvailable(std::vector<int>& input, float sizeMin);
		static bool smoothBoundaryOfRetina(std::vector<int>& input, std::vector<int>& output, float filtSize);

	public:
		static bool createGradientMap(const SegmImage* imgSrc, SegmImage* imgOut);
		static bool buildBoundaryPairOfRetina(const SegmImage* imgSrc, SegmLayer* layerInn, SegmLayer* layerOut);

	private:
		static bool locateBoundaryPeaksOfRetina(const SegmImage* srcImg, std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra);
		static bool correctFalseOuterPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra);
		static bool replaceIntrudedInnerPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra);
		static bool replaceIntrudedOuterPoints(std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra);
		static bool removeInnerFractions(std::vector<int>& inner);
		static bool removeOuterFractions(std::vector<int>& outer);

		static bool locateBoundaryPointsOfRetina(ImageMat& srcImg, std::vector<int>& inner, std::vector<int>& outer, std::vector<int>& extra);
		static bool interpolateBoundaryByLinearFitting(Boundary& inBound, Boundary& outBound);
	public:
		static bool createGradientMap(ImageMat& srcImg, ImageMat& outImg);
		static bool buildBoundaryPairOfRetina(ImageMat& srcImg, Boundary& innerBound, Boundary& outerBound, Boundary& boundRPE);

	public:
		static bool calculateCurvatureOfCornea(float resolutionX, float resolutionY, std::vector<int>& layerEPI, std::vector<int>& layerBOW, std::vector<int>& layerEND, std::vector<int>& layerEpiForEpi, std::vector<int>& layerBowForEpi,
			std::vector<float>& axialAnteriorRadius, std::vector<float>& axialAnteriorRadiusSimK, std::vector<float>& axialPosteriorRadius, std::vector<float>& tangentialAnteriorRadius, std::vector<float>& tangentialPosteriorRadius,
			std::vector<float>& refractivePowerKerato, std::vector<float>& refractivePowerAnterior, std::vector<float>& refractivePowerPosterior, std::vector<float>& refractivePowerTotal, 
			std::vector<float>& netMap, std::vector<float>& axialTrueNet, std::vector<float>& equivalentKeratometer, std::vector<float>& elevationAnterior, std::vector<float>& elevationPosterior,
			std::vector<float>& height, std::vector<float>& pachymetry, std::vector<float>& epithelium, std::vector<double>& incidenceAngle, int bscanIndex, bool modelEye = false);

		static void getCurvature(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& tangentialRadius, std::vector<float>& axialRadius, std::vector<double>& incidenceAngle, int& axialX, int step, bool isAnterior);
		static void getCurvatureSimK(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& axialRadius);

		static void correctionPeripheral(std::vector<float>&curveR, float minR, float maxR, int ofsset);
		static void correctionCenter(std::vector<float>& curveR, int centerIndex, int ofsset);
	
		static void correctCurveCenter(std::vector<float>& dataY, double sigma, bool isOpen);
		static void getGaussianDerivs(double sigma, int M, std::vector<double>& gaussian, std::vector<double>& dg, std::vector<double>& d2g);
		static void getdX(std::vector<double> x, int n, double sigma, double & gx, double & dgx, double & d2gx, std::vector<double> g, std::vector<double> dg, std::vector<double> d2g, bool isOpen = true);
		static void getdXcurve(std::vector<double> x, double sigma, std::vector<double>& gx, std::vector<double>& dx, std::vector<double>& d2x, std::vector<double> g, std::vector<double> dg, std::vector<double> d2g, bool isOpen = true);

		static bool getAxialCenter(std::vector<double>& dataX, std::vector<double>& dataY, double & axialX, double & axialY);
		static bool calculateAnteriorTangential(std::vector<double>& dataX, std::vector<double>& dataEPI, std::vector<float>& tangentailAnt, std::vector<double>& incidenceAngle, int splitRange);
		static bool calculateAnteriorCurvature(std::vector<double>& dataX, std::vector<double>& dataEPI, std::vector<float>& tangentialAnt, std::vector<float>& axialAnt, std::vector<float>& refractiveAnt, std::vector<float>& refractiveKerato, std::vector<double>& incidenceAngle, double centerX);
		static bool calculatePosteriorTangential(std::vector<double>& dataX, std::vector<double>& dataEND, std::vector<float>& tangentailPost, std::vector<double>& incidenceAngle, int splitRange);
		static bool calculatePosteriorCurvature(std::vector<double>& dataX, std::vector<double>& dataEND, std::vector<float>& tangentailPost, std::vector<float>& axialPost, std::vector<float>& refractivePost, std::vector<double>& incidenceAngle, double centerX);
		static bool dewarpingCornea(std::vector<double>&dataX, std::vector<double>& dewarpX, std::vector<double>& layerUp, std::vector<double>& layerDown, std::vector<double>& incidenceAngle);
		static bool calculateThickness(std::vector<double>& dataX, std::vector<double>& dewarpX, std::vector<double>& layerUp, std::vector<double>& LayerDown, std::vector<double>& theta, std::vector<float>& thickness);
		static bool calculateElevation(std::vector<double>& dataX, std::vector<double>& dataY, std::vector<float>& elevation);
			
	};
}
