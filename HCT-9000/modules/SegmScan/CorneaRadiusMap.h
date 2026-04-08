#pragma once

#include "SegmScanDef.h"
#include "CorneaEnfaceMap.h"

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

	class SEGMSCAN_DLL_API CorneaRadiusMap : public CorneaEnfaceMap
	{
	public:
		CorneaRadiusMap();
		virtual ~CorneaRadiusMap();

		CorneaRadiusMap(CorneaRadiusMap&& rhs);
		CorneaRadiusMap& operator=(CorneaRadiusMap&& rhs);
		CorneaRadiusMap(const CorneaRadiusMap& rhs);
		CorneaRadiusMap& operator=(const CorneaRadiusMap& rhs);

	public:


	private:
		struct CorneaRadiusMapImpl;
		std::unique_ptr<CorneaRadiusMapImpl> d_ptr;
		CorneaRadiusMapImpl& getImpl(void) const;
	};
}