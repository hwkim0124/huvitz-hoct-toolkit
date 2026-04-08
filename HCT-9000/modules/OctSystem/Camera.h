#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice {
	class MainBoard;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API Camera
	{
	public:
		Camera();
		virtual ~Camera();

	public:
		static bool initCamera(OctDevice::MainBoard* board);
		static bool isInitiated(void);
		static bool isPosting(void);

		static bool takeColorFundus(const OctFundusMeasure& measure, bool imageProc = true);

	protected:
		static bool prepareShot(void);
		static bool takePicture(void);
		static bool postPicture(void);

		static void beginFundusMeasure(const OctFundusMeasure& measure);
		static void completeFundusMeasure(void);

		static bool startFundusPost(void);
		static bool closeFundusPost(void);
		static void threadPostFunction(void);

		static OctDevice::MainBoard* getMainBoard(void);

	private:
		struct CameraImpl;
		static std::unique_ptr<CameraImpl> d_ptr;
		static CameraImpl& getImpl(void);
	};
}
