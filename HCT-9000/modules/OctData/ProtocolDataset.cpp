#include "stdafx.h"
#include "ProtocolDataset.h"
#include "LineScanData.h"
#include "CubeScanData.h"
#include "CircleScanData.h"
#include "CrossScanData.h"
#include "RadialScanData.h"
#include "RasterScanData.h"

#include <map>

#include "CppUtil2.h"
#include "OctResult2.h"

using namespace OctData;
using namespace OctResult;
using namespace CppUtil;
using namespace std;



struct ProtocolDataset::ProtocolDatasetImpl
{
	std::map<int, unique_ptr<ProtocolData>> dataset;

	ProtocolDatasetImpl()
	{
	}
};


ProtocolDataset::ProtocolDataset() :
	d_ptr(make_unique<ProtocolDatasetImpl>())
{
}


OctData::ProtocolDataset::~ProtocolDataset() = default;
OctData::ProtocolDataset::ProtocolDataset(ProtocolDataset && rhs) = default;
ProtocolDataset & OctData::ProtocolDataset::operator=(ProtocolDataset && rhs) = default;


ProtocolData * OctData::ProtocolDataset::getProtocolData(int index)
{
	auto data = fetchProtocolData<ProtocolData>(index);
	return data;
}


LineScanData * OctData::ProtocolDataset::getOrCreateLineScanData(int index)
{
	auto data = static_cast<LineScanData*>(emplaceProtocolData<LineScanData>(index));
	return data;
}


CubeScanData * OctData::ProtocolDataset::getOrCreateCubeScanData(int index)
{
	auto data = static_cast<CubeScanData*>(emplaceProtocolData<CubeScanData>(index));
	return data;
}


CircleScanData * OctData::ProtocolDataset::getOrCreateCircleScanData(int index)
{
	auto data = static_cast<CircleScanData*>(emplaceProtocolData<CircleScanData>(index));
	return data;
}


CrossScanData * OctData::ProtocolDataset::getOrCreateCrossScanData(int index)
{
	auto data = static_cast<CrossScanData*>(emplaceProtocolData<CrossScanData>(index));
	return data;
}


RadialScanData * OctData::ProtocolDataset::getOrCreateRadialScanData(int index)
{
	auto data = static_cast<RadialScanData*>(emplaceProtocolData<RadialScanData>(index));
	return data;
}


RasterScanData * OctData::ProtocolDataset::getOrCreateRasterScanData(int index)
{
	auto data = static_cast<RasterScanData*>(emplaceProtocolData<RasterScanData>(index));
	return data;
}


void OctData::ProtocolDataset::clear(void)
{
	d_ptr->dataset.clear();
	return;
}


ProtocolDataset::ProtocolDatasetImpl & OctData::ProtocolDataset::getImpl(void) const
{
	return *d_ptr;
}
