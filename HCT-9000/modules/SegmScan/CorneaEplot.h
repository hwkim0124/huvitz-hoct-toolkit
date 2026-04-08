#pragma once

#include "SegmScanDef.h"
#include "CorneaThicknessMap.h"

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

	class SEGMSCAN_DLL_API CorneaEplot : public CorneaThicknessMap
	{
	public:
		CorneaEplot();
		virtual ~CorneaEplot();

		CorneaEplot(CorneaEplot&& rhs);
		CorneaEplot& operator=(CorneaEplot&& rhs);
		CorneaEplot(const CorneaEplot& rhs);
		CorneaEplot& operator=(const CorneaEplot& rhs);

	public:


	private:
		struct CorneaEplotImpl;
		std::unique_ptr<CorneaEplotImpl> d_ptr;
		CorneaEplotImpl& getImpl(void) const;
	};
}

