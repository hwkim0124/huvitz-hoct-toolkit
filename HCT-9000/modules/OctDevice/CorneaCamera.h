#pragma once

#include "OctDeviceDef.h"
#include "IrCamera.h"

#include <memory>

namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API CorneaCamera : public IrCamera
	{
	public:
		CorneaCamera();
		CorneaCamera(MainBoard *board);
		virtual ~CorneaCamera();

	public:
		virtual bool initialize(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

	private:
		struct CorneaCameraImpl;
		std::unique_ptr<CorneaCameraImpl> d_ptr;
		CorneaCameraImpl& getImpl(void) const;
	};
}

