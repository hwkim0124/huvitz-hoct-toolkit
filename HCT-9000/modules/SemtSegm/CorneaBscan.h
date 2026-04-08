#pragma once
#include "LayeredBscan.h"

namespace SemtSegm
{
	class SEMTSEGM_DLL_API CorneaBscan :
		public LayeredBscan
	{
	public:
		CorneaBscan();
		virtual ~CorneaBscan();

		CorneaBscan(CorneaBscan&& rhs);
		CorneaBscan& operator=(CorneaBscan&& rhs);
		CorneaBscan(const CorneaBscan& rhs) = delete;
		CorneaBscan& operator=(const CorneaBscan& rhs) = delete;

	private:
		struct CorneaBscanImpl;
		std::unique_ptr<CorneaBscanImpl> d_ptr;
		CorneaBscanImpl& getImpl(void) const;
	};
}

