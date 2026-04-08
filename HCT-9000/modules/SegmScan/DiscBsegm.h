#pragma once

#include "SegmScanDef.h"
#include "OcularBsegm.h"
#include "MacularBsegm.h"

#include <memory>
#include <vector>
#include <map>


namespace SegmScan
{
	class SEGMSCAN_DLL_API DiscBsegm : public MacularBsegm
	{
	public:
		DiscBsegm();
		virtual ~DiscBsegm();

		DiscBsegm(DiscBsegm&& rhs);
		DiscBsegm& operator=(DiscBsegm&& rhs);
		DiscBsegm(const DiscBsegm& rhs) = delete;
		DiscBsegm& operator=(const DiscBsegm& rhs) = delete;

	public:
		bool performAnalysis(bool meye = false) override;
		bool elaborateParams(const OcularLayer* layerILM, const OcularLayer* layerRPE);
		bool makeupRetinaBorders(bool isDisc, float rangeX);

		bool isOpticNerveDisc(void) const override;
		bool getOpticNerveDiscRange(int& x1, int& x2) const override;
		bool getOpticNerveDiscPixels(int& pixels) const override;
		void setOpticNerveDiscRange(int x1, int x2) override;
		void setOpticNerveDiscPixels(int pixels) override;
		int getOpticNerveDiscX1(void);
		int getOpticNerveDiscX2(void);

		bool isOpticNerveCup(void) const override;
		bool getOpticNerveCupRange(int& x1, int& x2) const override;
		bool getOpticNerveCupPixels(int& pixels) const override;
		void setOpticNerveCupRange(int x1, int x2) override;
		void setOpticNerveCupPixels(int pixels) override;
		void clearOpticNerveRange(void) override;

	protected:
		bool buildLayerILM(const OcularImage* imgSrc, OcularLayer* layerInn, OcularLayer * layerOut, OcularLayer* layerILM) override;
		bool buildLayerOut(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer * layerOut) override;
		bool buildLayerOPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIOS, OcularLayer* layerOPL) override;
		bool buildLayerIPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerOPL, OcularLayer* layerIPL) override;
		bool buildLayerNFL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIPL, OcularLayer* layerNFL) override;

		bool buildLayerOPR(const OcularImage* imgSrc, OcularLayer * layerILM, OcularLayer * layerOut, OcularLayer * layerOPR) override;
		bool buildLayerIOS(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer * layerOut, OcularLayer* layerOPR, OcularLayer* layerIOS) override;
		bool buildLayerBRM(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerOPR, OcularLayer* layerBRM) override;
		bool buildLayerRPE(const OcularImage* imgSrc, OcularLayer * layerIOS, OcularLayer * layerOPR, OcularLayer * layerBRM, OcularLayer* layerRPE) override;

		bool elaborateILM(const OcularImage* imgSrc, OcularLayer* layerNFL, OcularLayer* layerILM) override;
		bool elaborateNFL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerIPL, OcularLayer * layerOut, OcularLayer* layerNFL) override;
		bool elaborateIPL(const OcularImage* imgSrc, OcularLayer* layerILM, OcularLayer* layerNFL, OcularLayer* layerOPL, OcularLayer * layerOut, OcularLayer* layerIPL) override;
		bool elaborateOPL(const OcularImage* imgSrc, OcularLayer* layerIPL, OcularLayer* layerRPE, OcularLayer* layerOut, OcularLayer* layerOPL) override;

		bool elaborateIOS(const OcularImage* imgSrc, OcularLayer* layerOPL, OcularLayer* layerRPE, OcularLayer* layerBRM, OcularLayer* layerIOS) override;
		bool elaborateRPE(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerBRM, OcularLayer * layerOut, OcularLayer* layerRPE) override;
		bool elaborateBRM(const OcularImage* imgSrc, OcularLayer* layerIOS, OcularLayer* layerRPE, OcularLayer* layerBRM) override;


		bool estimateOpticDiscRegion(const OcularImage* imgSrc, const OcularImage* imgAsc, OcularLayer* layerInn, OcularLayer* layerOut);
		bool inflateOpticDiscRegion(void);
		bool assignOpticDiscRegion(const OcularLayer* layerILM, const OcularLayer* layerRPE);
		bool assignOpticCupRegion(const OcularLayer* layerILM, const OcularLayer* layerRPE);
		bool assignOpticRimRegion(const OcularLayer* layerILM, const OcularLayer* layerRPE);


	private:
		struct DiscBsegmImpl;
		std::unique_ptr<DiscBsegmImpl> d_ptr;
		DiscBsegmImpl& getImpl(void) const;
	};
}
