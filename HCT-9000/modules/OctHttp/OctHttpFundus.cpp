#include "stdafx.h"

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

#include "cpprest/json.h"

#include "OctHttp.h"
#include "OctHttpSetup.h"

#define __OCTHTTP_FUNDUS__
#include "OctHttpFundus.h"

using namespace octhttp;

Fundus::Fundus(MeasurementType pat, EyeSide es)
    :pattern(pat), eyeSide(es)
{
	measureTime.SetStatus(COleDateTime::null);
}

Fundus::~Fundus()
{
}

void Fundus::setPatientId(const std::wstring &pid)
{
    patientId = pid;
}

void Fundus::setDeviceId(const std::wstring &devid)
{
    deviceId = devid;
}

void Fundus::setMeasureTime(const ATL::COleDateTime &mt)
{
	measureTime = mt;
}

void Fundus::setFixation(unsigned short fx)
{
    fixation = fx;
}

void Fundus::setFlashLevel(unsigned short v)
{
    flashLevel = v;
}

void Fundus::setShutterSpeed(unsigned short v)
{
    shutterSpeed = v;
}

void Fundus::setIso(unsigned short v)
{
    iso = v;
}

void Fundus::setPanorama(unsigned short v)
{
    panorama = v;
}

void Fundus::setStereo(unsigned short v)
{
    stereo = v;
}


int Fundus::post()
{
    web::json::value jsonFundus;
    jsonFundus[L"PatientId"]   = web::json::value::string(patientId);
    jsonFundus[L"DeviceName"]  = web::json::value::string(deviceId);
    jsonFundus[L"Measurement"] = web::json::value::string(L"Color Fundus");
    jsonFundus[L"MeasureTime"] = web::json::value::string(OctHttpSetup::timeToString(measureTime));
    jsonFundus[L"Fixation"] = web::json::value::string(std::to_wstring(fixation));
    jsonFundus[L"EyeSide"] = web::json::value::string(OctHttpSetup::eyeSideToString(eyeSide));
    jsonFundus[L"FlashLevel"] = web::json::value::string(std::to_wstring(flashLevel));
    jsonFundus[L"ShutterSpeed"] = web::json::value::string(std::to_wstring(shutterSpeed));
    jsonFundus[L"Iso"] = web::json::value::string(std::to_wstring(iso));
    jsonFundus[L"Panorama"] = web::json::value::string(std::to_wstring(panorama));
    jsonFundus[L"Stereo"] = web::json::value::string(std::to_wstring(stereo));

    int id;
    try {
        id = OctHttp::getInstance().postJson(L"fundus/", jsonFundus);
    }
    catch (...) {
        throw;
    }

    return id;

}

int Fundus::process(int examId)
{
    std::wstring api(L"process/");
    std::wstring id = std::to_wstring(examId);
    std::wstring path(api + id);

    auto ret = OctHttp::getInstance().get(path);
    return ret;
}

void Fundus::debug()
{
    std::wcout << L"Patient ID = " << patientId << std::endl;
    std::wcout << L"Device ID = " << deviceId << std::endl;
    std::wcout << L"Fundus Pattern = " << OctHttpSetup::patternToString(pattern) << std::endl;
    std::wcout << L"Eye side = " << int(eyeSide) << std::endl;
    std::wcout << L"Measure Time = " << OctHttpSetup::timeToString(measureTime) << std::endl;
}



