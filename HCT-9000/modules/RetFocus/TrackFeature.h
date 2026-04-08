#pragma once

#include "RetFocusDef.h"

#include <array>
#include <numeric>


namespace CppUtil {
	class CvImage;
}


namespace RetFocus
{
	class RETFOCUS_DLL_API TrackFeature
	{
	public:
		TrackFeature();
		virtual ~TrackFeature();

		TrackFeature(TrackFeature&& rhs);
		TrackFeature& operator=(TrackFeature&& rhs);

	public:
		bool setupFrameImage(bool isOD, bool isDisc, const CppUtil::CvImage& image);
		bool evaluateValidity(void) const;
		bool calculateFeature(void) const;
		bool calculateEntropy(void) const;
		bool calculateSimilarity(const CppUtil::CvImage& image) const;
		bool calculateSimilarityOnFrame(const CppUtil::CvImage& image) const;
		bool estimateMovement(const CppUtil::CvImage& image);
		bool estimateMovementOnFrame(const CppUtil::CvImage& image) const;

		float getMovedX(void) const;
		float getMovedY(void) const;
		float getFeatureMean(void) const;
		float getFeatureStdev(void) const;
		float getSimilarity(void) const;
		float getEntropy(void) const;
		float getCoefficientOfVariation(void) const;

		bool isEmpty(void) const;
		bool isFeatureCovered(void) const;
		bool isValidFeature(void) const;
		void getFeatureRect(int& sx, int& sy, int& width, int& height) const;
		
		bool checkUpperRegionIsCovered(void) const;
		bool checkLowerRegionIsCovered(void) const;
		
	protected:

	private:
		struct TrackFeatureImpl;
		std::unique_ptr<TrackFeatureImpl> d_ptr;
		TrackFeatureImpl& impl(void) const;
	};
}