#pragma once

#include "OctSystemDef.h"

#include <memory>

namespace RetFocus {
	class RetinaFocus2;
	class RetinaFrame;
	class TrackImage2;
}

namespace CppUtil {
	class CvImage;
}


namespace OctSystem
{
	class OCTSYSTEM_DLL_API RetinaTrack2
	{
	public:
		RetinaTrack2();
		virtual ~RetinaTrack2();

	public:
		static void initiateTracking(void);
		static void prepareTracking(OctScanMeasure& meas);
		static void prepareTracking(EyeSide side, bool disc=false, float rangeX=6.0f, float rangeY=6.0f);
		static void startTracking(void);
		static void cancelTracking(void);

		static bool getTrackingMatchedInfo(int& cx, int& cy, int& w, int& h, float& val, bool cropped = false);
		static bool getTrackingTargetInfo(int& cx, int& cy, int& w, int& h, float& val, bool cropped = false);
		static bool getTrackingRegionInfo(int& cx, int& cy, int& w, int& h, float& val, bool cropped=false);
		static bool getScanningRegionInfo(int& cx, int& cy, int& w, int& h, bool& pending, bool cropped=false);
		
		static void getScanCenterPosition(int& cx, int& cy);
		static void getScanCenterOffset(float& cx, float& cy);
		static void setScanCenterOffset(float cx, float cy);
		static void getScanRangeSize(int& w, int& h);

		static bool registRetinaFrame(const RetFocus::RetinaFrame& frame);
		static bool searchTargetOnFrame(const RetFocus::RetinaFrame& frame);
		static bool adjustScanningRegion(void);

		static bool isTrackingPrepared(void);
		static bool isTrackingCanceled(void);
		static bool isTrackingRunning(void);
		static bool isTargetRegistered(void);

		static bool isTargetDisplaced(void);
		static bool isTargetLocated(void);
		static bool isPendingScanning(void);

		static void setTargetRegistered(bool flag);
		static void setTargetDisplaced(bool flag);
		static void setPendingScanning(bool flag);
		static void setTargetValidRange(float range);

		static int getLastImageIndexGrabbed(void);
		static void setLastImageIndexGrabbed(int index);

		static RetFocus::TrackImage2& getCameraImage(void);
		static RetFocus::TrackImage2& getRegistImage(void);

		static void resetScanCenterPosition(float offsetX, float offsetY);
		static void updateScanCenterPosition(float dx, float dy);

	private:
		struct RetinaTrack2Impl;
		static std::unique_ptr<RetinaTrack2Impl> d_ptr;
		static RetinaTrack2Impl& getImpl(void);
	};
}

