#include "stdafx.h"
#include "ScanResult.h"

using namespace PatientDB;


ScanResult::ScanResult()
{
	clear();
}

ScanResult::~ScanResult()
{
	clear();
}

void ScanResult::clear(void)
{
	m_index = INVALID_INDEX;
	m_examIdx = INVALID_INDEX;
	m_patternIdx = INVALID_INDEX;
	m_deviceIdx = INVALID_INDEX;
	m_measureTime = COleDateTime::GetCurrentTime();
	m_eyeSide = -1;
	m_SSI = 0;
	m_SQI = 0;
	m_AScans = 0;
	m_BScans = 0;
	m_scanWidth = 0.0;
	m_scanHeight = 0.0;
	m_scanDirection = Direction::UNKNOWN;
	m_xOffset = 0.0;
	m_yOffset = 0.0;
	m_baseLine = 0;
	m_filePath.clear();
	m_status = Status::USE;

	return;
}

bool ScanResult::operator==(ScanResult& s) const
{
	if (m_index == s.getIndex() &&
		m_examIdx == s.getExamIdx() &&
		m_patternIdx == s.getPatternIdx() &&
		m_deviceIdx == s.getDeviceIdx() &&
		m_measureTime == s.getMeasureTime() &&
		m_eyeSide == s.getEyeSide() &&
		m_fixationTarget == s.getFixationTarget() &&
		m_SSI == s.getSSI() &&
		m_SQI == s.getSQI() &&
		m_AScans == s.getAScans() &&
		m_BScans == s.getBScans() &&
		m_scanWidth == s.getScanWidth() &&
		m_scanHeight == s.getScanHeight() &&
		m_scanDirection == s.getScanDirection() &&
		m_xOffset == s.getXOffset() &&
		m_yOffset == s.getYOffset() &&
		m_baseLine == s.getBaseLine() &&
		m_filePath == s.getFilePath() &&
		m_status == s.getStatus()) {
		return true;
	}
	else {
		return false;
	}
}

void ScanResult::setIndex(long idx)
{
	m_index = idx;
	return;
}

void ScanResult::setExamIdx(long idx)
{
	m_examIdx = idx;
	return;
}

void ScanResult::setPatternIdx(long idx)
{
	m_patternIdx = idx;
	return;
}

void ScanResult::setDeviceIdx(long idx)
{
	m_deviceIdx = idx;
	return;
}

void ScanResult::setMeasureTime(COleDateTime time)
{
	m_measureTime = time;
	return;
}

void ScanResult::setEyeSide(int side)
{
	m_eyeSide = side;
	return;
}

void ScanResult::setFixationTarget(int idx)
{
	m_fixationTarget = idx;
	return;
}

void ScanResult::setSSI(int value)
{
	m_SSI = value;
	return;
}

void ScanResult::setSQI(int value)
{
	m_SQI = value;
	return;
}

void ScanResult::setAScans(int scans)
{
	m_AScans = scans;
	return;
}

void ScanResult::setBScans(int scans)
{
	m_BScans = scans;
	return;
}

void ScanResult::setScanWidth(double width)
{
	m_scanWidth = width;
	return;
}

void ScanResult::setScanHeight(double height)
{
	m_scanHeight = height;
	return;
}

void ScanResult::setScanDirection(Direction direction)
{
	m_scanDirection = direction;
	return;
}

void ScanResult::setXOffset(double value)
{
	m_xOffset = value;
	return;
}

void ScanResult::setYOffset(double value)
{
	m_yOffset = value;
	return;
}

void ScanResult::setBaseLine(int value)
{
	m_baseLine = value;
	return;
}

void ScanResult::setFilePath(wstring filePath)
{
	m_filePath = filePath;
	return;
}

void ScanResult::setStatus(Status status)
{
	m_status = status;
	return;
}