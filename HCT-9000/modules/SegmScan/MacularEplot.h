#pragma once

#include "SegmScanDef.h"
#include "MacularThicknessMap.h"

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

	class SEGMSCAN_DLL_API MacularEplot : public MacularThicknessMap
	{
	public:
		MacularEplot();
		virtual ~MacularEplot();

		MacularEplot(MacularEplot&& rhs);
		MacularEplot& operator=(MacularEplot&& rhs);
		MacularEplot(const MacularEplot& rhs);
		MacularEplot& operator=(const MacularEplot& rhs);

	public:

	private:
		struct MacularEplotImpl;
		std::unique_ptr<MacularEplotImpl> d_ptr;
		MacularEplotImpl& getImpl(void) const;
	};
}


