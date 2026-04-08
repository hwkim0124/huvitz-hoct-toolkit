#include "stdafx.h"
#include "WorkList.h"

using namespace PatientDB;


Worklist::Worklist()
{
	clear();
}

Worklist::Worklist(const Worklist &w)
{
	m_index = w.m_index;
	m_scheduledModality = w.m_scheduledModality;
	m_scheduledStation = w.m_scheduledStation;
	m_scheduledTime = w.m_scheduledTime;
	m_studyInstanceUID = w.m_studyInstanceUID;
	m_requestProcedureID = w.m_requestProcedureID;
	m_accessionNumber = w.m_accessionNumber;
	m_patientIdx = w.m_patientIdx;
	m_status = w.m_status;
}

Worklist::~Worklist()
{
}

void Worklist::clear(void)
{
	m_index = INVALID_INDEX;
	m_scheduledStation.clear();
	m_scheduledModality.clear();
	m_scheduledTime = COleDateTime::GetCurrentTime();
	m_studyInstanceUID.clear();
	m_requestProcedureID.clear();
	m_accessionNumber.clear();
	m_patientIdx = INVALID_INDEX;
	m_status = Status::USE;
}

void Worklist::setIndex(long idx)
{
	m_index = idx;
}

void Worklist::setScheduledModality(wstring modality)
{
	m_scheduledModality = modality;
}

void Worklist::setScheduledTime(COleDateTime time)
{
	m_scheduledTime = time;
}

void Worklist::setScheduledStation(wstring station)
{
	m_scheduledStation = station;
}

void Worklist::setStudyInstanceUID(wstring uid)
{
	m_studyInstanceUID = uid;
}

void Worklist::setRequestProcedureID(wstring id)
{
	m_requestProcedureID = id;
}

void Worklist::setAccessionNumber(wstring number)
{
	m_accessionNumber = number;
}

void Worklist::setPatientIdx(long idx)
{
	m_patientIdx = idx;
}

void Worklist::setStatus(Status status)
{
	m_status = status;
}