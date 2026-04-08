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
	class CPPUTIL_DLL_API Retinex
	{
	public:
		Retinex();
		~Retinex();

		static cv::Mat MSRCP_Lab_Fundus(const cv::Mat image, const cv::Mat lumin, std::vector<double> sigmas);


	public:
		static cv::Mat multiScaleRetinex(const cv::Mat image, std::vector<double> sigmas);
		static cv::Mat singleScaleRetiex(const cv::Mat image, double sigma);
		static cv::Mat boxFilter(const cv::Mat image, double sigma);

	private:
		struct RetinexImpl;
		static std::unique_ptr<RetinexImpl> d_ptr;
		static RetinexImpl& getImpl(void);
	};
}

