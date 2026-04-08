#pragma once

#include "CppUtilDef.h"
#include "StrFunc.h"

#include <sstream>
#include <thread>
#include <memory>
#include <string>


namespace CppUtil
{
	class CPPUTIL_DLL_API Logger
	{
	public:
		Logger();
		virtual ~Logger();

		enum class LogLevel { LEVEL_TRACE = 0, LEVEL_DEBUG, LEVEL_INFO,
								LEVEL_WARNING, LEVEL_ERROR, LEVEL_FATAL };

	public:
		static bool initLogger(Logger::LogLevel level = LogLevel::LEVEL_INFO, 
								bool srcLine = false, bool outFile = true,
								bool timeTag = true, bool fileKeep = false,
								std::string dirPath = ".//logs", 
								std::string filePrefix = "system",
								std::string fileExt = ".log");
		static bool isInitiated(void);
		static void releaseLogger(void);

		static void setLogLevel(Logger::LogLevel level);
		static void setOutputDebug(bool flag);
		static void setOutputFile(bool flag);
		static bool isDebugMode(void);

		static void write(Logger::LogLevel logLevel, std::string cstr);

		static void trace(const char* format, ...);
		static void trace(std::string cstr);
		static void debug(const char* format, ...);
		static void debug(std::string cstr);
		static void info(const char* format, ...);
		static void info(std::string cstr);
		static void warn(const char* format, ...);
		static void warn(std::string cstr);
		static void error(const char* format, ...);
		static void error(std::string cstr);
		static void fatal(const char* format, ...);
		static void fatal(std::string cstr);

		static bool writeLine(const std::wstring& text);

	protected:
		static bool makeLogsPath(std::string path);
		static bool makeFileName(std::string prefix, std::string fileExt, bool timeTag);
		static void clearOldFiles(std::string prefix, std::string fileExt, bool fileKeep);

		static void initBoostLog(void);
		static std::wstring getTimeTag(void);

	private:
		struct LoggerImpl;
		static std::unique_ptr<LoggerImpl> d_ptr;
		static LoggerImpl& getImpl(void);
	};


	struct CPPUTIL_DLL_API LoggerOut
	{
	public:
		LoggerOut() : level(Logger::LogLevel::LEVEL_INFO) {
		}

		LoggerOut(Logger::LogLevel logLevel) : level(logLevel) {
		}

		LoggerOut(Logger::LogLevel logLevel, const char* location, int line) : level(logLevel) {
			ss << location << "(" << line << "): " << std::this_thread::get_id() << " - ";
		}

		virtual ~LoggerOut() {
			Logger::write(this->level, ss.str());
		}

		std::ostringstream& operator()() { return ss; };
		void operator()(const char* msg) { ss << msg; };

	private:
		std::ostringstream ss;
		Logger::LogLevel level;
	};


#ifdef _DEBUG
#define LogT	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_TRACE, __FILE__, __LINE__)
#define LogD	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_DEBUG, __FILE__, __LINE__)
#define LogI		CppUtil::LoggerOut(Logger::LogLevel::LEVEL_INFO, __FILE__, __LINE__)
#define LogW		CppUtil::LoggerOut(Logger::LogLevel::LEVEL_WARNING, __FILE__, __LINE__)
#define LogE	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_ERROR, __FILE__, __LINE__)
#define LogF	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_FATAL, __FILE__, __LINE__)
#else 
#define LogT	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_TRACE)
#define LogD	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_DEBUG)
#define LogI		CppUtil::LoggerOut(Logger::LogLevel::LEVEL_INFO)
#define LogW		CppUtil::LoggerOut(Logger::LogLevel::LEVEL_WARNING)
#define LogE	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_ERROR)
#define LogF	CppUtil::LoggerOut(Logger::LogLevel::LEVEL_FATAL)
#endif

}
