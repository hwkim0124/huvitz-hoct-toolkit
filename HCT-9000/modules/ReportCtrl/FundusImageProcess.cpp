#include "stdafx.h"
#include "FundusImageProcess.h"
#include "CppUtil2.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <array>
#include <thread>
#include <vector>
#include <algorithm>

using namespace ReportCtrl;
using namespace std;


FundusImageProcess::FundusImageProcess()
{
}


FundusImageProcess::~FundusImageProcess()
{
}


void FundusImageProcess::edgeEnhance(cv::Mat& mat, unsigned int ksize)
{
	if (ksize == 0 || ksize > 100) {
		return;
	}

	if (ksize % 2 == 0) {
		ksize += 1;
	}

	double coeff = 1.0;
	cv::Mat msmooth;
	cv::GaussianBlur(mat, msmooth, cv::Size(ksize, ksize), 0);
	cv::addWeighted(mat, 1.0 + coeff, msmooth, -coeff, 0.0, mat);

	return;
}

void FundusImageProcess::translateColor(cv::Mat& mat, ReportCtrl::FundusColor color, EmbossType emboss)
{
	cv::Mat channels[3] =
	{
		cv::Mat(mat.rows, mat.cols, CV_8UC1)
	};

	switch (color)
	{
	case FundusColor::color:
		break;
	case FundusColor::red:
		cv::split(mat, channels);
		mat = channels[2];
		break;
	case FundusColor::green:
		cv::split(mat, channels);
		mat = channels[1];
		break;
	case FundusColor::blue:
		cv::split(mat, channels);
		mat = channels[0];
		break;
	case FundusColor::redFree:
		cv::split(mat, channels);
		mat = (channels[0] * 0.5f + channels[1] * 0.5f);
		break;
	default:
		break;
	}

	if (emboss != EMBOSS_TYPE_NONE) {
		// redfree first
		if (color == FundusColor::color) {
			cv::split(mat, channels);
			mat = (channels[0] * 0.5f + channels[1] * 0.5f);
		}
		// apply embossing
		applyEmbossing(mat, emboss);
	}

	return;
}

void FundusImageProcess::masking(cv::Mat &mat, unsigned int radius)
{
	int sx = mat.rows;
	int sy = mat.cols;

	// masking
	cv::Mat mask = cv::Mat::ones(sx, sy, CV_8UC1);
	cv::Scalar colorMask(0, 0, 0);
	cv::circle(mask, cv::Point(sx / 2, sy / 2), radius, colorMask, -1);
	mat.setTo(0, mask);

	// direction mark
	int mark_x = int(sx / 64.0f * 56);
	int mark_y = int(sy / 64.0f * 8); 
	int mark_radius = int(sx / 64.0f);
	cv::Scalar colorMark(120, 153, 238);
	cv::circle(mat, cv::Point(mark_x, mark_y), mark_radius, colorMark, -1);

	return;
}

bool FundusImageProcess::correctGamma(cv::Mat& mat, double gamma)
{
	if (gamma < 0.0) {
		return false;
	}

	cv::Mat gammaTable(1, 256, CV_8UC1);

	double invGamma = 1.0 / gamma;
	uchar * ptr = gammaTable.ptr();
	for (int i = 0; i < 256; i++) {
		ptr[i] = (int)(pow((double)i / 255.0, invGamma) * 255.0);
	}

	if (mat.empty()) {
		return false;
	}

	LUT(mat, gammaTable, mat);
	return true;
}


cv::Mat FundusImageProcess::enhance_contrast(cv::Mat& image, float clip = 4.0, int tile = 8)
{
	auto clahe = cv::createCLAHE(clip, cv::Size(tile, tile));
	clahe->apply(image, image);
	return image;
}

cv::Mat FundusImageProcess::cicle_mask(cv::Mat& mat, int rad_out, int rad_inn)
{
	cv::Mat mask = cv::Mat::ones(mat.rows, mat.cols, CV_8UC1);
	cv::Point center(mat.rows/2, mat.cols/2);
	cv::circle(mask, center, rad_out, (1,1,1), -1);

	if (rad_inn > 0)
	{
		cv::circle(mask, center, rad_inn, (0, 0, 0), -1);
	}

	return mask;
}

void FundusImageProcess::shading_correction(cv::Mat& mat)
{	
	cv::Mat blurImg;
	cv::Mat splitBGR[3];
	cv::Mat blurBGR[3];
	cv::Mat mergeImg;
	cv::Size ksize(99, 99);
	cv::MatND hist;

	int histSize = 256;
	double histSum = 0.0;
	float range[] = { 0, 255 };
	const float *ranges[] = { range };
	int img_rad = mat.cols / 2;
	double thresholdBGR[3];

	double threshold[3];
	double averageHisto;
	double C = 0.8; // Constant value 
	int darknessHisto;

	cv::blur(mat, blurImg, ksize);
	cv::split(mat, splitBGR);
	cv::split(blurImg, blurBGR);
	cv::Mat mask2 = cicle_mask(splitBGR[2], img_rad);

	for (int i = 0; i < 3; i++)
	{
		cv::calcHist(&splitBGR[i], 1, 0, mask2, hist, 1, &histSize, ranges, true, false);
		averageHisto = cv::mean(splitBGR[i], mask2)[0];
		double histSum = 0.0;
		for (int i = 0; i < 255; i++)
		{
			histSum += hist.at<float>(i);
			if (histSum > mat.rows * mat.cols * 0.40)
			{
				darknessHisto = i;
				break;
			}
		}

		if (darknessHisto != 0)
		{
			threshold[i] = averageHisto  * (averageHisto / darknessHisto) * C;
		}
		else
		{
			threshold[i] = averageHisto;
		}
	}

	thresholdBGR[2] = threshold[2];
	thresholdBGR[1] = threshold[1];
	thresholdBGR[0] = threshold[0];

	cv::Mat dataBGR[3];
	cv::Mat diffBGR[3];

	for (int i = 0; i < 3; i++)
	{
		blurBGR[i].copyTo(dataBGR[i]);
		cv::multiply(dataBGR[i], mask2, dataBGR[i]);
		cv::subtract(thresholdBGR[i], dataBGR[i], diffBGR[i], mask2);
		cv::blur(diffBGR[i], diffBGR[i], ksize);
	}

	cv::Mat correctBGR[3];
	correctBGR[2] = diffBGR[2] + splitBGR[2];
	correctBGR[1] = diffBGR[1] + splitBGR[1];
	correctBGR[0] = diffBGR[0] + splitBGR[0];
	
	cv::Mat merge[] = { correctBGR[0], correctBGR[1], correctBGR[2] };
	cv::merge(merge, 3, mergeImg);

	mergeImg.copyTo(mat);
}

void FundusImageProcess::postImageProcess(cv::Mat &mat, int m_fundusImageType)
{
	cv::Mat BGR[3];
	cv::Mat stretchBGR[3];
	cv::Mat mergeImg;
	cv::split(mat, BGR);

	correctGamma(BGR[2], 3.5f);
	correctGamma(BGR[1], 1.97f);
	correctGamma(BGR[0], 1.33f);

	switch(m_fundusImageType)
	{
	case 1:
		stretchBGR[2] = enhance_contrast(BGR[2], 1.2f, 30);
		stretchBGR[1] = enhance_contrast(BGR[1], 2.5f, 30);
		stretchBGR[0] = enhance_contrast(BGR[0], 2.0f, 30);
		break;

	case 2:
		stretchBGR[2] = enhance_contrast(BGR[2], 1.2f, 30);
		stretchBGR[1] = enhance_contrast(BGR[1], 2.5f, 30);
		stretchBGR[0] = enhance_contrast(BGR[0], 2.0f, 30);
		break;

	case 3:
		stretchBGR[2] = enhance_contrast(BGR[2], 1.2f, 8);
		stretchBGR[1] = enhance_contrast(BGR[1], 2.5f, 18);
		stretchBGR[0] = enhance_contrast(BGR[0], 2.0f, 8);
		break;

	// modify 2025.05.19 - hwajunlee
	// Fundus image Enhancement option Level 4 
	// White balance -> Central Brightness -> Domain Transform
	case 4:
		//if (GlobalSettings::useFundusFILR_Enable()) {
		//	stretchBGR[2] = enhance_contrast(BGR[2], 1.2f, 8);
		//	stretchBGR[1] = enhance_contrast(BGR[1], 2.5f, 18);
		//	stretchBGR[0] = enhance_contrast(BGR[0], 2.0f, 8);
		//}
		break;
	}
	
	cv::Mat merge[] = { stretchBGR[0], stretchBGR[1], stretchBGR[2] };
	cv::merge(merge, 3, mergeImg);

	mergeImg.copyTo(mat);

	return;
}


void FundusImageProcess::postImageProcess2(cv::Mat & image, int level)
{
	cv::Mat BGR[3];
	cv::Mat Lab[3];
	cv::Mat stretchBGR[3];
	cv::Mat outImage;

	cv::split(image, BGR);
	cv::Scalar meanBGR[3];
	cv::Scalar stdeBGR[3];

	const double indexRR = 1.0;
	const double indexGR = 0.35;
	const double indexBG = 0.55;

	auto n_workers = std::thread::hardware_concurrency();
	bool threading = (n_workers >= 3 ? true : false);

	if (level == 1) {
		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], 1.4f, 8);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], 1.4f, 8);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 ;
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[0], stretchBGR[1], stretchBGR[2] };
		cv::merge(merge, 3, outImage);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}
	else if (level == 2) {
		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], 2.0f, 12);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], 2.0f, 12);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 / rateBGR[2];
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[0], stretchBGR[1], stretchBGR[2] };
		cv::merge(merge, 3, outImage);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}
	else if (level == 3) {
		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], 1.0f, 8);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], 1.0f, 8);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 / rateBGR[2];
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[2], stretchBGR[1], stretchBGR[0] };
		cv::merge(merge, 3, outImage);

		cv::Mat labImage;
		cv::cvtColor(outImage, labImage, CV_RGB2Lab);
		cv::split(labImage, Lab);

		outImage = CppUtil::Retinex::MSRCP_Lab_Fundus(outImage, Lab[0], { 2.0, 13.0, 35.0 });

		cv::cvtColor(outImage, labImage, CV_RGB2Lab);
		cv::split(labImage, Lab);

		Lab[0] = enhance_contrast(Lab[0], 0.5f, 8);
		cv::merge(Lab, 3, labImage);
		cv::cvtColor(labImage, outImage, CV_Lab2BGR);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}

	return;
}


void FundusImageProcess::postImageProcess2_SonyCamera(cv::Mat & image, int level, bool oldSet)
{
	cv::Mat BGR[3];
	cv::Mat Lab[3];
	cv::Mat stretchBGR[3];
	cv::Mat outImage;

	cv::split(image, BGR);
	cv::Scalar meanBGR[3];
	cv::Scalar stdeBGR[3];

	const double indexRR = 1.0;
	const double indexGR = 0.25;
	const double indexBG = 0.75;

	auto n_workers = std::thread::hardware_concurrency();
	bool threading = (n_workers >= 3 ? true : false);

	if (level == 1) {
		float clip = (oldSet ? 1.0f : 1.4f);
		float tile = (oldSet ? 8 : 8);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, clip, tile, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], clip, tile);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], clip, tile);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 ;
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[0], stretchBGR[1], stretchBGR[2] };
		cv::merge(merge, 3, outImage);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}
	else if (level == 2) {
		float clip = (oldSet ? 1.4f : 2.0f);
		float tile = (oldSet ? 8 : 12);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, clip, tile, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], clip, tile);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], clip, tile);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 / rateBGR[2];
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[0], stretchBGR[1], stretchBGR[2] };
		cv::merge(merge, 3, outImage);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}
	else if (level == 3) {
		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&BGR, &stretchBGR, k, &meanBGR, &stdeBGR, this]() {
					stretchBGR[k] = enhance_contrast(BGR[k], 1.0f, 8);
					cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				stretchBGR[k] = enhance_contrast(BGR[k], 1.0f, 8);
				cv::meanStdDev(stretchBGR[k], meanBGR[k], stdeBGR[k]);
			}
		}

		double rateBGR[3] = { 0.0 };
		rateBGR[2] = (meanBGR[2](0) + stdeBGR[2](0)) / 256.0;
		rateBGR[1] = (meanBGR[1](0) + stdeBGR[1](0)) / (meanBGR[2](0) + stdeBGR[2](0) + 1.0);
		rateBGR[0] = (meanBGR[0](0) + stdeBGR[0](0)) / (meanBGR[1](0) + stdeBGR[1](0) + 1.0);

		double gammaBGR[3] = { 0.0 };
		double illum = 1.0 / rateBGR[2];
		gammaBGR[2] = exp(indexRR - rateBGR[2]) * sqrt(illum);
		gammaBGR[1] = exp(indexGR - rateBGR[1]) * sqrt(gammaBGR[2]);
		gammaBGR[0] = exp(indexBG - rateBGR[0]) * sqrt(gammaBGR[1]);

		if (threading) {
			std::vector<std::thread> workers;
			for (unsigned int k = 0; k < 3; k++) {
				workers.push_back(std::thread([&stretchBGR, k, &gammaBGR, this]() {
					correctGamma(stretchBGR[k], gammaBGR[k]);
				}));
			}
			std::for_each(workers.begin(), workers.end(), [](std::thread& t) {
				t.join();
			});
		}
		else {
			for (int k = 0; k < 3; k++) {
				correctGamma(stretchBGR[k], gammaBGR[k]);
			}
		}

		cv::Mat merge[] = { stretchBGR[2], stretchBGR[1], stretchBGR[0] };
		cv::merge(merge, 3, outImage);

		cv::Mat labImage;
		cv::cvtColor(outImage, labImage, CV_RGB2Lab);
		cv::split(labImage, Lab);

		outImage = CppUtil::Retinex::MSRCP_Lab_Fundus(outImage, Lab[0], { 2.0, 13.0, 35.0 });

		cv::cvtColor(outImage, labImage, CV_RGB2Lab);
		cv::split(labImage, Lab);

		Lab[0] = enhance_contrast(Lab[0], 0.5f, 8);
		cv::merge(Lab, 3, labImage);
		cv::cvtColor(labImage, outImage, CV_Lab2BGR);

		// CppUtil::guidedFilter(outImage, 1, 0.01);
		outImage.copyTo(image);
	}

	return;
}

cv::Mat FundusImageProcess::loadROIData(const std::string& filename, int index) {
	std::ifstream ifs(filename, std::ios::binary);
	if (!ifs.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + filename);
	}

	int imageCount;
	ifs.read((char*)&imageCount, sizeof(int));

	if (index < 0 || index >= imageCount) {
		throw std::out_of_range("Index out of range.");
	}

	std::vector<size_t> offsets(imageCount);
	std::vector<size_t> sizes(imageCount);
	for (int i = 0; i < imageCount; ++i) {
		ifs.read((char*)&offsets[i], sizeof(size_t));
		ifs.read((char*)&sizes[i], sizeof(size_t));
	}

	size_t offset = offsets[index];
	size_t size = sizes[index];

	ifs.seekg(offset, std::ios::beg);
	std::vector<char> buffer(size);
	ifs.read(buffer.data(), size);

	ifs.close();

	return cv::imdecode(buffer, cv::IMREAD_COLOR);
}

void FundusImageProcess::applyRemoveReflectionLight2(cv::Mat &mat, int lightRadius, int shadowRadius, int peripheryRadius, int flashValue, float focusValue)
{
	cv::Mat src, dst, filter;
	cv::Mat splitBGR[3];
	cv::Mat splitFilterBGR[3];

	if (mat.empty()) {
		return;
	}

	if (mat.type() != CV_8UC3 && mat.type() != CV_16UC3) {
		return;
	}

	if (lightRadius == 0 || shadowRadius == 0 || peripheryRadius == 0) {
		return;
	}
	if (lightRadius == shadowRadius ||
		shadowRadius == peripheryRadius ||
		peripheryRadius == lightRadius) {
		CppUtil::Logger::info("Remove Light parameters are same : %d, %d, %d", lightRadius, shadowRadius, peripheryRadius);
		return;
	}

	mat.copyTo(src);
	split(src, splitBGR);

	int indexA, indexB, indexFilter = 0;
	if (flashValue == 10) {
		indexA = 1;
	}
	else if (flashValue == 30) {
		indexA = 2;
	}
	else if (flashValue == 60) {
		indexA = 3;
	}

	//TEST -> only flash : 60
	//if (focusValue <= -13.0f) {
	//	indexFilter = (105 * (indexA - 1));
	//}
	//else {
	//	indexFilter = (105 * (indexA - 1)) + static_cast<int>((focusValue + 13) / 0.25);
	//}

	if (focusValue <= -13.0f) {
		indexFilter = 0;
	}
	else {
		indexFilter = static_cast<int>((focusValue + 13) / 0.25);
	}

	auto loadedROIs = loadROIData(".\\Fundus_Calibration\\roi_data.bin", indexFilter);

	//*********************** Crop filter ***********************//
	int cropSize = src.rows;
	int startX = (loadedROIs.cols - cropSize) / 2;
	int startY = (loadedROIs.rows - cropSize) / 2;

	cv::Rect roi(startX, startY, cropSize, cropSize);
	loadedROIs = loadedROIs(roi);

	//*********************** Remove stain ***********************//
	//cv::Mat splitRoi[3];
	//cv::split(loadedROIs, splitRoi);
	//for (int i = 0; i < 3; i++)
	//{
	//	double sum = 0.0;
	//	int count = 0;
	//	int center_x = src.cols / 2;
	//	int center_y = src.rows / 2;
	//	for (int y = 0; y < splitRoi[i].rows; y++) {
	//		for (int x = 0; x < splitRoi[i].cols; x++) {
	//			int dx = x - center_x;
	//			int dy = y - center_y;

	//			if (dx*dx + dy*dy > shadowRadius*shadowRadius) {
	//				sum += splitRoi[i].at<uchar>(y, x);
	//				count++;
	//			}
	//		}
	//	}
	//	double avg = count > 0 ? sum / count : 0.0;

	//	for (int y = 0; y < splitRoi[i].rows; y++) {
	//		for (int x = 0; x < splitRoi[i].cols; x++) {
	//			int dx = x - center_x;
	//			int dy = y - center_y;

	//			if (dx*dx + dy*dy > shadowRadius*shadowRadius) {
	//				if (splitRoi[i].at<uchar>(y, x) > avg) {
	//					splitRoi[i].at<uchar>(y, x) = avg;
	//				}
	//			}
	//		}
	//	}
	//}

	//merge(splitRoi, 3, filter);

	bool isTest = false;

	if (true) {
		split(filter, splitFilterBGR);
		cv::Point centerPoint(src.cols / 2, src.rows / 2);

		//filter copy
		cv::Mat filterMask;
		filter.copyTo(filterMask);

		//check value
		float shadowHist[3];
		float lightHist[3];
		float filterLightHist[3];

		float lightMeanVec[3];
		float shadowMeanVec[3];
		float filterLightMeanVec[3];

		//calculate differ
		for (int i = 0; i < 3; i++)
		{
			cv::Mat light, shadow, periphery, filterLight;

			splitBGR[i].copyTo(light);
			splitBGR[i].copyTo(shadow);
			splitBGR[i].copyTo(periphery);
			splitFilterBGR[i].copyTo(filterLight);

			cv::Mat lightMask = cv::Mat::ones(light.rows, light.cols, light.type());
			cv::Mat shadowMask = cv::Mat::ones(shadow.rows, shadow.cols, shadow.type());
			cv::Mat peripheryMask = cv::Mat::ones(periphery.rows, periphery.cols, periphery.type());

			std::vector<int> peripheryVector;

			cv::circle(lightMask, centerPoint, lightRadius, 0, -1);
			cv::circle(shadowMask, centerPoint, shadowRadius - 30, 0, -1);
			cv::circle(peripheryMask, centerPoint, peripheryRadius, 0, -1);

			light.setTo(0, lightMask);
			shadow.setTo(0, shadowMask);
			periphery.setTo(0, peripheryMask);
			filterLight.setTo(0, lightMask);

			cv::multiply(lightMask, shadow, shadow);
			cv::multiply(shadowMask, periphery, periphery);

			int cnt = 0;

			int lightMean = 0;
			int shadowMean = 0;
			int shadowMeanLeft = 0;
			int shadowMeanRight = 0;
			int peripheryMean = 0;
			int filterMean = 0;

			//shadowMean
			for (int y = 0; y < shadow.rows; y++)
			{
				for (int x = 0; x < shadow.cols; x++)
					if (x <= shadow.cols / 2)
					{
						if (shadow.at<uchar>(y, x) != 0) {
							shadowMean += shadow.at<uchar>(y, x);
							cnt++;
							shadowMeanLeft += shadow.at<uchar>(y, x);
						}
					}
					else {
						if (shadow.at<uchar>(y, x) != 0) {
							shadowMean += shadow.at<uchar>(y, x);
							cnt++;
							shadowMeanRight += shadow.at<uchar>(y, x);
						}
					}
			}

			if (shadowMeanLeft < shadowMeanRight) {
				shadowMean = (float)shadowMeanLeft * 2;
			}
			else {
				shadowMean = (float)shadowMeanRight * 2;
			}

			if (shadowMean != 0 && cnt != 0)
			{
				shadowMean /= cnt;
			}

			//filterMean
			cnt = 0;
			for (int y = 0; y < filterLight.rows; y++)
			{
				for (int x = 0; x < filterLight.cols; x++)
					if (filterLight.at<uchar>(y, x) != 0) {
						filterMean += filterLight.at<uchar>(y, x);
						cnt++;
					}
			}

			if (filterMean != 0 && cnt != 0)
			{
				filterMean /= cnt;
			}

			//lightMean
			cnt = 0;
			for (int y = 0; y < light.rows; y++)
			{
				for (int x = 0; x < light.cols; x++)
					if (light.at<uchar>(y, x) != 0) {
						lightMean += light.at<uchar>(y, x);
						cnt++;
					}
			}

			if (lightMean != 0 && cnt != 0)
			{
				lightMean /= cnt;
			}

			//// histogram
			//cv::MatND hist;
			//int histSize = 256;
			//double histSum = 0.0;
			//float range[] = { 0, 255 };
			//const float *ranges[] = { range };

			//int mostHisto1 = 0;
			//int mostHisto2 = 0;
			//int mostHisto3 = 0;
			//double oldHistSum = 0.0;

			////shadow hist
			//cv::calcHist(&shadow, 1, 0, shadow, hist, 1, &histSize, ranges, true, false);
			//for (int i = 1; i < 255; i++)
			//{
			//	histSum = hist.at<float>(i);

			//	if (histSum >= oldHistSum) {
			//		oldHistSum = histSum;
			//		mostHisto1 = i;
			//	}
			//}

			////light hist
			//cv::calcHist(&light, 1, 0, light, hist, 1, &histSize, ranges, true, false);
			//oldHistSum = 0;
			//for (int i = 1; i < 255; i++)
			//{
			//	histSum = hist.at<float>(i);

			//	if (histSum >= oldHistSum) {
			//		oldHistSum = histSum;
			//		mostHisto2 = i;
			//	}
			//}

			////filterLight hist
			//cv::calcHist(&filterLight, 1, 0, filterLight, hist, 1, &histSize, ranges, true, false);
			//oldHistSum = 0;
			//for (int i = 1; i < 255; i++)
			//{
			//	histSum = hist.at<float>(i);

			//	if (histSum >= oldHistSum) {
			//		oldHistSum = histSum;
			//		mostHisto3 = i;
			//	}
			//}

			//shadowHist[i] = mostHisto1;
			//lightHist[i] = mostHisto2;
			//filterLightHist[i] = mostHisto3;

			shadowMeanVec[i] = shadowMean;
			lightMeanVec[i] = lightMean;
			filterLightMeanVec[i] = filterMean;
		}

		cv::Mat resultBGR[3];
		cv::Mat result;

		cv::Mat resultBlurBGR[3];
		cv::Mat resultBlur;
		float valueB = 1.0f;
		float valueG = 1.0f;
		float valueR = 1.0f;

		{
			valueB = (lightMeanVec[0] - shadowMeanVec[0]) / filterLightMeanVec[0];
			valueG = (lightMeanVec[1] - shadowMeanVec[1]) / filterLightMeanVec[1];
			valueR = (lightMeanVec[2] - shadowMeanVec[2]) / filterLightMeanVec[2];

			if (valueB <= 0.0f)
			{
				valueB = 1.0f;
			}
			if (valueB >= 2.7f) {
				valueB = 2.7f;
			}
			if (valueG <= 0.0f)
			{
				valueG = 1.0f;
			}
			if (valueG >= 2.7f) {
				valueG = 2.7f;
			}
			if (valueR <= 0.0f)
			{
				valueR = 1.0f;
			}
			if (valueR >= 2.7f) {
				valueR = 2.7f;
			}

			float valueBGR = (valueB + valueG + valueR) / 3.0f;

			result = src - (filter * valueBGR);
			result.copyTo(mat);
		}

		//test txt file
		//if (isTest) {
		//	std::ofstream outFile("output.txt");

		//	vector<float> correct;

		//	for (int i = 0; i < 3; i++) {
		//		correct.push_back(lightMeanVec[i]);
		//		correct.push_back(shadowMeanVec[i]);
		//		correct.push_back(filterLightMeanVec[i]);
		//	}

		//	/*for (int i = 0; i < 3; i++) {
		//		correct.push_back(lightHist[i]);
		//		correct.push_back(shadowHist[i]);
		//		correct.push_back(filterLightHist[i]);
		//	}*/

		//	for (int i = 0; i < 3; i++) {
		//		correct.push_back((lightMeanVec[i] - shadowMeanVec[i]) / filterLightMeanVec[i]);
		//	}

		//	/*for (int i = 0; i < 3; i++) {
		//		correct.push_back((lightHist[i] - shadowHist[i]) / filterLightHist[i]);
		//	}*/

		//	if (outFile.is_open()) {
		//		for (const auto& value : correct) {
		//			outFile << std::fixed << std::setprecision(2) << value << std::endl;
		//		}
		//		outFile.close();
		//	}
		//}
		return;
	}
}

bool FundusImageProcess::isAvailableValue(int flash, float focus)
{
	//if (flash == 10 || flash == 30 || flash == 60) {
	if (flash == 60) {
		return true;
	}
	else {
		return false;
	}
}

void FundusImageProcess::applyRemoveReflectionLight(cv::Mat &mat, int lightRadius, int shadowRadius, int peripheryRadius, int flashValue, float focusValue)
{
	wstring path, name;
	/*
	if ((_waccess(_T(".\\Fundus_Calibration\\roi_data.bin"), 0)) != -1)
	{
		if (isAvailableValue(flashValue, focusValue)) {
			CppUtil::Logger::info("start remove relfection2");
			applyRemoveReflectionLight2(mat, lightRadius, shadowRadius, peripheryRadius, flashValue, focusValue);
			return;
		}
	}
	*/
	//for no reflection test
	if ((_waccess(_T(".\\Fundus_Calibration\\no_reflect.txt"), 0)) != -1)
	{
		return;
	}

	cv::Mat src, dst;
	cv::Mat splitBGR[3];

	if (mat.empty()) {
		return;
	}

	if (mat.type() != CV_8UC3 && mat.type() != CV_16UC3) {
		return;
	}

	if (lightRadius == 0 || shadowRadius == 0 || peripheryRadius == 0) {
		return;
	}
	if (lightRadius == shadowRadius ||
		shadowRadius == peripheryRadius ||
		peripheryRadius == lightRadius) {
		CppUtil::Logger::info("Remove Light parameters are same : %d, %d, %d", lightRadius, shadowRadius, peripheryRadius);
		return;
	}

	mat.copyTo(src);
	split(src, splitBGR);

	for (int i = 0; i < 3; i++)
	{
		// removal reflection light
		cv::Mat light, shadow, periphery;
		splitBGR[i].copyTo(light);
		splitBGR[i].copyTo(shadow);
		splitBGR[i].copyTo(periphery);

		cv::Mat lightMask = cv::Mat::ones(light.rows, light.cols, light.type());
		cv::Mat shadowMask = cv::Mat::ones(shadow.rows, shadow.cols, shadow.type());
		cv::Mat peripheryMask = cv::Mat::ones(periphery.rows, periphery.cols, periphery.type());
		cv::Point centerPoint(src.cols / 2, src.rows / 2);
		std::vector<int> peripheryVector;

		cv::circle(lightMask, centerPoint, lightRadius, 0, -1);
		cv::circle(shadowMask, centerPoint, shadowRadius, 0, -1);
		cv::circle(peripheryMask, centerPoint, peripheryRadius, 0, -1);

		light.setTo(0, lightMask);
		shadow.setTo(0, shadowMask);
		periphery.setTo(0, peripheryMask);
		cv::multiply(lightMask, shadow, shadow);
		cv::multiply(shadowMask, periphery, periphery);

		int cnt = 0;
		int shadowMean = 0;
		int peripheryMean = 0;
		for (int y = 0; y < shadow.rows; y++)
		{
			for (int x = 0; x < shadow.cols; x++)
				if (shadow.at<uchar>(y, x) != 0) {
					shadowMean += shadow.at<uchar>(y, x);
					cnt++;
				}
		}

		if (shadowMean != 0 && cnt != 0)
		{
			shadowMean /= cnt;
		}

		for (int y = 0; y < periphery.rows; y++)
		{
			for (int x = 0; x < periphery.cols; x++)
				if (periphery.at<uchar>(y, x) != 0) {
					peripheryVector.push_back(periphery.at<uchar>(y, x));
				}
		}

		std::vector<int>::iterator start = peripheryVector.begin();
		std::vector<int>::iterator end = peripheryVector.end();

		std::vector<int>::iterator med = start;
		std::advance(med, peripheryVector.size() / 2);

		// This makes the 2nd position hold the median.
		std::nth_element(start, med, end);

		if (peripheryVector.size() > 3) {
			peripheryMean = peripheryVector[2];
		}

		light -= shadowMean;

		cv::blur(light, light, cv::Size(47, 47));
		splitBGR[i] -= light;

		// Correction shadow
		cv::Mat shadowCorrection = cv::Mat::zeros(shadow.rows, shadow.cols, shadow.type());
		auto angle = 0.25;
		for (int j = 0; j < 1440; j++) {
			cv::Point innerPoint;
			cv::Point outterPoint;

			innerPoint.x = (int)round(centerPoint.x + shadowRadius * cos(angle * j * CV_PI / 180.0));
			innerPoint.y = (int)round(centerPoint.y + shadowRadius * sin(angle * j * CV_PI / 180.0));

			outterPoint.x = (int)round(centerPoint.x + peripheryRadius * cos(angle * j * CV_PI / 180.0));
			outterPoint.y = (int)round(centerPoint.y + peripheryRadius * sin(angle * j * CV_PI / 180.0));

			cv::LineIterator innerIt(splitBGR[i], centerPoint, innerPoint, 4);
			cv::LineIterator outterIt(splitBGR[i], innerPoint, outterPoint, 4);

			double outterData = 0.0f;

			for (int k = 0; k < outterIt.count; k++, ++outterIt)
			{
				outterData += splitBGR[i].at<uchar>(outterIt.pos());
			}

			if (outterData != 0.0 && outterIt.count != 0) {
				outterData /= (double)outterIt.count;
			}

			if (outterData > peripheryMean)
			{
				outterData = peripheryMean;
			}

			for (int k = 0; k < innerIt.count; k++, ++innerIt)
			{
				if (splitBGR[i].at<uchar>(innerIt.pos()) > outterData)
				{
					shadowCorrection.at<uchar>(innerIt.pos()) = 0;
				}
				else
				{
					double differ = abs(splitBGR[i].at<uchar>(innerIt.pos()) - outterData);
					shadowCorrection.at<uchar>(innerIt.pos()) = differ;
				}
			}
		}

		cv::blur(shadowCorrection, shadowCorrection, cv::Size(66, 66));
		addWeighted(splitBGR[i], 1.0, shadowCorrection, 1.0, 0.0, splitBGR[i]);
	}

	merge(splitBGR, 3, dst);
	dst.copyTo(mat);

	return;
}



void FundusImageProcess::correct(cv::Mat & mat, float mtgt, float dtgt, float ub, float vr, float cb, float gm, int fundusImageType, int fixation, int flashValue, float focusValue)
{
	if (fundusImageType == 4) {
		if (cb >= 1.1f)
			ApplyRadialBrightnessMask(mat, 1.5f, cb);

		correctGamma(mat, gm);

		return;
	}


	auto data = (unsigned char *)mat.data;
	auto med = getMedian(data, mat.rows);
	float msrc = med.first;
	float dsrc = med.second;

	//if (GlobalSettings::useRemoveLight()) { /* && fixation == 0) { */
	//	auto light = GlobalSettings::removeReflectionLight1();
	//	auto shadow = GlobalSettings::removeReflectionLight2();
	//	auto outer = GlobalSettings::removeReflectionLight3();
	//	applyRemoveReflectionLight(mat, light, shadow, outer, flashValue, focusValue);
	//}


 	/*if (fundusImageType > 0)
	{
		if (fundusImageType != 1) {
			shading_correction(mat);
		}
		postImageProcess(mat, fundusImageType);
	}*/
	
	/*
	if (GlobalSettings::useFundusFILR_Enable()) {
		postImageProcess2_SonyCamera(mat, fundusImageType);
	}
	else {
		postImageProcess2(mat, fundusImageType);
	}
	*/

	//cv::imwrite("funcCorrection.jpg", mat);

	if (fundusImageType > 0) {
		//if (fundusImageType > 2) {
		if (fundusImageType == 3) {
			shading_correction(mat);
		}

		bool oldVer = true;
		switch (fundusImageType)
		{
		case 2:
		// case 3:
			if (GlobalSettings::useFundusFILR_Enable()) {
				postImageProcess2_SonyCamera(mat, fundusImageType, oldVer);
			}
			else {
				postImageProcess2(mat, fundusImageType);
			}
			break;
		case 3:
			postImageProcess(mat, fundusImageType);
			break;

		// modify 2025.05.19 - hwajunlee
		// Fundus image Enhancement option Level 4 
		// White balance -> Central Brightness -> Domain Transform
		case 4:   
			//if (GlobalSettings::useFundusFILR_Enable()) {
			//}
			break;
		default:
			postImageProcess(mat, fundusImageType);
			break;
		}
	}
	

	CppUtil::Logger::info("correct : %lf, %lf, %lf, %lf - %lf, %lf, %d", mtgt, dtgt, ub, vr, med.first, med.second, fixation);
	//cv::imwrite("funcCorrection_1.jpg", mat);

	if (fundusImageType != 4) {
		for (int i = 0; i < (mat.rows * mat.rows); i++) {
			float b = float(data[3 * i + 0]);
			float g = float(data[3 * i + 1]);
			float r = float(data[3 * i + 2]);
			float y, u, v;

			getYUV(&y, &u, &v, r, g, b);

			if (dsrc != 0.0) {
				y = (y - msrc) * (dtgt / dsrc) + mtgt;
			}
			else {
				y = (y - msrc) + mtgt;
			}

			u *= ub;
			v *= vr;

			getRGB(&r, &g, &b, y, u, v);

			if (r > 255.0) r = 255.0;
			if (r < 0.0) r = 0.0;
			if (g > 255.0) g = 255.0;
			if (g < 0.0) g = 0.0;
			if (b > 255.0) b = 255.0;
			if (b < 0.0) b = 0.0;

			data[3 * i + 0] = (unsigned char)b;
			data[3 * i + 1] = (unsigned char)g;
			data[3 * i + 2] = (unsigned char)r;
		}
	}

	//cv::imwrite("funcCorrection_2.jpg", mat);
	if (cb >= 1.1f)
		ApplyRadialBrightnessMask(mat, 1.5f, cb);

	correctGamma(mat, gm);
	//cv::imwrite("funcCorrection_3.jpg", mat);
	return;
}

void FundusImageProcess::getYUV(float *y, float *u, float *v, float r, float g, float b)
{
	// Coefficient refered from Wikipedia : YUV BT.709
	static const float yr = 0.2126f, yg = 0.7152f, yb = 0.0722f;
	static const float ur = -0.09991f, ug = -0.33609f, ub = 0.436f;
	static const float vr = 0.615f, vg = -0.55861f, vb = -0.05639f;
	if (y != nullptr)
		*y = yr*r + yg*g + yb*b;
	if (u != nullptr)
		*u = ur*r + ug*g + ub*b;
	if (v != nullptr)
		*v = vr*r + vg*g + vb*b;
}

void FundusImageProcess::getRGB(float *r, float *g, float *b, float y, float u, float v)
{
	// Coefficient refered from Wikipedia : YUV BT.709
	static const float ry = 1.0f, ru = 0.0f, rv = 1.28003f;
	static const float gy = 1.0f, gu = -0.21482f, gv = -0.38059f;
	static const float by = 1.0f, bu = 2.12798f, bv = 0.0f;
	if (r != nullptr)
		*r = ry*y + ru*u + rv*v;
	if (g != nullptr)
		*g = gy*y + gu*u + gv*v;
	if (b != nullptr)
		*b = by*y + bu*u + bv*v;
}

std::pair<float, float> FundusImageProcess::getMeanStdDev(unsigned char *buf, int width, int xoff, int yoff, int side)
{
	int pidx; // pixel index

	float sumMean = 0.0;
	for (int y = yoff; y < (yoff + side); y++) {
		for (int x = xoff; x < (xoff + side); x++) {
			pidx = y*width + x;
			float b = float(buf[3 * pidx + 0]);
			float g = float(buf[3 * pidx + 1]);
			float r = float(buf[3 * pidx + 2]);
			float lumi;
			getYUV(&lumi, nullptr, nullptr, r, g, b);
			sumMean += lumi;
		}
	}

	float mean = sumMean / (side * side);

	float sumDev = 0.0;
	for (int y = yoff; y < (yoff + side); y++) {
		for (int x = xoff; x < (xoff + side); x++) {
			pidx = y*width + x;
			float b = float(buf[3 * pidx + 0]);
			float g = float(buf[3 * pidx + 1]);
			float r = float(buf[3 * pidx + 2]);
			float lumi;
			getYUV(&lumi, nullptr, nullptr, r, g, b);
			sumDev += (lumi - mean) * (lumi - mean);
		}
	}

	float stddev = std::sqrtf(sumDev / (side*side));

	return std::pair<float, float>(mean, stddev);
}

std::pair<float, float> FundusImageProcess::getMedian(unsigned char *buf, int width)
{
	std::vector<int> xoffs, yoffs;
	for (int x = 575; x <= 2075; x += 300) {
		xoffs.push_back(x);
		yoffs.push_back(x);
	}
	int side = 50;

	std::vector<std::pair<float, float>> mdevs;

	for (auto yoff : yoffs) {
		for (auto xoff : xoffs) {
			auto mdev = getMeanStdDev(buf, width, xoff, yoff, side);
			mdevs.push_back(mdev);
		}
	}

	std::sort(mdevs.begin(), mdevs.end(), [](std::pair<float, float> a, std::pair<float, float> b) {
		return b.first < a.first;
	});

	return mdevs[mdevs.size() / 2];
}

void FundusImageProcess::applyEmbossing(cv::Mat& mat, EmbossType type)
{
	std::array<int, 9> data = { 0, };

	switch (type) {
	case EMBOSS_TYPE_1:
		//data = { -10, -5, 0, -5, 1, 5, 0, 5, 10 };
		//data = { 0, 1, 3, 1, 1, -1, -3, -1, 0 };
		data = { 0, 3, 9, 3, 1, -3, -9, -3, 0 };
		break;
	case EMBOSS_TYPE_2:
		//data = { 10, 5, 0, 5, 1, -5, 0, -5, -10 };
		//data = { -3, -1, 0, 1, 1, -1, 3, 1, 0 };
		data = { -9, -3, 0, 3, 1, -3, 9, 3, 0 };
		break;
	default:
		return;
	}
	cv::Mat kernel(3, 3, CV_32S, data.data());
	cv::filter2D(mat, mat, -1, kernel);
}

void FundusImageProcess::ApplyRadialBrightnessMask(cv::Mat & mat,float brightness, float gamma)
{
	int width = mat.cols;
	int height = mat.rows;
	int channels = mat.channels();
	cv::Point center(width / 2, height / 2);

	float radius = mat.cols / 2.0f;


	cv::Mat mask(height, width, CV_32FC1);
	for (int y = 0; y < height; ++y)
	{
		float* maskRow = mask.ptr<float>(y);
		for (int x = 0; x < width; ++x)
		{
			float dx = float(x - center.x);
			float dy = float(y - center.y);
			float dist = std::sqrt(dx * dx + dy * dy);


			float alpha = (std::min)(dist / float(radius), 1.0f);
			float decay = std::pow(alpha, float(gamma));


			float brightnessFactor = float(brightness * (1.0 - 0.8 * decay));
			maskRow[x] = brightnessFactor;
		}
	}


	cv::Mat mask3C;
	if (channels == 3) {
		cv::Mat mv[] = { mask, mask, mask };
		merge(mv, 3, mask3C);
	}
	else {
		mask3C = mask;
	}


	cv::Mat inputFloat;
	if (channels == 3)
		mat.convertTo(inputFloat, CV_32FC3);
	else
		mat.convertTo(inputFloat, CV_32FC1);


	cv::Mat resultFloat;
	multiply(inputFloat, mask3C, resultFloat);


	cv::Mat result;
	resultFloat.convertTo(result, mat.type());

	cv::Mat& m = const_cast<cv::Mat&>(mat);
	m = result;
}