#include "stdafx.h"
#include "AngioGridChart.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>



struct AngioGridChart::AngioGridChartImpl
{
	float gridSize = 1.0f;
	float flowsSection[9] = { 0.0f };
	float ratioSection[9] = { 0.0f };

	AngioGridChartImpl() {

	};
};



AngioGridChart::AngioGridChart()
	: d_ptr(make_unique<AngioGridChartImpl>())
{
}


AngioGridChart::~AngioGridChart()
{
}


OctAngio::AngioGridChart::AngioGridChart(AngioGridChart && rhs) = default;
AngioGridChart & OctAngio::AngioGridChart::operator=(AngioGridChart && rhs) = default;


OctAngio::AngioGridChart::AngioGridChart(const AngioGridChart & rhs)
	: d_ptr(make_unique<AngioGridChartImpl>(*rhs.d_ptr))
{
}


AngioGridChart & OctAngio::AngioGridChart::operator=(const AngioGridChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctAngio::AngioGridChart::setSectionSize(float size)
{
	getImpl().gridSize = size;
	return;
}


bool OctAngio::AngioGridChart::updateAngioChart(void)
{
	auto& image = getAngioImage();
	if (image.isEmpty()) {
		return false;
	}

	Mat matSrc = image.getCvMatConst();

	Point center = Point(centerPosX(), centerPosY());
	float wPixMM = pixelsPerXmm();
	float hPixMM = pixelsPerYmm();
	float pixSize = (1.0f / wPixMM) * (1.0f / hPixMM);

	Mat matRes;
	int thresh = (int)(getThreshold() * 255);
	cv::threshold(matSrc, matRes, thresh, 1.0, THRESH_TOZERO);

	for (int i = 0; i < 9; i++) {
		Mat matMask = Mat::zeros(matRes.rows, matRes.cols, CV_8UC1);

		Point pt1 = Point((int)(center.x + ((i % 3 - 1) - 0.5f)*wPixMM), (int)(center.y + ((i / 3 - 1) - 0.5f)*hPixMM));
		Point pt2 = Point((int)(center.x + ((i % 3 - 1) + 0.5f)*wPixMM), (int)(center.y + ((i / 3 - 1) + 0.5f)*hPixMM));
		cv::rectangle(matMask, pt1, pt2, 1, -1);

		Mat matSect = matRes.mul(matMask);
		double flows = cv::sum(matSect)[0];
		double total = cv::sum(matMask)[0];

		getImpl().flowsSection[i] = (float)(flows * pixSize);
		getImpl().ratioSection[i] = (float)(flows / total);
	}
	return true;
}


float OctAngio::AngioGridChart::flowsSection(int index) const
{
	if (index >= 0 && index < 9) {
		return getImpl().flowsSection[index];
	}
	return 0.0f;
}


float OctAngio::AngioGridChart::densitySection(int index) const
{
	if (index >= 0 && index < 9) {
		return getImpl().ratioSection[index];
	}
	return 0.0f;
}


AngioGridChart::AngioGridChartImpl & OctAngio::AngioGridChart::getImpl(void) const
{
	return *d_ptr;
}

