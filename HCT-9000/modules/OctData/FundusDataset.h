#pragma once

#include "OctDataDef.h"
#include "FundusData.h"

#include <string>
#include <memory>
#include <vector>


namespace OctData
{
	class OCTDATA_DLL_API FundusDataset
	{
	public:
		FundusDataset();
		virtual ~FundusDataset();

		FundusDataset(FundusDataset&& rhs);
		FundusDataset& operator=(FundusDataset&& rhs);
		FundusDataset(const FundusDataset& rhs) = delete;
		FundusDataset& operator=(const FundusDataset& rhs) = delete;

	public:
		FundusData* getFundusData(int index);
		FundusData* getOrCreateFundusData(int index);

		void clear(void);

	protected:
		template <typename T>
		FundusData* emplaceFundusData(int index);

		template <typename T>
		FundusData* fetchFundusData(int index);

		template <typename T>
		FundusData * createFundusData(int index);

	private:
		struct FundusDatasetImpl;
		std::unique_ptr<FundusDatasetImpl> d_ptr;
		FundusDatasetImpl& getImpl(void) const;
	};


	template<typename T>
	inline FundusData * FundusDataset::emplaceFundusData(int index)
	{
		auto data = fetchFundusData<T>(index);
		if (data == nullptr) {
			return createFundusData<T>(index);
		}
		return data;
	}

	template<typename T>
	inline FundusData * FundusDataset::fetchFundusData(int index)
	{
		auto iter = d_ptr->dataset.find(index);
		if (iter != end(d_ptr->dataset)) {
			return dynamic_cast<T*>(iter->second.get());
		}
		return nullptr;
	}

	template<typename T>
	inline FundusData * FundusDataset::createFundusData(int index)
	{
		if (index >= 0) {
			d_ptr->dataset[index] = make_unique<T>();
			return d_ptr->dataset[index].get();
		}
		return nullptr;
	}
}

