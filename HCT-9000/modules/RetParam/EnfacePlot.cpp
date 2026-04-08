#include "stdafx.h"
#include "EnfacePlot.h"
#include "RetSegm2.h"
#include "CppUtil2.h"

using namespace RetParam;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct EnfacePlot::EnfacePlotImpl
{
	float meanDiscCenter;
	float meanDiscInner[4];
	float meanDiscOuter[4];
	float meanTotal;
	float meanSuperior;
	float meanInferior;
};


EnfacePlot::EnfacePlot() :
	d_ptr(make_unique<EnfacePlotImpl>())
{
}


RetParam::EnfacePlot::~EnfacePlot() = default;
RetParam::EnfacePlot::EnfacePlot(EnfacePlot && rhs) = default;
EnfacePlot & RetParam::EnfacePlot::operator=(EnfacePlot && rhs) = default;


RetParam::EnfacePlot::EnfacePlot(const EnfacePlot & rhs)
	: d_ptr(make_unique<EnfacePlotImpl>(*rhs.d_ptr))
{
}


EnfacePlot & RetParam::EnfacePlot::operator=(const EnfacePlot & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void RetParam::EnfacePlot::buildPlate(void)
{
	int h = getPlateHeight();
	int w = getPlateWidth();
	Mat mat = Mat::zeros(h, w, CV_32FC1);

	int thick;
	int ypos1, ypos2;

	vector<int> ys1_def = vector<int>(mat.cols, -1);
	vector<int> ys2_def = vector<int>(mat.cols, -1); // getBscanHeight() - 1);

	//DebugOut2() << "\n\n\n\n";
	for (int r = 0; r < mat.rows; r++) {
		float* p = (float *)mat.ptr(r);
		auto l1 = getUpperLayer(r);
		auto l2 = getLowerLayer(r);

		auto ys1 = (l1 == nullptr ? ys1_def : l1->getYs());
		auto ys2 = (l2 == nullptr ? ys2_def : l2->getYs());

		// Both of upper and lower layers should be valid to get a thickness.
		for (int c = 0; c < mat.cols; c++) {
			ypos1 = ys1[c];
			ypos2 = ys2[c];
			if (ypos1 >= 0 && ypos2 >= 0) {
				thick = ypos2 - ypos1;
				if (thick > 0) {
					p[c] = (float) thick;
				}
			}
		}

		/*
		std::string str;
		for (int c = 0; c < mat.cols; c++) {
			str += to_string(p[c]);
			str += " ";
		}
		DebugOut2() << str;
		*/
	}

	// DebugOut2() << "\n\n\n\n";
	getPlate()->getCvMat() = mat;

	updateSectionThickness();
	return;
}


void RetParam::EnfacePlot::copyToImage(CppUtil::CvImage* image, int width, int height)
{
	// Source's type should not be 32SC1, instead 32FC1. 
	Size dsize(width, height);
	cv::resize(getPlate()->getCvMatConst(), image->getCvMat(), dsize, INTER_CUBIC);
	return;
}


void RetParam::EnfacePlot::updateSectionThickness(void)
{
	if (getPlate()->isEmpty()) {
		return;
	}

	Mat image = getPlate()->getCvMat();
	Mat mask1 = Mat::zeros(image.rows, image.cols, CV_8UC1);

	Point center = Point(image.cols / 2, image.rows / 2);
	float wPixMM = getHorzPixelPerMM();
	float hPixMM = getVertPixelPerMM();
	float radius = DISC_CENTER_RADIUS;

	Size size1 = Size((int)(radius*wPixMM), (int)(radius*hPixMM));
	cv::ellipse(mask1, center, size1, 0.0, 0.0, 360.0, 255, -1);
	d_ptr->meanDiscCenter = (float)cv::mean(image, mask1)(0);

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	Size size2 = Size((int)(DISC_INNER_RADIUS*wPixMM), (int)(DISC_INNER_RADIUS*hPixMM));
	cv::ellipse(mask2, center, size2, 0.0, 225.0, 315.0, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscInner[0] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 315, 405, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscInner[1] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 45, 135, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscInner[2] = (float)cv::mean(image, mask2)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size2, 0.0, 135, 225, 255, -1);
	cv::ellipse(mask2, center, size1, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscInner[3] = (float)cv::mean(image, mask2)(0);

	// Sectional thickness on inner disk in counter-clockwised direction starting from superior.
	Mat mask3 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	Size size3 = Size((int)(DISC_OUTER_RADIUS*wPixMM), (int)(DISC_OUTER_RADIUS*hPixMM));
	cv::ellipse(mask2, center, size3, 0.0, 225.0, 315.0, 255, -1);
	cv::ellipse(mask2, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscOuter[0] = (float)cv::mean(image, mask3)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size3, 0.0, 315, 405, 255, -1);
	cv::ellipse(mask2, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscOuter[1] = (float)cv::mean(image, mask3)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size3, 0.0, 45, 135, 255, -1);
	cv::ellipse(mask2, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscOuter[2] = (float)cv::mean(image, mask3)(0);

	mask2 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::ellipse(mask2, center, size3, 0.0, 135, 225, 255, -1);
	cv::ellipse(mask2, center, size2, 0.0, 0.0, 360.0, 0, -1);
	d_ptr->meanDiscOuter[3] = (float)cv::mean(image, mask3)(0);

	// Regional Average.
	d_ptr->meanTotal = (float)cv::mean(image)(0);
	
	Mat mask4 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::rectangle(mask4, Rect(0, 0, image.cols, image.rows / 2), 255, -1);
	d_ptr->meanSuperior = (float)cv::mean(image, mask4)(0);

	mask4 = Mat::zeros(image.rows, image.cols, CV_8UC1);
	cv::rectangle(mask4, Rect(0, image.rows/2, image.cols, image.rows / 2), 255, -1);
	d_ptr->meanInferior = (float)cv::mean(image, mask4)(0);
	return;
}


float RetParam::EnfacePlot::getMeanCenter(void) const
{
	return d_ptr->meanDiscCenter;
}


float RetParam::EnfacePlot::getMeanInner(int index) const
{
	return d_ptr->meanDiscInner[index];
}


float RetParam::EnfacePlot::getMeanOuter(int index) const
{
	return d_ptr->meanDiscOuter[index];
}


float RetParam::EnfacePlot::getMeanTotal(void) const
{
	return d_ptr->meanTotal;
}


float RetParam::EnfacePlot::getMeanSuperior(void) const
{
	return d_ptr->meanSuperior;
}


float RetParam::EnfacePlot::getMeanInferior(void) const
{
	return d_ptr->meanInferior;
}
