#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API MacularEnfaceImage : public OcularEnfaceImage
	{
	public:
		MacularEnfaceImage();
		virtual ~MacularEnfaceImage();

		MacularEnfaceImage(MacularEnfaceImage&& rhs);
		MacularEnfaceImage& operator=(MacularEnfaceImage&& rhs);
		MacularEnfaceImage(const MacularEnfaceImage& rhs);
		MacularEnfaceImage& operator=(const MacularEnfaceImage& rhs);

	public:

	private:
		struct MacularEnfaceImageImpl;
		std::unique_ptr<MacularEnfaceImageImpl> d_ptr;
		MacularEnfaceImageImpl& getImpl(void) const;
	};
}