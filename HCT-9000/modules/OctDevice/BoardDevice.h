#pragma once

#include "OctDeviceDef.h"

#include <memory>

namespace OctConfig {
	class DeviceSettings;
}


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API BoardDevice
	{
	public:
		BoardDevice();
		BoardDevice(MainBoard* board);
		virtual ~BoardDevice();

		BoardDevice(BoardDevice&& rhs);
		BoardDevice& operator=(BoardDevice&& rhs);
		BoardDevice(const BoardDevice& rhs);
		BoardDevice& operator=(const BoardDevice& rhs);

	public:
		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct BoardDeviceImpl;
		std::unique_ptr<BoardDeviceImpl> d_ptr;
		BoardDeviceImpl& getImpl(void) const;
	};
}

