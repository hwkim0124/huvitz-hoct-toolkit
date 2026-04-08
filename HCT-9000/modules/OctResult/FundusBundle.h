#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class FundusImage;
	class FundusFrame;
	class FundusSection;

	class OCTRESULT_DLL_API FundusBundle
	{
	public:
		FundusBundle();
		virtual ~FundusBundle();

		FundusBundle(FundusBundle&& rhs);
		FundusBundle& operator=(FundusBundle&& rhs);
		FundusBundle(const FundusBundle& rhs) = delete;
		FundusBundle& operator=(const FundusBundle& rhs) = delete;

	public:
		FundusImage* getSectionImage(int sectIdx, int imageIdx);
		FundusFrame* getSectionFrame(int sectIdx, int frameIdx);
		const OctFundusImage* getSectionImageDescript(int sectIdx, int imageIdx);

		bool addSectionImage(const OctFundusSection& section, const OctFundusImage& image, const OctFundusFrame& frame);
		bool addSectionImage(const OctFundusSection& section, const OctFundusImage& image);
		bool addSectionImage(const OctFundusSection& section, const std::wstring& path);

		bool addSection(const OctFundusSection& section);
		FundusSection* getSection(int index);
		FundusSection* getSectionLast(void);
		FundusSection* makeSection(const OctFundusSection& desc);
		int getSectionCount(void) const;
		std::vector<int> getSectionIndexList(void) const;
		void clearAllSections(void);

	private:
		struct FundusBundleImpl;
		std::unique_ptr<FundusBundleImpl> d_ptr;
		FundusBundleImpl& getImpl(void) const;
	};

}