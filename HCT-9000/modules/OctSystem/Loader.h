#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <string>


namespace OctSystem
{
	class OCTSYSTEM_DLL_API Loader
	{
	public:
		Loader();
		virtual ~Loader();

	public:
		static bool initializeSystem(LogMsgCallback* cbLogMsg = nullptr);
		static bool isInitiated(void);
		static void releaseSystem(void);

		static int getLogMsgLevel(void);
		static std::string getToolkitVersion(void);
		static std::string getFirmwareVersion(void);
		static std::string getFPGAVersion(void);

		static bool loadConfiguration(void);
		static bool loadTopoCalibration(void);
		static bool saveConfiguration(void);
		static bool applyConfiguration(void);

		static bool loadSystemSetup(void);
		static bool saveSystemSetup(void);

		static bool loadSetupProfile(bool fromfile = false, bool device=true);
		static bool saveSetupProfile(bool update = true, bool tofile = false);
		static void applySystemProfile(bool device);
		static void captureSystemProfile(void);

		static bool applySldFaultDetection(void);
		static bool loadDefectPixelsOfColorCamera(void);
		static bool saveDefectPixelsOfColorCamera(void);

		static void getRetinaCameraShift(int& shiftX, int& shiftY);
		static void getCorneaCameraShift(int& shiftX, int& shiftY);

		static bool checkCommport(void);
		static int getNumberOfWarnings(void);
		static bool setupUsbCmosCamera(void);
		static void releaseUsbCmosCamera(void);

		static bool useNoDevice(bool isset = false, bool flag = false);
		static bool useNoCameras(bool isset = false, bool flag = false);
		static bool useNoLeds(bool isset = false, bool flag = false);
		static bool useNoMotors(bool isset = false, bool flag = false);
		static bool useNoGrabber(bool isset = false, bool flag = false);
		static bool useNoColor(bool isset = false, bool flag = false);
		static bool useDcfFile(bool isSet = false, bool flag = false, const wchar_t* path = nullptr);
		static const wchar_t* getDcfFilePath(void);
		
	protected:
		static void initSystemBootMode(void);

	private:
		struct LoaderImpl;
		static std::unique_ptr<LoaderImpl> d_ptr;
		static LoaderImpl& getImpl(void);

		static bool updatePropertiesFromCommandLine(void);
	};
}
