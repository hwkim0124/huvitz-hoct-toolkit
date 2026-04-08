#pragma once

#include "CppUtilDef.h"

#include <string>
#include <cwchar>
#include <locale>
#include <codecvt>


namespace CppUtil
{
	std::string CPPUTIL_DLL_API wtoa(const wchar_t* ptr, size_t len = -1);
	std::string CPPUTIL_DLL_API wtoa(const std::wstring& wstr);


	// This requires automatic return type deduction!
	// http://stackoverflow.com/questions/4530756/c-function-template-with-flexible-return-type
	// http://stackoverflow.com/questions/29492242/c-deriving-a-function-return-type-using-decltype
	// http://www.cprogramming.com/c++11/c++11-auto-decltype-return-value-after-function.html
	// http://stackoverflow.com/questions/15737223/when-should-i-use-c14-automatic-return-type-deduction
	/*
	template<class char_type>
	auto fstring(const char_type * format, ...)
	{
		char_type buf[4096];
		va_list ap;
		va_start(ap, format);
		vsnprintf_s(buff, 4096, _TRUNCATE, format, ap);
		va_end(ap);
		return auto(buff);
	}
	*/

	std::wstring CPPUTIL_DLL_API atow(const std::string& cstr);

	std::string CPPUTIL_DLL_API format_string(const char* format, ...);
	std::wstring CPPUTIL_DLL_API format_string(const wchar_t* format, ...);

	
}

