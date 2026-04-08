#pragma once

#include "OctDeviceDef.h"

#include <memory>
#include <string>


namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API LightLed
	{
	public:
		LightLed();
		LightLed(MainBoard *board, LightLedType type);
		virtual ~LightLed();

		LightLed(LightLed&& rhs);
		LightLed& operator=(LightLed&& rhs);
		LightLed(const LightLed& rhs) ;
		LightLed& operator=(const LightLed& rhs);

	public:
		virtual bool initialize(void);
		bool isInitiated(void) const;

		bool setIntensity(unsigned short value);
		unsigned short getIntensity(void) const;
		unsigned short getValueSet1(void) const;
		unsigned short getValueSet2(void) const;

		const char* getName(void) const;
		LightLedType getType(void) const;

		bool lightOn(void);
		bool lightOff(void);
		bool control(bool flag);
		bool isLightOn(void);

		void setControls(CSliderCtrl* pSlider=nullptr, CEdit* pEdit=nullptr);
		bool updateIntensity(int value);
		bool updateIntensityByEdit(void);
		bool updateIntensityByOffset(int offset);

		bool loadConfig(OctConfig::DeviceSettings* dset);
		bool saveConfig(OctConfig::DeviceSettings* dset);

	protected:
		MainBoard* getMainBoard(void) const;
		std::uint8_t getLightLedId(void) const;
		std::uint16_t getInitValue(void) const;

	private:
		struct LightLedImpl;
		std::unique_ptr<LightLedImpl> d_ptr;
		LightLedImpl& getImpl(void) const;
	};
}
