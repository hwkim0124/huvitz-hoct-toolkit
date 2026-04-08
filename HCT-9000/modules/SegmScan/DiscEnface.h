#pragma once

#include "SegmScanDef.h"
#include "OcularEnfaceImage.h"

#include <memory>
#include <vector>

namespace SegmScan
{
	class DiscBsegm;

	class SEGMSCAN_DLL_API DiscEnface : public OcularEnfaceImage
	{
	public:
		DiscEnface();
		virtual ~DiscEnface();

		DiscEnface(DiscEnface&& rhs);
		DiscEnface& operator=(DiscEnface&& rhs);
		DiscEnface(const DiscEnface& rhs);
		DiscEnface& operator=(const DiscEnface& rhs);

	private:
		struct DiscEnfaceImpl;
		std::unique_ptr<DiscEnfaceImpl> d_ptr;
		DiscEnfaceImpl& getImpl(void) const;
	};
}



