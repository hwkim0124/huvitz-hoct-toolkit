#include "stdafx.h"
#include "Exam.h"

using namespace PatientDB;


Exam::Exam()
{
	clear();
}

Exam::Exam(const Exam &e)
{
	m_index = e.m_index;
	m_examDate = e.m_examDate;
	m_patientIdx = e.m_patientIdx;
	m_diagnosis = e.m_diagnosis;
	m_comment = e.m_comment;
	m_filePath = e.m_filePath;
	m_status = e.m_status;
	m_modality = e.m_modality;
	m_scheduledDate = e.m_scheduledDate;
	m_scheduledTime = e.m_scheduledTime;
	m_studyInstanceUID = e.m_studyInstanceUID;
}

Exam::~Exam()
{
}

void Exam::clear(void)
{
	m_index = INVALID_INDEX;
	m_examDate = COleDateTime::GetCurrentTime();
	m_patientIdx = INVALID_INDEX;
	m_diagnosis.clear();
	m_comment.clear();
	m_filePath.clear();
	m_status = Status::USE;
	m_modality.clear();
	m_scheduledDate.clear();
	m_scheduledTime.clear();
	m_studyInstanceUID.clear();
}

void Exam::setIndex(long idx)
{
	m_index = idx;
	return;
}

void Exam::setExamDate(COleDateTime time)
{
	m_examDate = time;
	return;
}

void Exam::setPatientIdx(long idx)
{
	m_patientIdx = idx;
	return;
}

void Exam::setDiagnosis(wstring diagnosis)
{
	m_diagnosis = diagnosis;
	return;
}

void Exam::setComment(wstring comment)
{
	m_comment = comment;
	return;
}

void Exam::setFilePath(wstring filePath)
{
	m_filePath = filePath;
	return;
}

void Exam::setStatus(Status status)
{
	m_status = status;
	return;
}

void Exam::setModality(wstring modality)
{
	m_modality = modality;
	return;
}

void Exam::setScheduledDate(wstring date)
{
	m_scheduledDate = date;
	return;
}

void Exam::setScheduledTime(wstring time)
{
	m_scheduledTime = time;
	return;
}

void Exam::setStudyInstanceUID(wstring uid)
{
	m_studyInstanceUID = uid;
	return;
}