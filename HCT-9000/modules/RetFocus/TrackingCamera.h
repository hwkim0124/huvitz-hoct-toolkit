#pragma once
#include "RetFocusDef.h"

#include <array>
#include <numeric>
#include <memory>
#include <mutex>

namespace OctDevice {
	class MainBoard;
}

namespace RetFocus 
{
    class TrackingObject;

    class RETFOCUS_DLL_API TrackingCamera
    {
    public:
        TrackingCamera();
        virtual ~TrackingCamera();

        TrackingCamera(TrackingCamera&& rhs);
        TrackingCamera& operator=(TrackingCamera&& rhs);

    public:
        void initializeTrackingCamera(OctDevice::MainBoard* mainboard);
        void initiateAdjustment(void);
        void adjustFrameIntensity(RetFocus::TrackingObject* object);
        void completeAdjustment(void);
        void releaseAdjustment(void);

        bool isHighSpeedCamera(void) const;
        bool isFrameLevelOptimal(void) const;
        bool isFrameLevelBelowRange(void) const;
        bool isFrameLevelAboveRange(void) const;
        bool isFrameLevelTooLow(void) const;
        bool isFrameLevelTooHigh(void) const;

    protected:
        bool increaseCameraIntensity(bool bigStep);
        bool decreaseCameraIntensity(bool bigStep);
        bool stepUpCameraGain(void);
        bool stepDownCameraGain(void);
        float getFrameLevelMean(void) const;

    private:
        struct TrackingCameraImpl;
        std::unique_ptr<TrackingCameraImpl> d_ptr;
        TrackingCameraImpl& impl(void) const;
    };

}