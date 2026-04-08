#include "stdafx.h"
#include "Logger.h"

#include <fstream>

#include "CppUtil2.h"

using namespace OctSystem;
using namespace CppUtil;
using namespace std;


struct Logger::LoggerImpl
{
	wstring logsPath;
	wstring logsName;

	LoggerImpl() : logsPath(L".//logs"), logsName(L"OctSystem.txt")
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<Logger::LoggerImpl> Logger::d_ptr(new LoggerImpl());


Logger::Logger()
{
	
}


Logger::~Logger()
{
}


bool OctSystem::Logger::initializeLogger(void)
{
	bool result = createLogsDirectory();
	return result;
}


bool OctSystem::Logger::writeLine(std::wstring text)
{
	wstring line;
	line = getTimeTag();
	line += L" ";
	line += text;
	line += L"\r\n";

	try {
		wstring path = getImpl().logsPath + L"//" + getImpl().logsName;
		wofstream file(path, wofstream::out|wofstream::app);
		file << line;
		file.close();
		return true;
	}
	catch (...) {
		return false;
	}
}


bool OctSystem::Logger::createLogsDirectory(void)
{
	if (CreateDirectory(getImpl().logsPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		return true;
	}
	return false;
}


std::wstring OctSystem::Logger::getTimeTag(void)
{
	CTime time = CTime::GetCurrentTime();
	CString tag = time.Format(_T("[%y:%m:%d_%H:%M:%S]"));
	return wstring(tag);
}


Logger::LoggerImpl & OctSystem::Logger::getImpl(void)
{
	return *d_ptr;
}
