#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <string>


namespace OctDevice {
	class MainBoard;
}


namespace KerFocus {
	class KeratoFocus;
	class KeratoSetup;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API CorneaAlign
	{
	public:
		CorneaAlign();
		virtual ~CorneaAlign();


	public:
		static bool initCorneaAlign(OctDevice::MainBoard* board);
		static bool importKeratoSetupData(const char* path = nullptr);
		static bool exportKeratoSetupData(const char* path = nullptr);
		static bool loadKeratoSetupData(void);
		static bool saveKeratoSetupData(bool write);

		static bool getFocusStepData(int diopt, int istep, float& mlen, float& flen, float& fdif, float& m, float& n, float& a);
		static bool updateFocusStepDataFromImage(int diopt, int istep);
		static void clearFocusStepData(int diopt = -1, int istep = -1);

		static std::string getDefaultKeratoSetupDataPath(void);
		static bool getFocusDistance(float& distZ);
		static bool getCenterDistance(float& distX, float& distY);

		static KerFocus::KeratoFocus* getKeratoFocus(void);
		static KerFocus::KeratoSetup* getKeratoSetup(void);
		static void setCameraImageCallback(CorneaCameraImageCallback* callback);

		static void startAutoAlignment(CorneaAlignTarget target);
		static void cancelAutoAlignment(void);
		static bool isAutoAligning(void);

		static bool isTargetCenterAligned(float distX, float distY, float scale= 0.9f);
		static bool isTargetFocusAligned(float distZ);
		static bool isTargetAlignedX(float distX, float scale = 0.9f);
		static bool isTargetAlignedY(float distY, float scale = 0.9f);
		static bool isTargetAlignedZ(float distZ);

		static bool enableStageX(bool isset = false, bool flag = true);
		static bool enableStageY(bool isset = false, bool flag = true);
		static bool enableStageZ(bool isset = false, bool flag = true);


	protected:
		static void callbackIrCameraFrame(unsigned char* data, unsigned int width, unsigned int height);
		static void doAutoAlignment(bool isCenter, bool isFocus, float distX, float distY, float distZ);
		static void doScanAlignment(bool isCenter, float distX=0.0f, float distY=0.0f);
		static void alignStageX(float dist, float scale = 0.9f);
		static void alignStageY(float dist, float scale = 0.9f);
		static void alignStageZ(float dist);

	private:
		struct CorneaAlignImpl;
		static std::unique_ptr<CorneaAlignImpl> d_ptr;
		static CorneaAlignImpl& getImpl(void);
	};
}
