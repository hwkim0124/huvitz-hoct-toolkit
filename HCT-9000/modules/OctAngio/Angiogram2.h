#pragma once

#include "OctAngioDef.h"

#include <memory>


namespace CppUtil {
	class CvImage;
}


namespace OctAngio
{
	class AngioFile;
	class AngioData;
	class AngioLayout;
	class AngioLayers;
	class AngioDecorr;
	class AngioChart;
	class AngioMotion;
	class AngioPost;

	class OCTANGIO_DLL_API Angiogram2
	{
	public:
		Angiogram2();
		virtual ~Angiogram2();

		Angiogram2(Angiogram2&& rhs);
		Angiogram2& operator=(Angiogram2&& rhs);

	public:
		void resetLayout(int lines, int points, int repeats, bool vertical = false);
		void resetScanRange(float rangeX, float rangeY, float centerX=0.0f, float centerY=0.0f, bool isDisc=false);
		void resetSlabRange(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset);

		std::string importPath(void);
		void setImportPath(std::string path);

		bool loadDataBuffer(void);
		bool loadDataFiles(const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		bool loadDataFileVer2(const std::string dirPath = ".//export", const std::string fileName = "angio2.dat");
		bool loadDataImages(const std::string dirPath = ".//export");
		bool saveDataFiles(const std::string dirPath = ".//export", const std::string fileName = "angio.dat");
		bool saveDataFileVer2(const std::string dirPath = ".//export", const std::string fileName = "angio2.dat");

		bool isAmplitduesValid(void);
		bool isDecorrelationsValid(void);
		bool isAngioImage(void);
		bool isFoveaAvascularZone(void);
		bool isModelEyeImage(void);

		bool prepareFlowData(bool recalc, bool align, bool motion=false);
		bool estimateMotionVectors(void);

		bool loadLayerSegments(const std::string dirPath = "");
		bool loadLayerSegments(OcularLayerType upper, OcularLayerType lower, float upperOffset=0.0f, float lowerOffset=0.0f, const std::string dirPath = "");
		void setLayerSegments(AngioLayers&& layers);
		void setLayerSegments(OcularLayerType upper, OcularLayerType lower, 
								LayerArrays& upperLayers, LayerArrays& lowerLayers,
								LayerArrays& upperLayers2, LayerArrays& lowerLayers2, 
								LayerArrays& upperLayers3, LayerArrays& lowerLayers3);
		bool generateProjection(bool calcStats);

		bool buildImageBitmap(bool decorr, bool outerFlows, bool modelEye);
		const unsigned char* imageBits(void);
		int imageWidth(void) ;
		int imageHeight(void) ;

		CppUtil::CvImage createAnigoImage(void);
		CppUtil::CvImage createOffsetImage(void);
		CppUtil::CvImage createDecorrImage(int lineIdx);
		CppUtil::CvImage createScanImage(int lineIdx);
	
		std::vector<float> getDecorrelationsOnHorzLines(bool axialMax=false);
		std::vector<float> getDecorrelationsOnVertLines(bool axialMax=false);
		std::vector<int> getUpperLayerPoints(int imageIdx);
		std::vector<int> getLowerLayerPoints(int imageIdx);

		int numberOfDecorrImages(void);
		int numberOfOverlapImages(void);
		float* getDecorrImageBits(int lineIdx);

		unsigned char* getScanImageBits(int lineIdx, int repeatIdx=0);
		int scanImageWidth(void);
		int scanImageHeight(void);
		float scanRangeX(void);
		float scanRangeY(void);

	protected:
		void performBscansAlignment(void);
		void performMotionRegistration(void);
		
		bool produceFlowSignals(bool align, bool motion);
		bool buildProjectionMasks(void);
		bool buildProjectionImages(bool calcStats);

		bool processProjectionImages(void);
		bool processProjectionImages2(void);
		bool normalizeProjectionImages(void);

	public:
		bool& useAlignAxial(void);
		bool& useAlignLateral(void);
		bool& useDecorrCircular(void);

		bool& useLayersSelected(void);
		bool& useVascularLayers(void);
		bool& useMotionCorrection(void);
		bool& useBiasFieldCorrection(void);
		bool& usePostProcessing(void);
		bool& useNormProjection(void);
		bool& useDecorrOutput(void);
		bool& useDifferOutput(void);
		bool& useProjectionArtifactRemoval(void);
		bool& useReflectionCorrection(void);
		bool& useProjectionStatistics(void);

		float& intensityUpperThreshold(void);
		float& intensityLowerThreshold(void);
		float& decorrLowerThreshold(void);
		float& decorrUpperThreshold(void);
		float& differLowerThreshold(void);
		float& differUpperThreshold(void);

		float& decorrBaseThreshold(void);
		float& biasFieldSigma(void);
		float& noiseReductionRate(void);
		float& normalizeDropOff(void);

		int& numberOfOverlaps(void);
		bool& usePixelAverage(void);
		int& pixelAverageOffset(void);
		
		int& avgOffsetInnFlows(void);
		int& avgOffsetOutFlows(void);
		float& thresholdRatioInnFlows(void);
		float& thresholdRatioOutFlows(void);

		int& avgOffsetVascularMask(void);
		int& avgOffsetProjectionMask(void);
		float& weightProjectionMask(void);
		int& avgOffsetInnProjection(void);
		int& avgOffsetOutProjection(void);

		OctAngio::AngioLayout& Layout(void);
		OctAngio::AngioData& Data(void);
		OctAngio::AngioLayers& Layers(void);
		OctAngio::AngioLayers& Layers2(void);
		OctAngio::AngioLayers& Layers3(void);
		OctAngio::AngioDecorr& Decorr(void);
		OctAngio::AngioDecorr& Decorr2(void);
		OctAngio::AngioDecorr& Decorr3(void);
		OctAngio::AngioMotion& Motion(void);
		OctAngio::AngioPost& Post(void);

	private:
		struct Angiogram2Impl;
		std::unique_ptr<Angiogram2Impl> d_ptr;
		Angiogram2Impl& getImpl(void) const;
	};
}

