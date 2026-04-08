#include "stdafx.h"
#include "EnfaceChart.h"

#include "CppUtil2.h"
#include "RetSegm2.h"

using namespace RetParam;
using namespace RetSegm;
using namespace CppUtil;
using namespace std;
using namespace cv;


struct EnfaceChart::EnfaceChartImpl
{
	const EnfaceData* enfaceData;

	EyeSide eyeSide;
	float chartPosX;
	float chartPosY;

	EnfaceChartImpl() : enfaceData(nullptr), chartPosX(0.0f), chartPosY(0.0f), eyeSide(EyeSide::OD)
	{
	}
};


EnfaceChart::EnfaceChart() :
	d_ptr(make_unique<EnfaceChartImpl>())
{
}


RetParam::EnfaceChart::~EnfaceChart() = default;
RetParam::EnfaceChart::EnfaceChart(EnfaceChart && rhs) = default;
EnfaceChart & RetParam::EnfaceChart::operator=(EnfaceChart && rhs) = default;


RetParam::EnfaceChart::EnfaceChart(const EnfaceChart & rhs)
	: d_ptr(make_unique<EnfaceChartImpl>(*rhs.d_ptr))
{
}


EnfaceChart & RetParam::EnfaceChart::operator=(const EnfaceChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void RetParam::EnfaceChart::setChartPosition(float xmm, float ymm)
{
	d_ptr->chartPosX = xmm;
	d_ptr->chartPosY = ymm;
	return;
}


void RetParam::EnfaceChart::setEyeSide(EyeSide side)
{
	d_ptr->eyeSide = side;
	return;
}


void RetParam::EnfaceChart::setEnfaceData(const EnfaceData * data)
{
	d_ptr->enfaceData = data;
	return;
}


EyeSide RetParam::EnfaceChart::eyeSide(void) const
{
	return d_ptr->eyeSide;
}


float RetParam::EnfaceChart::positionX(void) const
{
	return d_ptr->chartPosX;
}


float RetParam::EnfaceChart::positionY(void) const
{
	return d_ptr->chartPosY;
}


const EnfaceData * RetParam::EnfaceChart::enfaceData(void)
{
	return d_ptr->enfaceData;
}


bool RetParam::EnfaceChart::isEmpty(void) const
{
	return (d_ptr->enfaceData == nullptr);
}


bool RetParam::EnfaceChart::updateContent(void)
{
	return false;
}


bool RetParam::EnfaceChart::updateContent(const std::vector<float>& thicks)
{
	return false;
}

