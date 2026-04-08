#pragma once

#include "SegmScanDef.h"
#include "OcularLayers.h"


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaLayers : public OcularLayers
	{
	public:
		CorneaLayers();
		virtual ~CorneaLayers();

		CorneaLayers(CorneaLayers&& rhs);
		CorneaLayers& operator=(CorneaLayers&& rhs);
		CorneaLayers(const CorneaLayers& rhs) ;
		CorneaLayers& operator=(const CorneaLayers& rhs) ;

	public:
		void initialize(unsigned int width, unsigned int height) override;
		std::vector<int> getLayerPoints(OcularLayerType layer, int width = 0, int height = 0) const override;
		void setLayerPoints(OcularLayerType layer, const std::vector<int>& points) override;

		bool isLayerPoints(OcularLayerType layer) const override;
		
		OcularLayer* getEPI(void) const override;
		OcularLayer* getBOW(void) const override;
		OcularLayer* getEND(void) const override;

		OcularLayer* getInner(void) const override;
		OcularLayer* getOuter(void) const override;
	
	private:
		struct CorneaLayersImpl;
		std::unique_ptr<CorneaLayersImpl> d_ptr;
		CorneaLayersImpl& getImpl(void) const;
	};
}
