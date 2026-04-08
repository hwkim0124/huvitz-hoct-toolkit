#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <string>


namespace OctSystem
{
	class OCTSYSTEM_DLL_API Logger
	{
	public:
		Logger();
		virtual ~Logger();

	public:
		static bool initializeLogger(void);
		static bool writeLine(std::wstring text);

	protected:
		static bool createLogsDirectory(void);
		static std::wstring getTimeTag(void);

	private:
		struct LoggerImpl;
		static std::unique_ptr<LoggerImpl> d_ptr;
		static LoggerImpl& getImpl(void);
	};
}
