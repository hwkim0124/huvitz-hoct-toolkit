#include "stdafx.h"
#include "SystemProfile.h"
#include "DeviceSettings.h"
#include "SignalSettings.h"
#include "CameraSettings.h"
#include "FundusSettings.h"
#include "RetinaSettings.h"
#include "CorneaSettings.h"
#include "FixationSettings.h"
#include "PatternSettings.h"
#include "SystemSettings.h"
#include "AngioSettings.h"
#include "ConfigFile.h"
#include "SystemConfig.h"

#include "CppUtil2.h"

using namespace OctConfig;
using namespace CppUtil;
using namespace std;


struct SystemProfile::SystemProfileImpl
{
	DeviceSettings deviceSets;
	SignalSettings signalSets;
	CameraSettings cameraSets;
	FundusSettings fundusSets;
	RetinaSettings retinaSets;
	CorneaSettings corneaSets;
	FixationSettings fixationSets;
	SystemSettings systemSets;
	PatternSettings patternSets;
	AngioSettings angioSets;
	SystemConfig systemConfig;

	SystemProfileImpl()
	{
	}
};


// Direct initialization of static smart pointer.
std::unique_ptr<SystemProfile::SystemProfileImpl> SystemProfile::d_ptr(new SystemProfileImpl());


SystemProfile::SystemProfile()
{
}


SystemProfile::~SystemProfile()
{
}


DeviceSettings * OctConfig::SystemProfile::getDeviceSettings(void) 
{
	return &getImpl().deviceSets;
}


SignalSettings * OctConfig::SystemProfile::getSignalSettings(void) 
{
	return &getImpl().signalSets;
}


CameraSettings * OctConfig::SystemProfile::getCameraSettings(void)
{
	return &getImpl().cameraSets;
}


FundusSettings * OctConfig::SystemProfile::getFundusSettings(void)
{
	return &getImpl().fundusSets;
}


RetinaSettings * OctConfig::SystemProfile::getRetinaSettings(void)
{
	return &getImpl().retinaSets;
}

CorneaSettings * OctConfig::SystemProfile::getCorneaSettings(void)
{
	return &getImpl().corneaSets;
}


FixationSettings * OctConfig::SystemProfile::getFixationSettings(void)
{
	return &getImpl().fixationSets;
}


PatternSettings * OctConfig::SystemProfile::getPatternSettings(void)
{
	return &getImpl().patternSets;
}


SystemSettings * OctConfig::SystemProfile::getSystemSettings(void)
{
	return &getImpl().systemSets;
}


AngioSettings * OctConfig::SystemProfile::getAngioSettings(void)
{
	return &getImpl().angioSets;
}

SystemConfig * OctConfig::SystemProfile::getSystemCongigs(void)
{
	return &getImpl().systemConfig;
}


void OctConfig::SystemProfile::initializeSystemProfile(void)
{
	getDeviceSettings()->initialize();
	getSignalSettings()->initialize();
	getCameraSettings()->initialize();
	getFundusSettings()->initialize();
	getRetinaSettings()->initialize();
	getCorneaSettings()->initialize();
	getFixationSettings()->initialize();
	getSystemSettings()->initialize();
	getPatternSettings()->initialize();
	getAngioSettings()->initialize();
	getSystemCongigs()->initialize();

	return;
}


void OctConfig::SystemProfile::resetToDefaultValues(void)
{
	getDeviceSettings()->resetToDefaultValues();
	getCameraSettings()->resetToDefaultValues();
	getFundusSettings()->resetToDefaultValues();
	getRetinaSettings()->resetToDefaultValues();
	getCorneaSettings()->resetToDefaultValues();
	getFixationSettings()->resetToDefaultValues();
	getSystemSettings()->resetToDefaultValues();
	getPatternSettings()->resetToDefaultValues();
	getAngioSettings()->resetToDefaultValues();
	return;
}


bool OctConfig::SystemProfile::loadConfigFile(const char* name)
{
	string path = (name == nullptr ? getDefaultConfigFilePath() : name);
	if (!ConfigFile::loadSystemProfile(path.c_str())) {
		LogW() << "System profile load failed!, path: " << path;
		return false;
	}
	else {
		LogI() << "System profile loaded, path: " << path;
		return true;
	}
}


bool OctConfig::SystemProfile::saveConfigFile(const char* name)
{
	string path = (name == nullptr ? getDefaultConfigFilePath() : name);
	if (!ConfigFile::saveSystemProfile(path.c_str())) {
		LogW() << "System profile save failed!, path: " << path;
		return false;
	}
	else {
		LogI() << "System profile saved, path: " << path;
		return true;
	}
}


std::string OctConfig::SystemProfile::getDefaultConfigFilePath(void)
{
	string cstr = getDefaultConfigDirPath();
	cstr += "\\";
	cstr += SYSTEM_CONFIG_FILE_NAME;
	return cstr;
}


std::string OctConfig::SystemProfile::getDefaultConfigDirPath(void)
{
	wchar_t buffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, buffer);
	std::string path = wtoa(buffer);
	return path;
}


SystemProfile::SystemProfileImpl & OctConfig::SystemProfile::getImpl(void)
{
	return *d_ptr;
}
