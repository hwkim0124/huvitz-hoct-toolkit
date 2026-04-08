#pragma once

#include "OctReportDef.h"
#include "OcularReport.h"

#include <string>
#include <memory>
#include <vector>


namespace OctReport
{
	class MacularReport2;
	class DiscReport2;
	class CorneaReport2;

	class FundusReport;

	class OCTREPORT_DLL_API OcularReportSet
	{
	public:
		OcularReportSet();
		virtual ~OcularReportSet();

		OcularReportSet(OcularReportSet&& rhs);
		OcularReportSet& operator=(OcularReportSet&& rhs);
		OcularReportSet(const OcularReportSet& rhs) = delete;
		OcularReportSet& operator=(const OcularReportSet& rhs) = delete;

	public:
		MacularReport2* getOrCreateMacularReport(int index);
		DiscReport2* getOrCreateDiscReport(int index);
		CorneaReport2* getOrCreateCorneaReport(int index);

		FundusReport* getOrCreateFundusReport(int index);

		void clear(void);

	protected:
		template <typename T>
		OcularReport* emplaceOcularReport(int index);

		template <typename T>
		OcularReport* getOcularReport(int index);

		template <typename T>
		OcularReport * setOcularReport(int index);

	private:
		struct OcularReportSetImpl;
		std::unique_ptr<OcularReportSetImpl> d_ptr;
		OcularReportSetImpl& getImpl(void) const;
	};

	template<typename T>
	inline OcularReport * OcularReportSet::emplaceOcularReport(int index)
	{
		auto data = getOcularReport<T>(index);
		if (data == nullptr) {
			return setOcularReport<T>(index);
		}
		return data;
	}

	template<typename T>
	inline OcularReport * OcularReportSet::getOcularReport(int index)
	{
		auto iter = d_ptr->dataset.find(index);
		if (iter != end(d_ptr->dataset)) {
			return dynamic_cast<T*>(iter->second.get());
		}
		return nullptr;
	}

	template<typename T>
	inline OcularReport * OcularReportSet::setOcularReport(int index)
	{
		if (index >= 0) {
			d_ptr->dataset[index] = make_unique<T>();
			return d_ptr->dataset[index].get();
		}
		return nullptr;
	}
}

