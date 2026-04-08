#pragma once

#include "CppUtilDef.h"

#include <string>
#include <memory>
#include <vector>


namespace cv {
	class Mat;
}


namespace CppUtil
{
	class GuidedFilterImpl;

	class GuidedFilter
	{
	public:
		GuidedFilter(const cv::Mat &I, int r, double eps);
		~GuidedFilter();

		cv::Mat filter(const cv::Mat &p, int depth = -1) const;

	private:
		CppUtil::GuidedFilterImpl *impl_;
	};


	cv::Mat CPPUTIL_DLL_API guidedFilter(const cv::Mat &I, const cv::Mat &p, int r, double eps, int depth = -1);
	cv::Mat CPPUTIL_DLL_API guidedFilter(const cv::Mat &I, int r, double eps, int depth = -1);
}

