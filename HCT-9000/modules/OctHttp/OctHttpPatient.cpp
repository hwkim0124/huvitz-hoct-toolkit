#include "stdafx.h"

#include <iostream>
#include <string>
#include <iomanip>

#include "cpprest/json.h"

#include "OctHttp.h"
#include "OctHttpSetup.h"

#define __OCTHTTP_PATIENT__
#include "OctHttpPatient.h"

using namespace octhttp;

Patient::Patient(const std::wstring &id)
    :key(id)
{
	// key 가 추가되기 전, 생성자로 들어온 id 는 patientId로 사용되고 있었음.
	// 이제 key 를 사용하게 되었으나, 혹시나 빠뜨리는 케이스가 생길 것을 우려하여, 남겨 둠.
	patientId = id;
}


Patient::~Patient()
{
}

void Patient::setFirstName(const std::wstring &fn)
{
    firstName = std::wstring(fn);
}

void Patient::setMiddleName(const std::wstring &mn)
{
    middleName = std::wstring(mn);
}

void Patient::setLastName(const std::wstring &ln)
{
    lastName = std::wstring(ln);
}

void Patient::setGender(Gender g)
{
    gender = g;
}

void Patient::setBirthDate(unsigned int yyyy, unsigned int mm, unsigned int dd)
{
    wchar_t buf[16];
    _snwprintf_s(buf, 16, L"%04d-%02d-%02d", yyyy, mm, dd);
   
    birthDate = std::wstring(buf);
}

void Patient::setRefract(float od, float os)
{
    refractOs = os;
    refractOd = od;
}

void Patient::setRace(Race r)
{
    race = r;
}

void Patient::setPhysicianName(const std::wstring &pname)
{
    physicianName = std::wstring(pname);
}

void Patient::setOperatorName(const std::wstring &oname)
{
    operatorName = std::wstring(oname);
}

void Patient::setDescription(const std::wstring &desc)
{
    description = std::wstring(desc);
}

void Patient::setPatientId(const std::wstring &pid)
{
	patientId = pid;
}

void Patient::setModality(const std::wstring & value)
{
	modality = value;
}

void Patient::setAccessionNumber(const std::wstring & value)
{
	accessionNumber = value;
}

int Patient::postJson()
{
    web::json::value jsonPatient;
    jsonPatient[L"PatientId"]     = web::json::value::string(patientId);
    jsonPatient[L"FirstName"]     = web::json::value::string(firstName);
    jsonPatient[L"MiddleName"]    = web::json::value::string(middleName);
    jsonPatient[L"LastName"]      = web::json::value::string(lastName);
    jsonPatient[L"Gender"]        = web::json::value::string(OctHttpSetup::genderToString(gender));
    jsonPatient[L"BirthDate"]     = web::json::value::string(birthDate);
    jsonPatient[L"RefractOd"]     = web::json::value::string(std::to_wstring(refractOd));
    jsonPatient[L"RefractOs"]     = web::json::value::string(std::to_wstring(refractOs));
    jsonPatient[L"Race"]          = web::json::value::string(OctHttpSetup::raceToString(race));
    jsonPatient[L"PhysicianName"] = web::json::value::string(physicianName);
    jsonPatient[L"OperatorName"]  = web::json::value::string(operatorName);
    jsonPatient[L"Description"]   = web::json::value::string(description);
	jsonPatient[L"Modality"]	  = web::json::value::string(modality);
	jsonPatient[L"AccessionNumber"] = web::json::value::string(accessionNumber);

    int id;
    try {
        id = OctHttp::getInstance().postJson(L"patients/", jsonPatient);
    }
    catch (...) {
        throw;
    }

    return id;
}

int Patient::update()
{
    web::json::value jsonPatient;
    jsonPatient[L"PatientId"] = web::json::value::string(patientId);
    jsonPatient[L"FirstName"] = web::json::value::string(firstName);
    jsonPatient[L"MiddleName"] = web::json::value::string(middleName);
    jsonPatient[L"LastName"] = web::json::value::string(lastName);
    jsonPatient[L"Gender"] = web::json::value::string(OctHttpSetup::genderToString(gender));
    jsonPatient[L"BirthDate"] = web::json::value::string(birthDate);
    jsonPatient[L"RefractOd"] = web::json::value::string(std::to_wstring(refractOd));
    jsonPatient[L"RefractOs"] = web::json::value::string(std::to_wstring(refractOs));
    jsonPatient[L"Race"] = web::json::value::string(OctHttpSetup::raceToString(race));
    jsonPatient[L"PhysicianName"] = web::json::value::string(physicianName);
    jsonPatient[L"OperatorName"] = web::json::value::string(operatorName);
    jsonPatient[L"Description"] = web::json::value::string(description);
	jsonPatient[L"Modality"] = web::json::value::string(modality);
	jsonPatient[L"AccessionNumber"] = web::json::value::string(accessionNumber);

    int id;
    try {
		std::wstring strPath = L"patients/" + key;
        id = OctHttp::getInstance().putJson(strPath, jsonPatient);
    }
    catch (...) {
        throw;
    }

    return id;
}

void Patient::debug()
{
    std::wcout << "Patient ID = " << patientId << std::endl;
    std::wcout << "Patient Name = " << firstName + L" " + lastName << std::endl;
}



