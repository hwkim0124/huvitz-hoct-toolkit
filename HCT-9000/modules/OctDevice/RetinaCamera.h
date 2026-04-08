#pragma once

#include "OctDeviceDef.h"
#include "IrCamera.h"

#include <memory>


namespace OctDevice
{
	class MainBoard;

	class OCTDEVICE_DLL_API RetinaCamera : public IrCamera
	{
	public:
		RetinaCamera();
		RetinaCamera(MainBoard *board);
		virtual ~RetinaCamera();

	public:
		virtual bool initialize(void);

		virtual void loadSystemParameters(void);
		virtual void saveSystemParameters(void);

		bool applyCameraPreset(int preset);

	private:
		struct RetinaCameraImpl;
		std::unique_ptr<RetinaCameraImpl> d_ptr;
		RetinaCameraImpl& getImpl(void) const;
	};
}
