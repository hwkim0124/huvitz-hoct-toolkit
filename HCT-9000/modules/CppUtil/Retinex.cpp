#include "stdafx.h"
#include "Retinex.h"
#include "GuidedFilter.h"

#include <array>
#include <thread>
#include <vector>
#include <algorithm>

using namespace CppUtil;
using namespace cv;
using namespace std;


struct Retinex::RetinexImpl
{

};



// Direct initialization of static smart pointer.
std::unique_ptr<Retinex::RetinexImpl> Retinex::d_ptr(new RetinexImpl());


Retinex::Retinex()
{
}


Retinex::~Retinex()
{
}


cv::Mat CppUtil::Retinex::MSRCP_Lab_Fundus(const cv::Mat image, const cv::Mat lumin, std::vector<double> sigmas)
{
	cv::Mat matRGB;
	cv::Mat matLum;
	cv::Mat matRet;
	cv::Mat matOut;

	image.convertTo(matRGB, CV_64FC3);
	lumin.convertTo(matLum, CV_64FC1);
	lumin += 1.0;
	
	matRet = multiScaleRetinex(matLum, sigmas);

	double minV, maxV;
	cv::minMaxLoc(matRet, &minV, &maxV);
	matRet = ((matRet - minV) / (maxV - minV)) * 255.0 + 1.0;

	double valI, valR;
	double resC, resR, resG, resB;
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			valI = matLum.at<double>(y, x);
			valR = matRet.at<double>(y, x);
			resC = min(256.0/valI, valR / valI);
			resC = sqrt(resC);

			// resC = max(resC, 1.0) - 1.0;
			// resR = 1.0 + resC;
			// resG = 1.0 + resC;
			// resB = 1.0 + resC;

			resC = max(resC, 1.0);
			resR = resG = resB = resC;
			
			auto &color = matRGB.at<cv::Vec3d>(y, x);
			color[0] = color[0] * resR; //  max(min(color[0] * resR - 1.0, 255.0), 0.0);
			color[1] = color[1] * resG; // max(min(color[1] * resG - 1.0, 255.0), 0.0);
			color[2] = color[2] * resB; // max(min(color[2] * resB - 1.0, 255.0), 0.0);
		}
	}
	matRGB.convertTo(matOut, CV_8UC3);
	return matOut;
}


cv::Mat CppUtil::Retinex::multiScaleRetinex(const cv::Mat image, std::vector<double> sigmas)
{
	auto retinex = cv::Mat::zeros(image.rows, image.cols, CV_64FC1);
	auto count = sigmas.size();

	auto n_workers = std::thread::hardware_concurrency();
	bool threading = (n_workers >= count ? true : false);

	if (threading) {
		std::vector<std::thread> workers;
		std::vector<cv::Mat> tasks(count);

		for (unsigned int k = 0; k < count; k++) {
			workers.push_back(std::thread([&tasks, image, sigmas, k]() {
				tasks[k] = Retinex::singleScaleRetiex(image, sigmas[k]);
			}));
		}
		std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
			t.join();
		});

		for (unsigned int k = 0; k < count; k++) {
			retinex += tasks[k];
		}
	}
	else {
		for (double s : sigmas) {
			retinex += singleScaleRetiex(image, s);
		}
	}

	retinex /= (double)count;
	return retinex;
}


cv::Mat CppUtil::Retinex::singleScaleRetiex(const cv::Mat image, double sigma)
{
	auto blurr = boxFilter(image, sigma);

	cv::Mat logImage, logBlurr;
	cv::log((image+0.1), logImage);
	cv::log((blurr+0.1), logBlurr);
	logImage = logImage / log(10.0);
	logBlurr = logBlurr / log(10.0);

	auto retinex = logImage - logBlurr;
	return retinex;
}


cv::Mat CppUtil::Retinex::boxFilter(const cv::Mat image, double sigma)
{
	double eps = 0.1 * 0.1;
	int r = (int)sigma;

	auto outMat = guidedFilter(image, r, eps);

	return outMat;
}


Retinex::RetinexImpl & CppUtil::Retinex::getImpl(void)
{
	return *d_ptr;
}

