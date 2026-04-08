#include "stdafx.h"
#include "ScanReport.h"
#include "PatternOutput.h"
#include "RetParam2.h"
#include "SurfaceImage.h"


using namespace SegmScan;
using namespace RetParam;
using namespace std;


struct ScanReport::ScanReportImpl
{
	PatternOutput* output;
	EnfaceDrawVect draws;
	EnfacePlotVect plots;

	SurfaceImageVect corneaIRs;
	SurfaceImageVect retinaIRs;
	SurfaceImageVect retinaOCTs;

	ScanReportImpl() : output(nullptr) {
	}
};


ScanReport::ScanReport() :
	d_ptr(make_unique<ScanReportImpl>())
{
}


SegmScan::ScanReport::~ScanReport() = default;
SegmScan::ScanReport::ScanReport(ScanReport && rhs) = default;
ScanReport & SegmScan::ScanReport::operator=(ScanReport && rhs) = default;


SegmScan::ScanReport::ScanReport(const ScanReport & rhs)
	: d_ptr(make_unique<ScanReportImpl>(*rhs.d_ptr))
{
}


ScanReport & SegmScan::ScanReport::operator=(const ScanReport & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


ScanReport::ScanReportImpl & SegmScan::ScanReport::getImpl(void) const
{
	return *d_ptr;
}


int SegmScan::ScanReport::insertEnfaceDraw(RetParam::EnfaceDraw & draw)
{
	d_ptr->draws.push_back(std::move(draw));
	return (getEnfaceDrawCount()-1);
}


int SegmScan::ScanReport::insertEnfacePlot(RetParam::EnfacePlot & plot)
{
	d_ptr->plots.push_back(std::move(plot));
	return (getEnfacePlotCount()-1);
}


RetParam::EnfaceDraw * SegmScan::ScanReport::getEnfaceDraw(int index) const
{
	if (index < 0 || index >= getEnfaceDrawCount()) {
		return nullptr;
	}
	return &(d_ptr->draws[index]);
}


RetParam::EnfacePlot * SegmScan::ScanReport::getEnfacePlot(int index) const
{
	if (index < 0 || index >= getEnfacePlotCount()) {
		return nullptr;
	}
	return &(d_ptr->plots[index]);
}


int SegmScan::ScanReport::getEnfaceDrawCount(void) const
{
	return (int)d_ptr->draws.size();
}


int SegmScan::ScanReport::getEnfacePlotCount(void) const
{
	return (int)d_ptr->plots.size();
}


void SegmScan::ScanReport::setPatternOutput(PatternOutput * output, bool clear)
{
	d_ptr->output = output;

	if (clear) {
		clearAllContents();
	}
	return;
}


PatternOutput * SegmScan::ScanReport::getPatternOutput(void) const
{
	return d_ptr->output;
}


void SegmScan::ScanReport::clearAllContents(void)
{
	removeAllCorneaIRimages();
	removeAllRetinaIRimages();
	removeAllRetinaOCTimages();
	return;
}
  

int SegmScan::ScanReport::createCorneaIRimage(float width, float height, int wpix, int hpix, const wchar_t* path)
{
	SurfaceImage image(width, height);
	image.setPatternOutput(getPatternOutput());
	if (path != nullptr) {
		if (image.loadFile(path) == false) {
			return -1;
		}
		image.setDisplaySize(wpix, hpix);
	}
	getImpl().corneaIRs.push_back(std::move(image));
	return (getCorneaIRimageCount() - 1);
}


int SegmScan::ScanReport::createRetinaIRimage(float width, float height, int wpix, int hpix, const wchar_t* path)
{
	SurfaceImage image(width, height);
	image.setPatternOutput(getPatternOutput());
	if (path != nullptr) {
		if (image.loadFile(path) == false) {
			return -1;
		}
		image.setDisplaySize(wpix, hpix);
	}
	getImpl().retinaIRs.push_back(std::move(image));
	return (getRetinaIRimageCount() - 1);
}


int SegmScan::ScanReport::createRetinaOCTimage(float width, float height, int wpix, int hpix, const wchar_t* path)
{
	SurfaceImage image(width, height);
	image.setPatternOutput(getPatternOutput());
	if (path != nullptr) {
		if (image.loadFile(path) == false) {
			return -1;
		}
		image.setDisplaySize(wpix, hpix);
	}
	getImpl().retinaOCTs.push_back(std::move(image));
	return (getRetinaOCTimageCount() - 1);
}


SurfaceImage * SegmScan::ScanReport::getCorneaIRimage(int index) const
{
	if (index < 0 || index >= getCorneaIRimageCount()) {
		return nullptr;
	}
	return &(getImpl().corneaIRs[index]);
}


SurfaceImage * SegmScan::ScanReport::getRetinaIRimage(int index) const
{
	if (index < 0 || index >= getRetinaIRimageCount()) {
		return nullptr;
	}
	return &(getImpl().retinaIRs[index]);
}


SurfaceImage * SegmScan::ScanReport::getRetinaOCTimage(int index) const
{
	if (index < 0 || index >= getRetinaOCTimageCount()) {
		return nullptr;
	}
	return &(getImpl().retinaOCTs[index]);
}


int SegmScan::ScanReport::getCorneaIRimageCount(void) const
{
	return (int)getImpl().corneaIRs.size();
}


int SegmScan::ScanReport::getRetinaIRimageCount(void) const
{
	return (int)getImpl().retinaIRs.size();
}


int SegmScan::ScanReport::getRetinaOCTimageCount(void) const
{
	return (int)getImpl().retinaOCTs.size();
}


void SegmScan::ScanReport::removeAllCorneaIRimages(void)
{
	getImpl().corneaIRs.clear();
	return;
}


void SegmScan::ScanReport::removeAllRetinaIRimages(void)
{
	getImpl().retinaIRs.clear();
	return;
}


void SegmScan::ScanReport::removeAllRetinaOCTimages(void)
{
	getImpl().retinaOCTs.clear();
	return;
}

