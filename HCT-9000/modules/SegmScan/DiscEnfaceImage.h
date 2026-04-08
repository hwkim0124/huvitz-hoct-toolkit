#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API DiscEnfaceImage : public OcularEnfaceImage
	{
	public:
		DiscEnfaceImage();
		virtual ~DiscEnfaceImage();

		DiscEnfaceImage(DiscEnfaceImage&& rhs);
		DiscEnfaceImage& operator=(DiscEnfaceImage&& rhs);
		DiscEnfaceImage(const DiscEnfaceImage& rhs);
		DiscEnfaceImage& operator=(const DiscEnfaceImage& rhs);

	public:

	private:
		struct DiscEnfaceImageImpl;
		std::unique_ptr<DiscEnfaceImageImpl> d_ptr;
		DiscEnfaceImageImpl& getImpl(void) const;
	};
}