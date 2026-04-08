#include "stdafx.h"
#include "FrameBuffer.h"
#include "FrameData.h"
#include "FrameImage.h"
#include "ImageBuffer.h"
#include "OclSystem.h"
#include "OclSystem2.h"
#include "ChainSetup.h"
#include "AngioSetup.h"

#include <mutex>
#include <atomic>
#include <functional>

#include "CppUtil2.h"

using namespace SigChain;
using namespace CppUtil;
using namespace std;


struct FrameBuffer::FrameBufferImpl
{
	FrameDataVect previews;
	FrameDataVect measures;
	FrameDataVect enfaces;

	int indexToReceivePreview;
	int indexToReceiveMeasure;
	int indexToReceiveEnface;
	int indexToProcessPreview;
	int indexToProcessMeasure;
	int indexToProcessEnface;

	int previewDataWidth;
	int previewDataHeight;
	int previewDataDepth;

	int enfaceDataWidth;
	int enfaceDataHeight;
	int enfaceDataDepth;

	int measureDataWidth;
	int measureDataHeight;
	int measureDataDepth;

	int frameWidth;
	int frameHeight;
	int frameDepth;
	int enfaceWidth;
	int enfaceHeight;
	int enfaceDepth;

	bool initialized;

	mutex mutexBuffer;
	mutex mutexProcess;
	condition_variable condVarProcess;
	atomic<bool> stopped;
	atomic<bool> busying;
	thread threadWork;

	ImageBuffer imageBuffer;

	FrameBufferImpl() : indexToReceivePreview(0), indexToProcessPreview(0), indexToReceiveEnface(0), indexToProcessEnface(0),
		stopped(true), busying(false),
		previewDataWidth(FRAME_DATA_PREVIEW_WIDTH), previewDataHeight(FRAME_DATA_PREVIEW_HEIGHT), previewDataDepth(FRAME_DATA_PREVIEW_DEPTH),
		measureDataWidth(FRAME_DATA_MEASURE_WIDTH), measureDataHeight(FRAME_DATA_MEASURE_HEIGHT), measureDataDepth(FRAME_DATA_MEASURE_DEPTH),
		enfaceDataWidth(FRAME_DATA_ENFACE_WIDTH), enfaceDataHeight(FRAME_DATA_ENFACE_HEIGHT), enfaceDataDepth(FRAME_DATA_ENFACE_DEPTH),
		frameWidth(0), frameHeight(0), frameDepth(0), enfaceWidth(0), enfaceHeight(0), enfaceDepth(0),
		initialized(false)
	{

	}
};


FrameBuffer::FrameBuffer() :
	d_ptr(make_unique<FrameBufferImpl>())
{

}


// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
SigChain::FrameBuffer::~FrameBuffer() = default;
SigChain::FrameBuffer::FrameBuffer(FrameBuffer && rhs) = default;
FrameBuffer & SigChain::FrameBuffer::operator=(FrameBuffer && rhs) = default;


bool SigChain::FrameBuffer::initializeFrameBuffer(void)
{
	/*
	if (!OclSystem::initDevice()) {
		return false;
	}
	*/

	// Initialize GPU and load kernel functions.
	if (!OclSystem2::initializeOclSystem()) {
		return false;
	}

	if (!OclSystem2::prepareDefaultTransforms()) {
		LogD() << "Failed to prepare default transforms!";
		return false;
	}

	// resetFrameBufferList();

	getImpl().initialized = true;
	return true;
}


bool SigChain::FrameBuffer::isInitialized(void) const
{
	return getImpl().initialized;
}


void SigChain::FrameBuffer::releaseFrameBuffer(void)
{
	/*
	OclSystem::releaseDevice(true);
	*/

	// Release GPU devices with kernel functions and buffers.
	OclSystem2::releaseOclSystem();
	getImpl().initialized = false;
	return;
}


bool SigChain::FrameBuffer::startFrameBuffer(bool enface)
{
	if (!isStopped()) {
		closeFrameBuffer();
	}

	resetFrameBufferListToPreview();
	resetFrameBufferListToMeasure();

	if (enface) {
		resetFrameBufferListToEnface();
	}
	
	if (!prepareTransform(enface)) {
		return false;
	}

	startImageBuffer(true);
	startRunning();
	return true;
}


void SigChain::FrameBuffer::closeFrameBuffer(void)
{
	// Processing thread should exit preceding to output thread to manage to complete
	// data items to be processed in buffer.
	stopRunning();

	// if (reset) {
		closeImageBuffer();
		// OclSystem::releaseDevice(false);
	// }
	return;
}


bool SigChain::FrameBuffer::isBusying(void)
{
	return (getImpl().busying || getImpl().imageBuffer.isBusying());
}


bool SigChain::FrameBuffer::insertFrameDataForPreview(std::uint16_t * data, int numLines, int idxOfImage)
{
	if (isStopped()) {
		return false;
	}

	unsigned short* dptr = data;
	int left = numLines;

	while (left > 0 && dptr != nullptr) {
		FrameData* frame = getFrameDataToReceivePreview();
		if (frame == nullptr) {
			return false;
		}

		// Start of new frame which should be empty status.
		if (frame->isEmpty()) {
			frame->setType(FrameData::PREVIEW);
			frame->setDataLayout(d_ptr->previewDataWidth, d_ptr->previewDataHeight);
			frame->setIndexOfImage(idxOfImage);
			frame->setStatus(FrameData::RECEIVING);
		}
		else {
			// Frame data on receiving with type should be completed before.
			if (!frame->isPreview()) {
				return false;
			}
		}

		int read = frame->addLineData(data, left);

		if (frame->isReceivedFull()) {
			frame->setStatus(FrameData::COMPLETED);
			notifyFrameDataCompleted();
		}

		if (left > read) {
			dptr += (frame->getDataWidth() * read * sizeof(unsigned short));
			left -= read;
		}
		else {
			break;
		}
	}
	return true;
}


bool SigChain::FrameBuffer::insertFrameDataForMeasure(std::uint16_t * data, int numLines, int idxOfImage)
{
	if (isStopped()) {
		return false;
	}

	FrameData* frame = getFrameDataToReceiveMeasure();
	if (frame == nullptr) {
		return false;
	}

	frame->setBuffer(data);
	frame->setType(FrameData::MEASURE);
	frame->setDataLayout(d_ptr->measureDataWidth, d_ptr->measureDataHeight);
	frame->setIndexOfImage(idxOfImage);
	frame->setStatus(FrameData::COMPLETED);

	// LogD() << "Insert frame data for measure, index: " << idxOfImage;
	/*
	unsigned short* dptr = data;
	int left = numLines;

	while (left > 0 && dptr != nullptr) {
		FrameData* frame = getFrameDataToReceivePreview();
		if (frame == nullptr) {
			return false;
		}

		// Start of new frame which should be empty status.
		if (frame->isEmpty()) {
			frame->setType(FrameData::MEASURE);
			frame->setDataLayout(d_ptr->measureDataWidth, d_ptr->measureDataHeight);
			frame->setIndexOfImage(idxOfImage);
			frame->setStatus(FrameData::RECEIVING);
		}
		else {
			// Frame data on receiving with type should be completed before.
			if (!frame->isMeasure()) {
				return false;
			}
		}

		int read = frame->addLineData(data, left);

		if (frame->isReceivedFull()) {
			frame->setStatus(FrameData::COMPLETED);
			// notifyFrameDataCompleted();
		}

		if (left > read) {
			dptr += (frame->getDataWidth() * read * sizeof(unsigned short));
			left -= read;
		}
		else {
			break;
		}
	}
	*/
	return true;
}


bool SigChain::FrameBuffer::insertFrameDataForEnface(std::uint16_t * data, int numLines, int idxOfImage)
{
	if (isStopped()) {
		return false;
	}

	unsigned short* dptr = data;
	int left = numLines;

	while (left > 0 && dptr != nullptr) {
		FrameData* frame = getFrameDataToReceiveEnface();
		if (frame == nullptr) {
			return false;
		}

		// Start of new frame which should be empty status.
		if (frame->isEmpty()) {
			frame->setType(FrameData::ENFACE);
			frame->setDataLayout(d_ptr->enfaceDataWidth, d_ptr->enfaceDataHeight);
			frame->setIndexOfImage(idxOfImage);
			frame->setStatus(FrameData::RECEIVING);
		}
		else {
			// Frame data on receiving with type should be completed before.
			if (!frame->isEnface()) {
				return false;
			}
		}

		int read = frame->addLineData(data, left);

		if (frame->isReceivedFull()) {
			frame->setStatus(FrameData::COMPLETED);
			notifyFrameDataCompleted();
		}

		if (left > read) {
			dptr += (frame->getDataWidth() * read * sizeof(unsigned short));
			left -= read;
		}
		else {
			break;
		}
	}
	return true;
}


bool SigChain::FrameBuffer::setDataLayoutForPreview(int width, int height, int depth)
{
	getImpl().previewDataWidth = width;
	getImpl().previewDataHeight = height;
	getImpl().previewDataDepth = depth;

	// bool result = OclSystem2::prepareTransformForPreview(width, height);
	// return result;
	LogD() << "Preveiw data layout, w: " << width << " h: " << height << " d: " << depth;
	return true;
}


bool SigChain::FrameBuffer::setDataLayoutForMeasure(int width, int height, int depth)
{
	getImpl().measureDataWidth = width;
	getImpl().measureDataHeight = height;
	getImpl().measureDataDepth = depth;

	getImpl().imageBuffer.setMeasureResultLayout(height, depth);

	// bool result = OclSystem2::prepareTransformForMeasure(width, height);
	// return result;
	LogD() << "Measure data layout, w: " << width << " h: " << height << " d: " << depth;
	return true;
}


bool SigChain::FrameBuffer::setDataLayoutForEnface(int width, int height, int depth)
{
	getImpl().enfaceDataWidth = width;
	getImpl().enfaceDataHeight = height;
	getImpl().enfaceDataDepth = depth;

	getImpl().imageBuffer.setEnfaceResultLayout(height, depth);
	// bool result = OclSystem2::prepareTransformForEnface(width, height);
	// return result;
	LogD() << "Enface data layout, w: " << width << " h: " << height << " d: " << depth;
	return true;
}


void SigChain::FrameBuffer::resetFrameBufferListToPreview(void)
{
	// Consumer thread and data receiving should be stopped before resizing!
	if (isStopped() != true) {
		return;
	}

	// Frame size capable of any kinds of images. 
	int frameWidth = getFrameBufferWidth(true);
	int frameHeight = getFrameBufferHeight(true);
	int frameDepth = getFrameBufferDepth(true);

	if (frameWidth != getImpl().frameWidth 
		|| frameHeight != getImpl().frameHeight
		|| frameDepth != getImpl().frameDepth)
	{
		// Secure frame list with enough sized elements.
		getImpl().previews.clear();
		for (int i = 0; i < frameDepth; i++) {
			getImpl().previews.emplace_back(frameWidth, frameHeight, true);
		}

		getImpl().frameWidth = frameWidth;
		getImpl().frameHeight = frameHeight;
		getImpl().frameDepth = frameDepth;
	}

	getImpl().indexToProcessPreview = 0;
	getImpl().indexToReceivePreview = 0;
	return;
}


void SigChain::FrameBuffer::resetFrameBufferListToMeasure(void)
{
	clearFrameBufferListToMeasure();

	getImpl().indexToProcessMeasure = -1;
	getImpl().indexToReceiveMeasure = -1;
	return;
}


void SigChain::FrameBuffer::resetFrameBufferListToEnface(void)
{
	// Consumer thread and data receiving should be stopped before resizing!
	if (isStopped() != true) {
		return;
	}

	// Frame size capable of any kinds of images. 
	int frameWidth = getImpl().enfaceDataWidth;
	int frameHeight = getImpl().enfaceDataHeight;
	int frameDepth = getImpl().enfaceDataDepth;

	if (frameWidth != getImpl().enfaceWidth
		|| frameHeight != getImpl().enfaceHeight
		|| frameDepth != getImpl().enfaceDepth)
	{
		// Secure frame list with enough sized elements.
		getImpl().enfaces.clear();
		for (int i = 0; i < frameDepth; i++) {
			getImpl().enfaces.emplace_back(frameWidth, frameHeight, true);
		}

		getImpl().enfaceWidth = frameWidth;
		getImpl().enfaceHeight = frameHeight;
		getImpl().enfaceDepth = frameDepth;
	}

	getImpl().indexToProcessEnface = 0;
	getImpl().indexToReceiveEnface = 0;
	return;
}


void SigChain::FrameBuffer::clearFrameBufferListToPreview(void)
{
	if (isStopped() == false) {
		return;
	}

	for (auto iter = begin(getImpl().previews); iter != end(getImpl().previews); ++iter) {
		iter->setEmpty();
	}

	getImpl().indexToProcessPreview = 0;
	getImpl().indexToReceivePreview = 0;
	return;
}


void SigChain::FrameBuffer::clearFrameBufferListToMeasure(void)
{
	getImpl().measures.clear();

	getImpl().indexToProcessMeasure = -1;
	getImpl().indexToReceiveMeasure = -1;
	return;
}


void SigChain::FrameBuffer::clearFrameBufferListToEnface(void)
{
	if (isStopped() == false) {
		return;
	}

	for (auto iter = begin(getImpl().enfaces); iter != end(getImpl().enfaces); ++iter) {
		iter->setEmpty();
	}

	getImpl().indexToProcessEnface = 0;
	getImpl().indexToReceiveEnface = 0;
	return;
}


int SigChain::FrameBuffer::getFrameBufferWidth(bool preview) const
{
	int size = 0;
	if (size < getImpl().previewDataWidth) {
		size = getImpl().previewDataWidth;
	}
	if (size < getImpl().enfaceDataWidth) {
		size = getImpl().enfaceDataWidth;
	}
	if (!preview) {
		if (size < getImpl().measureDataWidth) {
			size = getImpl().measureDataWidth;
		}
	}
	return size;
}


int SigChain::FrameBuffer::getFrameBufferHeight(bool preview) const
{
	int size = 0;
	if (size < getImpl().previewDataHeight) {
		size = getImpl().previewDataHeight;
	}
	if (size < getImpl().enfaceDataHeight) {
		size = getImpl().enfaceDataHeight;
	}
	if (!preview) {
		if (size < getImpl().measureDataHeight) {
			size = getImpl().measureDataHeight;
		}
	}
	return size;
}


int SigChain::FrameBuffer::getFrameBufferDepth(bool preview) const
{
	int size = 0;
	if (size < getImpl().previewDataDepth) {
		size = getImpl().previewDataDepth;
	}
	if (size < getImpl().enfaceDataDepth) {
		size = getImpl().enfaceDataDepth;
	}
	if (!preview) {
		if (size < getImpl().measureDataDepth) {
			size = getImpl().measureDataDepth;
		}
	}
	return size;
}


bool SigChain::FrameBuffer::prepareTransform(bool enface)
{
	if (!OclSystem2::initializeTransform()) {
		return false;
	}

	int dataWidth = d_ptr->previewDataWidth;
	int dataHeight = d_ptr->previewDataHeight;

	if (!OclSystem2::prepareTransformForPreview(dataWidth, dataHeight)) {
		return false;
	}

	dataWidth = d_ptr->measureDataWidth;
	dataHeight = d_ptr->measureDataHeight;

	if (!OclSystem2::prepareTransformForMeasure(dataWidth, dataHeight)) {
		return false;
	}

	if (enface) {
		dataWidth = d_ptr->enfaceDataWidth;
		dataHeight = d_ptr->enfaceDataHeight;

		if (!OclSystem2::prepareTransformForEnface(dataWidth, dataHeight)) {
			return false;
		}
	}

	return true;
}


void SigChain::FrameBuffer::notifyFrameDataCompleted(bool wait)
{
	// This lock is held when the worker thread have processed all completed frame data, 
	// then falls into the waiting status. 
	if (wait) {
		unique_lock<mutex> lock(getImpl().mutexProcess);
		getImpl().condVarProcess.notify_all();
	}
	else {
		getImpl().condVarProcess.notify_all();
	}
	return;
}


FrameBuffer::FrameBufferImpl & SigChain::FrameBuffer::getImpl(void) const
{
	return *d_ptr;
}


void SigChain::FrameBuffer::startImageBuffer(bool reset)
{
	getImpl().imageBuffer.startCallbackThread(reset);
	return;
}


void SigChain::FrameBuffer::closeImageBuffer(void)
{
	getImpl().imageBuffer.closeCallbackThread();
	return;
}


void SigChain::FrameBuffer::processFrameBuffer(void)
{
	// Start processing loop. 
	unique_lock<mutex> lock(getImpl().mutexProcess);
	int count = 0;

	while (true)
	{
		FrameData* frame = nullptr;  ;
		auto predicate = [this, &frame]() mutable {
			frame = getFrameDataToProcessPreview();
			if (frame == nullptr) {
				frame = getFrameDataToProcessMeasure();
			}
			if (frame == nullptr) {
				frame = getFrameDataToProcessEnface();
			}

			if (isStopped() || (frame != nullptr)) {
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
		// in buffer, try to process before exiting.
		if (frame != nullptr)
		{
			if (frame->isPreview()) {
				processFrameDataForPreview(frame);
			}
			else if (frame->isEnface()) {
				processFrameDataForEnface(frame);
			}
			else if (frame->isMeasure()) {
				processFrameDataForMeasure(frame);
			}
			// frame->setStatus(FrameImage::PROCESSED);
		
			if (DEBUG_OUT) {
				count++;
				DebugOut2() << "Frame data processed count: " << count;
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


void SigChain::FrameBuffer::processFrameDataForPreview(FrameData * frame)
{
	if (frame == nullptr || !frame->isPreview()) {
		return;
	}

	uint16_t* input = frame->getBuffer();
	int width = frame->getDataWidth();
	int height = frame->getDataHeight();
	int index = frame->getIndexOfImage();

	FrameImage* image = nullptr;
	uint8_t* output = nullptr;
	float* output2 = nullptr;
	uint8_t* laterals = nullptr;
	float qindex = 0.0f;
	float snrRatio = 0.0f;
	int refPoint = 0;

	if ((image = getImpl().imageBuffer.getFrameImageToPreviewOutput()) != nullptr)
	{
		output = image->getBuffer();
		output2 = image->getIntensity();

		if (OclSystem2::executeTransformForPreview(input, width, height, output, &width, &height, &qindex, &snrRatio, &refPoint, output2))
		{
			image->setDimension(width, height);
			image->setQualityIndex(qindex);
			image->setReferencePoint(refPoint);
			image->setIndexOfImage(index);
			image->setSignalRatio(snrRatio);
			image->setCompleted();

			getImpl().imageBuffer.notifyPreviewImageOutput();

			if (ChainSetup::isAngioScan()) {
				AngioSetup::storeAmplitudesToPreviewBuffer(output2);
			}
		}
		else {
			image->setEmpty();
		}
	}
	frame->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::FrameBuffer::processFrameDataForMeasure(FrameData * frame)
{
	if (frame == nullptr || !frame->isMeasure()) {
		return;
	}

	uint16_t* input = frame->getBuffer();
	int width = frame->getDataWidth();
	int height = frame->getDataHeight();
	int index = frame->getIndexOfImage();

	FrameImage* image = nullptr;
	uint8_t* output = nullptr;
	float* output2 = nullptr;
	uint8_t* laterals = nullptr;
	float qindex = 0.0f;
	float snrRatio = 0.0f;
	int refPoint = 0;

	if ((image = getImpl().imageBuffer.getFrameImageToMeasureOutput()) != nullptr)
	{
		if (ChainSetup::isAngioScan()) {
			if (AngioSetup::isSavingFramesToFiles()) {
				// For Angio raw profiles.
				frame->dumpToFile(AngioSetup::numberOfAngioRepeats());
			}
		}

		output = image->getBuffer();
		output2 = image->getIntensity();
		laterals = image->getLateralBuffer();
		if (OclSystem2::executeTransformForMeasure(input, width, height, output, &width, &height,
			&qindex, &snrRatio, &refPoint, laterals, output2))
		{
			image->setDimension(width, height);
			image->setQualityIndex(qindex);
			image->setReferencePoint(refPoint);
			image->setIndexOfImage(index);
			image->setCompleted();

			getImpl().imageBuffer.notifyMeasureImageOutput();

			if (ChainSetup::isAngioScan() || ChainSetup::isScanAmplitudesStored()) {
				if (AngioSetup::isSavingBuffersToFiles()) {
					AngioSetup::dumpImageBuffersToFile(index);
				}
				
				// AngioSetup::storeImageBuffersToAngiogram(index);
				AngioSetup::storeAmplitudesToPatternBuffer(index, output2);
			}
		}
		else {
			image->setEmpty();
		}
	}

	// ??
	// frame->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::FrameBuffer::processFrameDataForEnface(FrameData * frame)
{
	if (frame == nullptr || !frame->isEnface()) {
		return;
	}

	uint16_t* input = frame->getBuffer();
	int width = frame->getDataWidth();
	int height = frame->getDataHeight();
	int index = frame->getIndexOfImage();

	FrameImage* image = nullptr;
	uint8_t* output = nullptr;
	uint8_t* laterals = nullptr;
	float qindex = 0.0f;
	float snrRatio = 0.0f;
	int refPoint = 0;

	if ((image = getImpl().imageBuffer.getFrameImageToEnfaceOutput()) != nullptr)
	{
		output = image->getBuffer();
		laterals = image->getLateralBuffer();
		if (OclSystem2::executeTransformForEnface(input, width, height, output, &width, &height,
			&qindex, &snrRatio, &refPoint, laterals))
		{
			image->setDimension(width, height);
			image->setQualityIndex(qindex);
			image->setReferencePoint(refPoint);
			image->setIndexOfImage(index);
			image->setSignalRatio(snrRatio);
			image->setCompleted();

			getImpl().imageBuffer.notifyEnfaceImageOutput();
		}
		else {
			image->setEmpty();
		}
	}
	frame->setStatus(FrameImage::PROCESSED);
	return;
}


void SigChain::FrameBuffer::startRunning(void)
{
	if (isStopped() != true) {
		stopRunning();
	}

	getImpl().stopped = false;
	getImpl().threadWork = thread{ &FrameBuffer::processFrameBuffer, this };

	return;
}


void SigChain::FrameBuffer::stopRunning(void)
{
	if (getImpl().stopped != true) {
		getImpl().stopped = true;

		// Caution:
		// If this routine owning the lock falls into waiting status by calling join, 
		// the worker thread can't wake up until the lock released. 
		// However, the lock should be held to be the waiting status of the worker thread 
		// before notification, RAII fashion within a bracket scope could resolve this problem.
		{
			lock_guard<mutex> lock(getImpl().mutexProcess);
			getImpl().condVarProcess.notify_all();
		}

		if (getImpl().threadWork.joinable()) {
			getImpl().threadWork.join();
		}
	}
	return;
}


bool SigChain::FrameBuffer::isStopped(void) const
{
	return getImpl().stopped;
}


FrameData * SigChain::FrameBuffer::getFrameDataToReceivePreview(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToReceivePreview;

	size_t size = getImpl().previews.size();
	if (size <= 0 || index >= size) {
		DebugOut2() << "Invalid Frame buffer index!";
		return nullptr;
	}

	FrameData* data = &getImpl().previews[index];
	if (data->isEmpty()) {
		// Starting frame should be identified. 
		// data->setStatus(FrameData::RECEIVING);
	}
	else if (data->isReceiving()) {
	}
	else {
		// If all lines corresponding to the current frame have been received, 
		// move the index forward to the next buffer slot.
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		// Processing is relatively too slow to be overlapped by data receiving.
		data = &getImpl().previews[next];
		getImpl().indexToReceivePreview = next;

		if (data->isProcessing()) {
			DebugOut2() << "Frame buffer to receive is full!";
			return nullptr;
		}
		else {
			// Return to empty status to recycle after processing. 
			data->setEmpty();
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Frame data to receive preview, index: " << getImpl().indexToReceivePreview;
	}
	return data;
}


FrameData * SigChain::FrameBuffer::getFrameDataToReceiveMeasure(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int frameWidth = getFrameBufferWidth(false);
	int frameHeight = getFrameBufferHeight(false);

	d_ptr->measures.emplace_back(frameWidth, frameHeight, false);

	int next = d_ptr->indexToReceiveMeasure + 1;
	FrameData* data = &d_ptr->measures[next];
	d_ptr->indexToReceiveMeasure = next;

	if (DEBUG_OUT) {
		LogD() << "Frame data to receive measure, index: " << d_ptr->indexToReceiveMeasure << ", buffSize: " << d_ptr->measures.size();
	}
	return data;
}


FrameData * SigChain::FrameBuffer::getFrameDataToReceiveEnface(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToReceiveEnface;

	size_t size = getImpl().enfaces.size();
	if (size <= 0 || index >= size) {
		LogD() << "Invalid Frame buffer index!";
		return nullptr;
	}

	FrameData* data = &getImpl().enfaces[index];
	if (data->isEmpty()) {
		// Starting frame should be identified. 
		// data->setStatus(FrameData::RECEIVING);
	}
	else if (data->isReceiving()) {
	}
	else {
		// If all lines corresponding to the current frame have been received, 
		// move the index forward to the next buffer slot.
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}

		// Processing is relatively too slow to be overlapped by data receiving.
		data = &getImpl().enfaces[next];
		getImpl().indexToReceiveEnface = next;

		if (data->isProcessing()) {
			LogD() << "Frame buffer to receive is full!";
			return nullptr;
		}
		else {
			// Return to empty status to recycle after processing. 
			data->setEmpty();
		}
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Frame data to receive enface, index: " << getImpl().indexToReceiveEnface;
	}
	return data;
}


FrameData * SigChain::FrameBuffer::getFrameDataToProcessPreview(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToProcessPreview;

	size_t size = getImpl().previews.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameData* data = &getImpl().previews[index];

	// If frame data has been received, the index to be processed should be increased,
	// before returning it. 
	if (data->isCompleted()) {
		data->setStatus(FrameData::PROCESSING);
	}
	else if (data->isProcessing()) {

	}
	else if (data->isProcessed()) {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}
		data = &getImpl().previews[next];
		if (data->isCompleted() == false) {
			return nullptr;
		}
		else {
			getImpl().indexToProcessPreview = next;
			data->setStatus(FrameData::PROCESSING);
		}
	}
	else {
		return nullptr;
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Frame data to process preview, index: " << getImpl().indexToProcessPreview;
	}
	return data;
}


FrameData * SigChain::FrameBuffer::getFrameDataToProcessMeasure(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int next = d_ptr->indexToProcessMeasure + 1;
	if (next < 0 || next >= d_ptr->measures.size()) {
		return nullptr;
	}

	FrameData* data = &d_ptr->measures[next];
	if (data == nullptr || !data->isCompleted()) {
		return nullptr;
	}

	data->setStatus(FrameData::PROCESSING);
	d_ptr->indexToProcessMeasure = next;

	if (DEBUG_OUT) {
		DebugOut2() << "Frame data to process measure, index: " << d_ptr->indexToProcessMeasure << ", buffSize: " << d_ptr->measures.size();
	}
	return data;
}


FrameData * SigChain::FrameBuffer::getFrameDataToProcessEnface(void) const
{
	lock_guard<mutex> lock(getImpl().mutexBuffer);

	int index = getImpl().indexToProcessEnface;

	size_t size = getImpl().enfaces.size();
	if (size <= 0 || index >= size) {
		return nullptr;
	}

	FrameData* data = &getImpl().enfaces[index];

	// If frame data has been received, the index to be processed should be increased,
	// before returning it. 
	if (data->isCompleted()) {
		data->setStatus(FrameData::PROCESSING);
	}
	else if (data->isProcessing()) {

	}
	else if (data->isProcessed()) {
		int next = index + 1;
		if (next >= size) {
			next = 0;
		}
		data = &getImpl().enfaces[next];
		if (data->isCompleted() == false) {
			return nullptr;
		}
		else {
			getImpl().indexToProcessEnface = next;
			data->setStatus(FrameData::PROCESSING);
		}
	}
	else {
		return nullptr;
	}

	if (DEBUG_OUT) {
		DebugOut2() << "Frame data to process enface, index: " << getImpl().indexToProcessEnface;
	}
	return data;
}
