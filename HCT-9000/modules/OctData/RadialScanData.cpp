#include "stdafx.h"
#include "RadialScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct RadialScanData::RadialScanDataImpl
{
	RadialScanDataImpl()
	{
	}
};


RadialScanData::RadialScanData() :
	d_ptr(make_unique<RadialScanDataImpl>())
{
}

OctData::RadialScanData::RadialScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<RadialScanDataImpl>()), ProtocolData(desc)
{
}


OctData::RadialScanData::RadialScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<RadialScanDataImpl>()), ProtocolData(std::move(result))
{
}


OctData::RadialScanData::RadialScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<RadialScanDataImpl>()), ProtocolData(result)
{
}


OctData::RadialScanData::~RadialScanData() = default;
OctData::RadialScanData::RadialScanData(RadialScanData && rhs) = default;
RadialScanData & OctData::RadialScanData::operator=(RadialScanData && rhs) = default;


RadialScanData::RadialScanDataImpl & OctData::RadialScanData::getImpl(void) const
{
	return *d_ptr;
}