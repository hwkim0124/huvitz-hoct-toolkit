#pragma once

#include "OctConfigDef.h"

#include <memory>
#include <string>
#include <minmax.h>


namespace OctConfig
{
	struct OCTCONFIG_DLL_API GalvanometerItems {
		short centerOffsetX;
		short centerOffsetY;
		float scaleRatioX;
		float scaleRatioY;

		GalvanometerItems() : centerOffsetX(0), centerOffsetY(0), scaleRatioX(0.0f), scaleRatioY(0.0f)
		{
		}
	};


	struct OCTCONFIG_DLL_API DeviceItem {
		std::string _name;
		int _value = 0;

		DeviceItem(const char* name) {
			_name = name;
		}

		DeviceItem(const char* name, int value) {
			_name = name;
			_value = value;
		}

		virtual void setValue(int value) {
			_value = value;
		}

		int getValue(void) {
			return _value;
		}

		const char* getName(void) {
			return _name.c_str();
		}

		bool isName(const char* name) {
			return (_name.compare(name) == 0);
		}
	};


	struct OCTCONFIG_DLL_API LightLedItem : public DeviceItem {
		LightLedType _type;

		LightLedItem(const char* name, LightLedType type) : DeviceItem(name) {
			_type = type;
		}

		LightLedItem(const char* name, LightLedType type, int value) : DeviceItem(name, value) {
			_type = type;
		}

		LightLedType getType(void) {
			return _type;
		}

		bool isType(LightLedType type) {
			return _type == type;
		}

		void setValue(int value) {
			_value = min(max(value, LIGHT_LED_VALUE_MIN), LIGHT_LED_VALUE_MAX);
		}

	};


	struct OCTCONFIG_DLL_API StepMotorItem : public DeviceItem {
		StepMotorType _type;
		int _setupPos[32] = { 0 };

		StepMotorItem(const char* name, StepMotorType type) : DeviceItem(name) {
			_type = type;
		}

		StepMotorItem(const char* name, StepMotorType type, int value) : DeviceItem(name, value) {
			_type = type;
		}

		StepMotorType getType(void) {
			return _type;
		}

		bool isType(StepMotorType type) {
			return _type == type;
		}

		int getSetupPosition(int index) {
			return _setupPos[index];
		}

		void setSetupPosition(int index, int value) {
			_setupPos[index] = value;
		}
	};


	class OCTCONFIG_DLL_API DeviceSettings
	{
	public:
		DeviceSettings();
		virtual ~DeviceSettings();

		DeviceSettings(DeviceSettings&& rhs);
		DeviceSettings& operator=(DeviceSettings&& rhs);
		DeviceSettings(const DeviceSettings& rhs);
		DeviceSettings& operator=(const DeviceSettings& rhs);

	public:
		void initialize(void);
		void resetToDefaultValues(void);

		LightLedItem* getRetinaIrLed(void) const;
		LightLedItem* getCorneaIrLed(void) const;
		LightLedItem* getKeratoRingLed(void) const;
		LightLedItem* getKeratoFocusLed(void) const;
		LightLedItem* getFundusFlashLed(void) const;
		LightLedItem* getSplitFocusLed(void) const ;
		LightLedItem* getExtFixationLed(void) const;
		LightLedItem* getWorkingDotsLed(void) const;
		LightLedItem* getWorkingDot2Led(void) const;
		LightLedItem* getOctSldLed(void) const ;
		LightLedItem* getPannelLed(void) const;
		LightLedItem* getRetinaIrLedBright(void) const;

		StepMotorItem* getOctFocusMotor(void) const;
		StepMotorItem* getReferenceMotor(void) const;
		StepMotorItem* getPolarizationMotor(void) const;
		StepMotorItem* getFundusFocusMotor(void) const;
		StepMotorItem* getOctSampleMotor(void) const;
		StepMotorItem* getSplitFocusMotor(void) const;
		StepMotorItem* getFundusDiopterMotor(void) const;
		StepMotorItem* getOctDiopterMotor(void) const;
		StepMotorItem* getPupilMaskMotor(void) const;

		LightLedItem* getLightLedItem(const char* name) const;
		LightLedItem* getLightLedItem(int index) const;
		LightLedItem* getLightLedItem(LightLedType type) const;
		int getLightLedItemsCount(void) const;

		StepMotorItem* getStepMotorItem(const char* name) const;
		StepMotorItem* getStepMotorItem(int index) const;
		StepMotorItem* getStepMotorItem(StepMotorType type) const;
		int getStepMotorItemsCount(void) const;

	private:
		struct DeviceSettingsImpl;
		std::unique_ptr<DeviceSettingsImpl> d_ptr;
		DeviceSettingsImpl& getImpl(void) const;

	};
}

 