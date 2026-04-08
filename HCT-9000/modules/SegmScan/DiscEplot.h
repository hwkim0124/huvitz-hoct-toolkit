#pragma once

#include "SegmScanDef.h"
#include "DiscThicknessMap.h"

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

	class SEGMSCAN_DLL_API DiscEplot : public DiscThicknessMap
	{
	public:
		DiscEplot();
		virtual ~DiscEplot();

		DiscEplot(DiscEplot&& rhs);
		DiscEplot& operator=(DiscEplot&& rhs);
		DiscEplot(const DiscEplot& rhs);
		DiscEplot& operator=(const DiscEplot& rhs);

	public:

	private:
		struct DiscEplotImpl;
		std::unique_ptr<DiscEplotImpl> d_ptr;
		DiscEplotImpl& getImpl(void) const;
	};
}


