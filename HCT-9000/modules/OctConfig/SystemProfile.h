#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <string>


namespace OctConfig
{
	class DeviceSettings;
	class SignalSettings;
	class CameraSettings;
	class FundusSettings;
	class RetinaSettings;
	class CorneaSettings;
	class FixationSettings;
	class SystemSettings;
	class PatternSettings;
	class AngioSettings;
	class SystemConfig;

	class OCTCONFIG_DLL_API SystemProfile
	{
	public:
		SystemProfile();
		virtual ~SystemProfile();

	public:
		static DeviceSettings* getDeviceSettings(void) ;
		static SignalSettings* getSignalSettings(void) ;
		static CameraSettings* getCameraSettings(void);
		static FundusSettings* getFundusSettings(void);
		static RetinaSettings* getRetinaSettings(void);
		static CorneaSettings* getCorneaSettings(void);
		static FixationSettings* getFixationSettings(void);
		static PatternSettings* getPatternSettings(void);
		static SystemSettings* getSystemSettings(void);
		static AngioSettings* getAngioSettings(void);
		static SystemConfig* getSystemCongigs(void);

		static void initializeSystemProfile(void);
		static void resetToDefaultValues(void);
		static bool loadConfigFile(const char* name = nullptr);
		static bool saveConfigFile(const char* name = nullptr);
		static std::string getDefaultConfigFilePath(void);
		static std::string getDefaultConfigDirPath(void);

	private:
		struct SystemProfileImpl;
		static std::unique_ptr<SystemProfileImpl> d_ptr;
		static SystemProfileImpl& getImpl(void);
	};
}
