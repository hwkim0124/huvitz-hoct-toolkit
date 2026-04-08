#include "stdafx.h"
#include "AngioChart.h"
#include "CppUtil2.h"

using namespace CppUtil;
using namespace OctAngio;
using namespace std;
using namespace cv;

#include <iterator>
#include <algorithm>
#include <numeric>


struct AngioChart::AngioChartImpl
{
	float rangeX = 3.0f;
	float rangeY = 3.0f;
	int centerPosX = 0;
	int centerPosY = 0;
	int imageW = 384;
	int imageH = 384;

	float threshold = 0.15f;
	CvImage image;

	AngioChartImpl() {

	};
};

AngioChart::AngioChart()
	: d_ptr(make_unique<AngioChartImpl>())
{
}


AngioChart::~AngioChart()
{
	// Destructor should be defined for unique_ptr to delete AngioChartImpl as an incomplete type.
}


OctAngio::AngioChart::AngioChart(AngioChart && rhs) = default;
AngioChart & OctAngio::AngioChart::operator=(AngioChart && rhs) = default;


OctAngio::AngioChart::AngioChart(const AngioChart & rhs)
	: d_ptr(make_unique<AngioChartImpl>(*rhs.d_ptr))
{
}


AngioChart & OctAngio::AngioChart::operator=(const AngioChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void OctAngio::AngioChart::setupAngioChart(CppUtil::CvImage& image, float rangeX, float rangeY, int imageW, int imageH)
{
	getImpl().rangeX = rangeX;
	getImpl().rangeY = rangeY;
	getImpl().imageW = imageW;
	getImpl().imageH = imageH;

	getImpl().centerPosX = imageW / 2;
	getImpl().centerPosY = imageH / 2;

	image.copyTo(&getImpl().image);
	return;
}


void OctAngio::AngioChart::setCenterPosition(float xmm, float ymm)
{
	int xpos = (int)(getImpl().imageW / 2 + xmm * pixelsPerXmm());
	int ypos = (int)(getImpl().imageH / 2 + ymm * pixelsPerYmm());
	xpos = min(max(xpos, 0), getImpl().imageW - 1);
	ypos = min(max(ypos, 0), getImpl().imageH - 1);
	getImpl().centerPosX = xpos;
	getImpl().centerPosY = ypos;
	return;
}


float OctAngio::AngioChart::getThreshold(void) const
{
	return getImpl().threshold;
}


void OctAngio::AngioChart::setThreshold(float threshold)
{
	getImpl().threshold = max(min(threshold, 1.0f), 0.0f);
	return;
}


bool OctAngio::AngioChart::updateAngioChart(void)
{
	return false;
}


int OctAngio::AngioChart::centerPosX(void) const
{
	return getImpl().centerPosX;
}

int OctAngio::AngioChart::centerPosY(void) const
{
	return getImpl().centerPosY;
}

float OctAngio::AngioChart::pixelsPerXmm(void) const
{
	return (getImpl().rangeX <= 0.0f ? 0.0f : getImpl().imageW / getImpl().rangeX);
}

float OctAngio::AngioChart::pixelsPerYmm(void) const
{
	return (getImpl().rangeY <= 0.0f ? 0.0f : getImpl().imageH / getImpl().rangeY);
}

CppUtil::CvImage & OctAngio::AngioChart::getAngioImage(void)
{
	return getImpl().image;
}


AngioChart::AngioChartImpl & OctAngio::AngioChart::getImpl(void) const
{
	return *d_ptr;
}
