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

	class OCTDEVICE_DLL_API FlashLed : public LightLed
	{
	public:
		FlashLed();
		FlashLed(MainBoard *board, LightLedType type);
		virtual ~FlashLed();

		FlashLed(FlashLed&& rhs);
		FlashLed& operator=(FlashLed&& rhs);
		FlashLed(const FlashLed& rhs);
		FlashLed& operator=(const FlashLed& rhs);

	public:
		bool setContinuousMode(bool flag);
		bool getContinuousMode(void);

	private:
		struct FlashLedImpl;
		std::unique_ptr<FlashLedImpl> d_ptr;
		FlashLedImpl& getImpl(void) const;
	};
}
