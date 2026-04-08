#include "stdafx.h"
#include "DiscClockChart.h"

#include "RetParam2.h"
#include "CppUtil2.h"
#include "NormData2.h"

using namespace SegmScan;
using namespace RetParam;
using namespace CppUtil;
using namespace NormData;
using namespace std;


struct DiscClockChart::DiscClockChartImpl
{
	RetParam::ClockChart chart;

	DiscClockChartImpl()
	{
	}
};


DiscClockChart::DiscClockChart() :
	d_ptr(make_unique<DiscClockChartImpl>())
{
	setEnfaceChart(&getImpl().chart);
}


SegmScan::DiscClockChart::~DiscClockChart() = default;
SegmScan::DiscClockChart::DiscClockChart(DiscClockChart && rhs) = default;
DiscClockChart & SegmScan::DiscClockChart::operator=(DiscClockChart && rhs) = default;


SegmScan::DiscClockChart::DiscClockChart(const DiscClockChart & rhs)
	: d_ptr(make_unique<DiscClockChartImpl>(*rhs.d_ptr))
{

}


DiscClockChart & SegmScan::DiscClockChart::operator=(const DiscClockChart & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::DiscClockChart::setCircleSize(float diam)
{
	d_ptr->chart.setOuterDiameter(diam);
	return;
}


float SegmScan::DiscClockChart::clockThickness(int section) const
{
	return d_ptr->chart.meanClock(section);
}


float SegmScan::DiscClockChart::quadrantThickness(int section) const
{
	return d_ptr->chart.meanQuadrant(section);
}


float SegmScan::DiscClockChart::clockThicknessPercentile(int section) const
{
	float value = clockThickness(section);

	switch (section) {
	case 11:
	case 0:
	case 1:
		if (value <= 66.78f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 83.41f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 166.85f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 2:
	case 3:
	case 4:
		if (value <= (eyeSide() == EyeSide::OD ? 34.53f : 35.42f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 47.29f : 43.96f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 115.21f : 93.98f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 5:
	case 6:
	case 7:
		if (value <= 65.47f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 82.34f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 166.67f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 8:
	case 9:
	case 10:
		if (value <= (eyeSide() == EyeSide::OD ? 35.42f : 34.53f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 43.96f : 47.29f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 93.98f : 115.21f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	}
	return 0.0f;
}


float SegmScan::DiscClockChart::quadrantThicknessPercentile(int section) const
{
	float value = quadrantThickness(section);

	switch (section) {
	case 0:
		if (value <= 66.78f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 83.41f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 166.85f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 1:
		if (value <= (eyeSide() == EyeSide::OD ? 34.53f : 35.42f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 47.29f : 43.96f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 115.21f : 93.98f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 2:
		if (value <= 65.47f) {
			return CHART_PERCENTILE_1;
		}
		if (value <= 82.34f) {
			return CHART_PERCENTILE_5;
		}
		if (value <= 166.67f) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	case 3:
		if (value <= (eyeSide() == EyeSide::OD ? 35.42f : 34.53f)) {
			return CHART_PERCENTILE_1;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 43.96f : 47.29f)) {
			return CHART_PERCENTILE_5;
		}
		if (value <= (eyeSide() == EyeSide::OD ? 93.98f : 115.21f)) {
			return CHART_PERCENTILE_95;
		}
		return CHART_PERCENTILE_100;
	}
	return 0.0f;
}


std::vector<float> SegmScan::DiscClockChart::getTSNITGraph(int size, int filter) const
{
	auto raws = d_ptr->chart.getGraphTSNIT(size);
	auto data = raws;
	auto dlen = (int) data.size();

	int half = filter / 2;
	int tcnt;
	float tsum;

	for (int i = 0; i < dlen; i++) {
		tsum = 0.0f; tcnt = 0;
		for (int j = i - half, k = 0; j <= i + half; j++) {
			if (j < 0) {
				k = dlen + j;
			}
			else if (j >= dlen) {
				k = j - dlen;
			}
			else {
				k = j;
			}
			tsum += raws[k];
			tcnt += 1;
		}

		if (tcnt > 0) {
			data[i] = tsum / tcnt;
		}
	}

	return data;
}


std::vector<float> SegmScan::DiscClockChart::getNormalLine(int size) const
{
	return NormData::DiscTsnitGraph::getNormalLine(size);
}


std::vector<float> SegmScan::DiscClockChart::getBorderLine(int size) const
{
	return NormData::DiscTsnitGraph::getBorderLine(size);
}


std::vector<float> SegmScan::DiscClockChart::getOutsideLine(int size) const
{
	return NormData::DiscTsnitGraph::getOutsideLine(size);
}


DiscClockChart::DiscClockChartImpl & SegmScan::DiscClockChart::getImpl(void) const
{
	return *d_ptr;
}
