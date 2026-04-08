#pragma once
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

	class SEGMSCAN_DLL_API CorneaThicknessMap : public CorneaEnfaceMap
	{
	public:
		CorneaThicknessMap();
		virtual ~CorneaThicknessMap();

		CorneaThicknessMap(CorneaThicknessMap&& rhs);
		CorneaThicknessMap& operator=(CorneaThicknessMap&& rhs);
		CorneaThicknessMap(const CorneaThicknessMap& rhs);
		CorneaThicknessMap& operator=(const CorneaThicknessMap& rhs);

	public:


	private:
		struct CorneaThicknessMapImpl;
		std::unique_ptr<CorneaThicknessMapImpl> d_ptr;
		CorneaThicknessMapImpl& getImpl(void) const;
	};
}