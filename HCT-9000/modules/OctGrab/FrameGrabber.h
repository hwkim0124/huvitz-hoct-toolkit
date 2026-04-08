#pragma once

#include "OctGrabDef.h"
#include "Mil.h"

#include <memory>
#include <string>
#include <vector>


namespace OctDevice
{
	class MainBoard;
}

namespace OctGrab
{
	class OCTGRAB_DLL_API FrameGrabber
	{
	public:
		FrameGrabber();
		virtual ~FrameGrabber();

		FrameGrabber(FrameGrabber&& rhs);
		FrameGrabber& operator=(FrameGrabber&& rhs);

		// Prevent copy construction and assignment. 
		FrameGrabber(const FrameGrabber& rhs) = delete;
		FrameGrabber& operator=(const FrameGrabber& rhs) = delete;


	public:
		void initFrameGrabber(OctDevice::MainBoard *board);
		bool startFrameGrabber(const std::wstring& dcf_filename);
		void releaseFrameGrabber(bool reset);
		bool isInitiated(void) const;

		bool allocateMilSystemByDCFfile(const std::wstring& dcf_filename);
		void releaseMilSystem(bool reset);

		bool updateDigitizerSettings(void);
		void hookJoystickFunction(void);
		void hookGrabPreviewBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);
		void hookGrabMeasureBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);
		void hookGrabEnfaceBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index);

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

		bool waitForFirstGrabOfPreviewBuffer(int firstIdx = 0);
		bool waitForFirstGrabOfMeasureBuffer(int firstIdx = 0);
		bool waitForFirstGrabOfEnfaceBuffer(int firstIdx = 0);


		bool isCameraConnected(void) const;
		bool isAtSideOd(void);
		EyeSide getEyeSide(void);

		void testFunction(void);

		bool prepareBuffersToPattern(std::vector<int> previews, std::vector<int> enfaces, std::vector<int> measures);
		bool prepareBuffersToPreview(std::vector<int> sizes);
		bool prepareBuffersToMeasure(std::vector<int> sizes, int frameSize=0);
		bool prepareBuffersToEnface(std::vector<int> sizes);
		void releaseBuffersToPreview(void);
		void releaseBuffersToMeasure(void);
		void releaseBuffersToEnface(void);
		void releaseBuffersToPattern(void);

		bool grabPreviewProcess(int index, int count = 1);
		bool grabPreviewProcessWait(int index, int count = 1);
		bool grabMeasureProcess(int index, int count = 1);
		bool grabMeasureProcessWait(int index, int count = 1);
		bool grabEnfaceProcess(int index, int count = 1);
		bool grabEnfaceProcessWait(int index, int count = 1);
		bool copyBufferOfPreviewImage(int index);
		bool copyBufferOfMeasureImage(int index);
		bool copyBufferOfEnfaceImage(int index);

		bool setSourceImageSizeY(int size);

		int getBufferIndexOfPreviewImage(std::uint64_t milBuffId);
		int getBufferIndexOfMeasureImage(std::uint64_t milBuffId);
		int getBufferIndexOfEnfaceImage(std::uint64_t milBuffId);
		int getBufferSizeXOfPreviewImage(int index);
		int getBufferSizeXOfMeasureImage(int index);
		int getBufferSizeXOfEnfaceImage(int index);
		int getBufferSizeYOfPreviewImage(int index);
		int getBufferSizeYOfMeasureImage(int index);
		int getBufferSizeYOfEnfaceImage(int index);

		unsigned short* getBufferOfPreviewImage(int index);
		unsigned short* getBufferOfMeasureImage(int index);
		unsigned short* getBufferOfEnfaceImage(int index);

		void setJoystickEventCallback(JoystickEventCallback* callback);
		void setEyeSideEventCallback(EyeSideEventCallback* callback);
		void setOptimizeKeyEventCallback(OptimizeKeyEventCallback* callback);
		void setBacklightOnEventCallback(BacklightOnEventCallback* callback);

		void setGrabPreviewBufferCallback(GrabPreviewBufferCallback* callback);
		void setGrabMeasureBufferCallback(GrabMeasureBufferCallback* callback);
		void setGrabEnfaceBufferCallback(GrabEnfaceBufferCallback* callback);

	protected:
		void startEventWorkerThread(void);
		void closeEventWorkerThread(void);
		void eventWorkerThreadFunction(void);

		bool updateCurrentEyeSide(void);
		bool updateDeviceStatus(void);

	private:
		struct FrameGrabberImpl;
		std::unique_ptr<FrameGrabberImpl> d_ptr;
		FrameGrabberImpl& getImpl(void) const;

	};
}
