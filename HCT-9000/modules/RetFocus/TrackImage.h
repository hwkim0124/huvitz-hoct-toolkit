#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>


namespace CppUtil {
	class CvImage;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API TrackImage
	{
	public:
		TrackImage();
		virtual ~TrackImage();

		TrackImage(TrackImage&& rhs);
		TrackImage& operator=(TrackImage&& rhs);

	public:
		bool decideTargetRegion(void);
		bool measureSimilarity(void);
		bool checkIfEyelidsCovered(void) const;

		bool isValidTarget(void) const;
		bool isEmpty(void) const;
		bool isMatched(void) const;

		float getTargetStdev(void) const;
		int getTargetMean(void) const;
		int getTargetCx(void) const;
		int getTargetCy(void) const;
		int getTargetWidth(void) const;
		int getTargetHeight(void) const;

		int getMatchedCx(void) const;
		int getMatchedCy(void) const;
		float getMatchedValue(void) const;

		void setSource(CppUtil::CvImage& image);
		void setTarget(CppUtil::CvImage& image);
		CppUtil::CvImage& getSource(void) const;
		CppUtil::CvImage& getTarget(void) const;

	private:
		struct TrackImageImpl;
		std::unique_ptr<TrackImageImpl> d_ptr;
		TrackImageImpl& getImpl(void) const;
	};
}

