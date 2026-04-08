#include "stdafx.h"
#include "FundusResult.h"

using namespace PatientDB;


FundusResult::FundusResult()
{
	clear();
}

FundusResult::~FundusResult()
{
	clear();
}

void FundusResult::clear(void)
{
	m_index = INVALID_INDEX;
	m_examIdx = INVALID_INDEX;
	m_deviceIdx = INVALID_INDEX;
	m_measureTime = COleDateTime::GetCurrentTime();
	m_eyeSide = -1;
	m_flashLevel = 0;
	m_shutterSpeed = 0;
	m_ISO = 0;
	m_panorma = 0;
	m_stereo = 0;
	m_filePath.clear();
	m_status = Status::USE;

	return;
}

bool FundusResult::operator==(FundusResult& f) const
{
	if (m_index == f.getIndex() &&
		m_examIdx == f.getExamIdx() &&
		m_deviceIdx == f.getDeviceIdx() &&
		m_measureTime == f.getMeasureTime() &&
		m_eyeSide == f.getEyeSide() &&
		m_fixaionTarget == f.getFixationTarget() &&
		m_flashLevel == f.getFlashLevel() &&
		m_shutterSpeed == f.getShutterSpeed() &&
		m_ISO == f.getISO() &&
		m_panorma == f.getPanorma() &&
		m_stereo == f.getStereo() &&
		m_filePath == f.getFilePath() &&
		m_status == f.getStatus()) {
		return true;
	}
	else {
		return false;
	}
}

void FundusResult::setIndex(long idx)
{
	m_index = idx;
	return;
}

void FundusResult::setExamIdx(long idx)
{
	m_examIdx = idx;
	return;
}

void FundusResult::setDeviceIdx(long idx)
{
	m_deviceIdx = idx;
	return;
}

void FundusResult::setMeasureTime(COleDateTime time)
{
	m_measureTime = time;
	return;
}

void FundusResult::setEyeSide(int side)
{
	m_eyeSide = side;
	return;
}

void FundusResult::setFixationTarget(int idx)
{
	m_fixaionTarget = idx;
	return;
}

void FundusResult::setFlashLevel(int level)
{
	m_flashLevel = level;
	return;
}

void FundusResult::setShutterSpeed(int speed)
{
	m_shutterSpeed = speed;
	return;
}

void FundusResult::setISO(int ISO)
{
	m_ISO = ISO;
	return;
}

void FundusResult::setPanorma(int panorma)
{
	m_panorma = panorma;
	return;
}

void FundusResult::setStereo(int stereo)
{
	m_stereo = stereo;
	return;
}

void FundusResult::setFilePath(wstring filePath)
{
	m_filePath = filePath;
	return;
}

void FundusResult::setStatus(Status status)
{
	m_status = status;
	return;
}




FundusStitchResult::FundusStitchResult()
{
	clear();
}

FundusStitchResult::~FundusStitchResult()
{
	clear();
}

void FundusStitchResult::clear(void)
{
	m_serialID = INVALID_INDEX;
	m_patientID = INVALID_INDEX;
	m_examID = INVALID_INDEX;
	m_deviceID = INVALID_INDEX;
	m_measureTime = COleDateTime::GetCurrentTime();
	m_eyeSide = -1;
	m_examPath.clear();
	m_resultPath.clear();
	m_fixaionTarget = -1;
	return;
}
	