#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class OCTRESULT_DLL_API FundusPhoto
	{
	public:
		FundusPhoto();
		virtual ~FundusPhoto();

		FundusPhoto(FundusPhoto&& rhs);
		FundusPhoto& operator=(FundusPhoto&& rhs);
		FundusPhoto(const FundusPhoto& rhs);
		FundusPhoto& operator=(const FundusPhoto& rhs);


	private:
		struct FundusPhotoImpl;
		std::unique_ptr<FundusPhotoImpl> d_ptr;
		FundusPhotoImpl& getImpl(void) const;
	};


}
