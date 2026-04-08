#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>


namespace CppUtil {
	class CvImage;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API TrackImage2
	{
	public:
		TrackImage2();
		virtual ~TrackImage2();

		TrackImage2(TrackImage2&& rhs);
		TrackImage2& operator=(TrackImage2&& rhs);

	public:
		bool appointTargetOnFrame(EyeSide side, bool isDisc);
		bool measureSimilarity(void);
		void initializeTrackImage(void);

		void setFrame(CppUtil::CvImage& image);
		void setTarget(CppUtil::CvImage& image);
		CppUtil::CvImage& getFrame(void) const;
		CppUtil::CvImage& getTarget(void) const;

		bool isTarget(void);
		bool getTargetInfo(float& cx, float& cy, float& w, float& h, float& val, int idx=0);
		float targetVal(int idx=0) const;
		float targetCx(int idx=0) const;
		float targetCy(int idx=0) const;
		float targetWidth(void) const;
		float targetHeight(void) const;

		bool isMatched(void);
		bool getMatchedInfo(float& cx, float& cy, float& w, float& h, float& val);
		float matchedCx(void) const;
		float matchedCy(void) const;
		float matchedVal(void) const;

	protected:

	private:
		struct TrackImage2Impl;
		std::unique_ptr<TrackImage2Impl> d_ptr;
		TrackImage2Impl& getImpl(void) const;
	};
}

