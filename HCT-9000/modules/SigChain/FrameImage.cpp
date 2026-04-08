#include "stdafx.h"
#include "FrameImage.h"


#include <memory>
#include <atomic>

using namespace SigChain;
using namespace std;


struct FrameImage::FrameImageImpl
{
	unique_ptr<unsigned char[]> buffer;
	unique_ptr<float[]> intensity;

	uint8_t lateralBuffer[FRAME_DATA_LINES_MAX];

	int width;
	int height;
	int indexOfImage;

	float qualityIndex;
	float signalRatio;
	int referPoint;

	OctRoute route;

	atomic<int> status;
	atomic<int> type;

	FrameImageImpl() : width(0), height(0), indexOfImage(0),
					qualityIndex(0.0f), signalRatio(0.0f), referPoint(0), 
					status(FrameImage::EMPTY), type(PREVIEW), 
					buffer(nullptr), intensity(nullptr), 
					lateralBuffer{0}
	{
		buffer = make_unique<unsigned char[]>(FRAME_IMAGE_BUFFER_SIZE);
		// intensity = nullptr;
	}
};


FrameImage::FrameImage() :
	d_ptr(make_unique<FrameImageImpl>())
{
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::FrameImage::~FrameImage() = default;
SigChain::FrameImage::FrameImage(FrameImage && rhs) = default;
FrameImage & SigChain::FrameImage::operator=(FrameImage && rhs) = default;


unsigned char * SigChain::FrameImage::getBuffer(void) const
{
	return &getImpl().buffer[0];
}


float * SigChain::FrameImage::getIntensity(void) const
{
	if (getImpl().intensity == nullptr) {
		getImpl().intensity = make_unique<float[]>(FFT_OUTPUT_BUFFER_SIZE);
	}
	return &getImpl().intensity[0];
}


void SigChain::FrameImage::setDimension(int width, int height)
{
	/*
	if (getImpl().buffer == nullptr || getImpl().width != width || getImpl().height != height) {
		getImpl().width = width;
		getImpl().height = height;
		getImpl().buffer = make_unique<unsigned char[]>(width * height);
	}
	*/
	getImpl().width = width;
	getImpl().height = height;

	return;
}


int SigChain::FrameImage::getWidth(void) const
{
	return getImpl().width;
}


int SigChain::FrameImage::getHeight(void) const
{
	return getImpl().height;
}


int SigChain::FrameImage::getIndexOfImage(void)
{
	return getImpl().indexOfImage;
}


void SigChain::FrameImage::setIndexOfImage(int index)
{
	getImpl().indexOfImage = index;
	return;
}


OctRoute SigChain::FrameImage::getRouteOfScan(void) const
{
	return getImpl().route;
}


void SigChain::FrameImage::setRouteOfScan(OctRoute route)
{
	getImpl().route = route;
	return;
}


float SigChain::FrameImage::getQualityIndex(void) const
{
	return getImpl().qualityIndex;
}


float SigChain::FrameImage::getSignalRatio(void) const
{
	return getImpl().signalRatio;
}


int SigChain::FrameImage::getReferencePoint(void) const
{
	return getImpl().referPoint;
}


uint8_t * SigChain::FrameImage::getLateralBuffer(void) const
{
	return getImpl().lateralBuffer;
}


void SigChain::FrameImage::setQualityIndex(float value)
{
	getImpl().qualityIndex = value;
	return;
}


void SigChain::FrameImage::setSignalRatio(float value)
{
	getImpl().signalRatio = value;
	return;
}


void SigChain::FrameImage::setReferencePoint(int value)
{
	getImpl().referPoint = value;
	return;
}


void SigChain::FrameImage::setStatus(int status)
{
	getImpl().status = status;
	return;
}


void SigChain::FrameImage::setCompleted(void)
{
	setStatus(FrameImage::COMPLETED);
	return;
}


void SigChain::FrameImage::setEmpty(void)
{
	setStatus(FrameImage::EMPTY);
	return;
}


bool SigChain::FrameImage::isEmpty(void) const
{
	return (getImpl().status == FrameImage::EMPTY);
}


bool SigChain::FrameImage::isReceiving(void) const
{
	return (getImpl().status == FrameImage::RECEIVING);
}


bool SigChain::FrameImage::isCompleted(void) const
{
	return (getImpl().status == FrameImage::COMPLETED);
}


bool SigChain::FrameImage::isProcessing(void) const
{
	return (getImpl().status == FrameImage::PROCESSING);
}


bool SigChain::FrameImage::isProcessed(void) const
{
	return (getImpl().status == FrameImage::PROCESSED);
}


void SigChain::FrameImage::setType(int type)
{
	getImpl().type = type;
	return;
}


bool SigChain::FrameImage::isPreview(void) const
{
	return (getImpl().type == FrameImage::PREVIEW);
}


bool SigChain::FrameImage::isEnface(void) const
{
	return (getImpl().type == FrameImage::ENFACE);
}


bool SigChain::FrameImage::isMeasure(void) const
{
	return (getImpl().type == FrameImage::MEASURE);
}


FrameImage::FrameImageImpl & SigChain::FrameImage::getImpl(void) const
{
	return *d_ptr;
}


