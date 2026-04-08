// This is the main DLL file.

#include "stdafx.h"
#include "libHttpRequestHelperCLR.h"

using namespace libHttpRequestHelperCLR;

bool VunoAi::registerLicense(String^ serverUrl, String^ licenseKey)
{
	return libHttpRequestHelper::VunoAi::registerLicense(serverUrl, licenseKey);
}

bool VunoAi::downloadLicenseInfo(String^ serverUrl, String^ resultFilePath)
{
	return libHttpRequestHelper::VunoAi::downloadLicenseInfo(serverUrl, resultFilePath);
}

String^ VunoAi::getUniqueKey(String^ serverUrl)
{
	return libHttpRequestHelper::VunoAi::getUniqueKey(serverUrl);
}

bool VunoAi::requestAnalysis(String^ serverUrl, String^ fileName, String^ filePath, String^ resultFilePath)
{
	return libHttpRequestHelper::VunoAi::requestAnalysis(serverUrl, fileName, filePath, resultFilePath);
}

// cni
bool VunoAi_RegisterLicense(char* serverUrl, char* licenseKey)
{
	String^ strServerUrl = gcnew String(serverUrl);
	String^ strLicenseKey = gcnew String(licenseKey);

	return libHttpRequestHelperCLR::VunoAi::registerLicense(strServerUrl, strLicenseKey);
}

bool VunoAi_DownloadLicenseInfo(char* serverUrl, char* resultFilePath)
{
	String^ strServerUrl = gcnew String(serverUrl);
	String^ strResultFilePath = gcnew String(resultFilePath);

	return libHttpRequestHelperCLR::VunoAi::downloadLicenseInfo(strServerUrl, strResultFilePath);
}


char* VunoAi_GetUniqueKey(char* serverUrl)
{
	String^ strServerUrl = gcnew String(serverUrl);

	auto sysStrUniqueKey = libHttpRequestHelperCLR::VunoAi::getUniqueKey(strServerUrl);
	if (sysStrUniqueKey == nullptr) {
		return nullptr;
	}

	return (char*)(void*)Marshal::StringToHGlobalAnsi(sysStrUniqueKey);
}

bool VunoAi_RequestAnalysis(char* serverUrl, char* fileName, char* filePath, char* resultFilePath)
{
	String^ strServerUrl = gcnew String(serverUrl);
	String^ strFileName = gcnew String(fileName);
	String^ strFilePath = gcnew String(filePath);
	String^ strResultFilePath = gcnew String(resultFilePath);

	return libHttpRequestHelperCLR::VunoAi::requestAnalysis(strServerUrl, strFileName, strFilePath, strResultFilePath);
}