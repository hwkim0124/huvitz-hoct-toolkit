#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice {
	class MainBoard;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API RetinaFocus
	{
	public:
		RetinaFocus();
		virtual ~RetinaFocus();

	public:
		static bool initialize(OctDevice::MainBoard* board);
		static void setCameraImageCallback(RetinaCameraImageCallback* callback);

	private:
		struct RetinaFocusImpl;
		static std::unique_ptr<RetinaFocusImpl> d_ptr;
		static RetinaFocusImpl& getImpl(void);

		static void callbackIrCameraFrame(unsigned char* data, unsigned int width, unsigned int height);
	};
}
