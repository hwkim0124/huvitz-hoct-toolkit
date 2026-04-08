#ifndef OCTLICENSEMANAGER
#define OCTLICENSEMANAGER

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the OCTLICENSEMANAGER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// OCTLICENSEMANAGER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef OCTLICENSEMANAGER_EXPORTS
#define OCTLICENSEMANAGER_API __declspec(dllexport)
#else
#define OCTLICENSEMANAGER_API __declspec(dllimport)
#endif

#include <string>

// This class is exported from the OctLicenseManager.dll
class OCTLICENSEMANAGER_API COctLicenseManager {
public:
	typedef enum _OctLicenseTarget{
		HOCT_ANGIOGRAPHY = 0,
		HOCT_TOPOGRAPHY,
		HOCT_BIOMETRY,
	} OctLicenseTarget ;

public:
	const static int kProductIdLen = 10;
	const static int kLicenseCodeLen = 10;

public:
	static bool checkLicense(const std::wstring productId, const std::wstring licenseCode, OctLicenseTarget target);
	static bool checkLicenseTrial(const std::wstring productId, const std::wstring licenseCode, OctLicenseTarget target);
	static std::wstring getLicenseCode(const std::wstring productId, OctLicenseTarget target);
	static std::wstring getLicenseCodeTrial(const std::wstring productId, OctLicenseTarget target);
	static std::wstring getProductId(const std::wstring hwAddr);
	static std::wstring getHwAddr(OctLicenseTarget target = OctLicenseTarget::HOCT_ANGIOGRAPHY);
	static bool writeLicenseCode(const std::wstring licenseCode, OctLicenseTarget target);
	static bool writeExpireTime(CTime timeExpire, OctLicenseTarget target);
	static bool readLicenseCode(std::wstring& out_licenseCode, OctLicenseTarget target);
	static bool readExpireTime(CTime& out_timeExpire, OctLicenseTarget target);
	static bool readLicenseCodeState(OctLicenseTarget target);

private:
	static std::wstring toUpper(std::wstring input);
	static std::wstring simplify(std::wstring input);
	static std::wstring getLicenseCodeByLicenseKey(const std::wstring productId, const std::wstring licenseKey);
};

extern OCTLICENSEMANAGER_API int nOctLicenseManager;

OCTLICENSEMANAGER_API int fnOctLicenseManager(void);

#endif /*OCTLICENSEMANAGER*/
