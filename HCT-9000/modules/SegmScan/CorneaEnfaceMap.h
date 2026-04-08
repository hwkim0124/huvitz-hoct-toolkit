#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceMap.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace RetParam {
	class EnfaceTmap;
}



namespace SegmScan
{
	class OcularBsegm;

	class SEGMSCAN_DLL_API CorneaEnfaceMap : public OcularEnfaceMap
	{
	public:
		CorneaEnfaceMap();
		virtual ~CorneaEnfaceMap();

		CorneaEnfaceMap(CorneaEnfaceMap&& rhs);
		CorneaEnfaceMap& operator=(CorneaEnfaceMap&& rhs);
		CorneaEnfaceMap(const CorneaEnfaceMap& rhs);
		CorneaEnfaceMap& operator=(const CorneaEnfaceMap& rhs);

	public:
		bool setupThicknessMap(const OctScanPattern& pattern,
								OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
								const std::vector<OcularBsegm*>& bsegms) override;
		bool setupRadiusMap(const OctScanPattern &pattern, const std::vector<OcularBsegm*>& bsegms);

		float getCurvatureRadiusOnCenterHorz(void) const;
		float getCurvatureRadiusOnCenterVert(void) const;
		float getAverageThicknessOnCenter(void) const;
	
	private:
		struct CorneaEnfaceMapImpl;
		std::unique_ptr<CorneaEnfaceMapImpl> d_ptr;
		CorneaEnfaceMapImpl& getImpl(void) const;
	};
}

