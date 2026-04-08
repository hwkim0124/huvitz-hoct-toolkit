#include "stdafx.h"
#include "FundusData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct FundusData::FundusDataImpl
{
	std::unique_ptr<FundusResult> result;

	FundusDataImpl() : result(new FundusResult())
	{
	}
};



FundusData::FundusData() :
	d_ptr(make_unique<FundusDataImpl>())
{
}


OctData::FundusData::FundusData(const OctFundusMeasure & desc)
{
	setDescript(desc);
}


OctData::FundusData::FundusData(std::unique_ptr<OctResult::FundusResult>&& result)
{
	importFundusResult(std::move(result));
}


OctData::FundusData::~FundusData() = default;
OctData::FundusData::FundusData(FundusData && rhs) = default;
FundusData & OctData::FundusData::operator=(FundusData && rhs) = default;


void OctData::FundusData::importFundusResult(std::unique_ptr<OctResult::FundusResult>&& result)
{
	if (result != nullptr) {
		d_ptr->result = std::move(result);
	}
	return;
}


OctResult::FundusResult * OctData::FundusData::getResult(void) const
{
	return d_ptr->result.get();
}


OctFundusMeasure & OctData::FundusData::getDescript(void) const
{
	return getResult()->getDescript();
}


void OctData::FundusData::setDescript(const OctFundusMeasure & desc)
{
	getResult()->setDescript(desc);
	return;
}


const OctFundusImage * OctData::FundusData::getFundusImageDescript(int sectIdx, int imageIdx)
{
	if (getResult()) {
		return getResult()->getFundusOutput().getSectionImageDescript(sectIdx, imageIdx);
	}
	return nullptr;
}


const OctRetinaImage * OctData::FundusData::getRetinaImageDescript(void) const
{
	if (getResult()) {
		return getResult()->getRetinaImageDescript();
	}
	return nullptr;
}


const OctCorneaImage * OctData::FundusData::getCorneaImageDescript(void) const
{
	if (getResult()) {
		return getResult()->getCorneaImageDescript();
	}
	return nullptr;
}



void OctData::FundusData::clear(void)
{
	d_ptr->result.reset(new FundusResult());
	return;
}


FundusData::FundusDataImpl & OctData::FundusData::getImpl(void) const
{
	return *d_ptr;
}
