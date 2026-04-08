#include "stdafx.h"
#include "FundusDataset.h"


#include <map>

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusDataset::FundusDatasetImpl
{
	std::map<int, unique_ptr<FundusData>> dataset;

	FundusDatasetImpl()
	{
	}
};


FundusDataset::FundusDataset() :
	d_ptr(make_unique<FundusDatasetImpl>())
{
}


OctData::FundusDataset::~FundusDataset() = default;
OctData::FundusDataset::FundusDataset(FundusDataset && rhs) = default;
FundusDataset & OctData::FundusDataset::operator=(FundusDataset && rhs) = default;


FundusData * OctData::FundusDataset::getFundusData(int index)
{
	auto data = fetchFundusData<FundusData>(index);
	return data;
}


FundusData * OctData::FundusDataset::getOrCreateFundusData(int index)
{
	auto data = static_cast<FundusData*>(emplaceFundusData<FundusData>(index));
	return data;
}


void OctData::FundusDataset::clear(void)
{
	d_ptr->dataset.clear();
	return;
}


FundusDataset::FundusDatasetImpl & OctData::FundusDataset::getImpl(void) const
{
	return *d_ptr;
}
