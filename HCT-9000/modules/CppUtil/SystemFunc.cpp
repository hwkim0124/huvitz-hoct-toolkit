#include "stdafx.h"
#include "SystemFunc.h"

#include <windows.h>
#include <boost\filesystem.hpp>


using namespace CppUtil;
using namespace std;


SystemFunc::SystemFunc()
{
}


SystemFunc::~SystemFunc()
{
}


bool CppUtil::SystemFunc::isDirectory(const std::wstring& dirName)
{
	DWORD attr = GetFileAttributes(dirName.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
		return true;
	}
	return false;
}


bool CppUtil::SystemFunc::isFile(const std::wstring& fileName)
{
	DWORD attr = GetFileAttributes(fileName.c_str());
	if (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
		return true;
	}
	return false;
}

bool CppUtil::SystemFunc::lastModified(time_t& out_time, const std::wstring& filePath)
{
	if (!isFile(filePath)) {
		return false;
	}

	out_time = boost::filesystem::last_write_time(filePath);
	return true;
}

auto CppUtil::SystemFunc::findFilesInDirectory(const std::wstring & directory,
	const std::wstring & filter, bool includePath)->std::vector<std::wstring>
{
	vector<wstring> files;
	WIN32_FIND_DATA findData;
	HANDLE hFind;

	wstring path = directory + L"\\" + filter;

	if (!((hFind = FindFirstFile(path.c_str(), &findData)) == INVALID_HANDLE_VALUE)) {
		do {
			if (includePath) {
				files.push_back(wstring(directory + L"\\" + findData.cFileName));
			}
			else {
				files.push_back(findData.cFileName);
			}
		} while (FindNextFile(hFind, &findData));
	}
	FindClose(hFind);

	return files;
}

bool CppUtil::SystemFunc::moveFile(const std::wstring& pathFrom, const std::wstring& pathTo)
{
	if (!isFile(pathFrom)) {
		return false;
	}

	boost::filesystem::rename(pathFrom, pathTo);

	return true;
}
