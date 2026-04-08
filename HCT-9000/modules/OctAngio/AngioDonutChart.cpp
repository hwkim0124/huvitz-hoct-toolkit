#include "stdafx.h"
#include "AngioDonutChart.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>
#include <cmath>

struct AngioDonutChart::AngioDonutChartImpl
{
	float innerDiam = 1.0f;
	float outerDiam = 3.0f;

	float flowsSection[8] = { 0.0f };
	float flowsTotal = 0.0f;
	float flowsSuperior = 0.0f;
	float flowsInferior = 0.0f;

	float ratioSection[8] = { 0.0f };
	float ratioTotal = 0.0f;
	float ratioSuperior = 0.0f;
	float ratioInferior = 0.0f;

	AngioDonutChartImpl() {

	};
};

AngioDonutChart::AngioDonutChart()
	: d_ptr(make_unique<AngioDonutChartImpl>())
{
}


AngioDonutChart::~AngioDonutChart()
{
	// Destructor should be defined for unique_ptr to delete AngioDonutChartImpl as an incomplete type.
}


OctAngio::AngioDonutChart::AngioDonutChart(AngioDonutChart && rhs) = default;
AngioDonutChart & OctAngio::AngioDonutChart::operator=(AngioDonutChart && rhs) = default;


OctAngio::AngioDonutChart::AngioDonutChart(const AngioDonutChart & rhs)
	: d_ptr(make_unique<AngioDonutChartImpl>(*rhs.d_ptr))
{
}


AngioDonutChart & OctAngio::AngioDonutChart::operator=(const AngioDonutChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctAngio::AngioDonutChart::setSectionDiameters(float inner, float outer)
{
	getImpl().innerDiam = inner;
	getImpl().outerDiam = outer;
	return;
}


bool OctAngio::AngioDonutChart::updateAngioChart(void)
{
	auto& image = getAngioImage();
	if (image.isEmpty()) {
		return false;
	}

	Mat src = image.getCvMatConst();
	Mat mask = Mat::zeros(src.rows, src.cols, CV_8UC1);

	Point center = Point(centerPosX(), centerPosY());
	float wPixMM = pixelsPerXmm();
	float hPixMM = pixelsPerYmm();
	float pixSize = (1.0f / wPixMM) * (1.0f / hPixMM);

	Mat res;
	int thresh = (int)(getThreshold() * 255);
	cv::threshold(src, res, thresh, 1.0, THRESH_TOZERO);

	float radius;
	Size size1, size2;

	radius = d_ptr->innerDiam * 0.5f;
	size1 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	radius = d_ptr->outerDiam * 0.5f;
	size2 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 225.0, 315.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	Mat sect = res.mul(mask);
	double flows = cv::sum(sect)[0];
	double total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsSection[0] = (float)(flows * pixSize);
	d_ptr->ratioSection[0] = (float)(flows /  total);

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 315.0, 405.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	sect = res.mul(mask);
	flows = cv::sum(sect)[0];
	total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsSection[1] = (float)(flows * pixSize);
	d_ptr->ratioSection[1] = (float)(flows / total);

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 45.0, 135.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	sect = res.mul(mask);
	flows = cv::sum(sect)[0];
	total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsSection[2] = (float)(flows * pixSize);
	d_ptr->ratioSection[2] = (float)(flows / total);

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 135.0, 225.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	sect = res.mul(mask);
	flows = cv::sum(sect)[0];
	total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsSection[3] = (float)(flows * pixSize);
	d_ptr->ratioSection[3] = (float)(flows / total);

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 180.0, 360.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	sect = res.mul(mask);
	flows = cv::sum(sect)[0];
	total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsSuperior = (float)(flows * pixSize);
	d_ptr->ratioSuperior = (float)(flows / total);

	mask = Mat::zeros(res.rows, res.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 0.0, 180.0, 1, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);

	sect = res.mul(mask);
	flows = cv::sum(sect)[0];
	total = cv::sum(mask)[0];
	flows = isfinite(flows) ? flows : 0.0;
	total = isfinite(total) ? total : 0.0;
	d_ptr->flowsInferior = (float)(flows * pixSize);
	d_ptr->ratioInferior = (float)(flows / total);

	d_ptr->flowsTotal = d_ptr->flowsSuperior + d_ptr->flowsInferior;
	d_ptr->ratioTotal = (d_ptr->ratioSuperior + d_ptr->ratioInferior) / 2.0f;
	return true;
}


float OctAngio::AngioDonutChart::flowsSection(int index) const
{
	if (index >= 0 && index < 4) {
		return getImpl().flowsSection[index];
	}
	return 0.0f;
}


float OctAngio::AngioDonutChart::flowsSuperior(void) const
{
	return d_ptr->flowsSuperior;
}


float OctAngio::AngioDonutChart::flowsInferior(void) const
{
	return d_ptr->flowsInferior;
}


float OctAngio::AngioDonutChart::flowsTotal(void) const
{
	return getImpl().flowsTotal;
}


float OctAngio::AngioDonutChart::densitySection(int index) const
{
	if (index >= 0 && index < 4) {
		return getImpl().ratioSection[index];
	}
	return 0.0f;
}


float OctAngio::AngioDonutChart::densitySuperior(void) const
{
	return d_ptr->ratioSuperior;
}


float OctAngio::AngioDonutChart::densityInferior(void) const
{
	return d_ptr->ratioInferior;
}


float OctAngio::AngioDonutChart::densityTotal(void) const
{
	return getImpl().ratioTotal;
}


AngioDonutChart::AngioDonutChartImpl & OctAngio::AngioDonutChart::getImpl(void) const
{
	return *d_ptr;
}

