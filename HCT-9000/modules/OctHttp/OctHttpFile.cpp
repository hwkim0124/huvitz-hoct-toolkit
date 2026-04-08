#include "stdafx.h"

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

#include "cpprest/json.h"

#include "OctHttp.h"
#include "OctHttpSetup.h"

#define __OCTHTTP_FILE__
#include "OctHttpFile.h"

using namespace octhttp;

File::File(int id)
    : examId(id)
{
}

File::~File()
{
}

int File::post(const std::wstring& filePath)
{
    std::wstring api(L"files/");
    std::wstring id = std::to_wstring(examId);
    std::wstring path(api + id);

    int ret = OctHttp::getInstance().postFile(path, filePath);
    return ret;
}

int File::post2(const std::wstring& filePath, const std::wstring& customFileName)
{
	std::wstring api(L"files/");
	std::wstring id = std::to_wstring(examId);
	std::wstring path(api + id);

	int ret = OctHttp::getInstance().postFile2(path, filePath, customFileName);
	return ret;
}

void File::debug()
{
}



