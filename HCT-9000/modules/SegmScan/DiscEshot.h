#pragma once

#include "SegmScanDef.h"
#include "DiscEnfaceImage.h"

#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace SegmScan
{
	class OcularBsegm;

	class SEGMSCAN_DLL_API DiscEshot : public DiscEnfaceImage
	{
	public:
		DiscEshot();
		virtual ~DiscEshot();

		DiscEshot(DiscEshot&& rhs);
		DiscEshot& operator=(DiscEshot&& rhs);
		DiscEshot(const DiscEshot& rhs);
		DiscEshot& operator=(const DiscEshot& rhs);

	public:

	private:
		struct DiscEshotImpl;
		std::unique_ptr<DiscEshotImpl> d_ptr;
		DiscEshotImpl& getImpl(void) const;
	};
}

