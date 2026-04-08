#include "stdafx.h"
#include "GlobalLogger.h"

#include "CppUtil2.h"

using namespace OctGlobal;
using namespace CppUtil;
using namespace std;


struct GlobalLogger::GlobalLoggerImpl
{
	LogMsgCallback* cbLogMsg;

	GlobalLoggerImpl()
		: cbLogMsg(nullptr)
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<GlobalLogger::GlobalLoggerImpl> GlobalLogger::d_ptr(new GlobalLoggerImpl());


GlobalLogger::GlobalLogger()
{
}


GlobalLogger::~GlobalLogger()
{
}


void OctGlobal::GlobalLogger::setLogMsgCallback(LogMsgCallback * clb)
{
	d_ptr->cbLogMsg = clb;
	return;
}


void OctGlobal::GlobalLogger::runLogMsgCallback(std::string msg, int level)
{
	if (d_ptr->cbLogMsg) {
		(*d_ptr->cbLogMsg)(msg, level);
	}
	return;
}


GlobalLogger::GlobalLoggerImpl & OctGlobal::GlobalLogger::getImpl(void)
{
	return *d_ptr;
}


void OctGlobal::GlobalLogger::setLogMsg(std::string msg, int level)
{
	switch (level) {
	case LOG_MSG_TRACE:
		LogT() << msg;
		break;
	case LOG_MSG_DEBUG:
		LogD() << msg;
		break;
	case LOG_MSG_WARNING:
		LogW() << msg;
		break;
	case LOG_MSG_ERROR:
		LogE() << msg;
		break;
	case LOG_MSG_FATAL:
		LogF() << msg;
		break;
	case LOG_MSG_INFO:
	default:
		LogI() << msg;
		break;
	}

	runLogMsgCallback(msg, level);
	return;
}
