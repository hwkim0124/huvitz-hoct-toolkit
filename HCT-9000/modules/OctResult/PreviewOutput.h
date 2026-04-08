#pragma once

#include "OctResultDef.h"
#include "BscanBundle.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class BscanSection;
	class BscanBundle;

	class OCTRESULT_DLL_API PreviewOutput : public BscanBundle
	{
	public:
		PreviewOutput();
		virtual ~PreviewOutput();

		PreviewOutput(PreviewOutput&& rhs);
		PreviewOutput& operator=(PreviewOutput&& rhs);
		PreviewOutput(const PreviewOutput& rhs) = delete;
		PreviewOutput& operator=(const PreviewOutput& rhs) = delete;

	public:
		int updateImages(void);
		int exportImages(const std::wstring& dirName, const std::wstring& prefix = L"preview");
		int importImages(int numImages, const std::wstring& dirName, const std::wstring& prefix = L"preview");

		bool updateImage(int sectIdx);
		bool exportImage(int sectIdx, const std::wstring& dirPath, const std::wstring& prefix = L"preview");
		bool importImage(int sectIdx, const std::wstring dirPath, const std::wstring& prefix = L"preview");

		bool importImageWithExt(int sectIdx, const std::wstring dirPath,
			const std::wstring& prefix, const std::wstring& imageExt);

		bool applyAveraging(void);

	private:
		struct PreviewOutputImpl;
		std::unique_ptr<PreviewOutputImpl> d_ptr;
		PreviewOutputImpl& getImpl(void) const;
	};

	typedef std::vector<PreviewOutput> PreviewOutputList;
}
