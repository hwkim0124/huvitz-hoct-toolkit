#include "stdafx.h"
#include "FrameData.h"

#include <memory>
#include <atomic>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <iterator>

#include <boost/format.hpp>

using namespace SigChain;
using namespace std;


struct FrameData::FrameDataImpl
{
	unique_ptr<unsigned short[]> buffer;
	unique_ptr<float[]> sample;

	unsigned short* rawBuffer;

	int frameWidth;
	int frameHeight;
	int lineCount;

	int dataWidth;
	int dataHeight;
	int indexOfImage;
	int numLines;

	OctRoute route;

	atomic<int> status;
	atomic<int> type;

	FrameDataImpl() : lineCount(0), indexOfImage(0), status(EMPTY), type(PREVIEW), 
					frameWidth(0), frameHeight(0), dataWidth(0), dataHeight(0), 
					numLines(0),
					rawBuffer(nullptr)
	{
		// Initialize buffer with data size of the largest bscan image.
		// buffer = make_unique<unsigned short[]>(FRAME_DATA_BUFFER_SIZE);
		// sample = make_unique<float[]>(FRAME_DATA_BUFFER_SIZE);
	}
};


FrameData::FrameData() :
	d_ptr(make_unique<FrameDataImpl>())
{
	// setFrameLayout(FRAME_DATA_LINE_SIZE, FRAME_DATA_LINES_MAX);
}


SigChain::FrameData::FrameData(int width, int height, bool alloc) :
	d_ptr(make_unique<FrameDataImpl>())
{
	setFrameLayout(width, height, alloc);
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::FrameData::~FrameData() = default;
SigChain::FrameData::FrameData(FrameData && rhs) = default;
FrameData & SigChain::FrameData::operator=(FrameData && rhs) = default;


unsigned short * SigChain::FrameData::getBuffer(void) const
{
	// return &getImpl().buffer[0];
	return getImpl().rawBuffer;
}


unsigned short * SigChain::FrameData::getDataEnd(void) const
{
	return (getBuffer() + getLineCount() * getDataWidth()) ;
}


float * SigChain::FrameData::getSample(void) const
{
	if (getImpl().sample == nullptr) {
		int bufferSize = getFrameSize();
		getImpl().sample = make_unique<float[]>(bufferSize);
	}
	return &getImpl().sample[0];
}


void SigChain::FrameData::setBuffer(unsigned short * buff) 
{
	getImpl().buffer.reset(nullptr);
	getImpl().rawBuffer = buff;
	return;
}


int SigChain::FrameData::getFrameWidth(void) const
{
	return getImpl().frameWidth;
}


int SigChain::FrameData::getFrameHeight(void) const
{
	return getImpl().frameHeight;
}


int SigChain::FrameData::getFrameSize(void) const
{
	return getFrameWidth() * getFrameHeight();
}


int SigChain::FrameData::getDataWidth(void) const
{
	return getImpl().dataWidth;
}


int SigChain::FrameData::getDataHeight(void) const
{
	return getImpl().dataHeight;
}


int SigChain::FrameData::getDataSize(void) const
{
	return getDataWidth() * getDataHeight();
}


int SigChain::FrameData::getDataCount(void) const
{
	return getLineCount() * getDataWidth();
}


int SigChain::FrameData::getLineCount(void) const
{
	return getImpl().lineCount;
}


bool SigChain::FrameData::isProcessed(void) const
{
	return (getImpl().status == FrameData::PROCESSED);
}


bool SigChain::FrameData::isCompleted(void) const
{
	return (getImpl().status == FrameData::COMPLETED);
}


bool SigChain::FrameData::isReceiving(void) const
{
	return (getImpl().status == FrameData::RECEIVING);
}


bool SigChain::FrameData::isProcessing(void) const
{
	return (getImpl().status == FrameData::PROCESSING);
}


void SigChain::FrameData::setType(int type)
{
	getImpl().type = type;
	return;
}


bool SigChain::FrameData::isPreview(void) const
{
	return (getImpl().type == FrameData::PREVIEW);
}


bool SigChain::FrameData::isEnface(void) const
{
	return (getImpl().type == FrameData::ENFACE);
}


bool SigChain::FrameData::isMeasure(void) const
{
	return (getImpl().type == FrameData::MEASURE);
}


bool SigChain::FrameData::isEmpty(void) const
{
	return (getImpl().status == FrameData::EMPTY);
}


void SigChain::FrameData::setEmpty(void)
{
	getImpl().lineCount = 0;
	setStatus(FrameData::EMPTY);
	return;
}


bool SigChain::FrameData::isReceivedFull(void)
{
	return (getLineCount() == getDataHeight());
}


void SigChain::FrameData::setStatus(int status)
{
	getImpl().status = status;
	return;
}


void SigChain::FrameData::setIndexOfImage(int index)
{
	getImpl().indexOfImage = index;
	return;
}


int SigChain::FrameData::getIndexOfImage(void)
{
	return getImpl().indexOfImage;
}


void SigChain::FrameData::setNumberOfLines(int lines)
{
	getImpl().numLines = lines;
	return;
}


int SigChain::FrameData::getNumberOfLines(void)
{
	return getImpl().numLines;
}


void SigChain::FrameData::dumpToFile(int repeats)
{
	wstring exportPath = _T(".//export");
	if (CreateDirectory(exportPath.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError()) {
		wstring cameraPath = _T(".//export//camera");
		if (CreateDirectory(cameraPath.c_str(), NULL) ||
			ERROR_ALREADY_EXISTS == GetLastError()) {
		}
	}

	int major = getIndexOfImage() / repeats;
	int minor = getIndexOfImage() % repeats;
	std::string path = (boost::format(".//export//camera//%03d_%02d.bin") % major % minor).str();

	std::ofstream file(path, std::ios::out | std::ofstream::binary);
	file.write((char *)getBuffer(), sizeof(unsigned short)*getDataSize());
	file.close();
	return;
}


OctRoute SigChain::FrameData::getRouteOfScan(void) const
{
	return getImpl().route;
}


void SigChain::FrameData::setRouteOfScan(OctRoute route)
{
	getImpl().route = route;
	return;
}


void SigChain::FrameData::setFrameLayout(int lineSize, int frameLines, bool alloc)
{
	getImpl().frameWidth = lineSize;
	getImpl().frameHeight = frameLines;

	if (alloc) {
		// Initialize buffer with data size of the largest bscan image.
		int bufferSize = lineSize * frameLines;
		getImpl().buffer = make_unique<unsigned short[]>(bufferSize);
		getImpl().rawBuffer = getImpl().buffer.get();
		// getImpl().sample = make_unique<float[]>(bufferSize);
	}
	else {
		getImpl().buffer.reset(nullptr);
		getImpl().rawBuffer = nullptr;
	}
	return;
}


void SigChain::FrameData::setDataLayout(int width, int height)
{
	getImpl().dataWidth = width;
	getImpl().dataHeight = height;
	return;
}


int SigChain::FrameData::addLineData(unsigned short * data, int numLines, bool frameStart)
{
	if (data == nullptr) {
		return 0;
	}

	if (frameStart) {
		setEmpty();
	}

	int left = getDataHeight() - getLineCount();
	int read = (left < numLines ? left : numLines);

	if (read > 0) {
		memcpy((uint8_t*)getDataEnd(), (uint8_t*)data, sizeof(unsigned short) * read * getDataWidth());
		getImpl().lineCount += read;
	}
	return read;
}


FrameData::FrameDataImpl & SigChain::FrameData::getImpl(void) const
{
	return *d_ptr;
}
