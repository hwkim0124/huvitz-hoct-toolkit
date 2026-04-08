#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>
#include <map>


namespace RetSegm {
	class SegmLayer;
}


namespace SegmScan
{
	using OcularLayer = RetSegm::SegmLayer;


	class SEGMSCAN_DLL_API OcularLayers
	{
	public:
		OcularLayers();
		virtual ~OcularLayers();

		OcularLayers(OcularLayers&& rhs);
		OcularLayers& operator=(OcularLayers&& rhs);
		OcularLayers(const OcularLayers& rhs) ;
		OcularLayers& operator=(const OcularLayers& rhs) ;

	public:
		virtual void initialize(unsigned int width, unsigned int height);
		virtual std::vector<int> getLayerPoints(OcularLayerType layer, int width = 0, int height = 0) const;
		virtual void setLayerPoints(OcularLayerType layer, const std::vector<int>& points);
		virtual bool isLayerPoints(OcularLayerType layer) const;
	
		virtual OcularLayer* getILM(void) const;
		virtual OcularLayer* getNFL(void) const;
		virtual OcularLayer* getIPL(void) const;
		virtual OcularLayer* getOPL(void) const;
		virtual OcularLayer* getIOS(void) const;
		virtual OcularLayer* getRPE(void) const;
		virtual OcularLayer* getBRM(void) const;

		virtual OcularLayer* getBASE(void) const;
		virtual OcularLayer* getInner(void) const;
		virtual OcularLayer* getOuter(void) const;
		virtual OcularLayer* getOPR(void) const;

		virtual OcularLayer* getEPI(void) const;
		virtual OcularLayer* getBOW(void) const;
		virtual OcularLayer* getEND(void) const;

		void applyRegionSize(unsigned int width, unsigned int height);
		void setRegionSize(unsigned int width, unsigned int height);

	protected:
		OcularLayer* getLayer(unsigned short type) const;
		std::vector<int> getLayerPoints(unsigned short type, int width = 0, int height = 0) const;
		void setLayerPoints(unsigned short type, const std::vector<int>& points);

		void initializeAsRetina(unsigned int width, unsigned int height);
		void initializeAsCornea(unsigned int width, unsigned int height);

	public:
		int getNumberOfLayers(void);
		int getRegionWidth(void);
		int getRegionHeight(void);
 		void clear(void);

	private:
		struct OcularLayersImpl;
		std::unique_ptr<OcularLayersImpl> d_ptr;
		OcularLayersImpl& getImpl(void) const;
	};
}
