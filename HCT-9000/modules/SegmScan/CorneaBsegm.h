#pragma once

#include "SegmScanDef.h"
#include "OcularBsegm.h"

#include <memory>
#include <vector>
#include <map>


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaBsegm : public OcularBsegm
	{
	public:
		CorneaBsegm();
		virtual ~CorneaBsegm();

		CorneaBsegm(CorneaBsegm&& rhs);
		CorneaBsegm& operator=(CorneaBsegm&& rhs);
		CorneaBsegm(const CorneaBsegm& rhs) = delete;
		CorneaBsegm& operator=(const CorneaBsegm& rhs) = delete;

	public:
		virtual bool performAnalysis(bool meye = false) override;
		bool performDewarping(void);

		void initializeLayers(int width = 0, int height = 0) override;
		void makeFittingCurvature(float rangeX);

		OcularLayer* layerInn(void) const;
		OcularLayer* layerOut(void) const;
		OcularLayer* layerEPI(void) const;
		OcularLayer* layerBOW(void) const;
		OcularLayer* layerEND(void) const;
		OcularLayer* layerBowForEpi = nullptr;
		OcularLayer* layerEpiForEpi = nullptr;

		bool getThicknessLine(OcularLayerType upper, OcularLayerType lower, std::vector<float_t>& thicks, int width = 0, int height = 0, float upperOffset = 0.0f, float lowerOffset = 0.0f)  const override;
		bool getCurvatureRadius(std::vector<float>& radius) const;
		bool setCurvatureRadius(const std::vector<float>& rads);
		bool calculateCorneaCurvature(void);
		bool getCorneaCurvature(std::vector<float>& output, int mapType) const;
		bool applyDewarpingImage(void);

		bool getIncorrectIdx(void);
		void resetIncorrectIdx(void);
	protected:
		bool prepareSample(const OcularImage* imgSrc, OcularImage* imgSample);
		bool createGradients(const OcularImage* imgSrc, OcularImage* imgGradients, OcularImage* imgAscends, OcularImage* imgDescends, bool isMeye);

		bool elaborateEPI(const OcularImage* imgSrc, OcularLayer* layerInn, OcularLayer* layerOut, OcularLayer* layerEPI);
		bool elaborateEND(const OcularImage* imgSrc, OcularLayer* layerEPI, OcularLayer* layerOut, OcularLayer* layerEND);
		bool elaborateBOW(const OcularImage* imgSrc, OcularLayer* layerEPI, OcularLayer* layerEND, OcularLayer* layerBOW);

		void elaborateParams(void);

	private:
		struct CorneaBsegmImpl;
		std::unique_ptr<CorneaBsegmImpl> d_ptr;
		CorneaBsegmImpl& getImpl(void) const;
	};
}
