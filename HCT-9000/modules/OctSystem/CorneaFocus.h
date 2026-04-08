#pragma once

#include "OctSystemDef.h"

#include <memory>


namespace OctDevice {
	class MainBoard;
}


namespace KerFocus {
	class KeratoFocus;
	class KeratoSetup;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API CorneaFocus
	{
	public:
		CorneaFocus();
		virtual ~CorneaFocus();

	public:
		static bool initialize(OctDevice::MainBoard* board);
		static bool loadKeratoSetupData(const char* path = KER_SETUP_DATA_FILE_NAME);
		static bool saveKeratoSetupData(const char* path = KER_SETUP_DATA_FILE_NAME);
		static bool getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a);
		static bool updateFocusStepDataFromImage(int diopt, int istep);
		static void clearFocusStepData(int diopt = -1, int istep = -1);

		static bool getFocusDistance(float& dist);

		static KerFocus::KeratoFocus* getKeratoFocus(void);
		static KerFocus::KeratoSetup* getKeratoSetup(void);
		static void setCameraImageCallback(CorneaCameraImageCallback* callback);

		static void startAutoAlignment(void);
		static void cancelAutoAlignment(void);
		static bool isAutoAligning(void);

		static void setTargetRangeX(float x1, float x2);
		static void setTargetRangeY(float y1, float y2);
		static void setTargetRangeZ(float z1, float z2);
		static void getTargetRangeX(float& x1, float& x2);
		static void getTargetRangeY(float& y1, float& y2);
		static void getTargetRangeZ(float& z1, float& z2);

		static bool isTargetCenterAligned(float distX, float distY);
		static bool isTargetFocusAligned(float distZ);
		static bool isTargetAlignedX(float distX);
		static bool isTargetAlignedY(float distY);
		static bool isTargetAlignedZ(float distZ);

	private:
		struct CorneaFocusImpl;
		static std::unique_ptr<CorneaFocusImpl> d_ptr;
		static CorneaFocusImpl& getImpl(void);

		static void callbackIrCameraFrame(unsigned char* data, unsigned int width, unsigned int height);
	};
}
