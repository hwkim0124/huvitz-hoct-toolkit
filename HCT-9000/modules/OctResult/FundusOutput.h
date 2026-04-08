#pragma once

#include "OctResultDef.h"
#include "FundusBundle.h"

#include <memory>
#include <vector>
#include <string>
#include <map>

namespace OctResult
{
	class FundusSection;
	class FundusImage;


	class OCTRESULT_DLL_API FundusOutput : public FundusBundle
	{
	public:
		FundusOutput();
		virtual ~FundusOutput();

		FundusOutput(FundusOutput&& rhs);
		FundusOutput& operator=(FundusOutput&& rhs);
		FundusOutput(const FundusOutput& rhs) = delete;
		FundusOutput& operator=(const FundusOutput& rhs) = delete;

	public:
		int exportImages(unsigned int imgQuality, const std::wstring& dirPath, bool removeReflectionLight = false,
			const std::wstring& imageName = L"fundus", const std::wstring& thumbName = L"thumbnail",
			const std::wstring& frameExt = L"raw");
		int importImages(const std::wstring& dirPath, const std::wstring& imageName = L"fundus",
			const std::wstring& frameExt = L"raw");

		bool exportImage(int sectIdx, unsigned int imgQuality, const std::wstring& dirPath, bool removeReflectionLight = false,
			const std::wstring& imageName = L"fundus", const std::wstring& thumbName = L"thumbnail",
			const std::wstring& frameExt = L"raw");
		bool importImage(int sectIdx, const std::wstring dirPath, const std::wstring& imageName = L"fundus",
			const std::wstring& frameExt = L"raw");

		bool exportAdjustParams(std::wstring strVersion, std::map<std::wstring, float> values,
			const std::wstring& dirPath, const std::wstring& imageName);
		bool importAdjustParams(std::map<std::wstring, float>& out_values, const std::wstring& dirPath,
			const std::wstring& imageName);

	private:
		struct FundusOutputImpl;
		std::unique_ptr<FundusOutputImpl> d_ptr;
		FundusOutputImpl& getImpl(void) const;
	};

	typedef std::vector<FundusOutput> FundusOutputList;
}
