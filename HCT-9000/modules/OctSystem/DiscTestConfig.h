#pragma once

#include <windows.h>
#include <atlbase.h>
#include <string>

namespace DiscTest {
	inline bool IsEnabled() {
		CRegKey key;
		const wchar_t* kPath = L"SOFTWARE\\CodeGenerator\\DiscTestOption";
		const wchar_t* kValue = L"DiscTestEnabled";

		if (key.Open(HKEY_CURRENT_USER, kPath) == ERROR_SUCCESS) {
			DWORD val = 0;
			if (key.QueryDWORDValue(kValue, val) == ERROR_SUCCESS) {
				return val != 0;
			}
		}
		return false;
	}
}
