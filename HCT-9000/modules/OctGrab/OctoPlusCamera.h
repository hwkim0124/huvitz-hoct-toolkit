#pragma once

#include "OctGrabDef.h"

#include <memory>
#include <string>
#include <vector>

#include "UsbLineCamera.h"

namespace OctGrab
{
	class Usb3Grabber;

	class OCTGRAB_DLL_API OctoPlusCamera : public UsbLineCamera
	{
	public:
		OctoPlusCamera();
		virtual ~OctoPlusCamera();

		OctoPlusCamera(OctoPlusCamera&& rhs);
		OctoPlusCamera& operator=(OctoPlusCamera&& rhs);

		// Prevent copy construction and assignment. 
		OctoPlusCamera(const OctoPlusCamera& rhs) = delete;
		OctoPlusCamera& operator=(const OctoPlusCamera& rhs) = delete;

	public:
		bool openLineCamera(Usb3Grabber* grabber) override;
		bool closeLineCamera(void) override;
		bool startAcquisition(bool init) override;
		bool cancelAcquisition(void) override;

		bool updateFrameHeight(int height) override;
		bool setExposureToNormal(void) override;
		bool setExposureToFaster(void) override;
		bool setExposureToFastest(void) override;

		bool setExposureTime(float expTime) override;
		bool setAnalogGain(float gain) override;

		float getExposureTime(void) override;
		float getAnalogGain(void) override;

		bool acquirePreviewImages(std::vector<int> bufferIds) override;
		bool acquireMeasureImages(std::vector<int> bufferIds) override;
		bool acquireEnfaceImages(std::vector<int> bufferIds) override;
		
	private:
		struct OctoPlusCameraImpl;
		std::unique_ptr<OctoPlusCameraImpl> d_ptr;
		OctoPlusCameraImpl& impl(void) const;
	};
}

