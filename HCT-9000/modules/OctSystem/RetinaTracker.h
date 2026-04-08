#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <mutex>

namespace RetFocus {
	class RetinaFrame;
	class TrackFeature;
	class TrackCamera;
}

namespace CppUtil {
	class CvImage;
}

namespace OctDevice {
	class MainBoard;
}


namespace OctSystem
{
	enum RetinaTrackState
	{
		UNKNOWN = 0,
		PREPARED,
		AUTO_ADJUSTMENT,
		CAMERA_ADJUSTED,
		FEATURE_REQUESTED,
		FEATURE_REGISTERED,
		PAUSED,
		STARTED, 
		PENDING,
	};

	class OCTSYSTEM_DLL_API RetinaTracker
	{
	public:
		RetinaTracker();
		virtual ~RetinaTracker();

		RetinaTracker(const RetinaTracker& rhs) = delete;
		RetinaTracker& operator=(const RetinaTracker& rhs) = delete;

		static RetinaTracker* getInstance(void);
		static std::mutex singleMutex_;

	public:
		bool initializeRetinaTracker(OctDevice::MainBoard* board);
		void resetTrackingStatus(void);

		bool setupRetinaTracking(const OctScanMeasure& meas);
		bool setupRetinaTracking(EyeSide side, bool disc=false, float rangeX=6.0f, float rangeY=6.0f);
		bool startRetinaTracking(void);
		bool resumeRetinaTracking(void);
		void pauseRetinaTracking(void);
		void cancelRetinaTracking(void);
		void resetRetinaTracking(void);
		void setLightLedsInitStatus(bool splitOn, bool wdotsOn);

		bool processTrackingFrame(const RetFocus::RetinaFrame& frame);

		bool requestAutoAdjustment(bool withOct);
		bool requestRegistration(bool first);
		bool completeAutoAdjustment(void);
		bool waitForRegistration(int retryMax = 0);
		bool waitForTargetAlignment(void);
		bool waitForTargetIdentified(void);

		bool updateRetinaFrame(const RetFocus::RetinaFrame& frame);
		bool optimizeRetinaFrame(const RetFocus::RetinaFrame& frame);
		bool registerFeatureFrame(const RetFocus::RetinaFrame& frame);
		bool compareCurrentFrame(const RetFocus::RetinaFrame& frame);
		bool recordCurrentFrame(const RetFocus::RetinaFrame& frame);

		bool isFeatureRequested(void) const;
		bool isFeatureRegistered(void) const;
		bool isFeatureValid(void) const;

		int getTrackingState(void) const;
		bool isTrackingPrepared(void) const;
		bool isTrackingStarted(void) const;
		bool isTrackingPending(void) const;
		bool isTrackingRunning(void) const;
		bool isTrackingPaused(void) const;

		void beginTrackRecording(bool init) ;
		void pauseTrackRecording(void) ;
		bool isTrackRecordAvailable(int size);

		bool isTargetConsistent(bool align) const;
		bool isTargetDisplaced(void) const;
		bool isTargetIdentified(void) const;
		bool isTargetCenterAligned(void) const;
		bool isEyelidCovered(void) const;

		void relocateScanningRegion(float offsetX, float offsetY);
		void updateTargetMovement(float movedX, float movedY);

		void getTargetPosition(float& posX, float& posY);
		void getTargetDrifts(float& sizeX, float& sizeY);
		void getTargetOffsetInMM(float& mmX, float& mmY);
		void getScanningOffset(float& offsetX, float& offsetY);
		void getFeatureRegion(int& x, int& y, int& w, int& h) const;
		void getScanningRegion(int& x, int& y, int& w, int& h) const;
		bool getScanningRegionInfo(int& x, int& y, int& w, int& h, bool& pending, bool cropped = true) const;
		void getTrackingRegion(int& x, int& y, int& w, int& h, bool start) const;
		bool getTrackingRegionInfo(int& x, int& y, int& w, int& h, float& value, bool cropped = true) const;

		void setGrabbingImageIndex(int index);
		int getGrabbingImageIndex(void) const;
		float getGrabbingImageRatio(void) const;

		bool getFrameMovements(float& dx, float& dy) const;
		bool getFrameStatistics(float& mean, float& stdev) const;
		bool getFrameVariance(float& covar) const;
		bool getFrameSimilarity(float& match) const;

	protected:
		bool obtainOctPreviewQualityIndex(float& qidx);

	private:
		struct RetinaTrackerImpl;
		std::unique_ptr<RetinaTrackerImpl> d_ptr;
		RetinaTrackerImpl& impl(void) const;
	};
}
