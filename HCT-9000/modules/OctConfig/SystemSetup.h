#pragma once

#include "OctConfigDef.h"


#include <memory>
#include <string>


namespace OctConfig
{
	class OCTCONFIG_DLL_API SystemSetup
	{
	public:
		SystemSetup();
		virtual ~SystemSetup();

	public:
		static SysCal2_st* getSysCalc2(bool checksum=false);
		static bool loadDataFromProfile(bool update);
		static bool saveDataToProfile(void);

		static bool isValidChecksum(void);
		static std::uint64_t calculateCheckSum(void);
		static std::uint64_t getCheckSum(void);
		static void updateCheckSum(void);

	private:
		struct SystemSetupImpl;
		static std::unique_ptr<SystemSetupImpl> d_ptr;
		static SystemSetupImpl& getImpl(void);
	};
}

