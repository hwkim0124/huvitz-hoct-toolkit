#include "stdafx.h"
#include "Logger.h"

#include <fstream>
#include <sstream>
#include <thread>
#include <Windows.h>
#include <ctime>

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

using namespace CppUtil;
using namespace std;
using namespace boost;


//#define BOOST_LOG_DYN_LINK 1


// Register a global logger
BOOST_LOG_GLOBAL_LOGGER(logger, boost::log::sources::severity_logger_mt<boost::log::trivial::severity_level>)


// Just a helper macro used by the macros below.
#define LOG(severity) BOOST_LOG_SEV(logger::get(), boost::log::trivial::severity)


#include <boost/log/core/core.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/core/null_deleter.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <ostream>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace attrs = boost::log::attributes;


BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", logging::trivial::severity_level)


BOOST_LOG_GLOBAL_LOGGER_INIT(logger, src::severity_logger_mt) {
	src::severity_logger_mt<boost::log::trivial::severity_level> logger;

	// add attributes
	logger.add_attribute("LineID", attrs::counter<unsigned int>(1));     // lines are sequentially numbered
	logger.add_attribute("TimeStamp", attrs::local_clock());             // each log line gets a timestamp

	return logger;
}

// Log macros
#define LOG_TRACE   LOG(trace)
#define LOG_DEBUG   LOG(debug)
#define LOG_INFO    LOG(info)
#define LOG_WARNING LOG(warning)
#define LOG_ERROR   LOG(error)
#define LOG_FATAL   LOG(fatal)


struct Logger::LoggerImpl
{
	bool outputDebug;
	bool outputFile;
	bool initiated;

	string logDirPath;
	string logFileName;
	string logFilePath;
	string logFileTag;

	char cbuff[8192];
	const int buffSize = 8192;

	log::trivial::severity_level level;
	bool logSrcLine;
	bool logTimeTag;
	bool logFileKeep;

	LoggerImpl() : cbuff{ 0 }, level(log::trivial::info),
		logSrcLine(true), logTimeTag(false), logFileKeep(false), outputDebug(true), outputFile(false), initiated(false),
		logDirPath(".//logs"), logFileName("system.log"), logFilePath(".//system.log")
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


bool CppUtil::Logger::initLogger(Logger::LogLevel level, bool srcLine, bool outFile, bool timeTag, bool fileKeep,
							std::string dirPath,
							std::string filePrefix, std::string fileExt)
{
	setLogLevel(level);
	setOutputFile(outFile);
	d_ptr->logSrcLine = srcLine;
	d_ptr->logTimeTag = timeTag;
	d_ptr->logFileKeep = fileKeep;

#ifdef _DEBUG
	setOutputDebug(true);
	setOutputFile(false);
	d_ptr->logSrcLine = true;
#endif
	
	if (outFile) {
		if (!makeLogsPath(dirPath)) {
			return false;
		}
		if (!makeFileName(filePrefix, fileExt, timeTag)) {
			return false;
		}

		clearOldFiles(filePrefix, fileExt, fileKeep);
	}

	initBoostLog();

	d_ptr->initiated = true;
	return isInitiated();
}


bool CppUtil::Logger::isInitiated(void)
{
	return getImpl().initiated;
}


void CppUtil::Logger::releaseLogger(void)
{
	logging::core::get()->flush();
	return;
}


void CppUtil::Logger::setLogLevel(Logger::LogLevel level)
{
	if (level == LogLevel::LEVEL_TRACE) {
		getImpl().level = log::trivial::trace;
	}
	else if (level == LogLevel::LEVEL_DEBUG) {
		getImpl().level = log::trivial::debug;
	}
	else if (level == LogLevel::LEVEL_INFO) {
		getImpl().level = log::trivial::info;
	}
	else if (level == LogLevel::LEVEL_WARNING) {
		getImpl().level = log::trivial::warning;
	}
	else if (level == LogLevel::LEVEL_ERROR) {
		getImpl().level = log::trivial::error;
	}
	else if (level == LogLevel::LEVEL_FATAL) {
		getImpl().level = log::trivial::fatal;
	}
	else {
		getImpl().level = log::trivial::info;
	}
	return;
}


void CppUtil::Logger::setOutputDebug(bool flag)
{
	getImpl().outputDebug = flag;
	return;
}


void CppUtil::Logger::setOutputFile(bool flag)
{
	getImpl().outputFile = flag;
	return;
}


bool CppUtil::Logger::isDebugMode(void)
{
	return (d_ptr->level < log::trivial::info);
}


void CppUtil::Logger::write(Logger::LogLevel logLevel, std::string cstr)
{
	/*
	if (!d_ptr->logSrcLine) {
		cstr.empty();
	}
	*/

	switch (logLevel) {
		case Logger::LogLevel::LEVEL_DEBUG:
			LOG_DEBUG << cstr;
			break;
		case Logger::LogLevel::LEVEL_INFO:
			LOG_INFO << cstr;
			break;
		case Logger::LogLevel::LEVEL_WARNING:
			LOG_WARNING << cstr;
			break;
		case Logger::LogLevel::LEVEL_ERROR:
			LOG_ERROR << cstr;
			break;
		case Logger::LogLevel::LEVEL_FATAL:
			LOG_FATAL << cstr;
			break;
		case Logger::LogLevel::LEVEL_TRACE:
		default:
			LOG_TRACE << cstr;
			break;
	}
	
	if (static_cast<int>(logLevel) >= d_ptr->level) {
		logging::core::get()->flush();
	}
	/* for TAS
	const char* fixedLogFileName = "D:\//HOCT_LOG.log";
	std::ofstream fixedFile(fixedLogFileName, std::ios_base::app);
	if (fixedFile.is_open() && logLevel != Logger::LogLevel::LEVEL_DEBUG) {
		std::wstring wtime = getTimeTag();
		std::string timeStr(wtime.begin(), wtime.end());

		const char* levelStr = nullptr;
		switch (logLevel) {
		case Logger::LogLevel::LEVEL_TRACE:   levelStr = "[TRACE] "; break;
		case Logger::LogLevel::LEVEL_DEBUG:   levelStr = "[DEBUG] "; break;
		case Logger::LogLevel::LEVEL_INFO:    levelStr = "[INFO] "; break;
		case Logger::LogLevel::LEVEL_WARNING: levelStr = "[WARN] "; break;
		case Logger::LogLevel::LEVEL_ERROR:   levelStr = "[ERROR] "; break;
		case Logger::LogLevel::LEVEL_FATAL:   levelStr = "[FATAL] "; break;
		default: levelStr = "[INFO] "; break;
		}

		fixedFile << timeStr << " " << levelStr << cstr << std::endl;
		fixedFile.close();
	}
	*/

	// If _DEBUG defined, print log record into Visual stduio's output window. 
	if (d_ptr->outputDebug) {
		cstr += "\n";
		OutputDebugStringA(cstr.c_str());
	}
	return;
}


void CppUtil::Logger::trace(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_TRACE, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::trace(std::string cstr)
{
	write(LogLevel::LEVEL_TRACE, cstr);
	return;
}


void CppUtil::Logger::debug(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_DEBUG, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::debug(std::string cstr)
{
	write(LogLevel::LEVEL_DEBUG, cstr);
	return;
}


void CppUtil::Logger::info(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_INFO, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::info(std::string cstr)
{
	write(LogLevel::LEVEL_INFO, cstr);
	return;
}


void CppUtil::Logger::warn(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_WARNING, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::warn(std::string cstr)
{
	write(LogLevel::LEVEL_WARNING, cstr);
	return;
}


void CppUtil::Logger::error(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_ERROR, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::error(std::string cstr)
{
	write(LogLevel::LEVEL_ERROR, cstr);
	return;
}


void CppUtil::Logger::fatal(const char * format, ...)
{
	va_list ap;
	va_start(ap, format);
	::vsnprintf_s(d_ptr->cbuff, d_ptr->buffSize, _TRUNCATE, format, ap);
	va_end(ap);
	write(LogLevel::LEVEL_FATAL, d_ptr->cbuff);
	return;
}


void CppUtil::Logger::fatal(std::string cstr)
{
	write(LogLevel::LEVEL_FATAL, cstr);
	return;
}


bool CppUtil::Logger::writeLine(const std::wstring& text)
{
	wstring line;
	line = getTimeTag();
	line += L" ";
	line += text;
	line += L"\r\n";

	/*
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
	*/
	return false;
}


bool CppUtil::Logger::makeLogsPath(std::string path)
{
	if (CreateDirectory(atow(path).c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError()) {
		getImpl().logDirPath = path;
		return true;
	}
	return false;
}


bool CppUtil::Logger::makeFileName(std::string prefix, std::string fileExt, bool timeTag)
{
	if (!timeTag) {
		getImpl().logFileName = (format("%s%s") % prefix % fileExt).str();
	}
	else {
		CTime time = CTime::GetCurrentTime();
		CString ttag = time.Format(_T("%y%m%d_%H%M%S"));
		getImpl().logFileTag = wtoa(ttag);
		getImpl().logFileName = (format("%s-%s%s") % prefix % wtoa(ttag) % fileExt).str();
	}

	getImpl().logFilePath = (format("%s//%s") % d_ptr->logDirPath % d_ptr->logFileName).str();
	return true;
}


void CppUtil::Logger::clearOldFiles(std::string prefix, std::string fileExt, bool fileKeep)
{
	if (fileKeep) {
		return;
	}

	auto logfile = (format("%s-*%s") % prefix % fileExt).str();
	string pattern(d_ptr->logDirPath);
	pattern.append("//").append(logfile);
	WIN32_FIND_DATAA data;
	HANDLE hFind;

	CTime time = CTime::GetCurrentTime() - CTimeSpan(30, 0, 0, 0);
	auto ttag = wtoa(time.Format(_T("%y%m%d")));

	if ((hFind = FindFirstFileA(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
		do {
			string fileName = data.cFileName;
			string filePath(d_ptr->logDirPath);
			filePath.append("//").append(fileName);

			if (fileName.size() > 13) {
				auto ftag = fileName.substr(7, 6);
				if (atoi(ftag.c_str()) >= atoi(ttag.c_str())) {
					// LogD() << filePath;
				}
				else {
					LogD() << "Delete Log: " << filePath;
					DeleteFileA(filePath.c_str());
				}
			}
			
		} while (FindNextFileA(hFind, &data) != 0);
		FindClose(hFind);
	}

	return;
}


void CppUtil::Logger::initBoostLog(void)
{
	typedef sinks::synchronous_sink<sinks::text_ostream_backend> text_sink;
	boost::shared_ptr<text_sink> sink = boost::make_shared<text_sink>();

	if (getImpl().outputFile) {
		// add a logfile stream to our sink
		sink->locked_backend()->add_stream(boost::make_shared<std::ofstream>(getImpl().logFilePath));
	}

	// add "console" output stream to our sink
	// sink->locked_backend()->add_stream(boost::shared_ptr<std::ostream>(&std::clog, boost::null_deleter()));

	// specify the format of the log message
	logging::formatter formatter = expr::stream
		<< std::setw(7) << std::setfill('0') << line_id << std::setfill(' ') << " | "
		// << expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S.%f") << " "
		<< expr::format_date_time(timestamp, "%Y-%m-%d, %H:%M:%S") << " "
		<< "[" << logging::trivial::severity << "]"
		<< " - " << expr::smessage;
	sink->set_formatter(formatter);

	// only messages with severity >= SEVERITY_THRESHOLD are written
	sink->set_filter(severity >= getImpl().level);

	// "register" our sink
	logging::core::get()->add_sink(sink);
	return;
}


std::wstring CppUtil::Logger::getTimeTag(void)
{
	CTime time = CTime::GetCurrentTime();
	CString tag = time.Format(_T("[%y:%m:%d_%H:%M:%S]"));
	return wstring(tag);
}


Logger::LoggerImpl & CppUtil::Logger::getImpl(void)
{
	return *d_ptr;
}

