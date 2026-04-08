#pragma once

#include "OctGrabDef.h"

#include <memory>
#include <string>
#include <vector>


namespace OctGrab
{
	class Usb3Grabber;

	class OCTGRAB_DLL_API UsbLineCamera
	{
	public:
		UsbLineCamera();
		virtual ~UsbLineCamera();

		UsbLineCamera(UsbLineCamera&& rhs);
		UsbLineCamera& operator=(UsbLineCamera&& rhs);

		// Prevent copy construction and assignment. 
		UsbLineCamera(const UsbLineCamera& rhs) = delete;
		UsbLineCamera& operator=(const UsbLineCamera& rhs) = delete;

	public:
		virtual bool openLineCamera(Usb3Grabber* grabber);
		virtual bool closeLineCamera(void);
		virtual bool startAcquisition(bool restart);
		virtual bool cancelAcquisition(void);

		virtual bool isCameraOpened(void);
		virtual bool isCameraGrabbing(void);

		virtual bool updateFrameHeight(int height);
		virtual bool setExposureToNormal(void);
		virtual bool setExposureToFaster(void);
		virtual bool setExposureToFastest(void);
		virtual bool setExposureTime(float expTime);
		virtual bool setAnalogGain(float gain);

		virtual float getExposureTime(void);
		virtual float getAnalogGain(void);

		virtual bool acquirePreviewImages(std::vector<int> bufferIds);
		virtual bool acquireMeasureImages(std::vector<int> bufferIds);
		virtual bool acquireEnfaceImages(std::vector<int> bufferIds);

	protected:
		void setGrabber(Usb3Grabber* grabber);
		Usb3Grabber* getGrabber(void) const;

	private:
		struct UsbLineCameraImpl;
		std::unique_ptr<UsbLineCameraImpl> d_ptr;
		UsbLineCameraImpl& impl(void) const;
	};
}

