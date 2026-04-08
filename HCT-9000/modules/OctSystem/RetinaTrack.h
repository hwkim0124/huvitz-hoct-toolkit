#pragma once

#include "OctSystemDef.h"

#include <memory>

namespace RetFocus {
	class RetinaFocus2;
	class RetinaFrame;
	class TrackImage;
}

namespace CppUtil {
	class CvImage;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API RetinaTrack
	{
	public:
		RetinaTrack();
		virtual ~RetinaTrack();

	public:
		static void initializeRetinaTrack(void);
		static bool registRetinaFrame(const RetFocus::RetinaFrame& frame);
		static bool detectTargetOnFrame(const RetFocus::RetinaFrame& frame);
		static bool registCorneaTarget(float centerX, float centerY);
		static bool detectCorneaCenter(bool found, float centerX=0.0f, float centerY=0.0f);

		static void setTargetFocusDistance(float dist);
		static float getTargetFocusDistance(void);
		static bool getCorneaTargetOffset(float& distX, float& distY);

		static bool checkIfCorneaTargetDisplaced(void);
		static bool checkIfCorneaTargetMoving(void);

		static bool checkIfTargetDisplaced(void);
		static bool checkIfEyeBlinked(void);
		static bool checkIfDetectionSuspended(void);

		static bool getTargetImage(CppUtil::CvImage& image);
		static bool getTargetCenter(int& x, int& y);
		static bool getTargetSize(int& w, int& h);
		static bool getDetectedCenter(int& x, int& y);
		static bool getDetectedValue(float& value);

		static void startTracking(void);
		static void cancelTracking(void);
		static void setMotorControl(bool flag);

		static bool isTrackingStarted(void);
		static bool isTargetRegistered(void);
		static bool isTargetDetected(void);

		static bool isMotorEnabled(void);
		static bool isMotorMoving(void) ;
		static bool isGrabHolding(void) ;
		static void setMotorMoving(bool flag);
		static void setGrabHolding(bool flag);

		static int getLastImageIndexGrabbed(void);
		static void setLastImageIndexGrabbed(int index);

		static bool waitForTargetRegistered(void);



	protected:
		static RetFocus::TrackImage& getTrackCamera(void);
		static RetFocus::TrackImage& getTrackTarget(void);

	private:
		struct RetinaTrackImpl;
		static std::unique_ptr<RetinaTrackImpl> d_ptr;
		static RetinaTrackImpl& getImpl(void);
	};
}

