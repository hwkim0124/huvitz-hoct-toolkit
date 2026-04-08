#pragma once

#include <sstream>
#include <thread>
#include <Windows.h>

#include "CppUtilDef.h"
#include "StrFunc.h"

namespace CppUtil
{
	// Visual studio's output window doesn't support c++ standard cout, 
	// instead use OutputDebugStringA().
	// Refer: http://coliru.stacked-crooked.com/a/481b444c09361f1d
	struct CPPUTIL_DLL_API DebugOutImpl
	{
	public:
		DebugOutImpl(const char* location, int line, bool enabled_ = true)
			: enabled(enabled_)
		{
			ss << location << "(" << line << "): " << std::this_thread::get_id() << " - "; 
		}

		DebugOutImpl(bool enabled_ = true)
			: enabled(enabled_)
		{
		}
		
		~DebugOutImpl() { ss << "\n"; OutputDebugStringA(ss.str().c_str()); };

		std::ostringstream& operator()() { return ss; };
		void operator()(const char* msg) { if (enabled) ss << msg; };
		void operator()(const wchar_t* msg) { if (enabled) ss << wtoa(msg); };

		template<class char_type>
		void operator()(const char_type* format, ...) {
			if (enabled) {
				char_type buf[4096];
				va_list ap;
				va_start(ap, format);
				vsnprintf_s(buf, 4096, format, ap);
				va_end(ap);
				operator()(buf);
			}
		}

		static int vsnprintf_s(char* buffer, int bufsize, const char* format, va_list ap) { return ::vsnprintf_s(buffer, bufsize, _TRUNCATE, format, ap); };
		static int vsnprintf_s(wchar_t* buffer, int bufsize, const wchar_t* format, va_list ap) { return ::vswprintf_s(buffer, bufsize, format, ap); };

	private:
		std::ostringstream ss;
		bool enabled;
	};
}

#ifdef _DEBUG
#define DebugOut	CppUtil::DebugOutImpl()
#define DebugOut2	CppUtil::DebugOutImpl(__FILE__, __LINE__)
#else
#define DebugOut	/ ## / // Note that if DebugOut() is comprised of multiple lines, this can't make them commented out. 
#define DebugOut2	/ ## / // Use the _DEBUG directive instead. 
#endif