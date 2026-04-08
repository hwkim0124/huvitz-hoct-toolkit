// libHttpRequestHelperCLR.h

#pragma once

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::InteropServices;

__declspec(dllexport) bool VunoAi_RegisterLicense(char* serverUrl, char* licenseKey);
__declspec(dllexport) bool VunoAi_RequestAnalysis(char* serverUrl, char* fileName, char* filePath, char* resultFilePath);
__declspec(dllexport) char* VunoAi_GetUniqueKey(char* serverUrl);
__declspec(dllexport) bool VunoAi_DownloadLicenseInfo(char* serverUrl, char* resultFilePath);

namespace libHttpRequestHelperCLR {

	public ref class VunoAi
	{
	public:
		static bool registerLicense(String^ serverUrl, String^ licenseKey);
		static bool downloadLicenseInfo(String^ serverUrl, String^ resultFilePath);
		static String^ getUniqueKey(String^ serverUrl);
		static bool requestAnalysis(String^ serverUrl, String^ fileName, String^ filePath, String^ resultFilePath);
	};
}
