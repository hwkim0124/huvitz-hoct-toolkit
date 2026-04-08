#pragma once

#include <memory>
#include <string>
#include <vector>
#include "SegmScanDef.h"

class SegmScanResult;


class AFX_EXT_CLASS SegmentScan : public CObject
{
public:
	SegmentScan();
	virtual ~SegmentScan();

private:
	std::shared_ptr<SegmScanResult> m_pSegmScanResult;

public:
	bool initialize(const SegmScanDef::EyeSide side, const SegmScanDef::ScanRegion region,
					const SegmScanDef::PatternType type, const float scanAreaW=0.0f, const float scanAreaH=0.0f);
	bool addScanImage(const SegmScanDef::ScanImageType type, const std::wstring path);
	bool addScanImage(const SegmScanDef::ScanImageType type, const std::vector<std::wstring> paths);
	bool addScanImage(const SegmScanDef::ScanImageType type, const BITMAPINFO *pInfo, BYTE *pBits);

};

