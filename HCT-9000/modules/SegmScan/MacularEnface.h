#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>


namespace SegmScan
{
	class MacularBsegm;

	class SEGMSCAN_DLL_API MacularEnface : public OcularEnfaceImage
	{
	public:
		MacularEnface();
		virtual ~MacularEnface();

		MacularEnface(MacularEnface&& rhs);
		MacularEnface& operator=(MacularEnface&& rhs);
		MacularEnface(const MacularEnface& rhs);
		MacularEnface& operator=(const MacularEnface& rhs);

	public:

	private:
		struct MacularEnfaceImpl;
		std::unique_ptr<MacularEnfaceImpl> d_ptr;
		MacularEnfaceImpl& getImpl(void) const;
	};
}

