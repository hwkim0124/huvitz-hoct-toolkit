#pragma once

#include "SegmScanDef.h"

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

	class SEGMSCAN_DLL_API OcularEnfaceMap
	{
	public:
		OcularEnfaceMap();
		virtual ~OcularEnfaceMap();

		OcularEnfaceMap(OcularEnfaceMap&& rhs);
		OcularEnfaceMap& operator=(OcularEnfaceMap&& rhs);
		OcularEnfaceMap(const OcularEnfaceMap& rhs);
		OcularEnfaceMap& operator=(const OcularEnfaceMap& rhs);

	public:
		virtual bool setupThicknessMap(const OctScanPattern& pattern, 
										OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
										const std::vector<OcularBsegm*>& bsegms);
		virtual bool isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const;
		virtual bool isEmpty(void) const;

		CppUtil::CvImage makeImage(int width, int height) const;
		RetParam::EnfaceTmap* getThicknessMap(void) const;

	private:
		struct OcularEnfaceMapImpl;
		std::unique_ptr<OcularEnfaceMapImpl> d_ptr;
		OcularEnfaceMapImpl& getImpl(void) const;
	};
}

