#include "stdafx.h"
#include "ClockChart.h"


#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;



struct ClockChart::ClockChartImpl
{
	float outerDiam;
	std::vector<float> graphTSNIT;

	float meanClocks[ONH_DISC_RNFL_CLOCKS];
	float meanQuards[ONH_DISC_RNFL_QUADS];

	ClockChartImpl() : outerDiam(ONH_DISC_RNFL_CIRCLE_SIZE),
		meanClocks{ 0.0f }, meanQuards{0.0f}
	{
	}
};


ClockChart::ClockChart() :
	d_ptr(make_unique<ClockChartImpl>())
{
}


RetParam::ClockChart::~ClockChart() = default;
RetParam::ClockChart::ClockChart(ClockChart && rhs) = default;
ClockChart & RetParam::ClockChart::operator=(ClockChart && rhs) = default;


RetParam::ClockChart::ClockChart(const ClockChart & rhs)
	: d_ptr(make_unique<ClockChartImpl>(*rhs.d_ptr))
{
}


ClockChart & RetParam::ClockChart::operator=(const ClockChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void RetParam::ClockChart::setOuterDiameter(float diam)
{
	d_ptr->outerDiam = diam;
	return;
}


bool RetParam::ClockChart::updateContent(void)
{
	if (isEmpty()) {
		return false;
	}

	// calculateSections();
	bool result = calculateRnflThickness();
	return result;
}


bool RetParam::ClockChart::updateContent(const std::vector<float>& thicks)
{
	bool result = calculateRnflThickness(thicks);
	return result;
}


float RetParam::ClockChart::meanClock(int index) const
{
	if (index >= 0 && index < ONH_DISC_RNFL_CLOCKS) {
		return d_ptr->meanClocks[index];
	}
	return 0.0f;
}


float RetParam::ClockChart::meanQuadrant(int index) const
{
	if (index >= 0 && index < ONH_DISC_RNFL_QUADS) {
		return d_ptr->meanQuards[index];
	}
	return 0.0f;
}


const std::vector<float> RetParam::ClockChart::getGraphTSNIT(int size) const
{
	auto graph = vector<float>(size);
	if (size <= 0) {
		return graph;
	}

	int graphSize = (int)d_ptr->graphTSNIT.size();
	int index;
	float incr = (float)graphSize / (float)size;
	for (int i = 0; i < size; i++) {
		index = (int)(i * incr);
		if (index >= 0 && index < graphSize) {
			graph[i] = d_ptr->graphTSNIT[index];
		}
	}

	return graph;
}


std::vector<float> RetParam::ClockChart::makeGraphTSNIT(int size, EyeSide side, bool fromTemple)
{
	auto list = std::vector<float>();

	auto data = enfaceData()->getDataImage();
	if (data->isEmpty() || size <= 0) {
		return list;
	}

	Mat image = data->getCvMatConst();

	float centerX = positionX();
	float centerY = positionY();
	float radius = d_ptr->outerDiam / 2.0f;

	float currX, currY, mposX, mposY;
	float mapX1, mapX2, mapY1, mapY2;
	int posX1, posX2, posY1, posY2;
	float dataX, dataY, result;
	float sumX1, sumX2, sumY1, sumY2;

	double start;
	double angle, astep, radian;
	astep = (360.0 / size);
	angle = 0.0;

	if (fromTemple) {
		start = (side == EyeSide::OD ? -1.0 : +1.0);
	}
	else {
		start = -1.0;
	}

	for (int i = 0; i < size; i++) 
	{
		radian = NumericFunc::degreeToRadian(angle);
		currX = (float)(centerX + radius * cos(radian) * start);
		currY = (float)(centerY - radius * sin(radian));
		mposX = enfaceData()->getPositionXf(currX);
		mposY = enfaceData()->getPositionYf(currY);
		mapX1 = floor(mposX);
		mapX2 = ceil(mposX);
		mapY1 = floor(mposY);
		mapY2 = ceil(mposY);

		posX1 = min(max(int(mapX1),0), image.cols-1);
		posX2 = min(max(int(mapX2),0), image.cols-1);
		posY1 = min(max(int(mapY1),0), image.rows-1);
		posY2 = min(max(int(mapY2),0), image.rows-1);

		sumX1 = (image.at<float>(posY1, posX1) + image.at<float>(posY2, posX1)) * 0.5f;
		sumX2 = (image.at<float>(posY1, posX2) + image.at<float>(posY2, posX2)) * 0.5f;
		sumY1 = (image.at<float>(posY1, posX1) + image.at<float>(posY1, posX2)) * 0.5f;
		sumY2 = (image.at<float>(posY2, posX1) + image.at<float>(posY2, posX2)) * 0.5f;

		if (mapX1 < mapX2) {
			dataX = sumX1 * (mapX2 - mposX) + sumX2 * (mposX - mapX1);
		}
		else {
			dataX = (sumX1 + sumX2) * 0.5f;
		}

		if (mapY1 < mapY2) {
			dataY = sumY1 * (mapY2 - mposY) + sumY2 * (mposY - mapY1);
		}
		else {
			dataY = (sumY1 + sumY2) * 0.5f;
		}

		result = (dataX + dataY) * 0.5f;
		// LogD() << i << " : " << mposX << ", " << mposY << " : " << result;

		list.push_back(result);
		angle += astep;
	}

	return list;
}



bool RetParam::ClockChart::calculateSections(void)
{
	auto data = enfaceData()->getDataImage();
	if (data->isEmpty()) {
		return false;
	}

	Mat image = data->getCvMatConst();

	Point center;
	center.x = enfaceData()->getPositionX(positionX());
	center.y = enfaceData()->getPositionY(positionY());
	float wPixMM = enfaceData()->getPixelXperMM();
	float hPixMM = enfaceData()->getPixelYperMM();
	float radius = d_ptr->outerDiam / 2.0f;

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Size size1 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	Mat mask1 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	Mat mask2 = Mat::ones(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask1, center, size1, 0.0, 255, 285, 255, -1);

	mask2 = image >= 0;
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[0] = (float)cv::mean(image, mask1)(0);

	/*
	CString text, text2;
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%3f "), image.at<float>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";

	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask2.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 285, 315, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[1] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 315, 345, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[2] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 345, 375, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[3] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 15, 45, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[4] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/


	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 45, 75, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[5] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/


	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 75, 105, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[6] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 105, 135, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[7] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/


	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 135, 165, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[8] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 165, 195, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[9] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 195, 225, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[10] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	mask1.setTo(0);
	cv::ellipse(mask1, center, size1, 0.0, 225, 255, 255, -1);
	cv::bitwise_and(mask1, mask2, mask1);
	d_ptr->meanClocks[11] = (float)cv::mean(image, mask1)(0);

	/*
	for (int i = 0; i < image.rows; i++) {
	text = _T("");
	for (int j = 0; j < image.cols; j++) {
	text2.Format(_T("%03d "), mask1.at<unsigned char>(i, j));
	text += text2;
	}
	LogD() << wtoa(text);
	}

	LogD() << "";
	LogD() << "";
	LogD() << "";
	*/

	// Average thickness over quadrants from superior. 
	d_ptr->meanQuards[0] = (d_ptr->meanClocks[0] + d_ptr->meanClocks[1] + d_ptr->meanClocks[11]) / 3.0f;
	d_ptr->meanQuards[1] = (d_ptr->meanClocks[2] + d_ptr->meanClocks[3] + d_ptr->meanClocks[4]) / 3.0f;
	d_ptr->meanQuards[2] = (d_ptr->meanClocks[5] + d_ptr->meanClocks[6] + d_ptr->meanClocks[7]) / 3.0f;
	d_ptr->meanQuards[3] = (d_ptr->meanClocks[8] + d_ptr->meanClocks[9] + d_ptr->meanClocks[10]) / 3.0f;
	return true;
}



bool RetParam::ClockChart::calculateRnflThickness(const std::vector<float>& thicks)
{
	if (thicks.size() <= 0) {
		return false;
	}

	d_ptr->graphTSNIT = thicks;
	auto graph = getGraphTSNIT(360);
	updateClockSections(graph);
	return true;
}



bool RetParam::ClockChart::calculateRnflThickness(void)
{
	auto graph = makeGraphTSNIT(360, eyeSide(), true);

	d_ptr->graphTSNIT = graph;
	updateClockSections(graph);
	return true;
}


bool RetParam::ClockChart::updateClockSections(const std::vector<float>& graph)
{
	if (graph.size() != 360) {
		return false;
	}

	float dsum = 0.0f;
	int dcnt = 0;
	int sidx = 0;

	dsum = 0.0f; dcnt = 0;
	for (int i = 75; i < 105; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[0] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 45; i < 75; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[11] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 15; i < 45; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[10] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 0; i < 15; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	for (int i = 345; i < 360; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[9] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 315; i < 345; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[8] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 285; i < 315; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[7] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 255; i < 285; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[6] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 225; i < 255; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[5] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 195; i < 225; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[4] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 165; i < 195; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[3] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 135; i < 165; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[2] = (dcnt <= 0 ? 0.0f : dsum / dcnt);

	dsum = 0.0f; dcnt = 0;
	for (int i = 105; i < 135; i++) {
		if (graph[i] >= 0.0f) {
			dsum += graph[i];
			dcnt++;
		}
	}
	d_ptr->meanClocks[1] = (dcnt <= 0 ? 0.0f : dsum / dcnt);


	// Average thickness over quadrants from superior. 
	d_ptr->meanQuards[0] = (d_ptr->meanClocks[0] + d_ptr->meanClocks[1] + d_ptr->meanClocks[11]) / 3.0f;
	d_ptr->meanQuards[3] = (d_ptr->meanClocks[8] + d_ptr->meanClocks[9] + d_ptr->meanClocks[10]) / 3.0f;
	d_ptr->meanQuards[2] = (d_ptr->meanClocks[5] + d_ptr->meanClocks[6] + d_ptr->meanClocks[7]) / 3.0f;
	d_ptr->meanQuards[1] = (d_ptr->meanClocks[2] + d_ptr->meanClocks[3] + d_ptr->meanClocks[4]) / 3.0f;

	return true;
}



ClockChart::ClockChartImpl & RetParam::ClockChart::getImpl(void) const
{
	return *d_ptr;
}
