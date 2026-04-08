#include "stdafx.h"
#include "DeviceSettings.h"

#include <vector>

using namespace OctConfig;
using namespace std;



struct DeviceSettings::DeviceSettingsImpl
{
	GalvanometerItems galvano;
	vector<LightLedItem> lights;
	vector<StepMotorItem> motors;

	DeviceSettingsImpl() 
	{
	}
};


DeviceSettings::DeviceSettings() :
	d_ptr(make_unique<DeviceSettingsImpl>())
{
	initialize();
}


OctConfig::DeviceSettings::~DeviceSettings() = default;
OctConfig::DeviceSettings::DeviceSettings(DeviceSettings && rhs) = default;
DeviceSettings & OctConfig::DeviceSettings::operator=(DeviceSettings && rhs) = default;


OctConfig::DeviceSettings::DeviceSettings(const DeviceSettings & rhs)
	: d_ptr(make_unique<DeviceSettingsImpl>(*rhs.d_ptr))
{
}


DeviceSettings & OctConfig::DeviceSettings::operator=(const DeviceSettings & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctConfig::DeviceSettings::initialize(void)
{
	vector<LightLedItem>& lights = getImpl().lights;
	lights.clear();
	lights.emplace_back(LED_RETINA_IR_NAME, LightLedType::RETINA_IR);
	lights.emplace_back(LED_RETINA_IR_BRIGHT_NAME, LightLedType::RETINA_IR_BRIGHT);
	lights.emplace_back(LED_CORNEA_IR_NAME, LightLedType::CORNEA_IR);
	lights.emplace_back(LED_KER_RING_NAME, LightLedType::KER_RING);
	lights.emplace_back(LED_KER_FOCUS_NAME, LightLedType::KER_FOCUS);
	lights.emplace_back(LED_FUNDUS_FLASH_NAME, LightLedType::FUNDUS_FLASH);
	lights.emplace_back(LED_SPLIT_FOCUS_NAME, LightLedType::SPLIT_FOCUS);
	lights.emplace_back(LED_EXT_FIXATION_NAME, LightLedType::EXT_FIXATION);
	lights.emplace_back(LED_WORKING_DOTS_NAME, LightLedType::WORKING_DOTS);
	lights.emplace_back(LED_WORKING_DOT2_NAME, LightLedType::WORKING_DOT2);
	// lights.emplace_back(LED_OCT_SLD_NAME, LightLedType::OCT_SLD);
	// lights.emplace_back(LED_PANNEL_NAME, LightLedType::PANNEL);

	resetToDefaultValues();

	/*
	vector<StepMotorItem>& motors = getImpl().motors;
	motors.clear();
	motors.emplace_back(MOTOR_OCT_FOCUS_NAME, StepMotorType::OCT_FOCUS);
	motors.emplace_back(MOTOR_REFERENCE_NAME, StepMotorType::REFERENCE);
	motors.emplace_back(MOTOR_POLARIZATION_NAME, StepMotorType::POLARIZATION);
	motors.emplace_back(MOTOR_FUNDUS_FOCUS_NAME, StepMotorType::FUNDUS_FOCUS);
	motors.emplace_back(MOTOR_OCT_SAMPLE_NAME, StepMotorType::OCT_SAMPLE);
	motors.emplace_back(MOTOR_SPLIT_FOCUS_NAME, StepMotorType::SPLIT_FOCUS);
	motors.emplace_back(MOTOR_FUNDUS_DIOPTER_NAME, StepMotorType::FUNDUS_DIOPTER);
	motors.emplace_back(MOTOR_OCT_DIOPTER_NAME, StepMotorType::OCT_DIOPTER);
	motors.emplace_back(MOTOR_PUPIL_MASK_NAME, StepMotorType::PUPIL_MASK);
	*/
	return;
}


void OctConfig::DeviceSettings::resetToDefaultValues(void)
{
	getRetinaIrLed()->setValue(LED_RETINA_IR_INIT_VALUE);
	getRetinaIrLedBright()->setValue(LED_RETINA_IR_BRIGHT_INIT_VALUE);
	getCorneaIrLed()->setValue(LED_CORNEA_IR_INIT_VALUE);
	getKeratoRingLed()->setValue(LED_KER_RING_INIT_VALUE);
	getKeratoFocusLed()->setValue(LED_KER_FOCUS_INIT_VALUE);
	getFundusFlashLed()->setValue(LED_FUNDUS_FLASH_INIT_VALUE);
	getSplitFocusLed()->setValue(LED_SPLIT_FOCUS_INIT_VALUE);
	getExtFixationLed()->setValue(LED_EXT_FIXATION_INIT_VALUE);
	getWorkingDotsLed()->setValue(LED_WORKING_DOTS_INIT_VALUE);
	getWorkingDot2Led()->setValue(LED_WORKING_DOTS_INIT_VALUE);
	// getOctSldLed()->setValue(LED_OCT_SLD_INIT_VALUE);
	return;
}


LightLedItem * OctConfig::DeviceSettings::getRetinaIrLed(void) const
{
	return getLightLedItem(LightLedType::RETINA_IR);
}

LightLedItem * OctConfig::DeviceSettings::getCorneaIrLed(void) const
{
	return getLightLedItem(LightLedType::CORNEA_IR);
}

LightLedItem * OctConfig::DeviceSettings::getKeratoRingLed(void) const
{
	return getLightLedItem(LightLedType::KER_RING);
}

LightLedItem * OctConfig::DeviceSettings::getKeratoFocusLed(void) const
{
	return getLightLedItem(LightLedType::KER_FOCUS);
}

LightLedItem * OctConfig::DeviceSettings::getFundusFlashLed(void) const
{
	return getLightLedItem(LightLedType::FUNDUS_FLASH);
}

LightLedItem * OctConfig::DeviceSettings::getSplitFocusLed(void) const
{
	return getLightLedItem(LightLedType::SPLIT_FOCUS);
}

LightLedItem * OctConfig::DeviceSettings::getExtFixationLed(void) const
{
	return getLightLedItem(LightLedType::EXT_FIXATION);
}

LightLedItem * OctConfig::DeviceSettings::getWorkingDotsLed(void) const
{
	return getLightLedItem(LightLedType::WORKING_DOTS);
}


LightLedItem * OctConfig::DeviceSettings::getWorkingDot2Led(void) const
{
	return getLightLedItem(LightLedType::WORKING_DOT2);
}

LightLedItem * OctConfig::DeviceSettings::getOctSldLed(void) const
{
	return getLightLedItem(LightLedType::OCT_SLD);
}

LightLedItem * OctConfig::DeviceSettings::getPannelLed(void) const
{
	return getLightLedItem(LightLedType::PANNEL);
}


LightLedItem * OctConfig::DeviceSettings::getRetinaIrLedBright(void) const
{
	return getLightLedItem(LightLedType::RETINA_IR_BRIGHT);
}


StepMotorItem * OctConfig::DeviceSettings::getOctFocusMotor(void) const
{
	return getStepMotorItem(StepMotorType::OCT_FOCUS);
}

StepMotorItem * OctConfig::DeviceSettings::getReferenceMotor(void) const
{
	return getStepMotorItem(StepMotorType::REFERENCE);
}

StepMotorItem * OctConfig::DeviceSettings::getPolarizationMotor(void) const
{
	return getStepMotorItem(StepMotorType::POLARIZATION);
}

StepMotorItem * OctConfig::DeviceSettings::getFundusFocusMotor(void) const
{
	return getStepMotorItem(StepMotorType::FUNDUS_FOCUS);
}

StepMotorItem * OctConfig::DeviceSettings::getOctSampleMotor(void) const
{
	return getStepMotorItem(StepMotorType::OCT_SAMPLE);
}

StepMotorItem * OctConfig::DeviceSettings::getSplitFocusMotor(void) const
{
	return getStepMotorItem(StepMotorType::SPLIT_FOCUS);
}

StepMotorItem * OctConfig::DeviceSettings::getFundusDiopterMotor(void) const
{
	return getStepMotorItem(StepMotorType::FUNDUS_DIOPTER);
}

StepMotorItem * OctConfig::DeviceSettings::getPupilMaskMotor(void) const
{
	return getStepMotorItem(StepMotorType::PUPIL_MASK);
}

StepMotorItem * OctConfig::DeviceSettings::getOctDiopterMotor(void) const
{
	return getStepMotorItem(StepMotorType::OCT_DIOPTER);
}


LightLedItem * OctConfig::DeviceSettings::getLightLedItem(const char * name) const
{
	for (int i = 0; i < getImpl().lights.size(); i++) {
		if (getImpl().lights[i].isName(name)) {
			return &getImpl().lights[i];
		}
	}
	return nullptr;
}


LightLedItem * OctConfig::DeviceSettings::getLightLedItem(int index) const
{
	if (index < 0 || index >= getImpl().lights.size()) {
		return nullptr;
	}
	return &getImpl().lights[index];
}


LightLedItem * OctConfig::DeviceSettings::getLightLedItem(LightLedType type) const
{
	for (int i = 0; i < getImpl().lights.size(); i++) {
		if (getImpl().lights[i].isType(type)) {
			return &getImpl().lights[i];
		}
	}
	return nullptr;
}


int OctConfig::DeviceSettings::getLightLedItemsCount(void) const
{
	return (int)getImpl().lights.size();
}


StepMotorItem * OctConfig::DeviceSettings::getStepMotorItem(const char * name) const
{
	for (int i = 0; i < getImpl().motors.size(); i++) {
		if (getImpl().motors[i].isName(name)) {
			return &getImpl().motors[i];
		}
	}
	return nullptr;
}


StepMotorItem * OctConfig::DeviceSettings::getStepMotorItem(int index) const
{
	if (index < 0 || index >= getImpl().motors.size()) {
		return nullptr;
	}
	return &getImpl().motors[index];
}


StepMotorItem * OctConfig::DeviceSettings::getStepMotorItem(StepMotorType type) const
{
	for (int i = 0; i < getImpl().motors.size(); i++) {
		if (getImpl().motors[i].isType(type)) {
			return &getImpl().motors[i];
		}
	}
	return nullptr;
}


int OctConfig::DeviceSettings::getStepMotorItemsCount(void) const
{
	return (int)getImpl().motors.size();
}


DeviceSettings::DeviceSettingsImpl& OctConfig::DeviceSettings::getImpl(void) const
{
	return *d_ptr;
}






