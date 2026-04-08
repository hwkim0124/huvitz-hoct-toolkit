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
	class TrackFeature;

	class RETFOCUS_DLL_API TrackCamera
	{
	public:
		TrackCamera();
		~TrackCamera();

		TrackCamera(TrackCamera&& rhs);
		TrackCamera& operator=(TrackCamera&& rhs);

	public:
		void setupTrackCamera(OctDevice::MainBoard* mainboard);
		void adjustCameraIntensity(RetFocus::TrackFeature* frame);
		bool initiateCameraControl(void);
		bool releaseCameraControl(void);

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
		struct TrackCameraImpl;
		std::unique_ptr<TrackCameraImpl> d_ptr;
		TrackCameraImpl& impl(void) const;
	};
}
