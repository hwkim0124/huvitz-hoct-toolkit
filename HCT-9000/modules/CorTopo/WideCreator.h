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
	class CORTOPO_DLL_API WideCreator
	{
	public:
		WideCreator();
		virtual ~WideCreator();

		WideCreator(WideCreator&& rhs);
		WideCreator& operator=(WideCreator&& rhs);
		WideCreator(const WideCreator& rhs);
		WideCreator& operator=(const WideCreator& rhs);

	public:
		bool loadImages(const std::wstring& corneaPath = _T(""), const std::wstring& chamberPath = _T(""));
		bool loadImages(const unsigned char* corneaBits, const unsigned char* chamberBits, int width, int height);
		bool saveWideAnteriorImage(const std::wstring& dirPath = _T("./"), const std::wstring& fileName = _T("anterior.jpg"));
		bool isResult(void) const;
 		bool isImagesLoaded(void) const;
		std::wstring corneaImageName(void) const;
		std::wstring chamberImageName(void) const;

		int getStitchingOffsetX(void) const;
		int getStitchingOffsetY(void) const;

		bool process(void);
		bool makeupAnteriorCorneaBorder(void);
		bool makeupAnteriorChamberBorder(void);
		bool composeWideAnteriorImage(void);

		std::vector<int>& corneaAnteriorLine(void) const;
		std::vector<int>& corneaPosteriorLine(void) const;
		std::vector<int>& corneaAnteriorSides(void) const;
		std::vector<int>& chamberAnteriorLine(void) const;
		std::vector<int>& chamberPosteriorLine(void) const;
		std::vector<int>& chamberHingeLine(int index) const;

		RetSegm::SegmImage* imageAnterior(void) const;
		RetSegm::SegmImage* imageCornea(void) const;
		RetSegm::SegmImage* imageChamber(void) const;
		RetSegm::SegmImage* sampleCornea(void) const;
		RetSegm::SegmImage* sampleChamber(void) const;
		RetSegm::SegmImage* ascentCornea(void) const;
		RetSegm::SegmImage* descentCornea(void) const;
		RetSegm::SegmImage* ascentChamber(void) const;
		RetSegm::SegmImage* descentChamber(void) const;

	protected:
		bool prepareSamples(void);
		void upscaleBorderLines(void);
		bool segmentCorneaBorderLines(float rangeX, int& centerX, int& centerY, std::vector<int>& antes, std::vector<int>& posts, std::vector<int>& sides);
		bool segmentChamberBorderLines(float rangeX, int& centerX, int& centerY, std::vector<int>& antes, std::vector<int>& posts);

	private:
		struct WideCreatorImpl;
		std::unique_ptr<WideCreatorImpl> d_ptr;
		WideCreatorImpl& getImpl(void) const;
	};
}

