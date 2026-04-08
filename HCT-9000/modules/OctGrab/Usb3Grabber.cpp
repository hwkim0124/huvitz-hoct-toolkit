#include "stdafx.h"
#include "Usb3Grabber.h"

#include "UsbLineCamera.h"
#include "OctoPlusCamera.h"

#include "CppUtil2.h"
#include "OctDevice2.h"

#include <vector>
#include <memory>
#include <atomic>

using namespace OctGrab;
using namespace CppUtil;
using namespace OctDevice;
using namespace std;


struct Usb3Grabber::Usb3GrabberImpl
{
	bool initiated;
	atomic<bool> isAtSideOd;

	std::unique_ptr<UsbLineCamera> lineCamera = NULL;

	vector<int> previewBufferIds;
	vector<int> measureBufferIds;
	vector<int> enfaceBufferIds;
	vector<int> previewBufferXs;
	vector<int> previewBufferYs;
	vector<int> measureBufferXs;
	vector<int> measureBufferYs;
	vector<int> enfaceBufferXs;
	vector<int> enfaceBufferYs;

	int grabbedPreviewBufferIndex;
	int grabbedMeasureBufferIndex;
	int grabbedEnfaceBufferIndex;
	int startedMeasureBufferIndex;

	vector<unique_ptr<unsigned short[]>> previewBuffers;
	vector<unique_ptr<unsigned short[]>> measureBuffers;
	vector<unique_ptr<unsigned short[]>> enfaceBuffers;

	GrabPreviewBufferCallback* cbGrabPreviewBuffer;
	GrabMeasureBufferCallback* cbGrabMeasureBuffer;
	GrabEnfaceBufferCallback* cbGrabEnfaceBuffer;

	Usb3GrabberImpl(): initiated(false), isAtSideOd(true), grabbedPreviewBufferIndex(-1),
		cbGrabPreviewBuffer(nullptr), cbGrabMeasureBuffer(nullptr), cbGrabEnfaceBuffer(nullptr),
		grabbedMeasureBufferIndex(-1), grabbedEnfaceBufferIndex(-1),
		startedMeasureBufferIndex(-1) {

	}
};


Usb3Grabber::Usb3Grabber() :
	d_ptr(make_unique<Usb3GrabberImpl>())
{
	// Hamamatsu CCD USB camera.
	// getImpl().lineCamera = make_unique<SsdicCamera>();

	// E2V CMOS USB camera. 
	getImpl().lineCamera = make_unique<OctoPlusCamera>();
}


OctGrab::Usb3Grabber::~Usb3Grabber()
{

}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctGrab::Usb3Grabber::Usb3Grabber(Usb3Grabber && rhs) = default;
Usb3Grabber & OctGrab::Usb3Grabber::operator=(Usb3Grabber && rhs) = default;


Usb3Grabber::Usb3GrabberImpl & OctGrab::Usb3Grabber::getImpl(void) const
{
	return *d_ptr;
}



bool OctGrab::Usb3Grabber::initializeUsb3Grabber(void)
{
	auto* camera = getUsbLineCamera();
	if (camera->openLineCamera(this)) {
		setCameraSpeedToFastest();
		getImpl().initiated = true;
		return true;
	}
	return false;
}


void OctGrab::Usb3Grabber::releaseUsb3Grabber(void)
{
	auto* camera = getUsbLineCamera();
	if (camera->closeLineCamera()) {
		releaseBuffersToPattern();
	}
	getImpl().initiated = false;
	return;
}


bool OctGrab::Usb3Grabber::isInitiated(void) const
{
	return getImpl().initiated;
}


bool OctGrab::Usb3Grabber::startCameraAcquisition(bool init)
{
	auto* camera = getUsbLineCamera();
	bool rets = camera->startAcquisition(init);
	return rets;
}


void OctGrab::Usb3Grabber::cancelCameraAcquisition(void)
{
	auto* camera = getUsbLineCamera();
	camera->cancelAcquisition();
	return;
}


bool OctGrab::Usb3Grabber::setSourceImageSizeY(int size)
{
	if (!isInitiated()) {
		return false;
	}

	auto* camera = getUsbLineCamera();
	bool retv = camera->updateFrameHeight(size);
	return retv;
}


bool OctGrab::Usb3Grabber::setCamearaParameters(int imageSizeY, int numbOfBuffers)
{
	if (!isInitiated()) {
		return false;
	}

	auto* camera = getUsbLineCamera();
	bool retv = camera->updateFrameHeight(imageSizeY);
	return retv;
}

bool OctGrab::Usb3Grabber::setCameraSpeedToNormal(void)
{
	if (!isInitiated()) {
		return false;
	}

	auto* camera = getUsbLineCamera();
	bool retv = camera->setExposureToNormal();
	return retv;
}

bool OctGrab::Usb3Grabber::setCameraSpeedToFaster(void)
{
	if (!isInitiated()) {
		return false;
	}

	auto* camera = getUsbLineCamera();
	bool retv = camera->setExposureToFaster();
	return retv;
}

bool OctGrab::Usb3Grabber::setCameraSpeedToFastest(void)
{
	if (!isInitiated()) {
		return false;
	}

	auto* camera = getUsbLineCamera();
	bool retv = camera->setExposureToFastest();
	return retv;
}

bool OctGrab::Usb3Grabber::setLineCameraExposureTime(float expTime)
{
	auto* camera = getUsbLineCamera();
	auto ret = camera->setExposureTime(expTime);
	return ret;
}

bool OctGrab::Usb3Grabber::setLineCameraAnalogGain(float gain)
{
	auto* camera = getUsbLineCamera();
	if (camera) {
		return camera->setAnalogGain(gain);
	}
	return false;
}

float OctGrab::Usb3Grabber::getLineCameraExposureTime(void)
{
	auto* camera = getUsbLineCamera();
	auto expTime = camera->getExposureTime();
	return expTime;
}

float OctGrab::Usb3Grabber::getLineCameraAnalogGain(void)
{
	auto* camera = getUsbLineCamera();
	if (camera) {
		auto gain = camera->getAnalogGain();
		return gain;
	}
	return 0.0f;
}

bool OctGrab::Usb3Grabber::isAtSideOd(void)
{
	return getImpl().isAtSideOd;
}

EyeSide OctGrab::Usb3Grabber::getEyeSide(void)
{
	return (isAtSideOd() ? EyeSide::OD : EyeSide::OS);
}


bool OctGrab::Usb3Grabber::grabPreviewProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().previewBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().previewBufferIds[index + i];
		bufferIds.push_back(id);
	}

	if (!startCameraAcquisition(false)) {
		return false;
	}
	return true;
}


bool OctGrab::Usb3Grabber::grabPreviewProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().previewBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().previewBufferIds[index + i];
		bufferIds.push_back(id);
	}

	auto* camera = getUsbLineCamera();
	if (!camera->acquirePreviewImages(bufferIds)) {
		return false;
	}
	return true;
}


bool OctGrab::Usb3Grabber::grabMeasureProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().measureBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().measureBufferIds[index + i];
		bufferIds.push_back(id);
	}

	if (!startCameraAcquisition(false)) {
		return false;
	}
	return true;
}


bool OctGrab::Usb3Grabber::grabMeasureProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().measureBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().measureBufferIds[index + i];
		bufferIds.push_back(id);
	}

	auto* camera = getUsbLineCamera();
	if (!camera->acquireMeasureImages(bufferIds)) {
		return false;
	}

	//OctoPlus::cancelAcquisition();
	return true;
}


bool OctGrab::Usb3Grabber::grabEnfaceProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().enfaceBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().enfaceBufferIds[index + i];
		bufferIds.push_back(id);
	}

	if (!startCameraAcquisition(false)) {
		return false;
	}
	return true;
}


bool OctGrab::Usb3Grabber::grabEnfaceProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().enfaceBufferIds.size()) {
		return false;
	}

	vector<int> bufferIds;
	for (int i = 0; i < count; i++) {
		auto id = getImpl().enfaceBufferIds[index + i];
		bufferIds.push_back(id);
	}

	auto* camera = getUsbLineCamera();
	if (!camera->acquireEnfaceImages(bufferIds)) {
		return false;
	}

	//OctoPlus::cancelAcquisition();
	return true;
}


void OctGrab::Usb3Grabber::hookGrabPreviewBuffer(unsigned short * buff, int bufferId, size_t dataSize)
{
	int index = getBufferIndexOfPreviewImage(bufferId);

	if (index >= 0)
	{
		setGrabbedPreviewBuffer(index);
		int sizeX = getBufferSizeXOfPreviewImage(index);
		int sizeY = getBufferSizeYOfPreviewImage(index);
		unsigned short* bptr = getBufferOfPreviewImage(index);
		auto imageSize = sizeX * sizeY * sizeof(unsigned short);

		if (bptr != nullptr && buff != nullptr && imageSize == dataSize)
		{
			memcpy(bptr, buff, dataSize);
			hookGrabPreviewBufferFunction(bptr, sizeX, sizeY, index);
		}
	}
	return;
}


void OctGrab::Usb3Grabber::hookGrabMeasureBuffer(unsigned short * buff, int bufferId, size_t dataSize)
{
	int index = getBufferIndexOfMeasureImage(bufferId);

	// LogD() << "Hook grab buffer, bufferId: " << bufferId << ", buffIdx: " << index << ", dataSize: " << dataSize;

	if (index >= 0)
	{
		setGrabbedMeasureBuffer(index);
		int sizeX = getBufferSizeXOfMeasureImage(index);
		int sizeY = getBufferSizeYOfMeasureImage(index);
		unsigned short* bptr = getBufferOfMeasureImage(index);
		auto imageSize = sizeX * sizeY * sizeof(unsigned short);

		if (bptr != nullptr && buff != nullptr && imageSize == dataSize)
		{
			memcpy(bptr, buff, dataSize);
			hookGrabMeasureBufferFunction(bptr, sizeX, sizeY, index);
		}
	}
	return;
}


void OctGrab::Usb3Grabber::hookGrabEnfaceBuffer(unsigned short * buff, int bufferId, size_t dataSize)
{
	int index = getBufferIndexOfEnfaceImage(bufferId);

	if (index >= 0)
	{
		setGrabbedEnfaceBuffer(index);
		int sizeX = getBufferSizeXOfEnfaceImage(index);
		int sizeY = getBufferSizeYOfEnfaceImage(index);
		unsigned short* bptr = getBufferOfEnfaceImage(index);
		auto imageSize = sizeX * sizeY * sizeof(unsigned short);

		if (bptr != nullptr && buff != nullptr && imageSize == dataSize)
		{
			memcpy(bptr, buff, dataSize);
			hookGrabEnfaceBufferFunction(bptr, sizeX, sizeY, index);
		}
	}
	return;
}


bool OctGrab::Usb3Grabber::prepareBuffersToPattern(std::vector<int> previews, std::vector<int> enfaces, std::vector<int> measures)
{
	if (!isInitiated()) {
		return false;
	}

	releaseBuffersToPattern();

	if (!prepareBuffersToPreview(previews)) {
		return false;
	}
	if (!prepareBuffersToEnface(enfaces)) {
		return false;
	}
	if (!prepareBuffersToMeasure(measures)) {
		return false;
	}

	return true;
}


bool OctGrab::Usb3Grabber::prepareBuffersToPreview(std::vector<int> sizes)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToPreview();

	int count = 0;
	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);
		int buffId = count;

		auto buffer = make_unique<unsigned short[]>(sizeX * sizeY);
		getImpl().previewBuffers.push_back(move(buffer));

		getImpl().previewBufferIds.push_back(buffId);
		getImpl().previewBufferXs.push_back(sizeX);
		getImpl().previewBufferYs.push_back(sizeY);
		count++;
	}

	return true;
}


bool OctGrab::Usb3Grabber::prepareBuffersToMeasure(std::vector<int> sizes, int frameSize)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToMeasure();
	int buffId;
	int count = 0;
	vector<int> ids;

	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);

		if (frameSize == 0 || (count < frameSize)) {
			buffId = count;
			ids.push_back(buffId);
		}
		else {
			buffId = ids[count % frameSize];
		}

		auto buffer = make_unique<unsigned short[]>(sizeX * sizeY);
		getImpl().measureBuffers.push_back(move(buffer));

		getImpl().measureBufferIds.push_back(buffId);
		getImpl().measureBufferXs.push_back(sizeX);
		getImpl().measureBufferYs.push_back(sizeY);
		count++;
	}

	return true;
}


bool OctGrab::Usb3Grabber::prepareBuffersToEnface(std::vector<int> sizes)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToEnface();

	int count = 0;
	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);
		int buffId = count;

		auto buffer = make_unique<unsigned short[]>(sizeX * sizeY);
		getImpl().enfaceBuffers.push_back(move(buffer));
		
		getImpl().enfaceBufferIds.push_back(buffId);
		getImpl().enfaceBufferXs.push_back(sizeX);
		getImpl().enfaceBufferYs.push_back(sizeY);
		count++;
	}

	return true;
}


void OctGrab::Usb3Grabber::releaseBuffersToPreview(void)
{
	getImpl().previewBuffers.clear();
	getImpl().previewBufferIds.clear();
	getImpl().previewBufferXs.clear();
	getImpl().previewBufferYs.clear();
	return;
}


void OctGrab::Usb3Grabber::releaseBuffersToMeasure(void)
{
	getImpl().measureBuffers.clear();
	getImpl().measureBufferIds.clear();
	getImpl().measureBufferXs.clear();
	getImpl().measureBufferYs.clear();
	return;
}


void OctGrab::Usb3Grabber::releaseBuffersToEnface(void)
{
	getImpl().enfaceBuffers.clear();
	getImpl().enfaceBufferIds.clear();
	getImpl().enfaceBufferXs.clear();
	getImpl().enfaceBufferYs.clear();
	return;
}


void OctGrab::Usb3Grabber::releaseBuffersToPattern(void)
{
	if (!isInitiated()) {
		return;
	}

	releaseBuffersToPreview();
	releaseBuffersToMeasure();
	releaseBuffersToEnface();
	return;
}


void OctGrab::Usb3Grabber::clearGrabbedPreviewBuffer(void)
{
	setGrabbedPreviewBuffer(-1);
	return;
}


void OctGrab::Usb3Grabber::clearGrabbedMeasureBuffer(void)
{
	setGrabbedMeasureBuffer(-1);
	return;
}


void OctGrab::Usb3Grabber::clearGrabbedEnfaceBuffer(void)
{
	setGrabbedEnfaceBuffer(-1);
	return;
}


void OctGrab::Usb3Grabber::setGrabbedPreviewBuffer(int index)
{
	getImpl().grabbedPreviewBufferIndex = index;
	return;
}


void OctGrab::Usb3Grabber::setGrabbedMeasureBuffer(int index)
{
	getImpl().grabbedMeasureBufferIndex = index;
	return;
}


void OctGrab::Usb3Grabber::setGrabbedEnfaceBuffer(int index)
{
	getImpl().grabbedEnfaceBufferIndex = index;
	return;
}


int OctGrab::Usb3Grabber::getGrabbedPreviewBuffer(void)
{
	return getImpl().grabbedPreviewBufferIndex;
}


int OctGrab::Usb3Grabber::getGrabbedMeasureBuffer(void)
{
	return getImpl().grabbedMeasureBufferIndex;
}


int OctGrab::Usb3Grabber::getGrabbedEnfaceBuffer(void)
{
	return getImpl().grabbedEnfaceBufferIndex;
}

void OctGrab::Usb3Grabber::setStartMeasureBuffer(int index)
{
	getImpl().startedMeasureBufferIndex = index;
	return;
}

int OctGrab::Usb3Grabber::getStartMeasureBuffer(void)
{
	return getImpl().startedMeasureBufferIndex;
}

int OctGrab::Usb3Grabber::getBufferIndexOfPreviewImage(int buffId)
{
	for (int i = 0; i < getImpl().previewBufferIds.size(); i++) {
		if (getImpl().previewBufferIds[i] == buffId) {
			return i;
		}
	}
	return -1;
}

int OctGrab::Usb3Grabber::getBufferIndexOfMeasureImage(int buffId)
{
	int start = getStartMeasureBuffer();
	for (int i = start; i < getImpl().measureBufferIds.size(); i++) {
		if (getImpl().measureBufferIds[i] == buffId) {
			return i;
		}
	}
	return -1;
}

int OctGrab::Usb3Grabber::getBufferIndexOfEnfaceImage(int buffId)
{
	for (int i = 0; i < getImpl().enfaceBufferIds.size(); i++) {
		if (getImpl().enfaceBufferIds[i] == buffId) {
			return i;
		}
	}
	return -1;
}

unsigned short * OctGrab::Usb3Grabber::getBufferOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBuffers.size()) {
		return nullptr;
	}
	return getImpl().previewBuffers[index].get();
}

unsigned short * OctGrab::Usb3Grabber::getBufferOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBuffers.size()) {
		return nullptr;
	}
	return getImpl().measureBuffers[index].get();
}

unsigned short * OctGrab::Usb3Grabber::getBufferOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBuffers.size()) {
		return nullptr;
	}
	return getImpl().enfaceBuffers[index].get();
}


int OctGrab::Usb3Grabber::getBufferSizeXOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBufferXs.size()) {
		return 0;
	}
	return getImpl().previewBufferXs[index];
}


int OctGrab::Usb3Grabber::getBufferSizeXOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBufferXs.size()) {
		return 0;
	}
	return getImpl().measureBufferXs[index];
}


int OctGrab::Usb3Grabber::getBufferSizeXOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBufferXs.size()) {
		return 0;
	}
	return getImpl().enfaceBufferXs[index];
}


int OctGrab::Usb3Grabber::getBufferSizeYOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBufferYs.size()) {
		return 0;
	}
	return getImpl().previewBufferYs[index];
}


int OctGrab::Usb3Grabber::getBufferSizeYOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBufferYs.size()) {
		return 0;
	}
	return getImpl().measureBufferYs[index];
}


int OctGrab::Usb3Grabber::getBufferSizeYOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBufferYs.size()) {
		return 0;
	}
	return getImpl().enfaceBufferYs[index];
}

void OctGrab::Usb3Grabber::hookGrabPreviewBufferFunction(unsigned short * buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabPreviewBuffer != nullptr) {
		(*getImpl().cbGrabPreviewBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}

void OctGrab::Usb3Grabber::hookGrabMeasureBufferFunction(unsigned short * buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabMeasureBuffer != nullptr) {
		(*getImpl().cbGrabMeasureBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}

void OctGrab::Usb3Grabber::hookGrabEnfaceBufferFunction(unsigned short * buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabEnfaceBuffer != nullptr) {
		(*getImpl().cbGrabEnfaceBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}


void OctGrab::Usb3Grabber::setGrabPreviewBufferCallback(GrabPreviewBufferCallback * callback)
{
	getImpl().cbGrabPreviewBuffer = callback;
	return;
}


void OctGrab::Usb3Grabber::setGrabMeasureBufferCallback(GrabMeasureBufferCallback * callback)
{
	getImpl().cbGrabMeasureBuffer = callback;
	return;
}


void OctGrab::Usb3Grabber::setGrabEnfaceBufferCallback(GrabEnfaceBufferCallback * callback)
{
	getImpl().cbGrabEnfaceBuffer = callback;
	return;
}

UsbLineCamera * OctGrab::Usb3Grabber::getUsbLineCamera(void) const
{
	return getImpl().lineCamera.get();
}
