#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class FundusPhoto;

	class OCTRESULT_DLL_API FundusData
	{
	public:
		FundusData();
		virtual ~FundusData();

		FundusData(FundusData&& rhs);
		FundusData& operator=(FundusData&& rhs);
		FundusData(const FundusData& rhs);
		FundusData& operator=(const FundusData& rhs);

	public:
		int addPhoto(FundusPhoto& image);
		int addPhoto(FundusPhoto&& image);
		FundusPhoto* getPhoto(int index);
		int getPhotoCount(void) const;
		void clearAllPhotos(void);

	private:
		struct FundusDataImpl;
		std::unique_ptr<FundusDataImpl> d_ptr;
		FundusDataImpl& getImpl(void) const;
	};

	typedef std::vector<FundusPhoto> FundusPhotoList;
}

