#include "stdafx.h"
#include "RasterScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct RasterScanData::RasterScanDataImpl
{
	RasterScanDataImpl()
	{
	}
};


RasterScanData::RasterScanData() :
	d_ptr(make_unique<RasterScanDataImpl>())
{
}

OctData::RasterScanData::RasterScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<RasterScanDataImpl>()), ProtocolData(desc)
{
}


OctData::RasterScanData::RasterScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<RasterScanDataImpl>()), ProtocolData(std::move(result))
{
}


OctData::RasterScanData::RasterScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<RasterScanDataImpl>()), ProtocolData(result)
{
}


OctData::RasterScanData::~RasterScanData() = default;
OctData::RasterScanData::RasterScanData(RasterScanData && rhs) = default;
RasterScanData & OctData::RasterScanData::operator=(RasterScanData && rhs) = default;


RasterScanData::RasterScanDataImpl & OctData::RasterScanData::getImpl(void) const
{
	return *d_ptr;
}