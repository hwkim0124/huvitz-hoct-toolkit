#include "stdafx.h"

#include <iostream>
#include <vector>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "fundusCorrection.h"

RadialCorrFactor::RadialCorrFactor()
{
}

RadialCorrFactor::RadialCorrFactor(std::vector<float>& p, float R, int N)
{
    n = N;
    step = R / static_cast<float>(n);

    factor = std::vector<float>(n);
    float r, f;
    for (int i = 0; i < n; i++) {
        r = static_cast<float>(i) / static_cast<float>(n);
        f = p[0] * r*r*r*r
            + p[1] * r*r*r
            + p[2] * r*r
            + p[3] * r
            + p[4];

        if (f == 0.0)
            factor[i] = (float)(0.0);
        else
            factor[i] = (float)(1.0 / f);
    }
}

RadialCorrFactor::~RadialCorrFactor()
{

}

float RadialCorrFactor::at(float r)
{
    int i = static_cast<int>(r / step);
    if (i >= n)
        i = n - 1;

    return factor[i];
}

LevelCorrFactor::LevelCorrFactor()
{
}

LevelCorrFactor::LevelCorrFactor(std::vector<int>& xs, std::vector<int>& ys, bool useFundusFILR)
{
	int s = (int)xs.size();
	int N;
	if (useFundusFILR) {
		N = 32767;
	}
	else {
		N = 4095;
	}
	factor = std::vector<float>(N + 1);

	float x0, x1;
	float y0, y1;
	float slope, intercept, y;

	if (xs[0] != 0) {
		xs.insert(xs.begin(), 0);
		ys.insert(ys.begin(), 0);
	}

	if (xs[s - 1] != N) {
		xs.push_back(N);
		ys.push_back(N);
	}

	//// factor for input 0
 //   factor[0] = 0.0;

 //   // for the first interval after 0
 //   slope = static_cast<float>(ys[0]) / static_cast<float>(xs[0]);
 //   intercept = 0.0;
 //   for (int x = 1; x <= xs[0]; x++) {
 //       y = slope * static_cast<float>(x) + intercept;
 //       factor[x] = y / static_cast<float>(x);
 //   }

    for (int i = 0; i < xs.size() - 1; i++) {
        x0 = static_cast<float>(xs[i]);
        x1 = static_cast<float>(xs[i + 1]);
        y0 = static_cast<float>(ys[i]);
        y1 = static_cast<float>(ys[i + 1]);

		slope = 0.0;
		intercept = 0.0;
		if (x1 > x0) {
			slope = (y1 - y0) / (x1 - x0);
			intercept = (x1*y0 - x0*y1) / (x1 - x0);
		}

		for (int x = xs[i]; x <= xs[i+1]; x++) {
            y = slope * static_cast<float>(x) + intercept;
			factor[x] = 0.0;
			if (x != 0) {
				factor[x] = y / static_cast<float>(x);
			}
        }
    }
}

LevelCorrFactor::~LevelCorrFactor()
{

}

float LevelCorrFactor::at(int val)
{
    return factor[val];
}

cvMatCorrector::cvMatCorrector()
{
	/*
    roi_x0 = 260;
    roi_y0 = 80;
    roi_x1 = 3128;
    roi_y1 = 2844;
	*/
	ceil = 4095;
	roi_x0 = 530;
	roi_y0 = 60;
	roi_x1 = 2840 + roi_x0;
	roi_y1 = 2840 + roi_y0;
    roi_cx = (roi_x0 + roi_x1) / 2;
    roi_cy = (roi_y0 + roi_y1) / 2;
    radParm = std::vector<float>{ 1.56f, -4.02f, 3.11f, -1.00f, 0.99f };

    knee_xs = std::vector<int>{ 256, 512, 4095 };
    knee_ys = std::vector<int>{ 768, 1024, 4095 };

    radCorr = RadialCorrFactor(radParm, static_cast<float>(roi_cx - roi_x0));
    levCorr = LevelCorrFactor(knee_xs, knee_ys);
}

cvMatCorrector::~cvMatCorrector()
{
}

void cvMatCorrector::setROI(int x0, int y0, int x1, int y1)
{
    roi_x0 = x0;
    roi_y0 = y0;
    roi_x1 = x1;
    roi_y1 = y1;
    roi_cx = (roi_x0 + roi_x1) / 2;
    roi_cy = (roi_y0 + roi_y1) / 2;
    radCorr = RadialCorrFactor(radParm, static_cast<float>(roi_cx - roi_x0));
}

void cvMatCorrector::setRadialParameter(float p4, float p3, float p2, float p1, float p0)
{
    radParm = std::vector<float>{ p4, p3, p2, p1, p0 };
    radCorr = RadialCorrFactor(radParm, static_cast<float>(roi_cx - roi_x0));
}

void cvMatCorrector::setKnees(std::vector<int> xs, std::vector<int> ys)
{
    knee_xs = xs;
    knee_ys = ys;
	levCorr = LevelCorrFactor(knee_xs, knee_ys, isFundusFILR);
}

void cvMatCorrector::setCeil(int c)
{
	ceil = c;
}

bool cvMatCorrector::useFundusFILR(bool iset, bool flag)
{
	if (iset) {
		isFundusFILR = flag;
	}
	return isFundusFILR;
}
void cvMatCorrector::correctRadial(cv::Mat& m)
{
    float r;
    float corr_r;
    float val;
    for (int i = roi_y0; i < roi_y1; i++) {
        cv::Vec3b *pixel = m.ptr<cv::Vec3b>(i);
        for (int j = roi_x0; j < roi_x1; j++) {
            r = (float)std::sqrt((j - roi_cx)*(j - roi_cx) + (i - roi_cy)*(i - roi_cy));
            corr_r = radCorr.at(r);
            for (int k = 0; k < 3; k++) {
                val = static_cast<float>(pixel[j][k]);
                val *= corr_r;
                if (val > ceil) val = (float)ceil;
                pixel[j][k] = static_cast<int>(val);
            }
        }
    }
}

void cvMatCorrector::correctLevel(cv::Mat& m)
{
    float corr_l;
    float val;
	for (int i = roi_y0; i < roi_y1; i++) {
		if (isFundusFILR) {
			cv::Vec3w *pixel = m.ptr<cv::Vec3w>(i);
			for (int j = roi_x0; j < roi_x1; j++) {
				auto red = pixel[j][2];
				if (red > ceil) red = ceil;
				corr_l = levCorr.at(red);
				for (int k = 0; k < 3; k++) {
					val = static_cast<float>(pixel[j][k]);
					val *= corr_l;
					if (val > ceil) val = (float)ceil;
					pixel[j][k] = static_cast<int>(val);
				}
			}
		}
		else {
			cv::Vec3b *pixel = m.ptr<cv::Vec3b>(i);
			for (int j = roi_x0; j < roi_x1; j++) {
				auto red = pixel[j][2];
				if (red > ceil) red = ceil;
				corr_l = levCorr.at(red);
				for (int k = 0; k < 3; k++) {
					val = static_cast<float>(pixel[j][k]);
					val *= corr_l;
					if (val > ceil) val = (float)ceil;
					pixel[j][k] = static_cast<int>(val);
				}
			}
		}

	}
}

void cvMatCorrector::correctBoth(cv::Mat& m)
{
	float r;
	float corr_r = 1.0, corr_l = 1.0;
	float val;
	std::vector<float>& radTbl = radCorr.getTable();
	std::vector<float>& levTbl = levCorr.getTable();

	for (int i = roi_y0; i < roi_y1; i++) {
		if (isFundusFILR) {
			cv::Vec3w *pixel = m.ptr<cv::Vec3w>(i);
			for (int j = roi_x0; j < roi_x1; j++) {
				r = (float)std::sqrt((j - roi_cx)*(j - roi_cx) + (i - roi_cy)*(i - roi_cy));
				corr_r = radCorr.at(r);
				auto red = pixel[j][2];
				if (red > ceil) red = ceil;
				corr_l = levTbl[red];
				for (int k = 0; k < 3; k++) {
					val = static_cast<float>(pixel[j][k]);
					val *= (corr_r*corr_l);
					if (val > ceil) val = (float)ceil;
					pixel[j][k] = static_cast<int>(val);
				}
			}
		}
		else {
			cv::Vec3s *pixel = m.ptr<cv::Vec3s>(i);
			for (int j = roi_x0; j < roi_x1; j++) {
				r = (float)std::sqrt((j - roi_cx)*(j - roi_cx) + (i - roi_cy)*(i - roi_cy));
				corr_r = radCorr.at(r);
				auto red = pixel[j][2];
				if (red > ceil) red = ceil;
				corr_l = levTbl[red];
				for (int k = 0; k < 3; k++) {
					val = static_cast<float>(pixel[j][k]);
					val *= (corr_r*corr_l);
					if (val > ceil) val = (float)ceil;
					pixel[j][k] = static_cast<int>(val);
				}
			}
		}
	}
}
