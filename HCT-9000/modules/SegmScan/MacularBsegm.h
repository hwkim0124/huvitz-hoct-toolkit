#pragma once

#include "SegmScanDef.h"
#include "OcularBsegm.h"

#include <memory>
#include <vector>
#include <map>


namespace SegmScan
{
	class SEGMSCAN_DLL_API MacularBsegm : public OcularBsegm
	{
	public:
		MacularBsegm();
		virtual ~MacularBsegm();

		MacularBsegm(MacularBsegm&& rhs);
		MacularBsegm& operator=(MacularBsegm&& rhs);
		MacularBsegm(const MacularBsegm& rhs) = delete;
		MacularBsegm& operator=(const MacularBsegm& rhs) = delete;

	public:
		virtual bool performAnalysis(bool meye = false) override;
		virtual void initializeLayers(int width = 0, int height = 0) override;

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
		bool prepareSample(const OcularImage* imgSrc, OcularImage* imgSample);
		bool createGradients(const OcularImage* imgSrc, OcularImage* imgGradients, OcularImage* imgAscends, OcularImage* imgDescends);
		bool makeupRetinaBorders(bool isDisc, float rangeX);

		bool inflateOpticDiscRegion(void);

		virtual bool buildLayerILM(const OcularImage* imgSrc, OcularLayer* layerInn, OcularLayer * layerOut, OcularLayer* layerILM);
		virtual bool buildLayerOut(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer * layerOut);
		virtual bool buildLayerOPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIOS, OcularLayer* layerOPL);
		virtual bool buildLayerIPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerOPL, OcularLayer* layerIPL);
		virtual bool buildLayerNFL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIPL, OcularLayer* layerNFL);

		virtual bool buildLayerOPR(const OcularImage* imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR);
		virtual bool buildLayerIOS(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer * layerOut, OcularLayer* layerOPR, OcularLayer* layerIOS);
		virtual bool buildLayerBRM(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerOPR, OcularLayer* layerBRM);
		virtual bool buildLayerRPE(const OcularImage* imgSrc, OcularLayer * layerIOS, OcularLayer * layerOPR, OcularLayer * layerBRM, OcularLayer* layerRPE);

		virtual bool elaborateILM(const OcularImage* imgSrc, OcularLayer* layerNFL, OcularLayer* layerILM);
		virtual bool elaborateNFL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIPL, OcularLayer* layerOut, OcularLayer* layerNFL);
		virtual bool elaborateIPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerNFL, OcularLayer* layerOPL, OcularLayer* layerOut, OcularLayer* layerIPL);
		virtual bool elaborateOPL(const OcularImage* imgSrc, OcularLayer* layerIPL, OcularLayer* layerRPE, OcularLayer* layerOut, OcularLayer* layerOPL);

		virtual bool elaborateIOS(const OcularImage* imgSrc, OcularLayer* layerOPL, OcularLayer* layerRPE, OcularLayer* layerBRM, OcularLayer* layerIOS);
		virtual bool elaborateRPE(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerBRM, OcularLayer* layerOut, OcularLayer* layerRPE);
		virtual bool elaborateBRM(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerRPE, OcularLayer* layerBRM);

		virtual void reinstateBRM(const OcularImage* imgSrc, OcularLayer* layerOPR, OcularLayer* layerBRM, bool isDisc);
		virtual void initializeBASE(const OcularImage* imgSrc, OcularLayer* layerRPE, OcularLayer* layerBASE);


	private:
		struct MacularBsegmImpl;
		std::unique_ptr<MacularBsegmImpl> d_ptr;
		MacularBsegmImpl& getImpl(void) const;
	};
}

