#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>
#include <map>


namespace RetSegm {
	class SegmLayer;
	class SegmImage;
}


namespace SegmScan
{
	using OcularImage = RetSegm::SegmImage;
	using OcularLayer = RetSegm::SegmLayer;

	class OcularLayers;
	class RetinaLayers;
	class CorneaLayers;


	class SEGMSCAN_DLL_API OcularBsegm
	{
	public:
		OcularBsegm();
		virtual ~OcularBsegm();

		OcularBsegm(OcularBsegm&& rhs);
		OcularBsegm& operator=(OcularBsegm&& rhs);
		OcularBsegm(const OcularBsegm& rhs) = delete;
		OcularBsegm& operator=(const OcularBsegm& rhs) = delete;

	public:
		enum class OcularImageType {
			SOURCE = 0,
			SAMPLE, 
			DENOISED,
			GRADIENTS, 
			ASCENDS, 
			DESCENDS
		};

	public:
		bool loadSource(const unsigned char* bits, int width, int height, int padding = 0);
		bool isEmpty(void) const;
		bool isResult(void) const;
		void setResult(bool flag);

		void setImageIndex(int index); 
		int getImageIndex(void) const;

		int getSourceWidth(void) const;
		int getSourceHeight(void) const;

		void setPatternDescript(OctScanPattern desc);
		const OctScanPattern& getPatternDescript(void) const;
		float getResolutionX(bool inMM=false) const;
		float getResolutionY(bool inAir=false) const;

		float getSampleWidthRatio(void) const;
		float getSampleHeightRatio(void) const;

		OcularImage* getImageObject(OcularImageType type = OcularImageType::SOURCE);
		OcularLayers* getOcularLayers(void) const;
		void setOcularLayers(std::unique_ptr<OcularLayers>&& layers);

		virtual bool isOpticNerveDisc(void) const;
		virtual bool getOpticNerveDiscRange(int& x1, int& x2) const;
		virtual bool getOpticNerveDiscPixels(int& pixels) const;
		virtual void setOpticNerveDiscRange(int x1, int x2);
		virtual void setOpticNerveDiscPixels(int pixels);

		virtual bool elaborateParams(const OcularLayer* layerILM, const OcularLayer* layerRPE);
		virtual bool isOpticNerveCup(void) const;
		virtual bool getOpticNerveCupRange(int& x1, int& x2) const;
		virtual bool getOpticNerveCupPixels(int& pixels) const;
		virtual void setOpticNerveCupRange(int x1, int x2);
		virtual void setOpticNerveCupPixels(int pixels);
		virtual void clearOpticNerveRange(void);
		virtual bool getCurvatureRadius(std::vector<float>& radius) const;
		virtual bool getCorneaCurvature(std::vector<float>& output, int mapType) const;
		virtual bool setCurvatureRadius(const std::vector<float>& rads);

		virtual void initializeLayers(int width=0, int height=0);
		virtual RetinaLayers* getRetinaLayers(void) const;
		virtual CorneaLayers* getCorneaLayers(void) const;
		virtual bool performAnalysis(bool meye = false);

		virtual bool getThicknessLine(OcularLayerType upper, OcularLayerType lower, std::vector<std::float_t>& thicks, int width = 0, int height = 0, float upperOffset = 0.0f, float lowerOffset = 0.0f) const;
		virtual bool getDistanceLine(OcularLayerType upper, OcularLayerType lower, std::vector<std::float_t>& thicks, int width = 0, int height = 0) const;
		virtual bool getLateralLine(OcularLayerType upper, OcularLayerType lower, std::vector<std::uint8_t>& pixels, float upperOffset = 0.0f, float lowerOffset = 0.0f);
		virtual bool getMaxValueLine(OcularLayerType upper, OcularLayerType lower, std::vector<std::uint8_t>& pixels);
		
		OcularLayer* layerInn(void) const;
		OcularLayer* layerOut(void) const;
		OcularLayer* layerILM(void) const;
		OcularLayer* layerNFL(void) const;
		OcularLayer* layerIPL(void) const;
		OcularLayer* layerOPL(void) const;
		OcularLayer* layerOPR(void) const;
		OcularLayer* layerIOS(void) const;
		OcularLayer* layerRPE(void) const;
		OcularLayer* layerBRM(void) const;
		OcularLayer* layerBASE(void) const;

	protected:
		OcularImage* resized(void) const;
		OcularImage* sample(void) const;
		OcularImage* smoothed(void) const;
		OcularImage* source(void) const;
		OcularImage* gradients(void) const;
		OcularImage* ascends(void) const;
		OcularImage* descends(void) const;
		OcularImage* costs(void) const;

		void setSample(OcularImage&& image);
		void setAscends(OcularImage&& image);
		void setDescends(OcularImage&& image);

	private:
		struct OcularBsegmImpl;
		std::unique_ptr<OcularBsegmImpl> d_ptr;
		OcularBsegmImpl& getImpl(void) const;
	};
}

