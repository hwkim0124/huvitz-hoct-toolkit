#pragma once

#include "SegmScanDef.h"
#include "MacularEnfaceImage.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace SegmScan
{
	class OcularBsegm;

	class SEGMSCAN_DLL_API MacularEshot : public MacularEnfaceImage
	{
	public:
		MacularEshot();
		virtual ~MacularEshot();

		MacularEshot(MacularEshot&& rhs);
		MacularEshot& operator=(MacularEshot&& rhs);
		MacularEshot(const MacularEshot& rhs);
		MacularEshot& operator=(const MacularEshot& rhs);

	public:

	private:
		struct MacularEshotImpl;
		std::unique_ptr<MacularEshotImpl> d_ptr;
		MacularEshotImpl& getImpl(void) const;
	};
}

