#pragma once

#include "OctGrabDef.h"

#include <memory>
#include <string>
#include <vector>


namespace OctGrab
{
	class UsbLineCamera;

	class OCTGRAB_DLL_API Usb3Grabber
	{
	public:
		Usb3Grabber();
		virtual ~Usb3Grabber();

		Usb3Grabber(Usb3Grabber&& rhs);
		Usb3Grabber& operator=(Usb3Grabber&& rhs);

		// Prevent copy construction and assignment. 
		Usb3Grabber(const Usb3Grabber& rhs) = delete;
		Usb3Grabber& operator=(const Usb3Grabber& rhs) = delete;

	public:
		bool initializeUsb3Grabber(void);
		void releaseUsb3Grabber(void);
		bool isInitiated(void) const;

		bool startCameraAcquisition(bool init);
		void cancelCameraAcquisition(void);

		bool setSourceImageSizeY(int size);
		bool setCamearaParameters(int imageSizeY, int numbOfBuffers=0);
		bool setCameraSpeedToNormal(void);
		bool setCameraSpeedToFaster(void);
		bool setCameraSpeedToFastest(void);
		bool setLineCameraExposureTime(float expTime);
		bool setLineCameraAnalogGain(float gain);

		float getLineCameraExposureTime(void);
		float getLineCameraAnalogGain(void);

		bool isAtSideOd(void);
		EyeSide getEyeSide(void);

		bool grabPreviewProcess(int index, int count = 1);
		bool grabPreviewProcessWait(int index, int count = 1);
		bool grabMeasureProcess(int index, int count = 1);
		bool grabMeasureProcessWait(int index, int count = 1);
		bool grabEnfaceProcess(int index, int count = 1);
		bool grabEnfaceProcessWait(int index, int count = 1);

		void hookGrabPreviewBuffer(unsigned short* buff, int bufferId, size_t dataSize);
		void hookGrabMeasureBuffer(unsigned short* buff, int bufferId, size_t dataSize);
		void hookGrabEnfaceBuffer(unsigned short* buff, int bufferId, size_t dataSize);

		bool prepareBuffersToPattern(std::vector<int> previews, std::vector<int> enfaces, std::vector<int> measures);
		bool prepareBuffersToPreview(std::vector<int> sizes);
		bool prepareBuffersToMeasure(std::vector<int> sizes, int frameSize = 0);
		bool prepareBuffersToEnface(std::vector<int> sizes);
		void releaseBuffersToPreview(void);
		void releaseBuffersToMeasure(void);
		void releaseBuffersToEnface(void);
		void releaseBuffersToPattern(void);

		void clearGrabbedPreviewBuffer(void);
		void clearGrabbedMeasureBuffer(void);
		void clearGrabbedEnfaceBuffer(void);

		void setGrabbedPreviewBuffer(int index);
		void setGrabbedMeasureBuffer(int index);
		void setGrabbedEnfaceBuffer(int index);

		int getGrabbedPreviewBuffer(void);
		int getGrabbedMeasureBuffer(void);
		int getGrabbedEnfaceBuffer(void);

		void setStartMeasureBuffer(int index);
		int getStartMeasureBuffer(void);

		int getBufferIndexOfPreviewImage(int buffId);
		int getBufferIndexOfMeasureImage(int buffId);
		int getBufferIndexOfEnfaceImage(int buffId);

		unsigned short* getBufferOfPreviewImage(int index);
		unsigned short* getBufferOfMeasureImage(int index);
		unsigned short* getBufferOfEnfaceImage(int index);

		int getBufferSizeXOfPreviewImage(int index);
		int getBufferSizeXOfMeasureImage(int index);
		int getBufferSizeXOfEnfaceImage(int index);
		int getBufferSizeYOfPreviewImage(int index);
		int getBufferSizeYOfMeasureImage(int index);
		int getBufferSizeYOfEnfaceImage(int index);

		void hookGrabPreviewBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);
		void hookGrabMeasureBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);
		void hookGrabEnfaceBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);

		void setGrabPreviewBufferCallback(GrabPreviewBufferCallback* callback);
		void setGrabMeasureBufferCallback(GrabMeasureBufferCallback* callback);
		void setGrabEnfaceBufferCallback(GrabEnfaceBufferCallback* callback);

	protected:
		UsbLineCamera* getUsbLineCamera(void) const;

	private:
		struct Usb3GrabberImpl;
		std::unique_ptr<Usb3GrabberImpl> d_ptr;
		Usb3GrabberImpl& getImpl(void) const;
	};
}
