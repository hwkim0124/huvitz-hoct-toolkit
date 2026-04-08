#pragma once

#include "OctGlobalDef.h"

#include <cstdint>
#include <string>
#include <functional>


namespace OctGlobal
{
	constexpr char* SYSTEM_CONFIG_FILE_NAME = "HctConfig.xml";

	constexpr int LOG_MSG_TRACE = 0;
	constexpr int LOG_MSG_DEBUG = 1;
	constexpr int LOG_MSG_INFO = 2;
	constexpr int LOG_MSG_WARNING = 3;
	constexpr int LOG_MSG_ERROR = 4;
	constexpr int LOG_MSG_FATAL = 5;


	// Log Callback types 
	/////////////////////////////////////////////////////////////////////////////////////////////////////////
	typedef std::function<void(std::string, int)> LogMsgCallback;

};