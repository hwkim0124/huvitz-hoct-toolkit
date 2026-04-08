#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>


namespace OctResult
{
	class OCTRESULT_DLL_API FundusImage
	{
	public:
		FundusImage();
		FundusImage(const OctFundusImage& desc);
		FundusImage(const std::uint8_t* data, int width, int height);
		virtual ~FundusImage();

		FundusImage(FundusImage&& rhs);
		FundusImage& operator=(FundusImage&& rhs);
		FundusImage(const FundusImage& rhs);
		FundusImage& operator=(const FundusImage& rhs);

	public:
		void setData(const std::uint8_t* data, int width, int height);
		void setData(const OctFundusImage& desc);

		const std::uint8_t* getBuffer(void) const;
		const OctFundusImage& getDescript(void) const;

		bool isEmpty(void) const;
		int getWidth(void) const;
		int getHeight(void) const;

		bool exportFile(unsigned int imgQuality, const std::wstring& path);
		bool exportThumbnail(const std::wstring& path, bool removeReflectionLight = false, float ratio = FUNDUS_THUMBNAIL_SIZE_RATIO);
		bool importFile(const std::wstring& path);

	private:
		struct FundusImageImpl;
		std::unique_ptr<FundusImageImpl> d_ptr;
		FundusImageImpl& getImpl(void) const;
	};

	typedef std::vector<FundusImage> FundusImageList;
}
