#include "stdafx.h"
#include "OcularEnfaceChart.h"
#include "OcularEnfaceMap.h"
#include "OcularBsegm.h"

#include "RetParam2.h"
#include "CppUtil2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace std;


struct OcularEnfaceChart::OcularEnfaceChartImpl
{
	EyeSide side;
	RetParam::EnfaceChart* chart;
	RetParam::EnfaceTmap* tmap;

	OcularEnfaceChartImpl() : side(EyeSide::OD), tmap(nullptr), chart(nullptr)
	{
	}
};


OcularEnfaceChart::OcularEnfaceChart() :
	d_ptr(make_unique<OcularEnfaceChartImpl>())
{
}


SegmScan::OcularEnfaceChart::~OcularEnfaceChart() = default;
SegmScan::OcularEnfaceChart::OcularEnfaceChart(OcularEnfaceChart && rhs) = default;
OcularEnfaceChart & SegmScan::OcularEnfaceChart::operator=(OcularEnfaceChart && rhs) = default;


SegmScan::OcularEnfaceChart::OcularEnfaceChart(const OcularEnfaceChart & rhs)
	: d_ptr(make_unique<OcularEnfaceChartImpl>(*rhs.d_ptr))
{
}


OcularEnfaceChart & SegmScan::OcularEnfaceChart::operator=(const OcularEnfaceChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


EyeSide SegmScan::OcularEnfaceChart::eyeSide(void) const
{
	return d_ptr->side;
}


RetParam::EnfaceChart * SegmScan::OcularEnfaceChart::enfaceChart(void) const
{
	return getImpl().chart;
}


void SegmScan::OcularEnfaceChart::setEyeSide(EyeSide side)
{
	d_ptr->side = side;
	enfaceChart()->setEyeSide(eyeSide());
	return;
}


void SegmScan::OcularEnfaceChart::setEnfaceChart(RetParam::EnfaceChart * chart)
{
	getImpl().chart = chart;
	return;
}


void SegmScan::OcularEnfaceChart::setEnfaceData(RetParam::EnfaceTmap * tmap)
{
	d_ptr->tmap = tmap;
	return;
}


void SegmScan::OcularEnfaceChart::setLocation(float xInMM, float yInMM)
{
	if (enfaceChart()) {
		enfaceChart()->setChartPosition(xInMM, yInMM);
	}
	return;
}


bool SegmScan::OcularEnfaceChart::updateMetrics(void)
{
	if (enfaceData() && enfaceChart()) {
		enfaceChart()->setEnfaceData(enfaceData());
		return enfaceChart()->updateContent();
	}
	return false;
}


bool SegmScan::OcularEnfaceChart::updateMetrics(const std::vector<float>& thicks)
{
	if (enfaceChart()) {
		return enfaceChart()->updateContent(thicks);
	}
	return false;
}


bool SegmScan::OcularEnfaceChart::isEmpty(void) const
{
	// return (!enfaceData() || enfaceData()->isEmpty() || !enfaceChart() || enfaceChart()->isEmpty
	return (!enfaceChart() || enfaceChart()->isEmpty());
}


RetParam::EnfaceTmap* SegmScan::OcularEnfaceChart::enfaceData(void) const
{
	return d_ptr->tmap;
}


OcularEnfaceChart::OcularEnfaceChartImpl & SegmScan::OcularEnfaceChart::getImpl(void) const
{
	return *d_ptr;
}
