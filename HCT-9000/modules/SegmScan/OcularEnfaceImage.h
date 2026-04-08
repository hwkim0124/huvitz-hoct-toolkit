#pragma once

#include "SegmScanDef.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}

namespace SegmScan
{
	class OcularBsegm;

	class SEGMSCAN_DLL_API OcularEnfaceImage 
	{
	public:
		OcularEnfaceImage();
		virtual ~OcularEnfaceImage();

		OcularEnfaceImage(OcularEnfaceImage&& rhs);
		OcularEnfaceImage& operator=(OcularEnfaceImage&& rhs);
		OcularEnfaceImage(const OcularEnfaceImage& rhs);
		OcularEnfaceImage& operator=(const OcularEnfaceImage& rhs);

	public:
		virtual bool setupEnfaceImage(const OctScanPattern& pattern,
									OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset,
									const std::vector<OcularBsegm*>& bsegms);
		virtual bool isIdentical(OcularLayerType upper, OcularLayerType lower, float upperOffset, float lowerOffset) const;

		CppUtil::CvImage makeImage(int width, int height, bool enhance) const;

	private:
		struct OcularEnfaceImageImpl;
		std::unique_ptr<OcularEnfaceImageImpl> d_ptr;
		OcularEnfaceImageImpl& getImpl(void) const;
	};
}

