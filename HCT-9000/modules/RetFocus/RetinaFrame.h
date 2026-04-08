#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>

namespace CppUtil {
	class CvImage;
}


namespace RetFocus
{
	using SplitConvsArray = std::array<float, SPLIT_CONVS_RANGE>;
	using SplitConvsArrayConstIter = SplitConvsArray::const_iterator;

	class RETFOCUS_DLL_API RetinaFrame
	{
	public:
		RetinaFrame();
		RetinaFrame(const std::wstring& path);
		virtual ~RetinaFrame();

		RetinaFrame(RetinaFrame&& rhs);
		RetinaFrame& operator=(RetinaFrame&& rhs);

	public:
		bool loadFile(const std::wstring& path);
		bool loadData(const unsigned char* pBits, int width, int height, int padding = 0);
		bool isEmpty(void) const;

		bool isOpticDiscFound(void) const;
		bool getOpticDiscCenter(float& cx, float& cy);
		bool getOpticDiscRegion(int& x1, int& y1, int &x2, int& y2);

		bool isSplitFocus(void) const;
		bool isUpperSplit(void) const;
		bool isLowerSplit(void) const;

		float getSplitOffset(void) const;
		float getSplitUpperX(void) const;
		float getSplitUpperY(void) const;
		float getSplitLowerX(void) const;
		float getSplitLowerY(void) const;

		float getSplitUpperFwhm(void) const;
		float getSplitLowerFwhm(void) const;
		float getSplitUpperSNR(void) const;
		float getSplitLowerSNR(void) const;
		float getSplitUpperPeak(void) const;
		float getSplitLowerPeak(void) const;

		bool getSplitWindow(int index, int& x1, int& y1, int& x2, int& y2);
		bool getWorkingDot(int index, float& x, float& y, int& size, int& mean);

		CppUtil::CvImage& getImage(void) const;
		CppUtil::CvImage& getSample(void) const;

	private:
		struct RetinaFrameImpl;
		std::unique_ptr<RetinaFrameImpl> d_ptr;
		RetinaFrameImpl& getImpl(void) const;

		void initialize(void);
		bool performAnalysis(void);

		bool createSampleImage(void);
		bool locateWorkingDots(void);

		bool transformFeatureImage(void);
		bool locateOpticDiscFeature(void);
		bool locateOpticDiscRegion(void);

		bool makeUpperConvolutions(void);
		bool makeLowerConvolutions(void);
		bool findPeakInData(const SplitConvsArray& data, int* peakPos, float* peakVal, float* peakSize, float* snRatio);
		bool checkIfValidPeak(float peak, float fwhm, float snRatio);
		bool findUpperSplitPosition(int pidx);
		bool findLowerSplitPosition(int pidx);

		bool assignUpperSplit(void);
		bool assignLowerSplit(void);
		bool decideSplitOffset(void);
	};
}
