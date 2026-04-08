// OctLicenseManager.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "OctLicenseManager.h"
#include <vector>
#include <sstream>
#include <algorithm>
#include <tuple>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// The one and only application object


CWinApp theApp;

using namespace std;

const static std::wstring FAKE_ORG_NAME = L"CodeGenerator";
const static std::wstring FAKE_APP_NAME = L"CodeGenerator_";
// angiography
const static std::wstring LICENSE_REGCODE = L"RegistrationCode";
const static std::wstring LICENSE_EXPIREDATE = L"ExpireDate";
const static std::wstring LICENSE_EXPIREDATE_HASH = L"ExpireDateHash";
const static std::wstring LICENSE_STATE_ANGIO = L"LicenseStateAngio";
// biometry
const static std::wstring LICENSE_REGCODE_BIOMETRY = L"RegistrationCodeBiometry";
const static std::wstring LICENSE_EXPIREDATE_BIOMETRY = L"ExpireDateBiometry";
const static std::wstring LICENSE_EXPIREDATE_HASH_BIOMETRY = L"ExpireDateHashBiometry";
const static std::wstring LICENSE_STATE_BIOMETRY = L"LicenseStateBiometry";
// topography
const static std::wstring LICENSE_REGCODE_TOPOGRAPHY = L"RegistrationCodeTopography";
const static std::wstring LICENSE_EXPIREDATE_TOPOGRAPHY = L"ExpireDateTopography";
const static std::wstring LICENSE_EXPIREDATE_HASH_TOPOGRAPHY = L"ExpireDateHashTopography";
const static std::wstring LICENSE_STATE_TOPOGRAPHY = L"LicenseStateTopography";

const static std::wstring kHoctProductKey = L"HuvitzHoct";
const static std::wstring kHoctLicenseRegSubKey = L"SOFTWARE\\" + FAKE_ORG_NAME + L"\\" + FAKE_APP_NAME;
const static std::wstring kHoctLicenseKeyTrialPostFix = L"Trial";

static vector<IP_ADAPTER_INFO> _getMAC(void);

typedef struct _LicenseItem {
	COctLicenseManager::OctLicenseTarget target;
	std::wstring licenseKey;
	std::wstring regCode;
	std::wstring regExpireDate;
	std::wstring regExpireDateHash;
	std::wstring regLicenseKeyState;
} LicenseItem;

static std::vector<LicenseItem> s_licenseKeyItemList = {
	{
		COctLicenseManager::OctLicenseTarget::HOCT_ANGIOGRAPHY,
		L"OctAngio",
		LICENSE_REGCODE,
		LICENSE_EXPIREDATE,
		LICENSE_EXPIREDATE_HASH,
		LICENSE_STATE_ANGIO,
	},
	{
		COctLicenseManager::OctLicenseTarget::HOCT_TOPOGRAPHY,
		L"OctTopography",
		LICENSE_REGCODE_TOPOGRAPHY,
		LICENSE_EXPIREDATE_TOPOGRAPHY,
		LICENSE_EXPIREDATE_HASH_TOPOGRAPHY,
		LICENSE_STATE_TOPOGRAPHY,
	},
	{
		COctLicenseManager::OctLicenseTarget::HOCT_BIOMETRY,
		L"OctBiometry",
		LICENSE_REGCODE_BIOMETRY,
		LICENSE_EXPIREDATE_BIOMETRY,
		LICENSE_EXPIREDATE_HASH_BIOMETRY,
		LICENSE_STATE_BIOMETRY,
	},
};

bool COctLicenseManager::checkLicense(const std::wstring productId, const std::wstring licenseCode, OctLicenseTarget target)
{
	auto productIdU = toUpper(simplify(productId));
	auto licenseCodeU = toUpper(simplify(licenseCode));

	if (licenseCodeU == getLicenseCode(productIdU, target)) {
		return true;
	}
	else {
		return false;
	}
}

bool COctLicenseManager::checkLicenseTrial(const std::wstring productId, const std::wstring licenseCode, OctLicenseTarget target)
{
	auto productIdU = toUpper(simplify(productId));
	auto licenseCodeU = toUpper(simplify(licenseCode));

	if (licenseCodeU == getLicenseCodeTrial(productIdU, target)) {
		return true;
	}
	else {
		return false;
	}
}

std::wstring COctLicenseManager::getLicenseCode(const std::wstring productId, OctLicenseTarget target)
{
	auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
		[target](auto item) { return item.target == target; });
	if (licenseItemItr == s_licenseKeyItemList.end()) {
		return L"";
	}

	return getLicenseCodeByLicenseKey(productId, (*licenseItemItr).licenseKey);
}

std::wstring COctLicenseManager::getLicenseCodeTrial(const std::wstring productId, OctLicenseTarget target)
{
	auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
		[target](auto item) { return item.target == target; });
	if (licenseItemItr == s_licenseKeyItemList.end()) {
		return L"";
	}

	return getLicenseCodeByLicenseKey(productId, (*licenseItemItr).licenseKey + kHoctLicenseKeyTrialPostFix);
}

std::wstring COctLicenseManager::getProductId(const std::wstring hwAddr)
{
	auto simple = toUpper(simplify(hwAddr));

	auto hashCode = std::hash<std::wstring>{}(simple + kHoctProductKey);

	std::wstringbuf buffer;		// empty buffer
	std::wostream os(&buffer);	// associate stream buffer to stream

	os << hashCode;

	//
	auto strHashCode = buffer.str();
	std::wstring strResult;
	const int kLenGap = kProductIdLen - (int)strHashCode.length();
	if (kLenGap < 0) {
		strResult = strHashCode.substr(0, kProductIdLen);
	}
	else if (kLenGap > 0) {
		strResult = strHashCode;
		for (int i = 0; i < kLenGap; i++) {
			strResult.push_back('P');
		}
	}
	else {
		strResult = strHashCode;
	}

	return strResult;
}

std::wstring COctLicenseManager::getHwAddr(OctLicenseTarget target)
{
	auto hwAddrList = _getMAC();

	// get string version
	int cnt = 0;
	std::vector<CString> strHwAddrList;
	for (auto hwAddr : hwAddrList) {
		CString strHwAddr;
		strHwAddr.Format(_T("%02X%02X%02X%02X%02X%02X"),
			hwAddr.Address[0], hwAddr.Address[1],
			hwAddr.Address[2], hwAddr.Address[3],
			hwAddr.Address[4], hwAddr.Address[5]);

		strHwAddrList.push_back(strHwAddr);
		cnt++;
	}

	// sort - 생산에서 관리하는 대상과 달라질 수 있다. 소팅은 하지 않는다.
	//CString* pStrArr = &strHwAddrList[0];
	//std::qsort(pStrArr, cnt, sizeof CString, [](const void* a, const void* b)
	//{
	//	auto str1 = *static_cast<const CString*>(a);
	//	auto str2 = *static_cast<const CString*>(b);

	//	if (str1 < str2) return -1;
	//	if (str1 > str2) return 1;

	//	return 0;
	//});

	// return the first one
	
	std::wstring strResult;
	
	bool isFirst = readLicenseCodeState(target);
	
	if (isFirst) {
		strResult = CT2CW(strHwAddrList.at(0));
	}
	else {
		strResult = CT2CW(strHwAddrList.at(1));
	}

	return strResult;
}

bool COctLicenseManager::writeLicenseCode(const std::wstring licenseCode, OctLicenseTarget target)
{
	bool bResult(false);
	auto strSimple = toUpper(simplify(licenseCode));

	auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
		[target](auto item) { return item.target == target; });
	if (licenseItemItr == s_licenseKeyItemList.end()) {
		return false;
	}

	CRegKey cKey;
	if (ERROR_SUCCESS == cKey.Create(HKEY_CURRENT_USER, kHoctLicenseRegSubKey.c_str())) {
		bResult = (ERROR_SUCCESS == cKey.SetStringValue((*licenseItemItr).regCode.c_str(), strSimple.c_str()));
	}

	return bResult;
}

bool COctLicenseManager::writeExpireTime(CTime timeExpire, OctLicenseTarget target)
{
	auto szExpireTime = timeExpire.Format(_T("%m/%d/%Y"));

	CRegKey cKeyValue;
	if (ERROR_SUCCESS != cKeyValue.Create(HKEY_CURRENT_USER, kHoctLicenseRegSubKey.c_str())) {
		return false;
	}

	auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
		[target](auto item) { return item.target == target; });
	if (licenseItemItr == s_licenseKeyItemList.end()) {
		return false;
	}

	// expire date
	if (ERROR_SUCCESS != cKeyValue.SetStringValue((*licenseItemItr).regExpireDate.c_str(), szExpireTime)) {
		return false;
	}
	// hash
	std::wstring strExpireTime = CT2CW(szExpireTime);
	std::wstring strExpireTimeSimple = simplify(strExpireTime);
	auto hashedCode = std::hash<std::wstring>{}(strExpireTimeSimple);
	std::wstring strHashedCode;
	{
		std::wstringbuf buffer;		// empty buffer
		std::wostream os(&buffer);	// associate stream buffer to stream

		os << hashedCode;

		strHashedCode = buffer.str();
	}

	if (ERROR_SUCCESS != cKeyValue.SetStringValue((*licenseItemItr).regExpireDateHash.c_str(), strHashedCode.c_str())) {
		return false;
	}

	return true;
}

bool COctLicenseManager::readLicenseCodeState(OctLicenseTarget target)
{
	HKEY key;
	bool isFirst = false;

	do {
		RegOpenKeyEx(HKEY_CURRENT_USER, kHoctLicenseRegSubKey.c_str(), 0, KEY_READ, &key);
		if (key == NULL) {
			isFirst = true;
			break;
		}

		auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
			[target](auto item) { return item.target == target; });
		if (licenseItemItr == s_licenseKeyItemList.end()) {
			isFirst = true;
			return isFirst;
		}

		DWORD buf_size = sizeof(DWORD);
		DWORD value;

		if (RegQueryValueEx(key, (*licenseItemItr).regLicenseKeyState.c_str(), NULL, NULL, (LPBYTE)&value, &buf_size) != ERROR_SUCCESS) {
			isFirst = true;
			break;
		}

		isFirst = (bool)value;

		RegCloseKey(key);

		return isFirst;
	} while (0);

	RegCloseKey(key);

	return isFirst;
}

bool COctLicenseManager::readLicenseCode(std::wstring& out_licenseCode, OctLicenseTarget target)
{
	HKEY key;

	do {
		RegOpenKeyEx(HKEY_CURRENT_USER, kHoctLicenseRegSubKey.c_str(), 0, KEY_READ, &key);
		if (key == NULL) {
			break;
		}

		auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
			[target](auto item) { return item.target == target; });
		if (licenseItemItr == s_licenseKeyItemList.end()) {
			return false;
		}

		DWORD type;
		DWORD cbData;
		if (RegQueryValueEx(key, (*licenseItemItr).regCode.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS) {
			break;
		}

		if (type != REG_SZ) {
			break;
		}

		wstring value(cbData / sizeof(wchar_t), L'\0');
		if (RegQueryValueEx(key, (*licenseItemItr).regCode.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS) {
			break;
		}

		auto strSimple = simplify(value);
		if (strSimple.length() != kLicenseCodeLen) {
			break;
		}

		out_licenseCode = strSimple;

		RegCloseKey(key);

		return true;
	} while (0);

	RegCloseKey(key);

	return false;
}

bool COctLicenseManager::readExpireTime(CTime& out_timeExpire, OctLicenseTarget target)
{
	HKEY key;

	do {
		RegOpenKeyEx(HKEY_CURRENT_USER, kHoctLicenseRegSubKey.c_str(), 0, KEY_READ, &key);
		if (key == NULL) {
			break;;
		}

		auto licenseItemItr = std::find_if(s_licenseKeyItemList.begin(), s_licenseKeyItemList.end(),
			[target](auto item) { return item.target == target; });
		if (licenseItemItr == s_licenseKeyItemList.end()) {
			return false;
		}

		DWORD type;
		DWORD cbData;

		// expire date
		if (RegQueryValueEx(key, (*licenseItemItr).regExpireDate.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS) {
			break;
		}

		if (type != REG_SZ) {
			break;
		}

		wstring strExpireDate(cbData / sizeof(wchar_t), L'\0');
		if (RegQueryValueEx(key, (*licenseItemItr).regExpireDate.c_str(), NULL, NULL,
			reinterpret_cast<LPBYTE>(&strExpireDate[0]), &cbData) != ERROR_SUCCESS) {
			break;
		}

		// hash code
		if (RegQueryValueEx(key, (*licenseItemItr).regExpireDateHash.c_str(), NULL, &type, NULL, &cbData) != ERROR_SUCCESS) {
			break;
		}

		if (type != REG_SZ) {
			break;
		}

		wstring strExpireDateHash(cbData / sizeof(wchar_t), L'\0');
		if (RegQueryValueEx(key, (*licenseItemItr).regExpireDateHash.c_str(), NULL, NULL,
 			reinterpret_cast<LPBYTE>(&strExpireDateHash[0]), &cbData) != ERROR_SUCCESS) {
			break;
		}

		// check validation
		// - hash from expire date
		auto strExpireDateSimple = simplify(strExpireDate);
		auto hashedCodeFromExpireDate = std::hash<std::wstring>{}(strExpireDateSimple);
		std::wstring strHashedCodeFromExpireDate;
		{
			std::wstringbuf buffer;		// empty buffer
			std::wostream os(&buffer);	// associate stream buffer to stream

			os << hashedCodeFromExpireDate;

			strHashedCodeFromExpireDate = buffer.str();
		}
		if (simplify(strExpireDateHash) != simplify(strHashedCodeFromExpireDate)) {
			break;
		}

		CString szExpireDate(strExpireDateSimple.c_str());

		COleDateTime coleTimeExpireDate;
		if (!coleTimeExpireDate.ParseDateTime(szExpireDate)) {
			break;
		}
		SYSTEMTIME st;
		if (!coleTimeExpireDate.GetAsSystemTime(st)) {
			break;
		}

		out_timeExpire = st;

		RegCloseKey(key);

		return true;
	} while (0);

	RegCloseKey(key);

	return false;
}

std::wstring COctLicenseManager::toUpper(std::wstring input)
{
	auto strUpper = input;

	std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(), ::towupper);

	return strUpper;
}

std::wstring COctLicenseManager::simplify(std::wstring input)
{
	std::wstring strSimple;
	for (auto c : input) {
		if (c == L'\n' || c == L'\t' || c == L'\v' || c == L'\f' ||
			c == L'\r' || c == L' ' || c == L'\0') {
			continue;
		}

		strSimple.push_back(c);
	}

	return strSimple;
}

std::wstring COctLicenseManager::getLicenseCodeByLicenseKey(const std::wstring productId, const std::wstring licenseKey)
{
	std::wstring strCode1;
	{
		auto simple = toUpper(simplify(productId));

		auto hashedCode = std::hash<std::wstring>{}(simple + licenseKey);

		std::wstringbuf buffer;		// empty buffer
		std::wostream os(&buffer);	// associate stream buffer to stream

		os << L'H';
		os << hashedCode;

		strCode1 = buffer.str();
	}

	// length
	std::wstring strCode2;
	const int kLenGap = kLicenseCodeLen - (int)strCode1.length();
	if (kLenGap < 0) {
		strCode2 = strCode1.substr(0, kLicenseCodeLen);
	}
	else if (kLenGap > 0) {
		strCode2 = strCode1;
		for (int i = 0; i < kLenGap; i++) {
			strCode2.push_back('L');
		}
	}
	else {
		strCode2 = strCode1;
	}

	// shuffle
	std::wstring strCode3;
	{
		std::wstringbuf buffer;		// empty buffer
		std::wostream os(&buffer);	// associate stream buffer to stream

		for (auto c : strCode2) {
			if (c == L'0') {
				os << "X";
			}
			else if (c == L'1') {
				os << "N";
			}
			else {
				os << c;
			}
		}

		strCode3 = buffer.str();
	}

	return strCode3;
}

static vector<IP_ADAPTER_INFO> _getMAC()
{
	std::vector<IP_ADAPTER_INFO> adapterInfoList;

	PIP_ADAPTER_INFO pAdapterInfo;
	DWORD dwBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		printf("Error allocating memory needed to call GetAdaptersinfo\n");
		return std::vector<IP_ADAPTER_INFO>(); // it is safe to call free(NULL)
	}

	// Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)malloc(dwBufLen);
		if (pAdapterInfo == NULL) {
			printf("Error allocating memory needed to call GetAdaptersinfo\n");
			return std::vector<IP_ADAPTER_INFO>();
		}
	}

	if (GetAdaptersInfo(pAdapterInfo, &dwBufLen) == NO_ERROR) {
		// Contains pointer to current adapter info
		do {
			adapterInfoList.push_back(*pAdapterInfo);
			pAdapterInfo = pAdapterInfo->Next;
		} while (pAdapterInfo);
	}
	free(pAdapterInfo);
	return adapterInfoList; // caller must free.
}

int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // initialize MFC and print and error on failure
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: change error code to suit your needs
            wprintf(L"Fatal Error: MFC initialization failed\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: code your application's behavior here.
        }
    }
    else
    {
        // TODO: change error code to suit your needs
        wprintf(L"Fatal Error: GetModuleHandle failed\n");
        nRetCode = 1;
    }

    return nRetCode;
}
