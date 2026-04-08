#pragma once

#include "OctResultDef.h"
#include "SectionBundle.h"

#include <memory>
#include <vector>
#include <string>


namespace OctResult
{
	class SectionData;
	class SectionBundle;

	class OCTRESULT_DLL_API PatternData : public SectionBundle
	{
	public:
		PatternData();
		virtual ~PatternData();

		PatternData(PatternData&& rhs);
		PatternData& operator=(PatternData&& rhs);
		PatternData(const PatternData& rhs);
		PatternData& operator=(const PatternData& rhs);

	public:
		void setDescript(OctScanPattern desc);
		OctScanPattern& getDescript(void) const;

		bool exportFiles(std::wstring path);

	private:
		struct PatternDataImpl;
		std::unique_ptr<PatternDataImpl> d_ptr;
		PatternDataImpl& getImpl(void) const;
	};


	typedef std::vector<PatternData> PatternDataList;
}

