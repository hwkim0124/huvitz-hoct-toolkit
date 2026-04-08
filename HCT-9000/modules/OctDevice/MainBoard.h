#pragma once

#include "OctDeviceDef.h"


#include <memory>


namespace OctConfig
{
	class DeviceSettings;
	class CameraSettings;
}

namespace OctDevice
{
	class UsbComm;
	class IrCamera;
	class LightLed;
	class StepMotor;
	class StageMotor;
	class Galvanometer;

	class OctSldLed;
	class FlashLed;

	class OctFocusMotor;
	class PolarizationMotor;
	class ReferenceMotor;
	class FundusFocusMotor;
	class OctSampleMotor;
	class FundusDiopterMotor;
	class OctDiopterMotor;
	class PupilMaskMotor;
	class SplitFocusMotor;

	class StageXMotor;
	class StageYMotor;
	class StageZMotor;

	class CorneaCamera;
	class RetinaCamera;
	class ColorCamera;

	class SysConfig;


	class OCTDEVICE_DLL_API MainBoard
	{
	public:
		MainBoard();
		virtual ~MainBoard();

		MainBoard(MainBoard&& rhs);
		MainBoard& operator=(MainBoard&& rhs);

		// Prevent copy construction and assignment. 
		MainBoard(const MainBoard& rhs) = delete;
		MainBoard& operator=(const MainBoard& rhs) = delete;

	public:
		static constexpr const char* SYSTEM_CONFIG_FILE_NAME = "SysConfig.dat";

	public: 
		bool initMainBoard(int* numWarns);
		bool initDevices(int* numWarns);
		void release(void);
		bool initialized(void);

		void setDeviceMode(bool noDevice = false, bool noCams = false,
							bool noLeds = false, bool noMotors = false, bool noColor = false);
		bool isNoDevice(void) const;
		bool isNoCamera(void) const;
		bool isNoLeds(void) const;
		bool isNoMotors(void) const;
		bool isNoColor(void) const;

		bool loadSystemConfiguration(void);
		bool saveSystemConfiguration(bool write = true);
		bool exportSystemConfiguration(std::string filename = SYSTEM_CONFIG_FILE_NAME);
		bool importSystemConfiguration(std::string filename = SYSTEM_CONFIG_FILE_NAME);

		bool loadSystemProfile(void);
		bool saveSystemProfile(void);

		bool loadDeviceSettings(OctConfig::DeviceSettings* pset);
		bool loadCameraSettings(OctConfig::CameraSettings* pset);
		void saveDeviceSettings(OctConfig::DeviceSettings* pset);
		void saveCameraSettings(OctConfig::CameraSettings* pset);

		void performPackagingMode(void);

		bool loadAddressTable(std::uint32_t addr = USB_BOARD_ADDR);
		bool openFTDIdevices(bool reset, bool reload=false);
		bool openColorCamera(bool wait);

		bool checkInitStatus(int& status);
		std::string parseDeviceStatus(std::uint32_t status);
		bool resetFTDIdevice(void);
		bool resetUsbChannel(void);
		bool resetColorCamera(void);
		bool rebootSystem(void);
		bool requestUartCommand(std::string smsg, std::string& rmsg, int sizeMax = 128);

		bool moveChinrestUp(void);
		bool moveChinrestDown(void);
		bool stopChinrest(void);
		bool isChinrestAtHighLimit(void);
		bool isChinrestAtLowLimit(void);

		bool isUartDebugEnabled(void);
		bool setUartDebugEnabled(bool flag);

		bool setStageLimitOverride(bool flag);
		bool setSldWarningEnabled(int flag);
		bool updateScannerFaultParameters(std::uint16_t timer, std::uint16_t thresh);

		bool setLineCameraGain(unsigned short value);
		bool readLineCameraBlackOffset(int mode, std::string &value);
		bool saveLineCameraBlackOffset(int mode, std::string value);
		bool flashWriteLineCameraBlackOffset(void);

		bool prepareDevicesForScan(void);
		bool releaseDevicesForScan(void);

		bool setQuickReturnMirrorIn(bool flag);
		bool setSplitFocusMirrorIn(bool flag);
		bool setDiopterCompensationLens(int mode, int lens);
		bool setSmallPupilMask(bool flag);
		bool setFundusCameraCapture(void);

		bool setInternalFixationOn(bool flag, int row=14, int col=10);
		bool getInternalFixationOn(int& row, int& col);
		bool setLcdFixationOn(bool flag, int row=159, int col=127);
		bool getLcdFixationOn(int& row, int& col);
		bool updateLcdParameters(std::uint8_t bright, std::uint8_t blink, std::uint16_t period, std::uint16_t ontime, std::uint8_t type = 0x00);

		bool setScanBeamOn(bool flag);
		bool addGalvanoDynamicOffset(std::int16_t initX, std::int16_t initY, float mmPosX, float mmPosY, bool cornea);
		bool setGalvanoDynamicOffset(float mmPosX, float mmPosY, bool cornea);
		bool setGalvanoDynamicOffset(std::int16_t offsetX, std::int16_t offsetY);
		bool getGalvanoDynamicOffset(std::int16_t& offsetX, std::int16_t& offsetY);
		void setModelType(int modelType);

		std::string getApplicationVersion(void) const;
		std::string getFPGAVersion(void) const;

		std::uint32_t getBaseAddressOfSystemInitStatus(void) const;
		std::uint32_t getBaseAddressOfSystemVersion(void) const;
		std::uint32_t getBaseAddressOfMotorInfo(void) const;
		std::uint32_t getBaseAddressOfStageInfo(void) const;
		std::uint32_t getBaseAddressOfSysConfiguration(void) const;
		std::uint32_t getBaseAddressOfTrajectoryProfile(void) const;
		std::uint32_t getBaseAddressOfSysCalibration(void) const;
		std::uint32_t getBaseAddressOfDownload(void) const;
		std::uint32_t getBaseAddressOfGPStatus(void) const;
		std::uint32_t getBaseAddressOfGalvanoDynamic(void) const;
		std::uint32_t getBaseAddressOfSysProfile(void) const;

		OctDevice::IrCamera* getCorneaIrCamera(void);
		OctDevice::IrCamera* getRetinaIrCamera(void);

		OctDevice::CorneaCamera* getCorneaCamera(void);
		OctDevice::RetinaCamera* getRetinaCamera(void);
		OctDevice::ColorCamera* getColorCamera(void);

		OctDevice::LightLed* getRetinaIrLed(void);
		OctDevice::LightLed* getCorneaIrLed(void);
		OctDevice::LightLed* getKeratoRingLed(void);
		OctDevice::LightLed* getKeratoFocusLed(void);
		OctDevice::FlashLed* getFundusFlashLed(void);
		OctDevice::LightLed* getSplitFocusLed(void);
		OctDevice::LightLed* getExtFixationLed(void);
		OctDevice::LightLed* getWorkingDotsLed(void);
		OctDevice::LightLed* getWorkingDot2Led(void);

		OctDevice::OctSldLed* getOctSldLed(void);
		OctDevice::LightLed* getPannelLed(void);

		OctFocusMotor* getOctFocusMotor(void);
		ReferenceMotor* getReferenceMotor(void);
		PolarizationMotor* getPolarizationMotor(void);
		FundusFocusMotor* getFundusFocusMotor(void);
		OctSampleMotor* getOctSampleMotor(void);
		SplitFocusMotor* getSplitFocusMotor(void);
		FundusDiopterMotor* getFundusDiopterMotor(void);
		OctDiopterMotor* getOctDiopterMotor(void);
		PupilMaskMotor* getPupilMaskMotor(void);
	
		StageXMotor* getStageXMotor(void);
		StageYMotor* getStageYMotor(void);
		StageZMotor* getStageZMotor(void);

		OctDevice::IrCamera* getIrCamera(CameraType type);
		OctDevice::LightLed* getLightLed(LightLedType type);
		OctDevice::StepMotor* getStepMotor(StepMotorType type);
		OctDevice::StageMotor* getStageMotor(StageMotorType type);

		OctDevice::Galvanometer* getGalvanometer(void);
		UsbComm& getUsbComm(void) const;

	private:
		struct MainBoardImpl;
		std::unique_ptr<MainBoardImpl> d_ptr;
		MainBoardImpl& getImpl(void) const;
	};
}
