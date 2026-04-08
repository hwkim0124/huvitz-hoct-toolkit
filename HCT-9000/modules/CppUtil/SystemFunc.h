#pragma once

#include "CppUtilDef.h"

#include <string>
#include <vector>


namespace CppUtil
{
	class CPPUTIL_DLL_API SystemFunc
	{
	public:
		SystemFunc();
		virtual ~SystemFunc();

	public:
		static bool isDirectory(const std::wstring& dirName);
		static bool isFile(const std::wstring& fileName);
		static bool lastModified(time_t& out_time, const std::wstring& filePath);

		static auto findFilesInDirectory(const std::wstring& directory, const std::wstring& filter = std::wstring(L"*.*"), bool includePath = true)->std::vector<std::wstring>;
		static bool moveFile(const std::wstring& pathFrom, const std::wstring& pathTo);
	};
}
