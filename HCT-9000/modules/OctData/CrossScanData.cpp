#include "stdafx.h"
#include "CrossScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct CrossScanData::CrossScanDataImpl
{
	CrossScanDataImpl()
	{
	}
};


CrossScanData::CrossScanData() :
	d_ptr(make_unique<CrossScanDataImpl>())
{
}

OctData::CrossScanData::CrossScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<CrossScanDataImpl>()), ProtocolData(desc)
{
}


OctData::CrossScanData::CrossScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<CrossScanDataImpl>()), ProtocolData(std::move(result))
{
}

OctData::CrossScanData::CrossScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<CrossScanDataImpl>()), ProtocolData(result)
{
}


OctData::CrossScanData::~CrossScanData() = default;
OctData::CrossScanData::CrossScanData(CrossScanData && rhs) = default;
CrossScanData & OctData::CrossScanData::operator=(CrossScanData && rhs) = default;


CrossScanData::CrossScanDataImpl & OctData::CrossScanData::getImpl(void) const
{
	return *d_ptr;
}