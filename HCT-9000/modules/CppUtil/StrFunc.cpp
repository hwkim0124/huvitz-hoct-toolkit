#include "stdafx.h"
#include "StrFunc.h"

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

std::string CppUtil::wtoa(const wchar_t * ptr, size_t len)
{
	if (len == -1) {
		len = wcslen(ptr);
	}

	if (len <= 0) {
		return std::string("");
	}

	std::string str(WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, (int)len, nullptr, 0, 0, 0), '\0');

	if (str.size() == 0) {
		throw std::system_error(GetLastError(), std::system_category());
	}

	if (0 == WideCharToMultiByte(CP_THREAD_ACP, 0, ptr, (int)len, &str[0], (int)str.size(), 0, 0)) {
		throw std::system_error(GetLastError(), std::system_category(), "Error converting wide string to narrow");
	}

	return str;
}

std::string CppUtil::wtoa(const std::wstring & wstr)
{
	return wtoa(&wstr[0], wstr.size());
}


std::wstring CPPUTIL_DLL_API CppUtil::atow(const std::string & cstr)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	std::wstring wide = converter.from_bytes(cstr);
	return wide;
}


std::string CppUtil::format_string(const char * format, ...)
{
	char buff[4096];
	va_list ap;
	va_start(ap, format);
	vsnprintf_s(buff, 4096, _TRUNCATE, format, ap);
	va_end(ap);
	return std::string(buff);
}

std::wstring CppUtil::format_string(const wchar_t * format, ...)
{
	wchar_t buff[4096];
	va_list ap;
	va_start(ap, format);
	vswprintf_s(buff, 4096, format, ap);
	va_end(ap);
	return std::wstring(buff);
}
