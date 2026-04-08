#pragma once

#include "OctResultDef.h"
#include "SectionBundle.h"

#include <memory>
#include <vector>


namespace OctResult
{
	class SectionData;
	class SectionBundle;

	class OCTRESULT_DLL_API PreviewData : public SectionBundle
	{
	public:
		PreviewData();
		virtual ~PreviewData();

		PreviewData(PreviewData&& rhs);
		PreviewData& operator=(PreviewData&& rhs);
		PreviewData(const PreviewData& rhs);
		PreviewData& operator=(const PreviewData& rhs);

	public:


	private:
		struct PreviewDataImpl;
		std::unique_ptr<PreviewDataImpl> d_ptr;
		PreviewDataImpl& getImpl(void) const;
	};

	typedef std::vector<PreviewData> PreviewDataList;
}
