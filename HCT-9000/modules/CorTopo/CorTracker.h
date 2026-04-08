#pragma once


#include "CorTopoDef.h"

#include <string>
#include <memory>
#include <vector>



namespace CppUtil {
	class CvImage;
}


namespace RetSegm {
	class SegmImage;
}


namespace CorTopo
{
	class CORTOPO_DLL_API CorTracker
	{
	public:
		CorTracker();
		virtual ~CorTracker();

		CorTracker(CorTracker&& rhs);
		CorTracker& operator=(CorTracker&& rhs);
		CorTracker(const CorTracker& rhs);
		CorTracker& operator=(const CorTracker& rhs);

	public:
		PreviewCorneaCallback* getCallbackFunction(void);
		void callbackPreviewImage(const unsigned char* data, unsigned int width, unsigned int height, 
			float quality = 0.0f, float sigRatio = 0.0f, unsigned int refPoint = 0, unsigned int idxOfImage = 0);

		void initialize(OctScanPattern pattern);
		bool isResult(void) const;
		bool isImagesLoaded(void) const;
		bool process(void);

	protected:
		void setScanPreviewPattern(OctScanPattern pattern);
		bool loadImage(const unsigned char* bits, int width, int height);
		bool prepareSamples(void);
		bool segmentAnteriorBorderLine(float rangeX, int& centerX, int& centerY, std::vector<int>& antes);
		bool upscaleBorderLine(float rangeX, std::vector<int> antes);

		RetSegm::SegmImage* imageCornea(void) const;
		RetSegm::SegmImage* imageSample(void) const;
		RetSegm::SegmImage* imageAscent(void) const;

	private:
		struct CorTrackerImpl;
		std::unique_ptr<CorTrackerImpl> d_ptr;
		CorTrackerImpl& getImpl(void) const;
	};
}

