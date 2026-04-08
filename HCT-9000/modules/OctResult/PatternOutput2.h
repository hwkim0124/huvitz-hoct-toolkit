#pragma once

#include "OctResultDef.h"
#include "BscanBundle.h"

#include <memory>
#include <vector>
#include <string>


namespace OctResult
{
	class BscanSection;
	class BscanBundle;
	class EnfaceImage;

	class OCTRESULT_DLL_API PatternOutput2 : public BscanBundle
	{
	public:
		PatternOutput2();
		virtual ~PatternOutput2();

		PatternOutput2(PatternOutput2&& rhs);
		PatternOutput2& operator=(PatternOutput2&& rhs);
		PatternOutput2(const PatternOutput2& rhs) = delete;
		PatternOutput2& operator=(const PatternOutput2& rhs) = delete;

	public:
		int updateImages(void);
		int exportImages(const std::wstring dirPath, const std::wstring& prefix = L"");
		int importImages(int numImages, const std::wstring dirPath, const std::wstring& prefix = L"");
		int importImages(const std::vector<std::wstring>& fileList);

		bool updateImage(int sectIdx);
		bool exportImage(int sectIdx, const std::wstring& dirPath, const std::wstring& prefix);
		bool importImage(int sectIdx, const std::wstring& dirPath, const std::wstring& prefix);
		bool importImage(int sectIdx, const std::wstring& filePath);


		bool applyAveraging(void);
		float getAverageOfQualityIndex(void);

	private:
		struct PatternOutput2Impl;
		std::unique_ptr<PatternOutput2Impl> d_ptr;
		PatternOutput2Impl& getImpl(void) const;
	};


	typedef std::vector<PatternOutput2> PatternOutput2List;
}

