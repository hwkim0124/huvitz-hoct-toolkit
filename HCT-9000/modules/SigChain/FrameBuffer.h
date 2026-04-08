#pragma once

#include "SigChainDef.h"

#include <memory>


namespace SigChain
{
	class FrameData;

	class SIGCHAIN_DLL_API FrameBuffer
	{
	public:
		FrameBuffer();
		virtual ~FrameBuffer();

		FrameBuffer(FrameBuffer&& rhs);
		FrameBuffer& operator=(FrameBuffer&& rhs);

		// Prevent copy construction and assignment. 
		FrameBuffer(const FrameBuffer& rhs) = delete;
		FrameBuffer& operator=(const FrameBuffer& rhs) = delete;

		enum {
			PREVIEW = 0, PHASE, PATTERN
		};

	public:
		bool initializeFrameBuffer(void);
		bool isInitialized(void) const;
		void releaseFrameBuffer(void);

		bool startFrameBuffer(bool enface);
		void closeFrameBuffer(void);
		bool isBusying(void);

		bool insertFrameDataForPreview(std::uint16_t* data, int numLines, int idxOfImage);
		bool insertFrameDataForMeasure(std::uint16_t* data, int numLines, int idxOfImage);
		bool insertFrameDataForEnface(std::uint16_t* data, int numLines, int idxOfImage);

		bool setDataLayoutForPreview(int width, int height, int depth);
		bool setDataLayoutForMeasure(int width, int height, int depth);
		bool setDataLayoutForEnface(int width, int height, int depth);

		void notifyFrameDataCompleted(bool wait = true);

	protected:
		void startImageBuffer(bool reset);
		void closeImageBuffer(void);
		bool prepareTransform(bool enface);

		void startRunning(void);
		void stopRunning(void);
		bool isStopped(void) const;

		void processFrameBuffer(void);
		void processFrameDataForPreview(FrameData* frame);
		void processFrameDataForMeasure(FrameData* frame);
		void processFrameDataForEnface(FrameData* frame);

		void resetFrameBufferListToPreview(void);
		void resetFrameBufferListToMeasure(void);
		void resetFrameBufferListToEnface(void);
		void clearFrameBufferListToPreview(void);
		void clearFrameBufferListToMeasure(void);
		void clearFrameBufferListToEnface(void);

		int getFrameBufferWidth(bool preview) const;
		int getFrameBufferHeight(bool preview) const;
		int getFrameBufferDepth(bool preview) const;

		FrameData* getFrameDataToReceivePreview(void) const;
		FrameData* getFrameDataToReceiveMeasure(void) const;
		FrameData* getFrameDataToReceiveEnface(void) const;
		FrameData* getFrameDataToProcessPreview(void) const;
		FrameData* getFrameDataToProcessMeasure(void) const;
		FrameData* getFrameDataToProcessEnface(void) const;

	private:
		struct FrameBufferImpl;
		std::unique_ptr<FrameBufferImpl> d_ptr;
		FrameBufferImpl& getImpl(void) const;
	};
}
