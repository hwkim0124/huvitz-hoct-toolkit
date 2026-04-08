#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <string>


namespace tinyxml2 {
	class XMLElement;
}


namespace OctConfig
{
	class SystemProfile;
	class DeviceSettings;
	class SignalSettings;
	class CameraSettings;
	class FundusSettings;
	class CorneaSettings;
	class RetinaSettings;
	class FixationSettings;
	class SystemSettings;
	class PatternSettings;
	class AngioSettings;


	class OCTCONFIG_DLL_API ConfigFile
	{
	public:
		ConfigFile();
		virtual ~ConfigFile();

	public:
		static bool loadSystemProfile(const char* filename);
		static bool saveSystemProfile(const char* filename);

	private:
		struct ConfigFileImpl;
		static std::unique_ptr<ConfigFileImpl> d_ptr;
		static ConfigFileImpl& getImpl(void);

		static bool checkXMLResult(int result);
		static bool loadDeviceSettings(DeviceSettings* pset);
		static bool loadSignalSettings(SignalSettings* pset);
		static bool loadCameraSettings(CameraSettings* pset);
		static bool loadFundusSettings(FundusSettings* pset);
		static bool loadCorneaSettings(CorneaSettings* pset);
		static bool loadRetinaSettings(RetinaSettings* pset);
		static bool loadFixationSettings(FixationSettings* pset);
		static bool loadSystemSettings(SystemSettings* pset);
		static bool loadPatternSettings(PatternSettings* pset);
		static bool loadAngioSettings(AngioSettings* pset);

		static bool loadSectionOfLightLeds(DeviceSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfStepMotors(DeviceSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfCameras(CameraSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfFundus(FundusSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfRetina(RetinaSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfCornea(CorneaSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfFixation(FixationSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfSystemOptions(SystemSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfPatternRange(PatternSettings* pset, tinyxml2::XMLElement* group);
		static bool loadSectionOfAngioParams(AngioSettings* pset, tinyxml2::XMLElement* group);

		static bool saveDeviceSettings(const DeviceSettings* pset);
		static bool saveSignalSettings(const SignalSettings* pset);
		static bool saveCameraSettings(const CameraSettings* pset);
		static bool saveFundusSettings(const FundusSettings* pset);
		static bool saveCorneaSettings(const CorneaSettings* pset);
		static bool saveRetinaSettings(const RetinaSettings* pset);
		static bool saveFixationSettings(const FixationSettings* pset);
		static bool saveSystemSettings(SystemSettings* pset);
		static bool savePatternSettings(PatternSettings* pset);
		static bool saveAngioSettings(AngioSettings* pset);

		static bool saveSectionOfLightLeds(const DeviceSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfStepMotors(const DeviceSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfCameras(const CameraSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfFundus(const FundusSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfRetina(const RetinaSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfCornea(const CorneaSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfFixation(const FixationSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfSystemOptions(SystemSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfPatternRange(PatternSettings* pset, tinyxml2::XMLElement* group);
		static bool saveSectionOfAngioParams(AngioSettings* pset, tinyxml2::XMLElement* group);
	};
}

