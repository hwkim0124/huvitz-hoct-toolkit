#pragma once

#include "OctSystemDef.h"

#include <memory>
#include <mutex>

namespace RetFocus {
    class TrackingRecords;
    class TrackingCamera; 
    class TrackingObject;
    class RetinaFrame;
}

namespace CppUtil {
	class CvImage;
}

namespace OctDevice {
	class MainBoard;
}

namespace OctSystem
{
    enum RetinaTrackingStatus
    {
        RTS_UNKNOWN = 0,
        RTS_CANCELED,
        RTS_PREPARED,
        RTS_ADJUSTING,
        RTS_ADJUSTED,
        RTS_PAUSED, 
        RTS_REGISTERING,
        RTS_REGISTERED,
        RTS_TRACKING, 
    };


	class OCTSYSTEM_DLL_API RetinaTracking
	{
	public:
		RetinaTracking();
		virtual ~RetinaTracking();

        RetinaTracking(const RetinaTracking& rhs) = delete;
        RetinaTracking& operator=(const RetinaTracking& rhs) = delete;

        static RetinaTracking* getInstance(void);
        static std::mutex singleMutex_;

    public:
        bool initializeRetinaTracking(OctDevice::MainBoard* board);
        bool prepareRetinaTracking(const OctScanMeasure& meas); 
        bool prepareRetinaTracking(EyeSide side, bool disc, float rangeX, float rangeY);

        bool startRetinaTracking(bool testMode = false);
        void pauseRetinaTracking(void);
        void cancelRetinaTracking(bool release = true);
        void releaseRetinaTracking(void);
        
        bool requestFrameAdjustment(bool withOct);
        bool completeFrameAdjustment(void);
        bool requestTargetRegistration(void);
        bool waitForTargetRegistration(void);
        bool waitForTargetAlignment(void);
        bool waitForTargetNextFrame(int count);

        void processTrackingFrame(const CppUtil::CvImage& frame);
        void beginRecordingTarget(bool last_track);
        void closeRecordingTarget(void);
        bool isRecordingMotionFree(void) const;
        bool isRecordingEyelidCovered(void) const;

        void setupScanningRegion(float centXmm, float centYmm);
        bool updateScanningRegionByMotion(void);
        void getScanningCenterPosition(float& xmm, float& ymm);

        bool getScanningRegionInfo(int& x, int& y, int& w, int& h, bool& pending, bool cropped = true) const;
        bool getTrackingRegionInfo(int& x, int& y, int& w, int& h, float& value, bool cropped = true) const;
        bool getScanningRegionInfo2(int& x, int& y, int& w, int& h, bool& pending, bool center = false) const;
        bool getTrackingRegionInfo2(int& x, int& y, int& w, int& h, float& value, bool center = false) const;

        void getFrameStatistics(float& roi_mean, float& roi_stdev, float& targ_mean, float& targ_stdev);
        bool getTrackingTargetResult(float& score, float& targ_cx, float& targ_cy, float& delta_x, float& delta_y);
        bool isTrackingTargetOn(void) const;
        bool isTrackingReference(void) const;
        bool isTrackingReady(void) const;
        bool isTrackingInProgress(void) const;
        
        void setGrabbingImageIndex(int index);
        int getGrabbingImageIndex(void) const;
        float getGrabbingImageRatio(void) const;

        void setGuideLedsInitStatus(bool splitOn, bool wdotsOn);
        void setMirrorMotorInitStatus(bool splitIn);

    protected:
        bool updateTrackingFrame(const CppUtil::CvImage& frame);
        void performCameraAdjustment(void);
        void performTargetRegistration(void);
        void performTargetDetection(void);

        void setInitialScanningCenter(float startXmm, float startYmm);
        void relocateScanningCenter(float deltaX, float deltaY);

        void getTrackingScanRegion(int& x, int& y, int& w, int& h, bool center) const;
        bool getTrackingTargetRegion(int& x, int& y, int& w, int& h, bool center) const;

        bool isTargetAvailable(void) const;
        bool isTargetRegistered(void) const;
        bool isTargetPending(void) const;
        bool isTargetTracking(void) const;
        void clearTrackingStatus(void);

        bool obtainOctPreviewQualityIndex(float& qidx);

    private:
        struct RetinaTrackingImpl;
        std::unique_ptr<RetinaTrackingImpl> d_ptr;
        RetinaTrackingImpl& impl(void) const;
	};
}
