#pragma once

#include "OctDeviceDef.h"
#include "LightLed.h"

#include <memory>


namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API OctSldLed : public LightLed
	{
	public:
		OctSldLed();
		OctSldLed(MainBoard *board, LightLedType type);
		virtual ~OctSldLed();

		OctSldLed(OctSldLed&& rhs);
		OctSldLed& operator=(OctSldLed&& rhs);
		OctSldLed(const OctSldLed& rhs);
		OctSldLed& operator=(const OctSldLed& rhs);

	public:
		virtual bool initialize(void);
		void lightOn(void);
		void lightOff(void);

		bool updateParametersToSldMemory(void);
		bool fetchParametersToMainMemory(void);
		std::uint16_t highCode(void);
		std::uint16_t lowCode1(void);
		std::uint16_t lowCode2(void);
		std::uint16_t rsiCode(void);

		bool setHighCode(std::uint16_t code);
		bool setLowCode1(std::uint16_t code);
		bool setLowCode2(std::uint16_t code);
		bool setRsiCode(std::uint16_t code);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct OctSldLedImpl;
		std::unique_ptr<OctSldLedImpl> d_ptr;
		OctSldLedImpl& getImpl(void) const;
	};
}
