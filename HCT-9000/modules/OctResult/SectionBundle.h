#pragma once

#include "OctResultDef.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class SectionData;

	class OCTRESULT_DLL_API SectionBundle
	{
	public:
		SectionBundle();
		virtual ~SectionBundle();

		SectionBundle(SectionBundle&& rhs);
		SectionBundle& operator=(SectionBundle&& rhs);
		SectionBundle(const SectionBundle& rhs);
		SectionBundle& operator=(const SectionBundle& rhs);

	public:
		int addSection(SectionData& section);
		int addSection(SectionData&& section);
		SectionData* getSection(int index);
		SectionData* getSectionLast(void);
		SectionData* makeSection(int index, OctRoute route);
		int getSectionCount(void) const;
		void clearAllSections(void);

	private:
		struct SectionBundleImpl;
		std::unique_ptr<SectionBundleImpl> d_ptr;
		SectionBundleImpl& getImpl(void) const;
	};

}