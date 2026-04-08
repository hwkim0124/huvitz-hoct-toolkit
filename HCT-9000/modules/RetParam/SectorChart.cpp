#include "stdafx.h"
#include "SectorChart.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;



struct SectorChart::SectorChartImpl
{
	float centerDiam;
	float innerDiam;
	float outerDiam;
	bool isCornea;

	float meanCenter;
	float meanInner[8];
	float meanOuter[8];
	float meanHexagon[6];

	float meanTotal;
	float meanSuperior;
	float meanInferior;

	float minCenter;
	float maxCenter;

	SectorChartImpl() :
		centerDiam(MACULAR_CENTER_CIRCLE_SIZE), innerDiam(MACULAR_INNER_CIRCLE_SIZE), outerDiam(MACULAR_OUTER_CIRCLE_SIZE),
		meanCenter(0.0f), meanInner{ 0.0f }, meanOuter{ 0.0f },
		meanTotal(0.0f), meanSuperior(0.0f), meanInferior(0.0f), meanHexagon{ 0.0f },
		minCenter{ 0.0f }, maxCenter{ 0.0f },
		isCornea(false)
	{
	}
};


SectorChart::SectorChart() :
	d_ptr(make_unique<SectorChartImpl>())
{
}


RetParam::SectorChart::~SectorChart() = default;
RetParam::SectorChart::SectorChart(SectorChart && rhs) = default;
SectorChart & RetParam::SectorChart::operator=(SectorChart && rhs) = default;


RetParam::SectorChart::SectorChart(const SectorChart & rhs)
	: d_ptr(make_unique<SectorChartImpl>(*rhs.d_ptr))
{
}


SectorChart & RetParam::SectorChart::operator=(const SectorChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void RetParam::SectorChart::setCenterDiameter(float diam)
{
	d_ptr->centerDiam = diam;
	return;
}


void RetParam::SectorChart::setInnerDiameter(float diam)
{
	d_ptr->innerDiam = diam;
	return;
}


void RetParam::SectorChart::setOuterDiameter(float diam)
{
	d_ptr->outerDiam = diam;
	return;
}


void RetParam::SectorChart::setCorneaThickness(bool flag)
{
	d_ptr->isCornea = flag;
	return;
}


bool RetParam::SectorChart::updateContent(void)
{
	if (isEmpty()) {
		return false;
	}

	bool result;

	if (d_ptr->isCornea) {
		result = calculateCorneaThickness();
	}
	else {
		result = calculateRetinaThickness();
	}
	return result;
}


float RetParam::SectorChart::meanCenter(void) const
{
	return d_ptr->meanCenter;
}


float RetParam::SectorChart::minCenter(void) const
{
	return d_ptr->minCenter;
}


float RetParam::SectorChart::maxCenter(void) const
{
	return d_ptr->maxCenter;
}


float RetParam::SectorChart::meanInnerSector(int index) const
{
	int size = (d_ptr->isCornea ? CORNEA_THICKNESS_CHART_SECTORS : RETINA_THICKNESS_CHART_SECTORS);

	if (index >= 0 && index < size) {
		return d_ptr->meanInner[index];
	}
	return 0.0f;
}


float RetParam::SectorChart::meanOuterSector(int index) const
{
	int size = (d_ptr->isCornea ? CORNEA_THICKNESS_CHART_SECTORS : RETINA_THICKNESS_CHART_SECTORS);

	if (index >= 0 && index < size) {
		return d_ptr->meanOuter[index];
	}
	return 0.0f;
}


float RetParam::SectorChart::meanTotal(void) const
{
	return d_ptr->meanTotal;
}


float RetParam::SectorChart::meanSuperior(void) const
{
	return d_ptr->meanSuperior;
}


float RetParam::SectorChart::meanInferior(void) const
{
	return d_ptr->meanInferior;
}


float RetParam::SectorChart::meanHexagonalSector(int index) const
{
	if (index >= 0 && index < 6) {
		return d_ptr->meanHexagon[index];
	}
	return 0.0f;
}


bool RetParam::SectorChart::calculateRetinaThickness(void)
{
	auto data = enfaceData()->getDataImage();
	if (data->isEmpty()) {
		return false;
	}

	Mat image = data->getCvMatConst();
	Mat mask1 = Mat::zeros(image.rows, image.cols, CV_8UC1);

	Point center;
	center.x = enfaceData()->getPositionX(positionX());
	center.y = enfaceData()->getPositionY(positionY());
	float wPixMM = enfaceData()->getPixelXperMM();
	float hPixMM = enfaceData()->getPixelYperMM();

	float radius = d_ptr->centerDiam * 0.5f;
	Size size1 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask1, center, size1, 0.0, 0.0, 360.0, 255, -1);
	d_ptr->meanCenter = (float)cv::mean(image, mask1)(0);

	double minVal = 0.0;
	double maxVal = 0.0;
	cv::minMaxLoc(image, &minVal, &maxVal, nullptr, nullptr, mask1);

	d_ptr->minCenter = (float)minVal;
	d_ptr->maxCenter = (float)maxVal;

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	radius = d_ptr->innerDiam * 0.5f;

	Size size2 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask2, center, size2, 0.0, 225.0, 315.0, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[0] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 315, 405, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[1] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 45, 135, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[2] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 135, 225, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[3] = (float)cv::mean(image, mask2)(0);

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	radius = d_ptr->outerDiam * 0.5f;

	Size size3 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask3, center, size3, 0.0, 225.0, 315.0, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[0] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 315, 405, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[1] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 45, 135, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[2] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 135, 225, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[3] = (float)cv::mean(image, mask3)(0);

	// Average thickness over entire sectional area. 
	Mat mask4 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask4, center, size3, 0.0, 0.0, 360.0, 255, -1);
	d_ptr->meanTotal = (float)cv::mean(image, mask4)(0);

	Mat mask5 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask5, center, size3, 0.0, 180.0, 360.0, 255, -1);
	d_ptr->meanSuperior = (float)cv::mean(image, mask5)(0);

	Mat mask6 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask6, center, size3, 0.0, 0.0, 180.0, 255, -1);
	d_ptr->meanInferior = (float)cv::mean(image, mask6)(0);
	return true;
}


bool RetParam::SectorChart::calculateCorneaThickness(void)
{
	auto data = enfaceData()->getDataImage();
	if (data->isEmpty()) {
		return false;
	}

	Mat image = data->getCvMatConst();
	Mat mask1 = Mat::zeros(image.rows, image.cols, CV_8UC1);

	Point center;
	center.x = enfaceData()->getPositionX(positionX());
	center.y = enfaceData()->getPositionY(positionY());
	float wPixMM = enfaceData()->getPixelXperMM();
	float hPixMM = enfaceData()->getPixelYperMM();

	float radius = d_ptr->centerDiam * 0.5f;
	Size size1 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask1, center, size1, 0.0, 0.0, 360.0, 255, -1);
	d_ptr->meanCenter = (float)cv::mean(image, mask1)(0);

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	radius = d_ptr->innerDiam * 0.5f;

	Size size2 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask2, center, size2, 0.0, 247.5, 292.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[0] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 292.5, 337.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[1] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 337.5, 382.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[2] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 22.5, 67.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[3] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 67.5, 112.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[4] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 112.5, 157.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[5] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 157.5, 202.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[6] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 202.5, 247.5, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanInner[7] = (float)cv::mean(image, mask2)(0);

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	radius = d_ptr->outerDiam * 0.5f;

	Size size3 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask3, center, size3, 0.0, 247.5, 292.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[0] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 292.5, 337.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[1] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 337.5, 382.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[2] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 22.5, 67.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[3] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 67.5, 112.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[4] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 112.5, 157.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[5] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 157.5, 202.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[6] = (float)cv::mean(image, mask3)(0);

	mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask3, center, size3, 0.0, 202.5, 247.5, 255, -1);
	cv::ellipse(mask3, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanOuter[7] = (float)cv::mean(image, mask3)(0);

	// Average thickness over entire sectional area. 
	Mat mask4 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask4, center, size3, 0.0, 0.0, 360.0, 255, -1);
	d_ptr->meanTotal = (float)cv::mean(image, mask4)(0);

	Mat mask5 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask5, center, size3, 0.0, 180.0, 360.0, 255, -1);
	d_ptr->meanSuperior = (float)cv::mean(image, mask5)(0);

	Mat mask6 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask6, center, size3, 0.0, 0.0, 180.0, 255, -1);
	d_ptr->meanInferior = (float)cv::mean(image, mask6)(0);
	return true;
}


SectorChart::SectorChartImpl & RetParam::SectorChart::getImpl(void) const
{
	return *d_ptr;
}
