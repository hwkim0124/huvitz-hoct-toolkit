#include "stdafx.h"
#include "CircleScanData.h"

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;


struct CircleScanData::CircleScanDataImpl
{
	CircleScanDataImpl()
	{
	}
};


CircleScanData::CircleScanData() :
	d_ptr(make_unique<CircleScanDataImpl>())
{
}

OctData::CircleScanData::CircleScanData(const OctScanPattern & desc) :
	d_ptr(make_unique<CircleScanDataImpl>()), ProtocolData(desc)
{
}


OctData::CircleScanData::CircleScanData(std::unique_ptr<OctResult::ScannerResult>&& result) :
	d_ptr(make_unique<CircleScanDataImpl>()), ProtocolData(std::move(result))
{
}


OctData::CircleScanData::CircleScanData(std::shared_ptr<OctResult::ScannerResult> result) :
	d_ptr(make_unique<CircleScanDataImpl>()), ProtocolData(result)
{
}


OctData::CircleScanData::~CircleScanData() = default;
OctData::CircleScanData::CircleScanData(CircleScanData && rhs) = default;
CircleScanData & OctData::CircleScanData::operator=(CircleScanData && rhs) = default;


CircleScanData::CircleScanDataImpl & OctData::CircleScanData::getImpl(void) const
{
	return *d_ptr;
}