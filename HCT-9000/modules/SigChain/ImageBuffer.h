#pragma once

#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class FrameImage;

	class SIGCHAIN_DLL_API ImageBuffer
	{
	public:
		ImageBuffer();
		virtual ~ImageBuffer();

		ImageBuffer(ImageBuffer&& rhs);
		ImageBuffer& operator=(ImageBuffer&& rhs);

		// Prevent copy construction and assignment. 
		ImageBuffer(const ImageBuffer& rhs) = delete;
		ImageBuffer& operator=(const ImageBuffer& rhs) = delete;

	public:
		FrameImage* getFrameImageToPreviewOutput(void);
		FrameImage* getFrameImageToMeasureOutput(void);
		FrameImage* getFrameImageToEnfaceOutput(void);

		void notifyPreviewImageOutput(void);
		void notifyMeasureImageOutput(void);
		void notifyEnfaceImageOutput(void);

		void setEnfaceResultLayout(int width, int height);
		void setMeasureResultLayout(int width, int height);

		void startCallbackThread(bool reset);
		void closeCallbackThread(void);
		bool isBusying(void);

	protected:
		void initialize(void);
		void clearImageBuffers(void);
		void clearPreviewImages(void);
		void clearMeasureImages(void);
		void clearEnfaceImages(void);

		FrameImage* getFrameImageToPreviewResult(void);
		FrameImage* getFrameImageToMeasureResult(void);
		FrameImage* getFrameImageToEnfaceResult(void);

		void processImageBuffer(void);
		void processPreviewImage(FrameImage* image);
		void processMeasureImage(FrameImage* image);
		void processEnfaceImage(FrameImage* image);

		void startRunning(void);
		void stopRunning(void);
		bool isStopped(void) const;

	private:
		struct ImageBufferImpl;
		std::unique_ptr<ImageBufferImpl> d_ptr;
		ImageBufferImpl& getImpl(void) const;
	};

}