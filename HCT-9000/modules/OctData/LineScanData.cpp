#include "stdafx.h"
#include "LineScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct LineScanData::LineScanDataImpl
{
	LineScanDataImpl()
	{
	}
};


LineScanData::LineScanData() :
	d_ptr(make_unique<LineScanDataImpl>())
{
}

OctData::LineScanData::LineScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<LineScanDataImpl>()), ProtocolData(desc)
{
}


OctData::LineScanData::LineScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<LineScanDataImpl>()), ProtocolData(std::move(result))
{
}


OctData::LineScanData::LineScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<LineScanDataImpl>()), ProtocolData(result)
{
}


OctData::LineScanData::~LineScanData() = default;
OctData::LineScanData::LineScanData(LineScanData && rhs) = default;
LineScanData & OctData::LineScanData::operator=(LineScanData && rhs) = default;


LineScanData::LineScanDataImpl & OctData::LineScanData::getImpl(void) const
{
	return *d_ptr;
}
