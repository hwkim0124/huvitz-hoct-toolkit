#include "stdafx.h"
#include "ImageBuffer.h"
#include "FrameImage.h"
#include "ChainSetup.h"
#include "ChainOutput.h"

#include "CppUtil2.h"

#include <mutex>
#include <atomic>
#include <functional>

using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct ImageBuffer::ImageBufferImpl
{
	FrameImageVect previews;
	FrameImageVect enfaces;
	FrameImageVect measures;

	int indexToPreviewOutput;
	int indexToPreviewResult;
	int indexToMeasureOutput;
	int indexToMeasureResult;
	int indexToEnfaceOutput;
	int indexToEnfaceResult;

	int enfaceWidth;
	int enfaceHeight;
	int enfaceDepth;
	int measureWidth;
	int measureHeight;
	int measureDepth;

	atomic<int> outputType;

	mutex mutexBuffer;
	mutex mutexProcess;
	condition_variable condVarProcess;
	atomic<bool> stopped;
	atomic<bool> busying;
	thread threadWork;

	ImageBufferImpl() : indexToPreviewOutput(0), indexToPreviewResult(0), 
			indexToMeasureOutput(0), indexToMeasureResult(0),
			indexToEnfaceOutput(0), indexToEnfaceResult(0),
			enfaceWidth(0), enfaceHeight(0), enfaceDepth(0),
			measureWidth(0), measureHeight(0), measureDepth(0),
			stopped(true), busying(false)
	{
	}
};


ImageBuffer::ImageBuffer() :
	d_ptr(make_unique<ImageBufferImpl>())
{
	initialize();
}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::ImageBuffer::~ImageBuffer() = default;
SigChain::ImageBuffer::ImageBuffer(ImageBuffer && rhs) = default;
ImageBuffer & SigChain::ImageBuffer::operator=(ImageBuffer && rhs) = default;


ImageBuffer::ImageBufferImpl & SigChain::ImageBuffer::getImpl(void) const
{
	return *d_ptr;
}


void SigChain::ImageBuffer::initialize(void)
{
	if (isStopped() != true) {
		return;
	}

	getImpl().previews.resize(IMAGE_BUFFER_LIST_SIZE_FOR_PREVIEW);
	getImpl().measures.resize(IMAGE_BUFFER_LIST_SIZE_FOR_MEASURE);
	getImpl().enfaces.resize(IMAGE_BUFFER_LIST_SIZE_FOR_ENFACE);

	clearImageBuffers();
	return;
}


void SigChain::ImageBuffer::clearImageBuffers(void)
{
	if (isStopped() != true) {
		return;
	}

	clearPreviewImages();
	clearMeasureImages();
	clearEnfaceImages();
	return;
}


void SigChain::ImageBuffer::clearPreviewImages(void)
{
	for (auto iter = begin(getImpl().previews); iter != end(getImpl().previews); ++iter) {
		iter->setEmpty();
	}

	getImpl().indexToPreviewOutput = 0;
	getImpl().indexToPreviewResult = 0;
	return;
}


void SigChain::ImageBuffer::clearMeasureImages(void)
{
	for (auto iter = begin(getImpl().measures); iter != end(getImpl().measures); ++iter) {
		iter->setEmpty();
	}

	getImpl().indexToMeasureOutput = 0;
	getImpl().indexToMeasureResult = 0;
	return;
}


void SigChain::ImageBuffer::clearEnfaceImages(void)
{
	for (auto iter = begin(getImpl().enfaces); iter != end(getImpl().enfaces); ++iter) {
		iter->setEmpty();
	}

	getImpl().indexToEnfaceOutput = 0;
	getImpl().indexToEnfaceResult = 0;
	return;
}


void SigChain::ImageBuffer::processImageBuffer(void)
{
	// Start processing loop. 
	unique_lock<mutex> lock(getImpl().mutexProcess);
	int count = 0;

	while (true)
	{
		FrameImage* image = nullptr;
		auto predicate = [this, &image]() mutable {
			if (image == nullptr) {
				image = getFrameImageToPreviewResult();
			}
			if (image == nullptr) {
				image = getFrameImageToEnfaceResult();
			}
			if (image == nullptr) {
				image = getFrameImageToMeasureResult();
			}

			if (isStopped() || image != nullptr) {
				getImpl().busying = true;
				return true;
			}
			else {
				getImpl().busying = false;
				return false;
			}
		};

		getImpl().condVarProcess.wait(lock, predicate);
	
		// Even though the stopping flag has been set, if there are still data items to be processed 
		// in buffer, try to process before exit.
		if (image != nullptr)
		{
			if (image->isPreview()) 
			{
				processPreviewImage(image);
			}
			else if (image->isEnface()) 
			{
				processEnfaceImage(image);
			}
			else if (image->isMeasure()) 
			{
				processMeasureImage(image);
			}

			if (DEBUG_OUT) {
				count++;
				// DebugOut2() << "Frame image output count: " << count;
			}
		}
		else {
			if (isStopped()) {
				break;
			}
		}
	}
	return;
}


void SigChain::ImageBuffer::processPreviewImage(FrameImage* image)
{
	uint8_t* buffer = image->getBuffer();
	int width = image->getWidth();
	int height = image->getHeight();
	float qindex = image->getQualityIndex();
	float sigRatio = image->getSignalRatio();
	int refPoint = image->getReferencePoint();
	int idxImage = image->getIndexOfImage();

	/*
	static int count = 0;
	if (count == 0) {
		CppUtil::ClockTimer::start(1);
	}
	*/

	if (image->isPreview())
	{
		if (ChainSetup::usePreviewCorneaCallback()) {
			PreviewCorneaCallback* callback = ChainSetup::getPreviewCorneaCallback();
			if (callback) {
				(*callback)(buffer, width, height, qindex, sigRatio, refPoint, idxImage);
			}
		}

		if (ChainSetup::usePreviewImageCallback2()) {
			PreviewImageCallback2* callback2 = ChainSetup::getPreviewImageCallback2();
			if (callback2) {
				(*callback2)(buffer, width, height, qindex, sigRatio, refPoint, idxImage);
			}
		}
		else {
			if (ChainSetup::usePreviewImageCallback()) {
				PreviewImageCallback* callback = ChainSetup::getPreviewImageCallback();
				if (callback) {
					(*callback)(buffer, width, height, qindex, idxImage);
				}
			}
		}

		if (idxImage == 0) {
			ChainOutput::setPreviewImageResult(qindex, sigRatio, refPoint);
		}

		GlobalRegister::triggerPreviewImageCompleted(buffer, width, height, qindex, sigRatio, refPoint, idxImage);
	
		/*
		count++;
		if (count > 100) {
			double msec = CppUtil::ClockTimer::elapsed_ms(1);
			DebugOut2() << "Preview image 100 elapsed: " << msec;
			count = 0;
		}
		*/
	}
	image->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::ImageBuffer::processMeasureImage(FrameImage* image)
{
	uint8_t* buffer = image->getBuffer();
	int width = image->getWidth();
	int height = image->getHeight();
	float qindex = image->getQualityIndex();
	float sigRatio = image->getSignalRatio();
	int refPoint = image->getReferencePoint();
	int idxImage = image->getIndexOfImage();
	uint8_t* lateral = image->getLateralBuffer();

	if (image->isMeasure()) {
		// uint8_t* lateral = image->getLateralBuffer();
		// ChainOutput::setLateralLineOfEnface(lateral, idxImage, width);

		// DebugOut2() << "Frame image to measure, index: " << image->getIndexOfImage();
		GlobalRegister::triggerMeasureImageCompleted(buffer, width, height, qindex, sigRatio, refPoint, idxImage, lateral);
	}
	image->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::ImageBuffer::processEnfaceImage(FrameImage* image)
{
	uint8_t* buffer = image->getBuffer();
	int width = image->getWidth();
	int height = image->getHeight();
	float qindex = image->getQualityIndex();
	float sigRatio = image->getSignalRatio();
	int refPoint = image->getReferencePoint();
	int idxImage = image->getIndexOfImage();

	int enfWidth = getImpl().enfaceWidth;
	int enfHeight = getImpl().enfaceHeight;
	int idxLast = getImpl().enfaceHeight - 1;

	if (image->isEnface()) {
		uint8_t* laterals = image->getLateralBuffer();
		ChainOutput::makeAxialProjectionOfRetina(image->getBuffer(), laterals, width, height);
		ChainOutput::setLateralLineOfEnface(laterals, idxImage, width, enfHeight, false, false);

		if (idxImage == idxLast) {
			ChainOutput::setEnfaceImageResult(enfWidth, enfHeight);
			uint8_t* enface = ChainOutput::getEnfaceImageBuffer();

			CvImage image;
			if (GlobalSettings::useEnfaceImageCorrection()) {
				image.fromBitsData(enface, enfWidth, enfHeight);
				image.equalizeHistogram(1.0);
				enface = image.getBitsData();
			}

			if (ChainSetup::useEnfaceImageCallback()) {
				EnfaceImageCallback* callback = ChainSetup::getEnfaceImageCallback();
				if (callback) {
					(*callback)(enface, enfWidth, enfHeight);
				}
			}

			GlobalRegister::triggerEnfaceImageCompleted(enface, enfWidth, enfHeight);
		}
	}
	image->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::ImageBuffer::startRunning(void)
{
	if (isStopped() == false) {
		stopRunning();
	}

	getImpl().stopped = false;
	getImpl().threadWork = thread{ &ImageBuffer::processImageBuffer, this };

	return;
}


void SigChain::ImageBuffer::stopRunning(void)
{
	if (getImpl().stopped != true) {
		getImpl().stopped = true;

		// Caution:
		// If this routine owning the lock falls into waiting status by calling join, 
		// the worker thread can't wake up until the lock released. 
		// However, the lock should be held to be the waiting status of the worker thread 
		// before notification, RAII fashion within a bracket scope could resolve this problem.
		{
			unique_lock<mutex> lock(getImpl().mutexProcess);
			getImpl().condVarProcess.notify_all();
		}

		if (getImpl().threadWork.joinable()) {
			getImpl().threadWork.join();
		}
	}
	return;
}


bool SigChain::ImageBuffer::isStopped(void) const
{
	return getImpl().stopped;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToPreviewOutput(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToPreviewOutput;

	size_t size = getImpl().previews.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().previews[index];
	if (image->isEmpty()) { 
		image->setStatus(FrameImage::RECEIVING);
	}
	else if (image->isReceiving()) {
		// ??, Return with receiving status.
	}
	else {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		/*
		// If the image buffer incorporates only one element with sized 1, 
		// and the image is currently on processing, return with null pointer.
		// (any frame image to be output is not available)
		if (next == getImpl().indexToPreviewResult) {
			if (size == 1) {
				image = &getImpl().previewImages[next];
				if (image->isProcessing()) {
					return nullptr;
				}
				else {
					next++;
					if (next >= size) {
						next = 0;
					}
				}
			}
		}
		*/

		// As long as the increased index doesn't coincide with the index for result, 
		// it need not care about the status of the image indexed. 
		getImpl().indexToPreviewOutput = next;
		image = &getImpl().previews[next];

		if (image->isProcessing()) {
			return nullptr;
		}
		else {
			// completed or processed (desired).
			image->setEmpty();
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Output preview image index: " << getImpl().indexToPreviewOutput;
	}
	return image;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToPreviewResult(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToPreviewResult;

	size_t size = getImpl().previews.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().previews[index];

	if (image->isCompleted()) { 
		image->setStatus(FrameImage::PROCESSING);
	}
	else if (image->isProcessing()) {
		// ??
	}
	else if (image->isProcessed()) {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		image = &getImpl().previews[next];
		if (image->isCompleted() == false) {
			return nullptr;
		}
		else {
			// Only completed image has been turned into processing status. 
			getImpl().indexToPreviewResult = next;
			image->setStatus(FrameImage::PROCESSING);
		}
	}
	else {
		return nullptr;
	}
	
	image->setType(FrameImage::PREVIEW);

	if (DEBUG_OUT) {
		DebugOut2() << "Result preview image index: " << getImpl().indexToPreviewResult;
	}
	return image;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToMeasureOutput(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToMeasureOutput;

	size_t size = getImpl().measures.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().measures[index];
	if (image->isEmpty()) {
		image->setStatus(FrameImage::RECEIVING);
	}
	else if (image->isReceiving()) {
		// ??, Return with receiving status.
	}
	else {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		getImpl().indexToMeasureOutput = next;
		image = &getImpl().measures[next];

		// Not processed measure image should not be overridden. 
		if (image->isCompleted() || image->isProcessing()) {
			return nullptr;
		}
		else {
			image->setEmpty();
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Output measure image index: " << getImpl().indexToMeasureOutput;
	}
	return image;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToMeasureResult(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToMeasureResult;

	size_t size = getImpl().measures.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().measures[index];

	if (image->isCompleted()) {
		image->setStatus(FrameImage::PROCESSING);
	}
	else if (image->isProcessing()) {
		// ??
	}
	else if (image->isProcessed()) {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		image = &getImpl().measures[next];
		if (image->isCompleted() == false) {
			return nullptr;
		}
		else {
			// Only completed image has been turned into processing status. 
			getImpl().indexToMeasureResult = next;
			image->setStatus(FrameImage::PROCESSING);
		}
	}
	else {
		return nullptr;
	}

	image->setType(FrameImage::MEASURE);

	if (DEBUG_OUT) {
		DebugOut2() << "Result measure image index: " << getImpl().indexToMeasureResult;
	}
	return image;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToEnfaceOutput(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToEnfaceOutput;

	size_t size = getImpl().enfaces.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().enfaces[index];
	if (image->isEmpty()) {
		image->setStatus(FrameImage::RECEIVING);
	}
	else if (image->isReceiving()) {
		// ??, Return with receiving status.
	}
	else {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		getImpl().indexToEnfaceOutput = next;
		image = &getImpl().enfaces[next];

		// Not processed measure image should not be overridden. 
		if (image->isCompleted() || image->isProcessing()) {
			return nullptr;
		}
		else {
			image->setEmpty();
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Output enface image index: " << getImpl().indexToEnfaceOutput;
	}
	return image;
}


FrameImage * SigChain::ImageBuffer::getFrameImageToEnfaceResult(void)
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToEnfaceResult;

	size_t size = getImpl().enfaces.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameImage* image = &getImpl().enfaces[index];

	if (image->isCompleted()) {
		image->setStatus(FrameImage::PROCESSING);
	}
	else if (image->isProcessing()) {
		// ??
	}
	else if (image->isProcessed()) {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		image = &getImpl().enfaces[next];
		if (image->isCompleted() == false) {
			return nullptr;
		}
		else {
			// Only completed image has been turned into processing status. 
			getImpl().indexToEnfaceResult = next;
			image->setStatus(FrameImage::PROCESSING);
		}
	}
	else {
		return nullptr;
	}

	image->setType(FrameImage::ENFACE);

	if (DEBUG_OUT) {
		DebugOut2() << "Result enface image index: " << getImpl().indexToEnfaceResult;
	}
	return image;
}


void SigChain::ImageBuffer::notifyPreviewImageOutput(void)
{
	// This lock is held when the worker thread have processed all completed frame data, 
	// then falls into the waiting status. 
	lock_guard<mutex> lock(getImpl().mutexProcess);
	//setOutputType(PREVIEW);
	getImpl().condVarProcess.notify_all();
	return;
}


void SigChain::ImageBuffer::notifyMeasureImageOutput(void)
{
	// This lock is held when the worker thread have processed all completed frame data, 
	// then falls into the waiting status. 
	// lock_guard<mutex> lock(getImpl().mutexProcess);

	//setOutputType(PATTERN);
	getImpl().condVarProcess.notify_all();
	return;
}


void SigChain::ImageBuffer::notifyEnfaceImageOutput(void)
{
	// This lock is held when the worker thread have processed all completed frame data, 
	// then falls into the waiting status. 
	lock_guard<mutex> lock(getImpl().mutexProcess);
	//setOutputType(PHASE);
	getImpl().condVarProcess.notify_all();
	return;
}


void SigChain::ImageBuffer::setEnfaceResultLayout(int width, int height)
{
	getImpl().enfaceWidth = width;
	getImpl().enfaceHeight = height;
	return;
}


void SigChain::ImageBuffer::setMeasureResultLayout(int width, int height)
{
	getImpl().measureWidth = width;
	getImpl().measureHeight = height;
	return;
}


void SigChain::ImageBuffer::startCallbackThread(bool reset)
{
	if (reset || isStopped()) {
		stopRunning();
		clearImageBuffers();
		startRunning();
	}
	return;
}


void SigChain::ImageBuffer::closeCallbackThread(void)
{
	stopRunning();
	return;
}


bool SigChain::ImageBuffer::isBusying(void)
{
	return getImpl().busying;
}

