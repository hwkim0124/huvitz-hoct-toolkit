#pragma once

#include "OctGlobalDef.h"
#include "GlobalConfigDef.h"

#include <cstdint>
#include <string>
#include <functional>
#include <memory>


namespace OctGlobal
{
	class OCTGLOBAL_DLL_API GlobalLogger
	{
	public:
		GlobalLogger();
		virtual ~GlobalLogger();

	public:
		static void setLogMsgCallback(LogMsgCallback* clb);
		static void runLogMsgCallback(std::string msg, int level);

		static void setLogMsg(std::string msg, int level);
		static void info(std::string msg) { setLogMsg(msg, LOG_MSG_INFO); }
		static void trace(std::string msg) { setLogMsg(msg, LOG_MSG_TRACE); }
		static void debug(std::string msg) { setLogMsg(msg, LOG_MSG_DEBUG); }
		static void error(std::string msg) { setLogMsg(msg, LOG_MSG_ERROR); }
		static void warn(std::string msg) { setLogMsg(msg, LOG_MSG_WARNING); }
		static void fatal(std::string msg) { setLogMsg(msg, LOG_MSG_FATAL); }

	private:
		struct GlobalLoggerImpl;
		static std::unique_ptr<GlobalLoggerImpl> d_ptr;
		static GlobalLoggerImpl& getImpl(void);
	};
}
