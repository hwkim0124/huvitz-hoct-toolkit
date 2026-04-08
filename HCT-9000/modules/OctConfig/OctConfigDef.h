#pragma once

#include "OctGlobal2.h"
#include "SysCalibDef.h"
#include "SysSetupDef.h"

using namespace OctGlobal;


namespace OctConfig
{
	constexpr char* CONFIG_FILE_NAME = "HctConfig.xml";

	struct ConfigSettingItem {
		// The default access specifier for a struct is public. 
		std::string _name;

	public:
		ConfigSettingItem(const char* name) {
			_name = name;
		}

		const char* getName(void) const {
			return _name.c_str();
		}

		bool isName(const char* name) {
			return (_name.compare(name) == 0);
		}
	};

	constexpr int WAVELENGTH_PARAMS_SIZE = 4;
	constexpr int DISPERSION_PARAMS_SIZE = 3;

};


#ifdef __OCTCONFIG_DLL
#define OCTCONFIG_DLL_API		__declspec(dllexport)
#else
#define OCTCONFIG_DLL_API		__declspec(dllimport)
#endif