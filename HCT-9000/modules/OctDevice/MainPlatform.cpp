#include "stdafx.h"
#include "MainPlatform.h"
#include "MainBoard.h"
#include "UsbComm.h"
#include "Susi4.h"

#include "CppUtil2.h"
#include "OctConfig2.h"

using namespace OctDevice;
using namespace OctConfig;
using namespace CppUtil;
using namespace std;

struct MainPlatform::MainPlatformImpl
{
	bool backlightOn;
	bool eventWorking;
	bool isAtSideOd;

	MainBoard* mainBoard;

	thread worker;
	JoystickEventCallback* cbJoystickEvent;
	EyeSideEventCallback* cbEyeSideEvent;
	OptimizeKeyEventCallback* cbOptimizeEvent;
	BacklightOnEventCallback* cbBacklightOnEvent;
	SldWarningEventCallback* cbSldWarningEvent;
	ScannerFaultEventCallback* cbScannerFaultEvent;

	MainPlatformImpl() : backlightOn(false), isAtSideOd(true), eventWorking(false),
		cbJoystickEvent(nullptr), cbEyeSideEvent(nullptr), cbOptimizeEvent(nullptr), cbBacklightOnEvent(nullptr), 
		cbSldWarningEvent(nullptr), cbScannerFaultEvent(nullptr), mainBoard(nullptr)
	{
	}
};


OctDevice::MainPlatform::MainPlatform()
{
}


OctDevice::MainPlatform::~MainPlatform()
{
}


// Direct initialization of static smart pointer.
std::unique_ptr<MainPlatform::MainPlatformImpl> MainPlatform::d_ptr(new MainPlatformImpl());


bool OctDevice::MainPlatform::initMainPlatform(OctDevice::MainBoard* board)
{
	auto result = SusiLibInitialize();
	if (result == SUSI_STATUS_NOT_INITIALIZED) {
		LogD() << "SUSI lib not initialized";
		return false;
	}

	if (true) { // GlobalSettings::checkMainboardAtStartup()) {
		auto bankId = SUSI_ID_GPIO_BANK(0);
		auto status = SusiGPIOSetDirection(bankId, 0xff, 0xff);

		auto gpioId = SUSI_ID_GPIO(4);
		uint32_t pvalue;
		uint32_t count = 0;

		LogD() << "GPIO level checking...";

		do
		{
			status = SusiGPIOGetLevel(gpioId, 0xff, &pvalue);
			this_thread::sleep_for(chrono::milliseconds(100));

			if (++count > 600) {
				// return false;
				LogE() << "GPIO level time expired";
				break;
			}
		} while (pvalue);
	}
	
	setBacklightEnable(true);
	getImpl().mainBoard = board;
	return true;
}

unsigned char OctDevice::MainPlatform::gpio_init(void)
{
	typedef struct{
		unsigned char supPinNum;	/* 0 ~ 31 */
		unsigned supInput;
		unsigned supOutput;
	} GPIOInfo;
	GPIOInfo info[5];

	unsigned int status, supportPin, id;
	unsigned char index, i, j;

	index = 0;
	for (i = 0; i < 5; i++)
	{
		//id = SUSI_ID_GPIO_BANK(i);
		id = SUSI_ID_GPIO_BANK(i);

		status = SusiGPIOGetCaps(id, SUSI_ID_GPIO_INPUT_SUPPORT, &info[i].supInput);
		if (status != SUSI_STATUS_SUCCESS)
			continue;

		status = SusiGPIOGetCaps(id, SUSI_ID_GPIO_OUTPUT_SUPPORT, &info[i].supOutput);
		if (status != SUSI_STATUS_SUCCESS)
			continue;

		supportPin = info[i].supInput | info[i].supOutput;

		if (supportPin > 0)
		{
			for (j = 32; (supportPin & (1 << (j - 1))) == 0; j--);
			info[i].supPinNum = j;
		}
		else
		{
			info[i].supPinNum = 0;
		}

		index++;
	}

	if (index == 0)
		return 0xff;

	return 0;
}

void OctDevice::MainPlatform::startGpioThread(void)
{
	closeGpioThread();

	getImpl().eventWorking = true;
	getImpl().worker = thread{ &MainPlatform::eventWorkerThread };
	return;
}

void OctDevice::MainPlatform::closeGpioThread(void)
{
	getImpl().eventWorking = false;
	if (getImpl().worker.joinable()) {
		getImpl().worker.join();
	}
	return;
}

void OctDevice::MainPlatform::eventWorkerThread(void)
{
	auto id = SUSI_ID_GPIO(4);
	auto status = SusiGPIOSetDirection(id, 0xff, SUSI_GPIO_INPUT);
	status = SusiGPIOSetDirection(SUSI_ID_GPIO(2), 0xff, SUSI_GPIO_INPUT);
	status = SusiGPIOSetDirection(SUSI_ID_GPIO(3), 0xff, SUSI_GPIO_INPUT);
	status = SusiGPIOSetDirection(SUSI_ID_GPIO(5), 0xff, SUSI_GPIO_INPUT);
	status = SusiGPIOSetDirection(SUSI_ID_GPIO(6), 0xff, SUSI_GPIO_INPUT);

	if (GlobalSettings::isSldFaultDetectionEnabled()) {
		status = SusiGPIOSetDirection(SUSI_ID_GPIO(1), 0xff, SUSI_GPIO_INPUT);
	}

	uint32_t pvalue, Optimize_Btn, Joystick_Btn, old_side, Eye_side, sld_fault;
	status = SusiGPIOGetLevel(SUSI_ID_GPIO(5), 0xff, &old_side);

	while (true)
	{
		if (!getImpl().eventWorking) {
			break;
		}
		status = SusiGPIOGetLevel(id, 0xff, &pvalue);
		if (status != SUSI_STATUS_SUCCESS)
		{
			GlobalLogger::error("SusiGPIOGetLevel() failed!");
			break;
		}

		status = SusiGPIOGetLevel(SUSI_ID_GPIO(2), 0xff, &Optimize_Btn);
		status = SusiGPIOGetLevel(SUSI_ID_GPIO(3), 0xff, &Joystick_Btn);
		status = SusiGPIOGetLevel(SUSI_ID_GPIO(5), 0xff, &Eye_side);

		if (old_side != Eye_side) {
			updateCurrentEyeSide();
			old_side = Eye_side;
		}

		if (Optimize_Btn) {
			updateOptimizeStatus();
		}
		if (Joystick_Btn) {
			updateJoystickStatus();
		}

		if (GlobalSettings::isSldFaultDetectionEnabled()) {
			status = SusiGPIOGetLevel(SUSI_ID_GPIO(1), 0xff, &sld_fault);
			if (sld_fault) {
				updateSldFaultStatus();
			}
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return;
}

bool OctDevice::MainPlatform::updateCurrentEyeSide(void)
{
	uint32_t Eye_side;
	auto status = SusiGPIOGetLevel(SUSI_ID_GPIO(5), 0xff, &Eye_side);

	if (Eye_side != 0) {
		getImpl().isAtSideOd = false;
		if (getImpl().cbEyeSideEvent) {
			(*getImpl().cbEyeSideEvent)(false);
		}
	}
	else {
		getImpl().isAtSideOd = true;
		if (getImpl().cbEyeSideEvent) {
			(*getImpl().cbEyeSideEvent)(true);
		}
	}
	return true;
}


bool OctDevice::MainPlatform::updateOptimizeStatus(void)
{
	uint32_t Optimize_Btn;
	for (int i = 0; i < 20; i++) {
		auto status = SusiGPIOGetLevel(SUSI_ID_GPIO(2), 0xff, &Optimize_Btn);
		if (Optimize_Btn == 0) {
			return false;
		}
		this_thread::sleep_for(chrono::milliseconds(1));
	}
	DebugOut2() << "Optimized key pressed event occurred!";
	if (!MainPlatform::isBacklightOn()) {
		MainPlatform::turnOnBacklight();
		if (d_ptr->cbBacklightOnEvent) {
			(*d_ptr->cbBacklightOnEvent)();
		}
	}
	else {
		if (d_ptr->cbOptimizeEvent) {
			(*d_ptr->cbOptimizeEvent)();
		}
	}
	return true;
}

bool OctDevice::MainPlatform::updateJoystickStatus(void)
{
	uint32_t Joystick_Btn;
	for (int i = 0; i < 3; i++) {
		auto status = SusiGPIOGetLevel(SUSI_ID_GPIO(3), 0xff, &Joystick_Btn);
		if (Joystick_Btn == 0) {
			return false;
		}
		this_thread::sleep_for(chrono::milliseconds(5));
	}
	if (!MainPlatform::isBacklightOn()) {
		MainPlatform::turnOnBacklight();
		if (d_ptr->cbBacklightOnEvent) {
			(*d_ptr->cbBacklightOnEvent)();
		}
	}
	else {
		if (getImpl().cbJoystickEvent != nullptr) {
			(*getImpl().cbJoystickEvent)();
		}
	}
	return true;
}

bool OctDevice::MainPlatform::updateSldFaultStatus(void)
{
	uint32_t fault;
	auto status = SusiGPIOGetLevel(SUSI_ID_GPIO(1), 0xff, &fault);

	if (fault) {
		auto* board = getImpl().mainBoard;
		if (board) {
			UsbComm& usbComm = board->getUsbComm();
			uint32_t int_src;
			board->getUsbComm().UpdateDeviceStatus(1);
			if (board->getUsbComm().Interrupt_ReadStatus(getImpl().mainBoard->getBaseAddressOfGPStatus(), &int_src)) {
				LogD() << "SLD fault status of GPIO checked: " << fault << ", interrupt source read : " << std::hex << int_src;
				if (int_src & 0x01) {
					if (getImpl().cbSldWarningEvent) {
						LogD() << "Invoking SLD warning event callback";
						(*getImpl().cbSldWarningEvent)();
					}
				}
				else if (int_src & 0x08) {
					if (getImpl().cbScannerFaultEvent) {
						LogD() << "Invoking scanner fault event callback";
						(*getImpl().cbScannerFaultEvent)();
					}
				}
			}
		}
	}
	return true;
}


void OctDevice::MainPlatform::setJoystickEventCallback(JoystickEventCallback * callback)
{
	getImpl().cbJoystickEvent = callback;
	return;
}


void OctDevice::MainPlatform::setEyeSideEventCallback(EyeSideEventCallback * callback)
{
	getImpl().cbEyeSideEvent = callback;
	return;
}


void OctDevice::MainPlatform::setOptimizeKeyEventCallback(OptimizeKeyEventCallback * callback)
{
	getImpl().cbOptimizeEvent = callback;
	return;
}


void OctDevice::MainPlatform::setBacklightOnEventCallback(BacklightOnEventCallback * callback)
{
	getImpl().cbBacklightOnEvent = callback;
	return;
}

void OctDevice::MainPlatform::setSldWarningEventCallback(SldWarningEventCallback* callback)
{
	getImpl().cbSldWarningEvent = callback;
	return;
}

void OctDevice::MainPlatform::setScannerFaultEventCallback(ScannerFaultEventCallback* callback)
{
	getImpl().cbScannerFaultEvent = callback;
	return;
}

bool OctDevice::MainPlatform::isAtSideOd(void)
{
	return getImpl().isAtSideOd;
}


EyeSide OctDevice::MainPlatform::getEyeSide(void)
{
	return (isAtSideOd() ? EyeSide::OD : EyeSide::OS);
}

void OctDevice::MainPlatform::releaseMainPlatform(void)
{
	SusiLibUninitialize();
	return;
}


bool OctDevice::MainPlatform::setBacklightEnable(bool flag)
{
	auto value = (flag ? SUSI_BACKLIGHT_SET_ON : SUSI_BACKLIGHT_SET_OFF);
	auto result = SusiVgaSetBacklightEnable(SUSI_ID_BACKLIGHT_1, value);

	d_ptr->backlightOn = flag;

	if (result == SUSI_STATUS_SUCCESS) {
		return true;
	}
	return false;
}


bool OctDevice::MainPlatform::isBacklightOn(void)
{
	return d_ptr->backlightOn;
}


void OctDevice::MainPlatform::turnOnBacklight(void)
{
	if (!isBacklightOn()) {
		setBacklightEnable(true);
	}
	return;
}


void OctDevice::MainPlatform::turnOffBacklight(void)
{
	if (isBacklightOn()) {
		setBacklightEnable(false);
	}
	return;
}


int OctDevice::MainPlatform::getCurrentEyeSide(void)
{
	auto gpioId = SUSI_ID_GPIO(5);
	uint32_t pvalue;
	uint32_t count = 0;

	auto status = SusiGPIOGetLevel(gpioId, 0xff, &pvalue);
	if (status != SUSI_STATUS_SUCCESS) {
		pvalue = -1;
	}

	return pvalue;
}


OctDevice::MainPlatform::MainPlatformImpl & OctDevice::MainPlatform::getImpl(void)
{
	return *d_ptr;
}
