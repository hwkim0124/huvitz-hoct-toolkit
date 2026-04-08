#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>

namespace SegmScan
{
	class CorneaBsegm;

	class SEGMSCAN_DLL_API CorneaEnface : public OcularEnfaceImage
	{
	public:
		CorneaEnface();
		virtual ~CorneaEnface();

		CorneaEnface(CorneaEnface&& rhs);
		CorneaEnface& operator=(CorneaEnface&& rhs);
		CorneaEnface(const CorneaEnface& rhs);
		CorneaEnface& operator=(const CorneaEnface& rhs);

	private:
		struct CorneaEnfaceImpl;
		std::unique_ptr<CorneaEnfaceImpl> d_ptr;
		CorneaEnfaceImpl& getImpl(void) const;
	};
}



