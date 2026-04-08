#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace OctResult
{
	class FundusImage;
	class FundusFrame;

	class OCTRESULT_DLL_API FundusSection
	{
	public:
		FundusSection();
		FundusSection(const OctFundusSection& desc);
		virtual ~FundusSection();

		FundusSection(FundusSection&& rhs);
		FundusSection& operator=(FundusSection&& rhs);
		FundusSection(const FundusSection& rhs) = delete;
		FundusSection& operator=(const FundusSection& rhs) = delete;

	public:
		bool setImage(const OctFundusImage& image);
		bool setFrame(const OctFundusFrame& frame);
		bool addImage(const OctFundusImage& image);
		bool addFrame(const OctFundusFrame& frame);

		FundusImage* getImage(int index = 0);
		FundusFrame* getFrame(int index = 0);
		int getImageCount(void) const;
		int getFrameCount(void) const;
		void clearAllImages(void);
		void clearAllFrames(void);

		void setDescript(const OctFundusSection& section);
		const OctFundusSection& getDescript(void) const;

		bool exportFiles(unsigned int imgQuality, const std::wstring& dirPath, bool removeReflectionLight = false,
			const std::wstring& imageName = L"fundus",
			const std::wstring& thumbName = L"thumbnail",
			const std::wstring& frameExt = _T("raw"));
		bool importFiles(const std::wstring& dirPath, const std::wstring& imageName = L"fundus",
							const std::wstring& frameExt = _T("raw"));
		bool importFileWithExt(const std::wstring& dirPath, const std::wstring& imageName,
			const std::wstring& frameExt, const std::wstring& imageExt);

		bool exportAdjustParams(std::wstring strVersion, std::map<std::wstring, float> values,
			const std::wstring& dirPath, const std::wstring& imageName);
		bool importAdjustParams(std::map<std::wstring, float>& out_values, const std::wstring& dirPath,
			const std::wstring& imageName);

		bool exportImage(unsigned int imgQuality, const std::wstring& path, int imageIdx = 0);
		bool exportThumbnail(const std::wstring& path, bool removeReflectionLight = false, int imageIdx = 0);
		bool exportFrame(const std::wstring& path, int frameIdx = 0);
		bool importImage(const std::wstring& path, bool append = false);
		bool importFrame(const std::wstring& path, bool append = false);

	private:
		struct FundusSectionImpl;
		std::unique_ptr<FundusSectionImpl> d_ptr;
		FundusSectionImpl& getImpl(void) const;
	};

	// typedef std::vector<FundusSection> FundusSectionList;
	typedef std::map<int, FundusSection> FundusSectionList;
}
