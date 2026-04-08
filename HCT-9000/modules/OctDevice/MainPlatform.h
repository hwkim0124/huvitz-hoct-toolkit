#pragma once

#include "OctDeviceDef.h"
#include "MainBoard.h"

namespace OctDevice
{
	class OCTDEVICE_DLL_API MainPlatform
	{
	public:
		MainPlatform();
		virtual ~MainPlatform();

	public:
		static bool initMainPlatform(OctDevice::MainBoard* board);
		static unsigned char gpio_init(void);
		static void startGpioThread(void);
		static void closeGpioThread(void);
		static void eventWorkerThread(void);
		static bool updateCurrentEyeSide(void);
		static bool updateOptimizeStatus(void);
		static bool updateJoystickStatus(void);
		static bool updateSldFaultStatus(void);

		void setJoystickEventCallback(JoystickEventCallback * callback);
		void setEyeSideEventCallback(EyeSideEventCallback * callback);
		void setOptimizeKeyEventCallback(OptimizeKeyEventCallback * callback);
		void setBacklightOnEventCallback(BacklightOnEventCallback * callback);
		void setSldWarningEventCallback(SldWarningEventCallback* callback);
		void setScannerFaultEventCallback(ScannerFaultEventCallback* callback);

		bool isAtSideOd(void);
		EyeSide getEyeSide(void);
		static void releaseMainPlatform(void);
		static bool setBacklightEnable(bool flag);
		static bool isBacklightOn(void);
		static void turnOnBacklight(void);
		static void turnOffBacklight(void);
		static int getCurrentEyeSide(void);


	private:
		struct MainPlatformImpl;
		static std::unique_ptr<MainPlatformImpl> d_ptr;
		static MainPlatformImpl& getImpl(void);
	};

}