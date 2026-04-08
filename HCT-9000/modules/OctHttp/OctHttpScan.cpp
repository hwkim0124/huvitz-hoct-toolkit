#include "stdafx.h"

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

#include "cpprest/json.h"

#include "OctHttp.h"
#define __OCTHTTP_SCAN__
#include "OctHttpScan.h"

using namespace octhttp;

Scan::Scan(MeasurementType pat, EyeSide es)
    :pattern(pat), eyeSide(es)
{
	measureTime.SetStatus(COleDateTime::null);
}

Scan::~Scan()
{
}

void Scan::setPatientId(const std::wstring &pid)
{
    patientId = pid;
}

void Scan::setDeviceId(const std::wstring &devid)
{
    deviceId = devid;
}

void Scan::setMeasureTime(const ATL::COleDateTime &mt)
{
    measureTime = mt;
}

void Scan::setFixation(unsigned short fx)
{
    fixation = fx;
}

void Scan::setSsi(int si)
{
    ssi = si;
}

void Scan::setSqi(int qi)
{
    sqi = qi;
}

void Scan::setAscans(unsigned short as)
{
    aScans = as;
}

void Scan::setBscans(unsigned short bs)
{
    bScans = bs;
}

void Scan::setScanWidth(float w)
{
    scanWidth = w;
}

void Scan::setScanHeight(float h)
{
    scanHeight = h;
}

void Scan::setScanDirection(unsigned short sd)
{
    scanDirection = sd;
}

void Scan::setXoffset(float xoff)
{
    xOffset = xoff;
}

void Scan::setYoffset(float yoff)
{
    yOffset = yoff;
}

void Scan::setBaseline(int bs)
{
    baseline = bs;
}

void Scan::setCenter(std::pair<double, double> &c)
{
    center = c;
}

void Scan::setDiscArea(std::vector<std::tuple<int, int, int>> &v)
{
    discBorders = v;
}

void Scan::setCupArea(std::vector<std::tuple<int, int, int>> &v)
{
    cupBorders = v;
}


int Scan::post()
{
    web::json::value jsonScan;
    jsonScan[L"PatientId"]   = web::json::value::string(patientId);
    jsonScan[L"DeviceName"]  = web::json::value::string(deviceId);
    jsonScan[L"Measurement"] = web::json::value::string(OctHttpSetup::patternToString(pattern));
    jsonScan[L"MeasureTime"] = web::json::value::string(OctHttpSetup::timeToString(measureTime));
    jsonScan[L"Fixation"] = web::json::value::string(std::to_wstring(fixation));
    jsonScan[L"EyeSide"] = web::json::value::string(OctHttpSetup::eyeSideToString(eyeSide));
    jsonScan[L"SSI"] = web::json::value::string(std::to_wstring(ssi));
    jsonScan[L"SQI"] = web::json::value::string(std::to_wstring(sqi));
    jsonScan[L"Ascans"] = web::json::value::string(std::to_wstring(aScans));
    jsonScan[L"Bscans"] = web::json::value::string(std::to_wstring(bScans));
    jsonScan[L"ScanWidth"] = web::json::value::string(std::to_wstring(scanWidth));
    jsonScan[L"ScanHeight"] = web::json::value::string(std::to_wstring(scanHeight));
    jsonScan[L"ScanDirection"] = web::json::value::string(std::to_wstring(scanDirection));
    jsonScan[L"Xoffset"] = web::json::value::string(std::to_wstring(xOffset));
    jsonScan[L"Yoffset"] = web::json::value::string(std::to_wstring(yOffset));
    jsonScan[L"Baseline"] = web::json::value::string(std::to_wstring(baseline));

    web::json::value jCenter;
    jCenter[L"cx"] = web::json::value::number(std::get<0>(center));
    jCenter[L"cy"] = web::json::value::number(std::get<1>(center));
    jsonScan[L"Center"] = web::json::value::value(jCenter);

    std::vector<web::json::value> vDiscBorder;
    for (auto &p : discBorders) {
        int y, x0, x1;
        std::tie(y, x0, x1) = p;
        web::json::value jBorder;
        jBorder[L"y"] = web::json::value::number(y);
        jBorder[L"x0"] = web::json::value::number(x0);
        jBorder[L"x1"] = web::json::value::number(x1);
        vDiscBorder.push_back(jBorder);
    }
    jsonScan[L"DiscBorders"] = web::json::value::array(vDiscBorder);

    std::vector<web::json::value> vCupBorder;
    for (auto &p : cupBorders) {
        int y, x0, x1;
        std::tie(y, x0, x1) = p;
        web::json::value jBorder;
        jBorder[L"y"] = web::json::value::number(y);
        jBorder[L"x0"] = web::json::value::number(x0);
        jBorder[L"x1"] = web::json::value::number(x1);
        vCupBorder.push_back(jBorder);
    }
    jsonScan[L"CupBorders"] = web::json::value::array(vCupBorder);

    int id;
    try {
        id = OctHttp::getInstance().postJson(L"scans/", jsonScan);
    }
    catch (...) {
        throw;
    }

    return id;
}

int Scan::process(int examId)
{
    std::wstring api(L"process/");
    std::wstring id = std::to_wstring(examId);
    std::wstring path(api + id);

    auto ret = OctHttp::getInstance().get(path);
    return ret;
}

int Scan::cancel(int examId)
{
	std::wstring api(L"cancel/");
	std::wstring id = std::to_wstring(examId);
	std::wstring path(api + id);

	auto ret = OctHttp::getInstance().get(path);
	return ret;
}

void Scan::debug()
{
    std::wcout << L"Patient ID = " << patientId << std::endl;
    std::wcout << L"Device ID = " << deviceId << std::endl;
    std::wcout << L"Scan Pattern = " << OctHttpSetup::patternToString(pattern) << std::endl;
    std::wcout << L"Eye side = " << int(eyeSide) << std::endl;
    std::wcout << L"Measure Time = " << measureTime << std::endl;
}



