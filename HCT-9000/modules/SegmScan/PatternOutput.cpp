#include "stdafx.h"
#include "PatternOutput.h"
#include "PatternDescript.h"
#include "BscanResult.h"


using namespace SegmScan;
using namespace std;


struct PatternOutput::PatternOutputImpl
{
	BscanResultVect bscans;
	PatternDescript descript;
};


PatternOutput::PatternOutput(EyeSide side, ScanRegion region, PatternType type, ScanPoint start, ScanPoint end, int size, float axialRes) :
	d_ptr(make_unique<PatternOutputImpl>())
{
	PatternDescript* desc = getDescript();
	desc->setSide(side);
	desc->setType(type);
	desc->setRegion(region);
	desc->setRange(ScanRange(start, end));
	desc->setAxialResolution(axialRes);
	desc->setNumberOfBscans(size);
}


SegmScan::PatternOutput::~PatternOutput() = default;
SegmScan::PatternOutput::PatternOutput(PatternOutput && rhs) = default;
PatternOutput & SegmScan::PatternOutput::operator=(PatternOutput && rhs) = default;


SegmScan::PatternOutput::PatternOutput(const PatternOutput & rhs)
	: d_ptr(make_unique<PatternOutputImpl>(*rhs.d_ptr))
{
}


PatternOutput & SegmScan::PatternOutput::operator=(const PatternOutput & rhs)
{
	*d_ptr = *rhs.d_ptr;
	return *this;
}


PatternDescript * SegmScan::PatternOutput::getDescript(void) const
{
	return &(d_ptr->descript);
}


void SegmScan::PatternOutput::setDescript(PatternDescript & descript)
{
	d_ptr->descript = std::move(descript);
	return;
}


void SegmScan::PatternOutput::addResult(BscanResult & result)
{
	if (result.isEmpty() == false) {
		d_ptr->bscans.push_back(std::move(result));
	}
	return;
}


void SegmScan::PatternOutput::addResult(BscanResult && result)
{
	if (result.isEmpty() == false) {
		d_ptr->bscans.push_back(result);
	}
	return;
}


BscanResult * SegmScan::PatternOutput::getResult(int index) const
{
	if (index < 0 || index >= getResultCount()) {
		return nullptr;
	}
	return &(d_ptr->bscans[index]);
}


int SegmScan::PatternOutput::getResultCount(void) const
{
	return (int)d_ptr->bscans.size();
}


BscanImage * SegmScan::PatternOutput::getImage(int resultIdx, int imageIdx) const
{
	if (resultIdx < 0 || resultIdx >= getResultCount()) {
		return nullptr;
	}
	return d_ptr->bscans[resultIdx].getImage(imageIdx);
}


std::vector<BscanImage*> SegmScan::PatternOutput::getFirstBscanImages(void) const
{
	int size = getResultCount();
	std::vector<BscanImage*> vect(size);
	for (int i = 0; i < size; i++) {
		vect[i] = getImage(i);
	}
	return vect;
}


std::vector<RetSegm::SegmImage*> SegmScan::PatternOutput::getSegmImages(ImageType type) const
{
	int size = getResultCount();
	std::vector<RetSegm::SegmImage*> vect(size);
	for (int i = 0; i < size; i++) {
		vect[i] = getImage(i)->getSegmImage(type);
	}
	return vect;
}


std::vector<RetSegm::SegmLayer*> SegmScan::PatternOutput::getSegmLayers(LayerType type) const
{
	int size = getResultCount();
	std::vector<RetSegm::SegmLayer*> vect(size);
	for (int i = 0; i < size; i++) {
		vect[i] = getImage(i)->getSegmLayer(type);
	}
	return vect;
}


int SegmScan::PatternOutput::getBscanIndexFromPosition(float mmX, float mmY) const
{
	return -1;
}


int SegmScan::PatternOutput::getBscanIndexFromPosition(int posX, int posY, int width, int height) const
{
	return -1;
}


bool SegmScan::PatternOutput::getBscanStartPosition(int index, float * mmX, float * mmY, bool center) const
{
	return false;
}


bool SegmScan::PatternOutput::getBscanStartPosition(int index, int width, int height, int * posX, int * posY) const
{
	return false;
}


bool SegmScan::PatternOutput::getBscanEndPosition(int index, float * mmX, float * mmY, bool center) const
{
	return false;
}


bool SegmScan::PatternOutput::getBscanEndPosition(int index, int width, int height, int * posX, int * posY) const
{
	return false;
}

