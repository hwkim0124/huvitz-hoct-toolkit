#pragma once

#include "SigChainDef.h"


#include <memory>
#include <vector>


namespace SigChain
{
	class SIGCHAIN_DLL_API FrameData
	{
	public:
		FrameData();
		FrameData(int width, int height, bool alloc);
		virtual ~FrameData();

		FrameData(FrameData&& rhs);
		FrameData& operator=(FrameData&& rhs);

		// Prevent copy construction and assignment. 
		FrameData(const FrameData& rhs) = delete;
		FrameData& operator=(const FrameData& rhs) = delete;

		enum {
			EMPTY = 0, RECEIVING, COMPLETED, PROCESSING, PROCESSED
		};

		enum {
			PREVIEW = 0, ENFACE, MEASURE
		};

	public:
		unsigned short* getBuffer(void) const;
		unsigned short* getDataEnd(void) const;
		float* getSample(void) const;

		void setBuffer(unsigned short* buff);

		int getFrameWidth(void) const;
		int getFrameHeight(void) const;
		int getFrameSize(void) const;

		int getDataWidth(void) const;
		int getDataHeight(void) const;
		int getDataSize(void) const;

		int getDataCount(void) const;
		int getLineCount(void) const;

		void setEmpty(void);
		void setStatus(int status);
		void setIndexOfImage(int index);
		int getIndexOfImage(void);
		void setNumberOfLines(int lines);
		int getNumberOfLines(void);

		void dumpToFile(int repeats);

		OctRoute getRouteOfScan(void) const;
		void setRouteOfScan(OctRoute route);

		bool isReceivedFull(void);
		bool isEmpty(void) const;
		bool isProcessed(void) const;
		bool isCompleted(void) const;
		bool isReceiving(void) const;
		bool isProcessing(void) const;

		void setType(int type);
		bool isPreview(void) const;
		bool isEnface(void) const;
		bool isMeasure(void) const;

		void setFrameLayout(int lineSize, int frameLines, bool alloc);
		void setDataLayout(int width, int height);

		int addLineData(unsigned short* data, int numLines, bool frameStart = false);

	private:
		struct FrameDataImpl;
		std::unique_ptr<FrameDataImpl> d_ptr;
		FrameDataImpl& getImpl(void) const;
	};

	typedef std::vector<FrameData> FrameDataVect;
}

