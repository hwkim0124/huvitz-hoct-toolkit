#pragma once

#include "RetSegmDef.h"
#include "CvImage.h"

#include <string>
#include <memory>
#include <vector>


namespace CppUtil {
	class CvImage;
}


namespace RetSegm
{
	class RETSEGM_DLL_API SegmImage : public CppUtil::CvImage
	{
	public:
		SegmImage();
		virtual ~SegmImage();

		SegmImage(SegmImage&& rhs) ;
		SegmImage& operator=(SegmImage&& rhs);
		SegmImage(const SegmImage& rhs);
		SegmImage& operator=(const SegmImage& rhs);

	private:
		struct SegmImageImpl;
		std::unique_ptr<SegmImageImpl> d_ptr;

	public:
		void setSampleRatio(float wRatio, float hRatio);
		float getSampleRatioWidth(void) const;
		float getSampleRatioHeight(void) const;

		float getWidthRatioToSample(void) const;
		float getHeightRatioToSample(void) const;

		float getMean(void) const;
		float getStddev(void) const;
		void setMeanStddev(float mean, float stddev);
		const std::vector<int>& getColumMaxLocs(void) const;
		const std::vector<int>& getColumMaxVals(void) const;
		const std::vector<int>& getColumMeans(void) const;

		const std::vector<float>& getColumnSnRatios(void) const;

		void setColumnMax(const std::vector<int>& maxLocs, const std::vector<int>& maxVals);
		void setColumnMean(const std::vector<int>& means);
		void setColumnStddev(const std::vector<float>& stddev);

		void updateImageStats(void);
		void updateColumnStats(void);

		void computeColumnSnRatios(void);
	};
}

