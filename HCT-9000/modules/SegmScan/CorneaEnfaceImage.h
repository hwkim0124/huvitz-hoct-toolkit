#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class SEGMSCAN_DLL_API CorneaEnfaceImage : public OcularEnfaceImage
	{
	public:
		CorneaEnfaceImage();
		virtual ~CorneaEnfaceImage();

		CorneaEnfaceImage(CorneaEnfaceImage&& rhs);
		CorneaEnfaceImage& operator=(CorneaEnfaceImage&& rhs);
		CorneaEnfaceImage(const CorneaEnfaceImage& rhs);
		CorneaEnfaceImage& operator=(const CorneaEnfaceImage& rhs);

	public:

	private:
		struct CorneaEnfaceImageImpl;
		std::unique_ptr<CorneaEnfaceImageImpl> d_ptr;
		CorneaEnfaceImageImpl& getImpl(void) const;
	};
}