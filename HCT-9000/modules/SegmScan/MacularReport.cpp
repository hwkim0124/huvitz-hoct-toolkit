#include "stdafx.h"
#include "MacularReport.h"
#include "RetSegm2.h"
#include "RetParam2.h"
#include "PatternOutput.h"
#include "MacularImage.h"
#include "MacularPlot.h"



using namespace SegmScan;
using namespace RetParam;
using namespace RetSegm;
using namespace std;


struct MacularReport::MacularReportImpl
{
	MacularImageVect images;
	MacularPlotVect plots;
};


MacularReport::MacularReport() 
	: d_ptr(make_unique<MacularReportImpl>())
{
}


SegmScan::MacularReport::~MacularReport() = default;
SegmScan::MacularReport::MacularReport(MacularReport && rhs) = default;
MacularReport & SegmScan::MacularReport::operator=(MacularReport && rhs) = default;


SegmScan::MacularReport::MacularReport(const MacularReport & rhs)
	: d_ptr(make_unique<MacularReportImpl>(*rhs.d_ptr))
{
}


MacularReport & SegmScan::MacularReport::operator=(const MacularReport & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


void SegmScan::MacularReport::clearAllContents(void)
{
	removeAllMacularImages();
	removeAllMacularPlots();

	ScanReport::clearAllContents();
	return;
}


int SegmScan::MacularReport::createMacularImage(int width, int height, LayerType upper, LayerType lower)
{
	MacularImage image(getPatternOutput());
	if (image.update(width, height, upper, lower) == false) {
		return -1;
	}
	d_ptr->images.push_back(std::move(image));
	return (getMacularImageCount() - 1);
}


int SegmScan::MacularReport::createMacularPlot(int width, int height, LayerType upper, LayerType lower)
{
	MacularPlot plot(getPatternOutput());
	if (plot.update(width, height, upper, lower) == false) {
		return -1;
	}
	d_ptr->plots.push_back(std::move(plot));
	return (getMacularPlotCount() - 1);
}


MacularImage * SegmScan::MacularReport::getMacularImage(int index) const
{
	if (index < 0 || index >= getMacularImageCount()) {
		return nullptr;
	}
	return &(d_ptr->images[index]);
}


MacularPlot * SegmScan::MacularReport::getMacularPlot(int index) const
{
	if (index < 0 || index >= getMacularPlotCount()) {
		return nullptr;
	}
	return &(d_ptr->plots[index]);
}


int SegmScan::MacularReport::getMacularImageCount(void) const
{
	return (int)d_ptr->images.size();
}


int SegmScan::MacularReport::getMacularPlotCount(void) const
{
	return (int)d_ptr->plots.size();
}


void SegmScan::MacularReport::removeAllMacularImages(void)
{
	d_ptr->images.clear();
	return;
}


void SegmScan::MacularReport::removeAllMacularPlots(void)
{
	d_ptr->plots.clear();
	return;
}


