#include "stdafx.h"
#include "DonutChart.h"
#include "EnfaceData.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct DonutChart::DonutChartImpl
{
	float innerDiam;
	float outerDiam;

	float meanSections[6];
	float meanSuperior;
	float meanInferior;
	float meanTotal;

	DonutChartImpl() : innerDiam(1.0f), outerDiam(4.0f), meanSections{ 0.0f },
		meanTotal(0.0f), meanSuperior(0.0f), meanInferior(0.0f) 
	{
	}
};


DonutChart::DonutChart() :
	d_ptr(make_unique<DonutChartImpl>())
{
}


RetParam::DonutChart::~DonutChart() = default;
RetParam::DonutChart::DonutChart(DonutChart && rhs) = default;
DonutChart & RetParam::DonutChart::operator=(DonutChart && rhs) = default;


RetParam::DonutChart::DonutChart(const DonutChart & rhs)
	: d_ptr(make_unique<DonutChartImpl>(*rhs.d_ptr))
{
}


DonutChart & RetParam::DonutChart::operator=(const DonutChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


bool RetParam::DonutChart::updateContent(void)
{
	if (isEmpty()) {
		return false;
	}

	bool result = calculateSections();
	return result;
}


void RetParam::DonutChart::setInnerDiameter(float diam)
{
	d_ptr->innerDiam = diam;
	return;
}


void RetParam::DonutChart::setOuterDiameter(float diam)
{
	d_ptr->outerDiam = diam;
	return;
}


float RetParam::DonutChart::meanTotal(void) const
{
	return d_ptr->meanTotal;
}


float RetParam::DonutChart::meanSuperior(void) const
{
	return d_ptr->meanSuperior;
}


float RetParam::DonutChart::meanInferior(void) const
{
	return d_ptr->meanInferior;
}


float RetParam::DonutChart::meanSection(int index) const
{
	if (index >= 0 && index < 6) {
		return d_ptr->meanSections[index];
	}
	return 0.0f;
}


bool RetParam::DonutChart::calculateSections(void)
{
	auto data = enfaceData()->getDataImage();
	if (data->isEmpty()) {
		return false;
	}

	Point center;
	center.x = enfaceData()->getPositionX(positionX());
	center.y = enfaceData()->getPositionY(positionY());
	float wPixMM = enfaceData()->getPixelXperMM();
	float hPixMM = enfaceData()->getPixelYperMM();

	Mat image, mask;
	float radius, stretch;

	image = data->getCvMatConst();

	stretch = 1.2f;
	radius = d_ptr->innerDiam * 0.5f;
	Size size1 = Size((int)(radius*wPixMM*stretch), (int)(radius*hPixMM));
	radius = d_ptr->outerDiam * 0.5f;
	Size size2 = Size((int)(radius*wPixMM*stretch), (int)(radius*hPixMM));

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 240.0, 300.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[0] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 300.0, 360.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[1] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 0.0, 60.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[2] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 60.0, 120.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[3] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 120.0, 180.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[4] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 180.0, 240.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSections[5] = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 180.0, 360.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanSuperior = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 0.0, 180.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInferior = (float)cv::mean(image, mask)(0);

	mask = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask, center, size2, 0.0, 0.0, 360.0, 255, -1);
	cv::ellipse(mask, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanTotal = (float)cv::mean(image, mask)(0);
	return false;
}


DonutChart::DonutChartImpl & RetParam::DonutChart::getImpl(void) const
{
	return *d_ptr;
}
