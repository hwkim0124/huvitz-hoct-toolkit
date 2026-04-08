#include "stdafx.h"
#include "Session.h"

#include "LineOutput.h"
#include "CubeOutput.h"
#include "RasterOutput.h"

#include "MacularReport.h"


using namespace std;
using namespace SegmScan;


struct Session::SessionImpl
{
	std::unique_ptr<PatternOutput> output;
	MacularReport macReport;
};


SegmScan::Session::Session() :
	d_ptr(make_unique<SessionImpl>())
{

}


SegmScan::Session::~Session() = default;
SegmScan::Session::Session(Session && rhs) = default;
Session & SegmScan::Session::operator=(Session && rhs) = default;


void SegmScan::Session::setPatternOutput(PatternOutput* output)
{
	d_ptr->output.reset(output);
	return;
}


PatternOutput * SegmScan::Session::getPatternOutput(void)
{
	return d_ptr->output.get();
}


LineOutput * SegmScan::Session::createLineOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, float axialRes)
{
	LineOutput* p = new LineOutput(side, region, start, end, axialRes);
	setPatternOutput(p);
	return p;
}


CubeOutput * SegmScan::Session::createCubeOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, bool vertical, float axialRes)
{
	CubeOutput* p = new CubeOutput(side, region, start, end, size, vertical, axialRes);
	setPatternOutput(p);
	return p;
}


RasterOutput * SegmScan::Session::createRasterOutput(EyeSide side, ScanRegion region, ScanPoint start, ScanPoint end, int size, float axialRes)
{
	RasterOutput* p = new RasterOutput(side, region, start, end, size, axialRes);
	setPatternOutput(p);
	return p;
}


bool SegmScan::Session::executeAnalysis(void)
{
	PatternOutput* output = getPatternOutput();
	if (output == nullptr) {
		return false;
	}
	if (output->getResultCount() != output->getDescript()->getNumberOfBscans()) {
		return false;
	}

	for (int i = 0; i < output->getResultCount(); i++) {
		BscanResult* result = output->getResult(i);
		if (result != nullptr) {
			for (int j = 0; j < result->getImageCount(); j++) {
				BscanImage* image = result->getImage(j);
				if (image != nullptr) {
					image->performAnalysis();
				}
			}
		}
	}

	updateMacularReport();
	return true;
}


bool SegmScan::Session::updateMacularReport(void)
{
	if (getPatternOutput() == nullptr) {
		return false;
	}
	d_ptr->macReport.setPatternOutput(getPatternOutput());
	return true;
}


MacularReport * SegmScan::Session::getMacularReport(void) const
{
	return &(d_ptr->macReport);
}

