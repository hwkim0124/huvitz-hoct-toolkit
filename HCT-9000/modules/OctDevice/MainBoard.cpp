#include "stdafx.h"
#include "MainBoard.h"
#include "UsbComm.h"
#include "IrCamera.h"
#include "LightLed.h"
#include "StepMotor.h"
#include "StageMotor.h"
#include "Galvanometer.h"

#include "OctSldLed.h"
#include "FlashLed.h"

#include "OctFocusMotor.h"
#include "PolarizationMotor.h"
#include "ReferenceMotor.h"
#include "FundusFocusMotor.h"
#include "OctSampleMotor.h"
#include "OctDiopterMotor.h"
#include "PupilMaskMotor.h"
#include "SplitFocusMotor.h"
#include "FundusDiopterMotor.h"

#include "StageXMotor.h"
#include "StageYMotor.h"
#include "StageZMotor.h"

#include "CorneaCamera.h"
#include "RetinaCamera.h"
#include "ColorCamera.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

#include <memory>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include <iostream>

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;



struct MainBoard::MainBoardImpl
{
	AddressTable addressTbl;
	SysVersionInfo sysVerInfo;
	SysInitStatus sysInitStatus;
	UsbComm usbComm;

	bool initiated;
	int deviceMode;

	int inFixationRow;
	int inFixationCol;
	bool inFixationOn;

	int lcdFixationRow;
	int lcdFixationCol;
	int lcdFixationOn;

	unique_ptr<ColorCamera> colorCamera;
	unique_ptr<Galvanometer> galvano;

	vector<unique_ptr<IrCamera>> irCameras;
	vector<unique_ptr<LightLed>> lightLeds;
	vector<unique_ptr<StepMotor>> stepMotors;
	vector<unique_ptr<StageMotor>> stageMotors;


	MainBoardImpl() : initiated(false), deviceMode(0), inFixationOn(false), lcdFixationOn(false)
	{
	}
};


MainBoard::MainBoard() :
	d_ptr(make_unique<MainBoardImpl>())
{
	getImpl().irCameras.emplace_back(new CorneaCamera(this));
	getImpl().irCameras.emplace_back(new RetinaCamera(this));

	getImpl().stepMotors.emplace_back(new OctFocusMotor(this, StepMotorType::OCT_FOCUS));
	getImpl().stepMotors.emplace_back(new PolarizationMotor(this, StepMotorType::POLARIZATION));
	getImpl().stepMotors.emplace_back(new ReferenceMotor(this, StepMotorType::REFERENCE));
	getImpl().stepMotors.emplace_back(new FundusFocusMotor(this, StepMotorType::FUNDUS_FOCUS));

	getImpl().stepMotors.emplace_back(new OctSampleMotor(this, StepMotorType::OCT_SAMPLE));
	getImpl().stepMotors.emplace_back(new OctDiopterMotor(this, StepMotorType::OCT_DIOPTER));
	getImpl().stepMotors.emplace_back(new PupilMaskMotor(this, StepMotorType::PUPIL_MASK));
	getImpl().stepMotors.emplace_back(new SplitFocusMotor(this, StepMotorType::SPLIT_FOCUS));
	getImpl().stepMotors.emplace_back(new FundusDiopterMotor(this, StepMotorType::FUNDUS_DIOPTER));

	getImpl().stageMotors.emplace_back(new StageXMotor(this));
	getImpl().stageMotors.emplace_back(new StageYMotor(this));
	getImpl().stageMotors.emplace_back(new StageZMotor(this));

	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::RETINA_IR));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::CORNEA_IR));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::KER_RING));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::KER_FOCUS));

	getImpl().lightLeds.emplace_back(new FlashLed(this, LightLedType::FUNDUS_FLASH));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::SPLIT_FOCUS));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::EXT_FIXATION));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::WORKING_DOTS));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::WORKING_DOT2));
	getImpl().lightLeds.emplace_back(new OctSldLed(this, LightLedType::OCT_SLD));
	getImpl().lightLeds.emplace_back(new LightLed(this, LightLedType::PANNEL));

	getImpl().colorCamera = make_unique<ColorCamera>(this);
	getImpl().galvano = make_unique<Galvanometer>(this);
}


MainBoard::~MainBoard()
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctDevice::MainBoard::MainBoard(MainBoard && rhs) = default;
MainBoard & OctDevice::MainBoard::operator=(MainBoard && rhs) = default;


bool OctDevice::MainBoard::initMainBoard(int* numWarns)
{
	// bool result = true;
	int warns = 0;

	*numWarns = warns;

	// No device mode is returned with not initiated status.
	if (isNoDevice()) {
		LogD() << "No device flag is set, skip init mainboard";
		return true;
	}

	int status = UART_INIT_STATUS_OK;

	if (!checkInitStatus(status)) {
		GlobalLogger::error("Check init status failed!");
		return false;
	}

	// Usb channel is not opened when no device mode. 
	if (!openFTDIdevices(true)) {
		GlobalLogger::error("Usb channel open failed!");
		return false;
	}

	if (!checkInitStatus(status)) {
		ostringstream msg;
		msg << "Check init status failed!, status: " << status;
		GlobalLogger::error(msg.str());
	}
	else {
		GlobalLogger::info("Checking init status and usb connection ... ok");
	}

	/*
	if (!openColorCamera()) {
		LogE() << "Color camera open failed!";
		return false;
	}
	*/

	if (!loadAddressTable()) {
		GlobalLogger::error("Address table load failed!");
		return false;
	}
	else {
		GlobalLogger::info("Loading address table ... ok");
	}

	if (status != UART_INIT_STATUS_OK) {

		if (getImpl().sysInitStatus.devStatus != HOCT_1_INIT_DONE_STATUS
			&& getImpl().sysInitStatus.devStatus != HOCT_1A_INIT_DONE_STATUS
			&& getImpl().sysInitStatus.devStatus != HOCT_1F_INIT_DONE_STATUS
			&& getImpl().sysInitStatus.devStatus != HOCT_1FA_INIT_DONE_STATUS
			&& getImpl().sysInitStatus.devStatus != HFC_INIT_DONE_STATUS) {
			ostringstream msg;
			msg << "Device init failed!, status: " << getImpl().sysInitStatus.status << ", devStatus: " << parseDeviceStatus(getImpl().sysInitStatus.devStatus) << " (" << std::hex << getImpl().sysInitStatus.devStatus << ") ";
			GlobalLogger::error(msg.str());

			warns += 1;
			/*
			if (GlobalSettings::isUserModeSettings()) {
				result = false;
			}
			*/
		}
	}

	setUartDebugEnabled(false);

	if (!loadSystemConfiguration()) {
		GlobalLogger::error("System configuration read failed!");
		// result = false;
		warns += 1;
	}
	else {
		GlobalLogger::info("Reading system configuration ... ok!");
	}

	return true;
}


bool OctDevice::MainBoard::initDevices(int * numWarns)
{
	int warns = 0;

	*numWarns = warns;
	SystemSettings* sset = SystemProfile::getSystemSettings();
	int modelType = sset->getModelType();
	setModelType(modelType);

	if (getImpl().sysInitStatus.devStatus == HFC_INIT_DONE_STATUS && modelType != 8) {
		setModelType(8);
		GlobalLogger::info("OCT motors not found, changing settings to HFC-1.");
	}

	if (getImpl().sysInitStatus.devStatus == HOCT_1_INIT_DONE_STATUS && (modelType != 1 && modelType != 3 && modelType != 5 && modelType != 7)) {
		setModelType(1);
		GlobalLogger::info("Fundus Camera not found, changing settings to HOCT-1.");
	}

	if (!getCorneaCamera()->initialize()) {
		GlobalLogger::warn("Cornea ir camera init failed!");
		warns += 1;
	}
	else {
		GlobalLogger::info("Cornea ir camera init ... ok!");

		if (getCorneaCamera()->play()) {
			bool is_error = false;
			for (int i = 0; i < 20; i++) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (getCorneaCamera()->getErrorCount() > 0) {
					GlobalLogger::warn("Cornea ir camera frame error!");
					warns += 1;
					is_error = true;
					break;
				}
			}
			getCorneaCamera()->pause();

			if (!is_error) {
				if (getCorneaCamera()->getFrameCount() == 0) {
					GlobalLogger::warn("Cornea ir camera frame error!");
					warns += 1;
				}
				else {
					GlobalLogger::info("Cornea ir camera frame test ok!");
				}
			}
		}
		else {
			GlobalLogger::warn("Cornea ir camera check error!");
			warns += 1;
		}
	}

	if (!getRetinaCamera()->initialize()) {
		GlobalLogger::warn("Retina ir camera init failed!");
		warns += 1;
	}
	else {
		GlobalLogger::info("Retina ir camera init ... ok!");

		if (getRetinaCamera()->play()) {
			bool is_error = false;
			for (int i = 0; i < 20; i++) {
				this_thread::sleep_for(chrono::milliseconds(50));
				if (getRetinaCamera()->getErrorCount() > 0) {
					GlobalLogger::warn("Retina ir camera frame error!");
					warns += 1;
					is_error = true;
					break;
				}
			}
			getRetinaCamera()->pause();

			if (!is_error) {
				if (getRetinaCamera()->getFrameCount() == 0) {
					GlobalLogger::warn("Retina ir camera frame error!");
					warns += 1;
				}
				else {
					GlobalLogger::info("Retina ir camera frame test ok!");
				}
			}
		}
		else {
			GlobalLogger::warn("Retina ir camera check error!");
			warns += 1;
		}
	}

	if (GlobalSettings::useFundusEnable() || GlobalSettings::useFundusFILR_Enable()) {
		bool color_init = false;
		for (int i = 0; i < 3; i++) {
			if (getColorCamera()->initialize()) {
				color_init = true;
				break;
			}
			else {
				GlobalLogger::debug("Color camera init failed!, retrying ...");
				warns += 1;
				if (!openColorCamera(true)) {
					GlobalLogger::error("Color camera open failed!");
					return false;
				}
			}
		}

		if (!color_init) {
			GlobalLogger::error("Color camera init failed!");
			// result = false;
			warns += 1;
		}

		if (!getFundusFlashLed()->initialize()) {
			GlobalLogger::warn("Fundus flash Led init failed!");
			warns += 1;
		}
	}

	if (!getRetinaIrLed()->initialize()) {
		GlobalLogger::warn("Retina ir Led init failed!");
		warns += 1;
	}
	if (!getCorneaIrLed()->initialize()) {
		GlobalLogger::warn("Cornea ir Led init failed!");
		warns += 1;
	}
	if (!getKeratoRingLed()->initialize()) {
		GlobalLogger::warn("Kerato ring Led init failed!");
		warns += 1;
	}
	if (!getKeratoFocusLed()->initialize()) {
		GlobalLogger::warn("Kerato focus Led init failed!");
		warns += 1;
	}
	if (!getSplitFocusLed()->initialize()) {
		GlobalLogger::warn("Split focus Led init failed!");
		warns += 1;
	}
	if (!getExtFixationLed()->initialize()) {
		GlobalLogger::warn("External fixation Led init failed!");
		warns += 1;
	}
	if (!getWorkingDotsLed()->initialize()) {
		GlobalLogger::warn("Working dots Led init failed!");
		warns += 1;
	}
	if (!getWorkingDot2Led()->initialize()) {
		GlobalLogger::warn("Working dot2 Led init failed!");
		warns += 1;
	}
	if (!getOctSldLed()->initialize()) {
		GlobalLogger::warn("Oct sld Led init failed!");
		warns += 1;
	}

	GlobalLogger::info("Light Leds init ... ok!");

	if (GlobalSettings::useOctEnable()) {
		if (!getOctFocusMotor()->initialize()) {
			GlobalLogger::warn("Oct focus motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Oct focus motor init, zero-diopt: " << getOctFocusMotor()->getPositionOfZeroDiopter();
			GlobalLogger::debug(msg.str());
		}

		if (!getReferenceMotor()->initialize()) {
			GlobalLogger::warn("Reference motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Reference motor init, retina-origin: " << getReferenceMotor()->getPositionOfRetinaOrigin();
			GlobalLogger::debug(msg.str());
		}

		if (!getPolarizationMotor()->initialize()) {
			GlobalLogger::warn("Polarization motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Polarization motor init, zero-degree: " << getPolarizationMotor()->getPositionOfZeroDegree();
			GlobalLogger::debug(msg.str());
		}
		if (!getOctDiopterMotor()->initialize()) {
			GlobalLogger::warn("Oct diopter motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Oct diopter motor init, zero-lens: " << getOctDiopterMotor()->getPositionOfZeroLens();
			GlobalLogger::debug(msg.str());
		}
	}
		
	if (!getOctSampleMotor()->initialize()) {
		GlobalLogger::warn("Oct sample motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Oct sample motor init, mirror-in: " << getOctSampleMotor()->getPositionOfMirrorIn();
		GlobalLogger::debug(msg.str());
	}

	if (!getFundusFocusMotor()->initialize()) {
		GlobalLogger::warn("Fundus focus motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Fundus focus motor init, zero-diopt: " << getFundusFocusMotor()->getPositionOfZeroDiopter();
		GlobalLogger::debug(msg.str());
	}

	if (!getFundusDiopterMotor()->initialize()) {
		GlobalLogger::warn("Fundus diopter motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Fudus diopter motor init, zero-lens: " << getFundusDiopterMotor()->getPositionOfZeroLens();
		GlobalLogger::debug(msg.str());
	}

	if (GlobalSettings::useFundusEnable() || GlobalSettings::useFundusFILR_Enable())
	{
		if (!getPupilMaskMotor()->initialize()) {
			GlobalLogger::warn("Pupil mask motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Pupil mask motor init, normal: " << getPupilMaskMotor()->getPositionOfNormal();
			GlobalLogger::debug(msg.str());
		}

		if (!getSplitFocusMotor()->initialize()) {
			GlobalLogger::warn("Split focus motor init failed!");
			warns += 1;
		}
		else {
			ostringstream msg;
			msg << "Split focus motor init, mirror-in: " << getSplitFocusMotor()->getPositionOfMirrorIn();
			GlobalLogger::debug(msg.str());
		}
	}
	
	GlobalLogger::info("System motors init ... ok!");

	if (!getStageXMotor()->initialize()) {
		GlobalLogger::warn("Stage x motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Stage x motor init, center: " << getStageXMotor()->getCenterPosition() << ", cur-pos: " << getStageXMotor()->getPosition();
		GlobalLogger::debug(msg.str());
		getStageXMotor()->updatePositionToCenter();
	}

	if (!getStageYMotor()->initialize()) {
		GlobalLogger::warn("Stage y motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Stage y motor init, cur-pos: " << getStageYMotor()->getPosition();
		GlobalLogger::debug(msg.str());
	}

	if (!getStageZMotor()->initialize()) {
		GlobalLogger::warn("Stage z motor init failed!");
		warns += 1;
	}
	else {
		ostringstream msg;
		msg << "Stage z motor init, center: " << getStageZMotor()->getCenterPosition() << ", cur-pos: " << getStageZMotor()->getPosition();
		GlobalLogger::debug(msg.str());
		getStageZMotor()->updatePositionToCenter();
	}

	GlobalLogger::info("Stage motors init ... ok!");

	if (GlobalSettings::useOctEnable()) {
		getGalvanometer()->initialize();
	}
	getImpl().initiated = true;

	if (!setQuickReturnMirrorIn(true)) {
		GlobalLogger::warn("Quick return mirror init failed!");
		warns += 1;
	}

	*numWarns = warns;
	// return result;
	return true;
}


void OctDevice::MainBoard::setModelType(int modelType)
{
	switch (modelType)
	{
	case 0:		// HOCT-1F 
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, true);
		GlobalSettings::useAngioEnable(true, false);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, false);
		break;
	case 1:		// HOCT-1
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, false);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, false);
		break;
	case 2:		// HOCT-1F New Version
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, false);
		GlobalSettings::useFundusFILR_Enable(true, true);
		GlobalSettings::useNewGpioRutine(true, true);
		break;
	case 3:		// HOCT-1 New Version
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, false);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, true);
		break;
	case 4:		// HOCT-1FA 
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, true);
		GlobalSettings::useAngioEnable(true, true);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, false);
		break;
	case 5:		// HOCT-1A
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, true);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, false);
		break;
	case 6:		// HOCT-1FA New Version
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, true);
		GlobalSettings::useFundusFILR_Enable(true, true);
		GlobalSettings::useNewGpioRutine(true, true);
		break;
	case 7:		// HOCT-1A New Version
		GlobalSettings::useOctEnable(true, true);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, true);
		GlobalSettings::useFundusFILR_Enable(true, false);
		GlobalSettings::useNewGpioRutine(true, true);
		break;
	case 8:		// HFC-1
		GlobalSettings::useOctEnable(true, false);
		GlobalSettings::useFundusEnable(true, false);
		GlobalSettings::useAngioEnable(true, false);
		GlobalSettings::useFundusFILR_Enable(true, true);
		GlobalSettings::useNewGpioRutine(true, true);
		break;

	}

}

void OctDevice::MainBoard::release(void)
{
	try {
		getRetinaCamera()->pause();
		getCorneaCamera()->pause();
		getUsbComm().release();

		if (GlobalSettings::useFundusEnable() || GlobalSettings::useFundusFILR_Enable()) {
			getColorCamera()->pauseLiveMode();
		}
	}
	catch (...) {
	}
	return;
}


bool OctDevice::MainBoard::initialized(void)
{
	return getImpl().initiated;
}


void OctDevice::MainBoard::setDeviceMode(bool noDevice, bool noCams, bool noLeds, bool noMotors, bool noColor)
{
	int devMode = 0x00;
	if (noDevice) {
		devMode |= DEV_MODE_NO_DEVICE;
	}
	if (noCams) {
		devMode |= DEV_MODE_NO_CAMERAS;
	}
	if (noLeds) {
		devMode |= DEV_MODE_NO_LEDS;
	}
	if (noMotors) {
		devMode |= DEV_MODE_NO_MOTORS;
	}
	if (noColor) {
		devMode |= DEV_MODE_NO_COLOR;
	}
	getImpl().deviceMode = devMode;
	return;
}


bool OctDevice::MainBoard::isNoDevice(void) const
{
	return ((getImpl().deviceMode & DEV_MODE_NO_DEVICE) == DEV_MODE_NO_DEVICE);
}


bool OctDevice::MainBoard::isNoCamera(void) const
{
	return (((getImpl().deviceMode & DEV_MODE_NO_CAMERAS) == DEV_MODE_NO_CAMERAS) || isNoDevice());
}


bool OctDevice::MainBoard::isNoLeds(void) const
{
	return (((getImpl().deviceMode & DEV_MODE_NO_LEDS) == DEV_MODE_NO_LEDS) || isNoDevice());
}


bool OctDevice::MainBoard::isNoMotors(void) const
{
	return (((getImpl().deviceMode & DEV_MODE_NO_MOTORS) == DEV_MODE_NO_MOTORS) || isNoDevice());
}


bool OctDevice::MainBoard::isNoColor(void) const
{
	return (((getImpl().deviceMode & DEV_MODE_NO_COLOR) == DEV_MODE_NO_COLOR) || isNoDevice());
}


bool OctDevice::MainBoard::loadSystemConfiguration(void)
{
	// Load system configuration from mainboard memory.
	if (getUsbComm().SysCalRead(getBaseAddressOfSysCalibration(), SystemConfig::getSysCalibData(false))) {
		return true;
	}
	return false;
}


bool OctDevice::MainBoard::saveSystemConfiguration(bool write)
{
	// Save system configuration to mainboard memory.
	if (getUsbComm().SysCalWrite(getBaseAddressOfSysCalibration(), SystemConfig::getSysCalibData(true))) 
	{
		getCorneaCamera()->loadSystemParameters();
		getRetinaCamera()->loadSystemParameters();

		getOctFocusMotor()->loadSystemParameters();
		getOctDiopterMotor()->loadSystemParameters();
		getFundusDiopterMotor()->loadSystemParameters();
		getFundusFocusMotor()->loadSystemParameters();
		getOctSampleMotor()->loadSystemParameters();
		getPolarizationMotor()->loadSystemParameters();
		getPupilMaskMotor()->loadSystemParameters();
		getReferenceMotor()->loadSystemParameters();
		getSplitFocusMotor()->loadSystemParameters();

		getOctSldLed()->loadSystemParameters();

		if (write) {
			Sleep(250);
			if (!getUsbComm().SysCalSave(0, sizeof(SysCal_st))) {
				return false;
			}
		}
		return true;
	}
	return false;
}


bool OctDevice::MainBoard::exportSystemConfiguration(std::string filename)
{
	fstream file(filename, ios::out | ios::binary);
	if (file.is_open()) {
		const char* p = (const char*)SystemConfig::getSysCalibData(true);
		file.write(p, sizeof(SysCal_st));
		file.close();
		LogD() << "System config file exported, name: " << filename;
		return true;
	}
	LogD() << "System config file export failed, name: " << filename;
	return false;
}


bool OctDevice::MainBoard::importSystemConfiguration(std::string filename)
{
	fstream file(filename, ios::in | ios::binary);
	if (file.is_open()) {
		char* p = (char*)SystemConfig::getSysCalibData(true);
		file.read(p, sizeof(SysCal_st));
		file.close();
		LogD() << "System config file imported, name: " << filename;
		return true;
	}
	LogD() << "System config file import failed, name: " << filename;
	return false;
}


bool OctDevice::MainBoard::loadSystemProfile(void)
{
	int size = sizeof(SysCal2_st);
	if (getUsbComm().SysCal2Read(getBaseAddressOfSysProfile(), SystemSetup::getSysCalc2())) {
		if (SystemSetup::isValidChecksum()) {
			SystemSetup::saveDataToProfile();
			LogD() << "System setup loaded from mainboard, chksum: " << SystemSetup::getCheckSum() << ", size: " << size;
			return true;
		}
		LogE() << "System setup checksum invalid!, load: " << SystemSetup::getCheckSum() << ", calc: " << SystemSetup::calculateCheckSum() << ", size: " << size;
	}
	else {
		LogE() << "System setup load from mainboard failed!";
	}
	return false;
}


bool OctDevice::MainBoard::saveSystemProfile(void)
{
	SystemSetup::loadDataFromProfile(false);
	int size = sizeof(SysCal2_st);
	if (getUsbComm().SysCal2Write(getBaseAddressOfSysProfile(), SystemSetup::getSysCalc2(true))) {
		Sleep(250);
		if (getUsbComm().SysCal2Save(0, sizeof(SysCal2_st))) {
			LogD() << "System setup saved to mainboard, chksum: " << SystemSetup::getCheckSum() << ", size: " << size;
			return true;
		}
	}
	LogE() << "System setup save to mainboard failed!";
	return false;
}


bool OctDevice::MainBoard::loadDeviceSettings(OctConfig::DeviceSettings * pset)
{
	getRetinaIrLed()->loadConfig(pset);
	getCorneaIrLed()->loadConfig(pset);
	getKeratoRingLed()->loadConfig(pset);
	getKeratoFocusLed()->loadConfig(pset);
	getWorkingDotsLed()->loadConfig(pset);
	getWorkingDot2Led()->loadConfig(pset);
	getSplitFocusLed()->loadConfig(pset);
	getExtFixationLed()->loadConfig(pset);
	getFundusFlashLed()->loadConfig(pset);
	return true;
}


bool OctDevice::MainBoard::loadCameraSettings(OctConfig::CameraSettings * pset)
{
	getRetinaCamera()->loadConfig(pset);
	getCorneaCamera()->loadConfig(pset);
	getColorCamera()->loadConfig(pset);
	return true;
}


void OctDevice::MainBoard::saveDeviceSettings(OctConfig::DeviceSettings * pset)
{
	getRetinaIrLed()->saveConfig(pset);
	getCorneaIrLed()->saveConfig(pset);
	getKeratoRingLed()->saveConfig(pset);
	getKeratoFocusLed()->saveConfig(pset);
	getFundusFlashLed()->saveConfig(pset);
	getSplitFocusLed()->saveConfig(pset);
	getExtFixationLed()->saveConfig(pset);
	getWorkingDotsLed()->saveConfig(pset);
	getWorkingDot2Led()->saveConfig(pset);
	// getOctSldLed()->saveConfig(pset);
	// getPannelLed()->saveConfig(pset);

	/*
	getOctFocusMotor()->saveConfig(pset);
	getReferenceMotor()->saveConfig(pset);
	getPolarizationMotor()->saveConfig(pset);
	getFundusFocusMotor()->saveConfig(pset);
	getOctSampleMotor()->saveConfig(pset);
	getSplitFocusMotor()->saveConfig(pset);
	getPupilMaskMotor()->saveConfig(pset);
	*/
	return;
}


void OctDevice::MainBoard::saveCameraSettings(OctConfig::CameraSettings * pset)
{
	getRetinaCamera()->saveConfig(pset);
	getCorneaCamera()->saveConfig(pset);
	getColorCamera()->saveConfig(pset);
	return;
}


void OctDevice::MainBoard::performPackagingMode(void)
{
	auto xmotor = getStageXMotor();
	auto ymotor = getStageYMotor();
	auto zmotor = getStageZMotor();

	xmotor->updateStatus();
	ymotor->updateStatus();
	zmotor->updateStatus();

	int posX = xmotor->getPosition();
	int posY = ymotor->getPosition();
	int posZ = zmotor->getPosition();

	int target, offset;

	target = (xmotor->getRangeMax() + xmotor->getRangeMin()) / 2;
	offset = target - posX;
	xmotor->updatePositionByOffset(offset);

	for (int count = 0; count < 30; count++) {
		Sleep(100);
		xmotor->updateStatus();
		if (abs(xmotor->getPosition() - target) <= 3) {
			break;
		}
	}
	xmotor->controlStop();

	target = (zmotor->getRangeMax() + zmotor->getRangeMin()) / 2;
	offset = target - posX;
	zmotor->updatePositionByOffset(offset);

	for (int count = 0; count < 30; count++) {
		Sleep(100);
		zmotor->updateStatus();
		if (abs(zmotor->getPosition() - target) <= 3) {
			break;
		}
	}
	zmotor->controlStop();

	target = ymotor->getRangeMin();
	offset = target - posY;
	ymotor->updatePositionByOffset(offset);

	for (int count = 0; count < 50; count++) {
		Sleep(100);
		ymotor->updateStatus();
		if (abs(ymotor->getPosition() - target) <= 25) {
			break;
		}
	}
	ymotor->controlStop();

	if (!isChinrestAtLowLimit()) {
		moveChinrestDown();
		for (int count = 0; count < 130; count++) {
			Sleep(100);
			if (isChinrestAtLowLimit()) {
				LogD() << "Chinrest low limit";
				break;
			}
		}
		stopChinrest();
	}
	return;
}


bool OctDevice::MainBoard::loadAddressTable(std::uint32_t addr)
{
	bool ret = getUsbComm().readAddress(addr, (uint8_t*)&getImpl().addressTbl, sizeof(AddressTable));

	if (ret) {
		LogD() << std::hex << "InitStat: " << getImpl().addressTbl.initStat;
		LogD() << std::hex << "SysVer: " << getImpl().addressTbl.sysVer;
		LogD() << std::hex << "SysCfg: " << getImpl().addressTbl.sysCfg;
		LogD() << std::hex << "Trajectory profile base: " << getImpl().addressTbl.trajProf;
		LogD() << std::hex << "ADC buffer: " << getImpl().addressTbl.adcBuffer;
		LogD() << std::hex << "Firmware download base: " << getImpl().addressTbl.downloadBase;
		LogD() << std::hex << "GP status: " << getImpl().addressTbl.gpStatus;
		LogD() << std::hex << "Step motor info: " << getImpl().addressTbl.motorInfo;
		LogD() << std::hex << "SysCal: " << getImpl().addressTbl.sysCal;
		LogD() << std::hex << "Galvano dynamic: " << getImpl().addressTbl.galvanoDynamic;
		LogD() << std::hex << "AutoStage motor info: " << getImpl().addressTbl.autoStageMotorInfo;
		LogD() << std::hex << "SysCal2: " << getImpl().addressTbl.sysCal2;
		// DebugOut2() << "SLD parameter = " << getImpl().addressTbl.sldParameter;

		getUsbComm().readAddress(getBaseAddressOfSystemInitStatus(), (std::uint8_t*)&getImpl().sysInitStatus, sizeof(SysInitStatus));
		getUsbComm().readAddress(getBaseAddressOfSystemVersion(), (std::uint8_t*)&getImpl().sysVerInfo, sizeof(SysVersionInfo));
	}
	return ret;
}


bool OctDevice::MainBoard::openFTDIdevices(bool reset, bool reload)
{
	bool openUsb = false;
	bool resetFTDI = false;

	if (reset) {
		resetUsbChannel();
		this_thread::sleep_for(chrono::milliseconds(USB_RESET_WAIT_TIME));
	}

	if (reload) {
		goto open_reload;
	}
	
open_retry:
	for (int i = 0; i < USB_RESET_RETRY_MAX; i++) {
		if (!getUsbComm().openChannel()) {
			GlobalLogger::debug("Usb channel open failed!, retrying ... ");
			resetUsbChannel();
			this_thread::sleep_for(chrono::milliseconds(USB_RESET_RETRY_DELAY));
		}
		else {
			openUsb = true;
			break;
		}
	}

open_reload:
	if (!openUsb && !resetFTDI) {
		if (resetFTDIdevice()) {
			GlobalLogger::debug("FTDI chipset reset ok, waiting ... ");
			this_thread::sleep_for(chrono::milliseconds(FTDI_RESET_WAIT_TIME));
			resetFTDI = true;
			goto open_retry;
		}
		else {
			GlobalLogger::debug("FTDI chipset reset failed!");
		}
	}
	return openUsb;
}


bool OctDevice::MainBoard::openColorCamera(bool wait)
{
	bool openCam = false;

	for (int i = 0; i < USB_RESET_RETRY_MAX; i++) {
		if (!resetColorCamera()) {
			LogD() << "Color camera open failed!, retry: " << (i + 1);
			this_thread::sleep_for(chrono::milliseconds(USB_RESET_RETRY_DELAY));
		}
		else {
			openCam = true;
			if (wait) {
				this_thread::sleep_for(chrono::milliseconds(USB_RESET_COLOR_WAIT_TIME));
			}
			break;
		}
	}
	return openCam;
}


bool OctDevice::MainBoard::checkInitStatus(int & status)
{
	std::string smsg = UART_CMD_INIT_STATUS;
	std::string rmsg;

	for (int i = 0; i < 10; i++) {
		if (requestUartCommand(smsg, rmsg)) {
			if (rmsg.find('0') != std::string::npos) {
				status = UART_INIT_STATUS_PROGRESS;
			}
			else if (rmsg.find('1') != std::string::npos) {
				status = UART_INIT_STATUS_ERROR;
			}
			else if (rmsg.find('2') != std::string::npos) {
				status = UART_INIT_STATUS_OK;
			}
			else {
				continue;
			}
			return true;
		}
		else {
			GlobalLogger::debug("Request init status failed!, retrying ... ");
		}
		this_thread::sleep_for(chrono::milliseconds(UART_INIT_STATUS_RETRY_DELAY));
	}
	// this_thread::sleep_for(chrono::milliseconds(UART_INIT_STATUS_TIMEOUT_WAIT));
	return false;
}


std::string OctDevice::MainBoard::parseDeviceStatus(std::uint32_t status)
{
	std::string sstr;

	sstr = "";
	if (!(status & 0x01)) {
		sstr += " CPU";
	}
	if (!(status & 0x02)) {
		sstr += " SDRAM";
	}
	if (!(status & 0x04)) {
		sstr += " FPGA";
	}
	if (!(status & 0x08)) {
		sstr += " OCT FOCUS MOTOR";
	}
	if (!(status & 0x10)) {
		sstr += " OCT REF MOTOR";
	}
	if (!(status & 0x20)) {
		sstr += " OCT POL MOTOR";
	}
	if (!(status & 0x40)) {
		sstr += " FUNDUS FOCUS MOTOR";
	}
	if (!(status & 0x80)) {
		sstr += " QUICK RETURN MIRROR";
	}
	if (!(status & 0x100)) {
		sstr += " SPLIT FOCUS MOTOR";
	}
	if (!(status & 0x200)) {
		sstr += " FUNDUS DIOPTER COMP MOTOR";
	}
	if (!(status & 0x400)) {
		sstr += " PUPIL MASK MOTOR";
	}
	if (!(status & 0x800)) {
		sstr += " OCT DIOPTER COMP MOTOR";
	}
	if (!(status & 0x1000)) {
		sstr += " X STAGE";
	}
	if (!(status & 0x2000)) {
		sstr += " Y STAGE";
	}
	if (!(status & 0x4000)) {
		sstr += " Z STAGE";
	}
	if (!(status & 0x8000)) {
		sstr += " CHINREST";
	}
	if (!(status & 0x10000)) {
		sstr += " GALVOX";
	}
	if (!(status & 0x20000)) {
		sstr += " GALVOY";
	}
	if (!(status & 0x40000)) {
		sstr += " CORNEA IR CAMERA";
	}
	if (!(status & 0x80000)) {
		sstr += " RETINA IR CAMERA";
	}
	if (!(status & 0x100000)) {
		sstr += " SLD";
	}
	if (!(status & 0x200000)) {
		sstr += " EEPROM";
	}
	if (!(status & 0x400000)) {
		sstr += " FIXATION LCD";
	}
	
	if (sstr.length() == 0) {
		sstr = "UNKNOWN";
	}
	return sstr;
}


bool OctDevice::MainBoard::resetFTDIdevice(void)
{
	std::string smsg = UART_CMD_FTDI_RESET;
	std::string rmsg;

	bool result = false;
	if (requestUartCommand(smsg, rmsg)) {
		if (rmsg.find(std::string(UART_CMD_ACK_PREFIX)) != std::string::npos) {
			result = true;
		}
	}
	return result;
}


bool OctDevice::MainBoard::resetUsbChannel(void)
{
	std::string smsg = UART_CMD_USB_RESET;
	std::string rmsg;

	bool result = false;
	if (requestUartCommand(smsg, rmsg)) {
		if (rmsg.find(std::string(UART_CMD_ACK_PREFIX)) != std::string::npos) {
			result = true;
		}
	}
	return result;
}


bool OctDevice::MainBoard::resetColorCamera(void)
{
	std::string smsg = UART_CMD_COLOR_RESET;
	std::string rmsg;

	bool result = false;
	if (requestUartCommand(smsg, rmsg)) {
		if (rmsg.find(std::string(UART_CMD_ACK_PREFIX)) != std::string::npos) {
			result = true;
		}
	}
	return result;
}


bool OctDevice::MainBoard::rebootSystem(void)
{
	std::string smsg = UART_CMD_SYSTEM_REBOOT;
	std::string rmsg;

	bool result = false;
	if (requestUartCommand(smsg, rmsg)) {
		if (rmsg.find(std::string(UART_CMD_ACK_PREFIX)) != std::string::npos) {
			result = true;
		}
	}
	return result;
}


bool OctDevice::MainBoard::requestUartCommand(std::string smsg, std::string& rmsg, int sizeMax)
{
	SerialPort serial;
	bool result = false;
	auto port = UART_PORT_NUM;

	if (serial.open(UART_PORT_NUM)) {
		rmsg = "";
		if (serial.write(smsg)) {
			serial.read(rmsg, sizeMax);
			if (!rmsg.empty()) {
				result = true;
			}
			else {
				LogD() << "Uart command not replied, smsg: " << smsg;
			}
		}
		serial.close();
	}
	else {
		LogE() << "Uart port open failed!, num: " << port;
	}
	return result;
}


bool OctDevice::MainBoard::moveChinrestUp(void)
{
	return getUsbComm().ChinrestMove(1);
}


bool OctDevice::MainBoard::moveChinrestDown(void)
{
	return getUsbComm().ChinrestMove(0);
}

bool OctDevice::MainBoard::stopChinrest(void)
{
	return getUsbComm().ChinrestStop();
}

bool OctDevice::MainBoard::isChinrestAtHighLimit(void)
{
	std::uint32_t status;
	if (getUsbComm().PI_ReadStatus(getBaseAddressOfGPStatus(), &status)) {
		if (status & (0x01 << 12)) {
			return true;
		}
	}
	return false;
}

bool OctDevice::MainBoard::isChinrestAtLowLimit(void)
{
	std::uint32_t status;
	if (getUsbComm().PI_ReadStatus(getBaseAddressOfGPStatus(), &status)) {
		if (status & (0x01 << 13)) {
			return true;
		}
	}
	return false;
}


bool OctDevice::MainBoard::isUartDebugEnabled(void)
{
	uint32_t baseAddr = getBaseAddressOfSysConfiguration();
	uint16_t flag;
	
	if (!getUsbComm().UartDebugGet(baseAddr, &flag)) {
		return false;
	}

	bool enabled = (flag ? true : false);
	return enabled;
}


bool OctDevice::MainBoard::setUartDebugEnabled(bool flag)
{
	uint32_t baseAddr = getBaseAddressOfSysConfiguration();
	uint16_t byteFlag = (flag ? 1 : 0);

	if (!getUsbComm().UartDebugSet(baseAddr, byteFlag)) {
		return false;
	}
	return true;
}


bool OctDevice::MainBoard::setStageLimitOverride(bool flag)
{
	uint32_t baseAddr = getBaseAddressOfSysConfiguration();
	uint16_t byteFlag = (flag ? 1 : 0);

	if (!getUsbComm().StageLimitOverride(baseAddr, byteFlag)) {
		return false;
	}
	return true;
}

bool OctDevice::MainBoard::setSldWarningEnabled(int flag)
{
	uint32_t baseAddr = getBaseAddressOfSysConfiguration();
	if (!getUsbComm().SldWariningEnabled(baseAddr, (std::uint16_t)flag)) {
		return false;
	}
	return true;
}

bool OctDevice::MainBoard::updateScannerFaultParameters(std::uint16_t timer, std::uint16_t thresh)
{
	if (!getUsbComm().ScannerFaultParameters(timer, thresh)) {
		return false;
	}
	return true;
}


bool OctDevice::MainBoard::setLineCameraGain(unsigned short value)
{
	string cmd, res;
	cmd = "agr";
	cmd += " ";
	cmd += (char) value;
	cmd += "\r";

	SerialPort sport;
	if (sport.open(4) || sport.open(3)) {
		if (sport.write(cmd)) {
			res.clear();
			sport.read(res, 12);
			if (res.length() > 0) {
				auto idx = res.find("OK");
				if (idx != string::npos) {
					stringstream stream;
					stream << std::hex << value;
					LogD() << "Line camera gain changed: " << stream.str();
					return true;
				}
			}
		}
	}
	return false;
}


bool OctDevice::MainBoard::readLineCameraBlackOffset(int mode, std::string & value)
{
	string cmd, res;
	cmd = "bo" + to_string(mode);
	cmd += "\r";

	SerialPort sport;
	if (sport.open(4) || sport.open(3)) {
		if (sport.write(cmd)) {
			res.clear();
			sport.read(res, 12);
			if (res.length() > 0) {
				auto idx = res.find('R');
				if (idx != string::npos) {
					auto end = res.find('>', idx);
					if (end != string::npos) {
						idx = idx + 1;
						value = res.substr(idx, end - idx);
						return true;
					}
				}
			}
		}
	}
	return false;
}


bool OctDevice::MainBoard::saveLineCameraBlackOffset(int mode, std::string value)
{
	string cmd, res;
	cmd = "bo" + to_string(mode);
	cmd += " ";
	cmd += value;
	cmd += "\r";

	SerialPort sport;
	if (sport.open(4) || sport.open(3)) {
		if (sport.write(cmd)) {
			res.clear();
			sport.read(res, 12);
			if (res.length() > 0) {
				auto idx = res.find("OK");
				if (idx != string::npos) {
					return true;
				}
			}
		}
	}
	return false;
}


bool OctDevice::MainBoard::flashWriteLineCameraBlackOffset(void)
{
	string cmd, res;
	cmd = "wus";
	cmd += "\r";

	SerialPort sport;
	if (sport.open(4) || sport.open(3)) {
		if (sport.write(cmd)) {
			sport.read(res, 12);
			if (res.length() > 0) {
				if (res.find("OK") != string::npos) {
					return true;
				}
			}
		}
	}
	return false;
}


bool OctDevice::MainBoard::prepareDevicesForScan(void)
{
	setScanBeamOn(true);
	// setQuickReturnMirrorIn(true);
	return true;
}


bool OctDevice::MainBoard::releaseDevicesForScan(void)
{
	setScanBeamOn(false);
	// setQuickReturnMirrorIn(false);
	return true;
}


bool OctDevice::MainBoard::setQuickReturnMirrorIn(bool flag)
{
	if (isNoMotors()) {
		return true;
	}
	return getUsbComm().QuickReturnMirror(flag == true ? MIRROR_IN : MIRROR_OUT) ;
}


bool OctDevice::MainBoard::setSplitFocusMirrorIn(bool flag)
{
	if (isNoMotors()) {
		return true;
	}
	return getUsbComm().SplitFocusMirror(flag == true ? MIRROR_IN : MIRROR_OUT);
}


bool OctDevice::MainBoard::setDiopterCompensationLens(int mode, int lens)
{
	if (isNoMotors()) {
		return true;
	}
	return getUsbComm().DiopterCompensationLens(mode, lens);
}


bool OctDevice::MainBoard::setSmallPupilMask(bool flag)
{
	if (isNoMotors()) {
		return true;
	}
	return getUsbComm().PupilMaskSelect(flag == true ? SMALL_PUPIL_MASK : NORMAL_PUPIL_MASK);
}


bool OctDevice::MainBoard::setFundusCameraCapture(void)
{
	return getUsbComm().FundusCameraCapture();
}


bool OctDevice::MainBoard::setInternalFixationOn(bool flag, int row, int col)
{
	getImpl().inFixationRow = row;
	getImpl().inFixationCol = col;
	getImpl().inFixationOn = flag;

	return getUsbComm().InternalFixationControl(row, col);
}


bool OctDevice::MainBoard::getInternalFixationOn(int & row, int & col)
{
	row = getImpl().inFixationRow;
	col = getImpl().inFixationCol;

	if (!getImpl().inFixationOn) {
		return false;
	}
	return true;
}


bool OctDevice::MainBoard::setLcdFixationOn(bool flag, int row, int col)
{
	getImpl().lcdFixationRow = row;
	getImpl().lcdFixationCol = col;
	getImpl().lcdFixationOn = flag;
	
	return getUsbComm().LcdFixationControl(row, col);
}


bool OctDevice::MainBoard::getLcdFixationOn(int & row, int & col)
{
	row = getImpl().lcdFixationRow;
	col = getImpl().lcdFixationCol;

	if (!getImpl().lcdFixationOn) {
		return false;
	}
	return true;
}


bool OctDevice::MainBoard::updateLcdParameters(std::uint8_t bright, std::uint8_t blink, std::uint16_t period, std::uint16_t ontime, std::uint8_t type)
{
	return getUsbComm().LcdFixationParameters(bright, blink, period, ontime, type);
}


bool OctDevice::MainBoard::setScanBeamOn(bool flag)
{
	return getOctSldLed()->setIntensity(flag == true ? TURN_ON : TURN_OFF) ;
}

bool OctDevice::MainBoard::addGalvanoDynamicOffset(std::int16_t initX, std::int16_t initY, float mmPosX, float mmPosY, bool cornea)
{
	std::int16_t offsetX = 0;
	std::int16_t offsetY = 0;

	float mmRevX = mmPosY * -1.0f;
	float mmRevY = mmPosX * -1.0f;

	offsetX = getGalvanometer()->getStepXat(mmRevX, cornea);
	offsetY = getGalvanometer()->getStepYat(mmRevY, cornea);

	offsetX += initX;
	offsetY += initY;

	if (getUsbComm().GalvanoDynamicOffsetWrite(getBaseAddressOfGalvanoDynamic(), offsetX, offsetY)) {
		LogD() << "Galvano dynamic offset added, initX: " << initX << ", initY: " << initY << ", mmX: " << mmPosX << ", mmY : " << mmPosY << " = > " << offsetX << ", " << offsetY;
		return true;
	}
	return false;
}

bool OctDevice::MainBoard::setGalvanoDynamicOffset(float mmPosX, float mmPosY, bool cornea)
{
	std::int16_t offsetX = 0;
	std::int16_t offsetY = 0;

	float mmRevX = mmPosY * -1.0f;
	float mmRevY = mmPosX * -1.0f;

	offsetX = getGalvanometer()->getStepXat(mmRevX, cornea);
	offsetY = getGalvanometer()->getStepYat(mmRevY, cornea);

	if (getUsbComm().GalvanoDynamicOffsetWrite(getBaseAddressOfGalvanoDynamic(), offsetX, offsetY)) { 
		LogD() << "Galvano dynamic offset written, mmX: " << mmPosX << ", mmY: " << mmPosY << " => " << offsetX << ", " << offsetY;
		return true;
	}
	return false;
}


bool OctDevice::MainBoard::setGalvanoDynamicOffset(std::int16_t offsetX, std::int16_t offsetY)
{
	if (getUsbComm().GalvanoDynamicOffsetWrite(getBaseAddressOfGalvanoDynamic(), offsetX, offsetY)) {
		LogD() << "Galvano dynamic offset written, x: " << offsetX << ", y: " << offsetY;
		return true;
	}
	return false;
}


bool OctDevice::MainBoard::getGalvanoDynamicOffset(std::int16_t & offsetX, std::int16_t & offsetY)
{
	if (getUsbComm().GalvanoDynamicOffsetRead(getBaseAddressOfGalvanoDynamic(), offsetX, offsetY)) {
		LogD() << "Galvano dynamic offset read, x: " << offsetX << ", y: " << offsetY;
		return true;
	}
	return false;
}


std::string OctDevice::MainBoard::getApplicationVersion(void) const
{
	std::string text;
	text += d_ptr->sysVerInfo.appVer[0];
	text += d_ptr->sysVerInfo.appVer[1];
	text += ".";
	text += d_ptr->sysVerInfo.appVer[2];
	text += d_ptr->sysVerInfo.appVer[3];
	text += ".";
	text += d_ptr->sysVerInfo.appVer[4];
	text += d_ptr->sysVerInfo.appVer[5];
	return text;
}


std::string OctDevice::MainBoard::getFPGAVersion(void) const
{
	std::string text;
	text += d_ptr->sysVerInfo.fpgaVer[0];
	text += d_ptr->sysVerInfo.fpgaVer[1];
	text += ".";
	text += d_ptr->sysVerInfo.fpgaVer[2];
	text += d_ptr->sysVerInfo.fpgaVer[3];
	text += ".";
	text += d_ptr->sysVerInfo.fpgaVer[4];
	text += d_ptr->sysVerInfo.fpgaVer[5];
	return text;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfSystemInitStatus(void) const
{
	return getImpl().addressTbl.initStat;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfSystemVersion(void) const
{
	return getImpl().addressTbl.sysVer;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfMotorInfo(void) const
{
	return getImpl().addressTbl.motorInfo;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfStageInfo(void) const
{
	return getImpl().addressTbl.autoStageMotorInfo;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfSysConfiguration(void) const
{
	return getImpl().addressTbl.sysCfg;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfTrajectoryProfile(void) const
{
	return getImpl().addressTbl.trajProf;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfSysCalibration(void) const
{
	return getImpl().addressTbl.sysCal;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfDownload(void) const
{
	return getImpl().addressTbl.downloadBase;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfGPStatus(void) const
{
	return getImpl().addressTbl.gpStatus;
}

std::uint32_t OctDevice::MainBoard::getBaseAddressOfGalvanoDynamic(void) const
{
	return getImpl().addressTbl.galvanoDynamic;
}


std::uint32_t OctDevice::MainBoard::getBaseAddressOfSysProfile(void) const
{
	return getImpl().addressTbl.sysCal2;
}


IrCamera * OctDevice::MainBoard::getCorneaIrCamera(void)
{
	return getIrCamera(CameraType::CORNEA);
}


IrCamera * OctDevice::MainBoard::getRetinaIrCamera(void)
{
	return getIrCamera(CameraType::RETINA);
}


OctDevice::CorneaCamera * OctDevice::MainBoard::getCorneaCamera(void)
{
	return (CorneaCamera*) getIrCamera(CameraType::CORNEA);
}


OctDevice::RetinaCamera * OctDevice::MainBoard::getRetinaCamera(void)
{
	return (RetinaCamera*) getIrCamera(CameraType::RETINA);
}


OctDevice::ColorCamera * OctDevice::MainBoard::getColorCamera(void)
{
	return (ColorCamera*) getImpl().colorCamera.get();
}


OctDevice::LightLed * OctDevice::MainBoard::getRetinaIrLed(void)
{
	return getLightLed(LightLedType::RETINA_IR);
}


OctDevice::LightLed * OctDevice::MainBoard::getCorneaIrLed(void)
{
	return getLightLed(LightLedType::CORNEA_IR);
}


OctDevice::LightLed * OctDevice::MainBoard::getKeratoRingLed(void)
{
	return getLightLed(LightLedType::KER_RING);
}


OctDevice::LightLed * OctDevice::MainBoard::getKeratoFocusLed(void)
{
	return getLightLed(LightLedType::KER_FOCUS);
}


OctDevice::FlashLed * OctDevice::MainBoard::getFundusFlashLed(void)
{
	return (FlashLed*)getLightLed(LightLedType::FUNDUS_FLASH);
}


OctDevice::LightLed * OctDevice::MainBoard::getSplitFocusLed(void)
{
	return getLightLed(LightLedType::SPLIT_FOCUS);
}


OctDevice::LightLed * OctDevice::MainBoard::getExtFixationLed(void)
{
	return getLightLed(LightLedType::EXT_FIXATION);
}


OctDevice::LightLed * OctDevice::MainBoard::getWorkingDotsLed(void)
{
	return getLightLed(LightLedType::WORKING_DOTS);
}


OctDevice::LightLed * OctDevice::MainBoard::getWorkingDot2Led(void)
{
	return getLightLed(LightLedType::WORKING_DOT2);
}


OctDevice::OctSldLed * OctDevice::MainBoard::getOctSldLed(void)
{
	return (OctSldLed*) getLightLed(LightLedType::OCT_SLD);
}


OctDevice::LightLed * OctDevice::MainBoard::getPannelLed(void)
{
	return getLightLed(LightLedType::PANNEL);
}


OctFocusMotor* OctDevice::MainBoard::getOctFocusMotor(void)
{
	return (OctFocusMotor*)getStepMotor(StepMotorType::OCT_FOCUS);
}


ReferenceMotor * OctDevice::MainBoard::getReferenceMotor(void)
{
	return (ReferenceMotor*)getStepMotor(StepMotorType::REFERENCE);
}


PolarizationMotor * OctDevice::MainBoard::getPolarizationMotor(void)
{
	return (PolarizationMotor*)getStepMotor(StepMotorType::POLARIZATION);
}


FundusFocusMotor * OctDevice::MainBoard::getFundusFocusMotor(void)
{
	return (FundusFocusMotor*)getStepMotor(StepMotorType::FUNDUS_FOCUS);
}


OctSampleMotor * OctDevice::MainBoard::getOctSampleMotor(void)
{
	return (OctSampleMotor*) getStepMotor(StepMotorType::OCT_SAMPLE);
}


SplitFocusMotor * OctDevice::MainBoard::getSplitFocusMotor(void)
{
	return (SplitFocusMotor*) getStepMotor(StepMotorType::SPLIT_FOCUS);
}


FundusDiopterMotor * OctDevice::MainBoard::getFundusDiopterMotor(void)
{
	return (FundusDiopterMotor*)getStepMotor(StepMotorType::FUNDUS_DIOPTER);
}


OctDiopterMotor * OctDevice::MainBoard::getOctDiopterMotor(void)
{
	return (OctDiopterMotor*)getStepMotor(StepMotorType::OCT_DIOPTER);
}


PupilMaskMotor * OctDevice::MainBoard::getPupilMaskMotor(void)
{
	return (PupilMaskMotor*) getStepMotor(StepMotorType::PUPIL_MASK);
} 


StageXMotor * OctDevice::MainBoard::getStageXMotor(void)
{
	return (StageXMotor*) getStageMotor(StageMotorType::STAGE_X);
}


StageYMotor * OctDevice::MainBoard::getStageYMotor(void)
{
	return (StageYMotor*)getStageMotor(StageMotorType::STAGE_Y);
}


StageZMotor * OctDevice::MainBoard::getStageZMotor(void)
{
	return (StageZMotor*)getStageMotor(StageMotorType::STAGE_Z);
}


OctDevice::Galvanometer * OctDevice::MainBoard::getGalvanometer(void)
{
	return getImpl().galvano.get();
}


IrCamera * OctDevice::MainBoard::getIrCamera(CameraType type)
{
	for (int i = 0; i < getImpl().irCameras.size(); i++) {
		IrCamera* irCam = getImpl().irCameras[i].get();
		if (irCam->getType() == type) {
			return irCam;
		}
	}
	return nullptr;
}


OctDevice::LightLed * OctDevice::MainBoard::getLightLed(LightLedType type)
{
	for (int i = 0; i < getImpl().lightLeds.size(); i++) {
		LightLed* light = getImpl().lightLeds[i].get();
		if (light->getType() == type) {
			return light;
		}
	}
	return nullptr;
}


OctDevice::StepMotor * OctDevice::MainBoard::getStepMotor(StepMotorType type)
{
	for (int i = 0; i < getImpl().stepMotors.size(); i++) {
		StepMotor* motor = getImpl().stepMotors[i].get();
		if (motor->getType() == type) {
			return motor;
		}
	}
	return nullptr;
}


OctDevice::StageMotor * OctDevice::MainBoard::getStageMotor(StageMotorType type)
{
	for (int i = 0; i < getImpl().stageMotors.size(); i++) {
		StageMotor* motor = getImpl().stageMotors[i].get();
		if (motor->getType() == type) {
			return motor;
		}
	}
	return nullptr;
}


MainBoard::MainBoardImpl & OctDevice::MainBoard::getImpl(void) const
{
	return *d_ptr;
}


UsbComm & OctDevice::MainBoard::getUsbComm(void) const
{
	return getImpl().usbComm;
}
