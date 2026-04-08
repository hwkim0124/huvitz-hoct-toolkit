#include "stdafx.h"
#include "CubeScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;



struct CubeScanData::CubeScanDataImpl
{
	CubeScanDataImpl()
	{
	}
};


CubeScanData::CubeScanData() :
	d_ptr(make_unique<CubeScanDataImpl>())
{
}

OctData::CubeScanData::CubeScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<CubeScanDataImpl>()), ProtocolData(desc)
{
}


OctData::CubeScanData::CubeScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<CubeScanDataImpl>()), ProtocolData(std::move(result))
{
}

OctData::CubeScanData::CubeScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<CubeScanDataImpl>()), ProtocolData(result)
{
}



OctData::CubeScanData::~CubeScanData() = default;
OctData::CubeScanData::CubeScanData(CubeScanData && rhs) = default;
CubeScanData & OctData::CubeScanData::operator=(CubeScanData && rhs) = default;



CubeScanData::CubeScanDataImpl & OctData::CubeScanData::getImpl(void) const
{
	return *d_ptr;
}
