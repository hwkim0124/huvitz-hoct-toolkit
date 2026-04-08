#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice {
	class MainBoard;
}

namespace RetFocus {
	class RetinaFocus2;
}

namespace CppUtil {
	class CvImage;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API RetinaAlign
	{
	public:
		RetinaAlign();
		virtual ~RetinaAlign();

	public:
		static bool initRetinaAlign(OctDevice::MainBoard* board);
		static void setCameraImageCallback(RetinaCameraImageCallback* callback);
        static float getIrGamma(void);
        static void setIrGamma(float);

		static CppUtil::CvImage getRetinaImage(void);
		static RetFocus::RetinaFocus2& getRetinaFocus(void);

	private:
		struct RetinaAlignImpl;
		static std::unique_ptr<RetinaAlignImpl> d_ptr;
		static RetinaAlignImpl& getImpl(void);

		static OctDevice::MainBoard* getMainBoard(void);

		static void processRetinaFocus(unsigned char * data, unsigned int width, unsigned int height);
		static void callbackIrCameraFrame(unsigned char* data, unsigned int width, unsigned int height);
	};
}
