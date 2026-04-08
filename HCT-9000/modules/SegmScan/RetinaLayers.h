#pragma once

#include "SegmScanDef.h"
#include "OcularLayers.h"


namespace SegmScan
{
	class SEGMSCAN_DLL_API RetinaLayers : public OcularLayers
	{
	public:
		RetinaLayers();
		virtual ~RetinaLayers();

		RetinaLayers(RetinaLayers&& rhs);
		RetinaLayers& operator=(RetinaLayers&& rhs);
		RetinaLayers(const RetinaLayers& rhs) ;
		RetinaLayers& operator=(const RetinaLayers& rhs) ;

	public:
		void initialize(unsigned int width, unsigned int height) override;
		std::vector<int> getLayerPoints(OcularLayerType layer, int width=0, int height=0) const override;
		void setLayerPoints(OcularLayerType layer, const std::vector<int>& points) override;
		bool isLayerPoints(OcularLayerType layer) const override;

		OcularLayer* getILM(void) const override;
		OcularLayer* getNFL(void) const override;
		OcularLayer* getIPL(void) const override;
		OcularLayer* getOPL(void) const override;
		OcularLayer* getIOS(void) const override;
		OcularLayer* getRPE(void) const override;
		OcularLayer* getBRM(void) const override;
		OcularLayer* getBASE(void) const override;

		OcularLayer* getInner(void) const override;
		OcularLayer* getOuter(void) const override;
		OcularLayer* getOPR(void) const override;

	private:
		struct RetinaLayersImpl;
		std::unique_ptr<RetinaLayersImpl> d_ptr;
		RetinaLayersImpl& getImpl(void) const;
	};
}
