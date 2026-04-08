#include "stdafx.h"
#include "SegmentScan.h"
#include "SegmScanDef.h"
#include "SegmScanResult.h"
#include "SegmScanImage.h"

using namespace std;

SegmentScan::SegmentScan()
{
}

SegmentScan::~SegmentScan()
{
}

bool SegmentScan::initialize(const SegmScanDef::EyeSide side, const SegmScanDef::ScanRegion region,
							 const SegmScanDef::PatternType type, const float scanAreaW, const float scanAreaH)
{
	// Using scoped type needs more efforts, but they should be explicitly seperated from the client's namespace. 
	m_pSegmScanResult = make_shared<SegmScanResult>(side, region, type, scanAreaW, scanAreaH);

	return true;
}


bool SegmentScan::addScanImage(const SegmScanDef::ScanImageType type, const std::wstring path)
{
	unique_ptr<SegmScanImage> pImage = make_unique<SegmScanImage>(type);

	if (!pImage->loadBitmapFile(path)) {
		return false;
	}
	//m_pSegmScanResult->addScanImage(move(pImage));
	return true;
}

bool SegmentScan::addScanImage(const SegmScanDef::ScanImageType type, const std::vector<std::wstring> paths)
{
	for (const wstring wstr : paths) {
		if (!addScanImage(type, wstr)) {
			return false;
		}
	}
	return true;
}

bool SegmentScan::addScanImage(const SegmScanDef::ScanImageType type, const BITMAPINFO * pInfo, BYTE * pBits)
{
	unique_ptr<SegmScanImage> pImage = make_unique<SegmScanImage>(type);

	if (!pImage->loadBitmapData(pInfo->bmiHeader.biWidth, pInfo->bmiHeader.biHeight, pBits)) {
		return false;
	}
	//m_pSegmScanResult->addScanImage(move(pImage));
	return true;
}
