#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class BscanSection;
	class BscanImage2;

	class OCTRESULT_DLL_API BscanBundle
	{
	public:
		BscanBundle();
		BscanBundle(const OctScanPattern& desc);
		virtual ~BscanBundle();

		BscanBundle(BscanBundle&& rhs);
		BscanBundle& operator=(BscanBundle&& rhs);
		BscanBundle(const BscanBundle& rhs) = delete;
		BscanBundle& operator=(const BscanBundle& rhs) = delete;

	public:
		void setDescript(const OctScanPattern& desc);
		OctScanPattern& getDescript(void) const;

		BscanImage2* getSectionImage(int sectIdx, int imageIdx = 0);
		int getSectionImageCount(int sectIdx);
		const OctScanImage* getSectionImageDescript(int sectIdx, int imageIdx = 0);

		bool addSectionImage(const OctScanSection& section, const OctScanImage& image, int sectSize);
		bool addSectionImage(const OctScanSection& section, const OctScanImage& image);
		bool addSectionImage(const OctScanSection& section, const std::wstring& path);
		bool setSectionImage(const OctScanSection& section, const OctScanImage& image);
		bool setSectionImage(const OctScanSection& section, const std::wstring& path);

		bool addSection(const OctScanSection& sect);
		BscanSection* getSection(int index);
		BscanSection* getSectionLast(void);
		BscanSection* makeSection(const OctScanSection& desc);
		int getSectionCount(void) const;
		std::vector<int> getSectionIndexList(void) const;
		void clearAllSections(void);

	private:
		struct BscanBundleImpl;
		std::unique_ptr<BscanBundleImpl> d_ptr;
		BscanBundleImpl& getImpl(void) const;
	};

}