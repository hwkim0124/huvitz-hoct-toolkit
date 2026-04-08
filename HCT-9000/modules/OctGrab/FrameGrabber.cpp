#include "stdafx.h"
#include "FrameGrabber.h"

#include "CppUtil2.h"
#include "OctDevice2.h"

#include <vector>
#include <memory>
#include <atomic>

using namespace OctGrab;
using namespace CppUtil;
using namespace OctDevice;
using namespace std;


struct FrameGrabber::FrameGrabberImpl
{
	bool initiated;

	atomic<bool> isAtSideOd;
	atomic<bool> eventWorking;
	thread worker;

	MIL_ID midApplication;
	MIL_ID midSystem;
	MIL_ID midDigitizer;

	vector<MIL_ID> previewBufferIds;
	vector<MIL_ID> measureBufferIds;
	vector<MIL_ID> enfaceBufferIds;
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

	JoystickEventCallback* cbJoystickEvent;
	EyeSideEventCallback* cbEyeSideEvent;
	OptimizeKeyEventCallback* cbOptimizeEvent;
	BacklightOnEventCallback* cbBacklightOnEvent;

	GrabPreviewBufferCallback* cbGrabPreviewBuffer;
	GrabMeasureBufferCallback* cbGrabMeasureBuffer;
	GrabEnfaceBufferCallback* cbGrabEnfaceBuffer;

	MainBoard* board;

	FrameGrabberImpl() : midApplication(M_NULL), midSystem(M_NULL), midDigitizer(M_NULL), 
		cbJoystickEvent(nullptr), cbEyeSideEvent(nullptr), cbOptimizeEvent(nullptr), 
		cbBacklightOnEvent(nullptr),
		cbGrabPreviewBuffer(nullptr), cbGrabMeasureBuffer(nullptr), cbGrabEnfaceBuffer(nullptr),
		initiated(false), isAtSideOd(true), eventWorking(false), board(nullptr),
		grabbedPreviewBufferIndex(-1), grabbedMeasureBufferIndex(-1), grabbedEnfaceBufferIndex(-1), 
		startedMeasureBufferIndex(-1)
	{
	}
};


FrameGrabber::FrameGrabber() :
	d_ptr(make_unique<FrameGrabberImpl>())
{
}


OctGrab::FrameGrabber::~FrameGrabber()
{

}

// Copy constructor and assignment by const reference are not allowed here 
// becuase the innert implementation struct has unique_ptr member for exclusive-ownership.
// It needs additional defintion of copy constructor and assignment for the struct to resolve 
// the situation. 
OctGrab::FrameGrabber::FrameGrabber(FrameGrabber && rhs) = default;
FrameGrabber & OctGrab::FrameGrabber::operator=(FrameGrabber && rhs) = default;



void OctGrab::FrameGrabber::initFrameGrabber(OctDevice::MainBoard * board)
{
	d_ptr->board = board;
	return;
}


bool OctGrab::FrameGrabber::startFrameGrabber(const std::wstring & dcf_filename)
{
	if (!d_ptr->board) {
		return false;
	}

	releaseFrameGrabber(false);

	if (!allocateMilSystemByDCFfile(dcf_filename)) {
		LogE() << "MIL system allocate failed!, dcf path: " << wtoa(dcf_filename);
		return false;
	}

	// MappControl(M_ERROR, M_PRINT_ENABLE);
	getImpl().initiated = true;

	updateDigitizerSettings();
	if (!GlobalSettings::useNewGpioRutine()) {
		startEventWorkerThread();
	}
	return true;
}


void OctGrab::FrameGrabber::releaseFrameGrabber(bool reset)
{
	closeEventWorkerThread();
	releaseBuffersToPattern();
	releaseMilSystem(reset);
	getImpl().initiated = false;
	return;
}


bool OctGrab::FrameGrabber::allocateMilSystemByDCFfile(const std::wstring & dcf_filename)
{
	if (getImpl().midApplication == M_NULL) {
		if (MappAlloc(M_QUIET, &getImpl().midApplication) == M_NULL) {
			return false;
		}
	}

	MappControl(M_ERROR, M_PRINT_DISABLE);
	MappControl(M_MEMORY, M_COMPENSATION_ENABLE);
	MappControl(M_TRACE, M_PRINT_DISABLE);
	MappControl(M_TRACE, M_LOG_DISABLE);

	if (getImpl().midSystem == M_NULL) {
		if (MsysAlloc(getImpl().midApplication, M_SYSTEM_SOLIOS, M_DEV0, M_DEFAULT, &getImpl().midSystem) == M_NULL) {
			return false;
		}
	}

	if (MdigAlloc(getImpl().midSystem, M_DEV0, dcf_filename.c_str(), M_DEFAULT, &getImpl().midDigitizer) == M_NULL) {
		return false;
	}
	return true;
}


void OctGrab::FrameGrabber::releaseMilSystem(bool reset)
{
	if (!isInitiated()) {
		return;
	}

	if (getImpl().midDigitizer != M_NULL) {
		MdigFree(getImpl().midDigitizer);
		getImpl().midDigitizer = M_NULL;
	}

	if (reset) {
		if (getImpl().midSystem != M_NULL) {
			MsysFree(getImpl().midSystem);
			getImpl().midSystem = M_NULL;
		}

		if (getImpl().midApplication != M_NULL) {
			MappFree(getImpl().midApplication);
			getImpl().midApplication = M_NULL;
		}
	}
	return;
}


bool OctGrab::FrameGrabber::isInitiated(void) const
{
	/*
	if (getImpl().midApplication != M_NULL &&
	getImpl().midSystem != M_NULL &&
	getImpl().midDigitizer != M_NULL) {
	return true;
	}
	return false;
	*/
	return getImpl().initiated;
}


MIL_INT MFTYPE hookWrapperJoystick(MIL_INT hookType, MIL_ID evtId, void *obj)
{
	FrameGrabber *grab = (FrameGrabber*)obj;
	grab->hookJoystickFunction();
	return 0;
}


void OctGrab::FrameGrabber::hookJoystickFunction(void)
{
	MIL_INT val;

	for (int i = 0; i < JOYSTICK_PRESSED_EVENT_COUNT; i++) {
		MdigInquire(getImpl().midDigitizer, M_IO_STATUS + M_AUX_IO11, &val);
		if (val == 0) {
			return;
		}
		this_thread::sleep_for(chrono::milliseconds(JOYSTICK_PRESSED_EVENT_DELAY));
	}

	if (!MainPlatform::isBacklightOn()) {
		MainPlatform::turnOnBacklight();
		if (d_ptr->cbBacklightOnEvent) {
			(*d_ptr->cbBacklightOnEvent)();
		}
	}
	else {
		if (getImpl().cbJoystickEvent != nullptr) {
			(*getImpl().cbJoystickEvent)();
		}
	}
	return;
}


bool OctGrab::FrameGrabber::updateDigitizerSettings(void)
{
	if (!isInitiated()) {
		return false;
	}

	MdigControl(getImpl().midDigitizer, M_GRAB_MODE, M_ASYNCHRONOUS);
	MdigControl(getImpl().midDigitizer, M_GRAB_DIRECTION_X, M_FORWARD);
	MdigControl(getImpl().midDigitizer, M_GRAB_DIRECTION_Y, M_FORWARD);// M_REVERSE);
	MdigControl(getImpl().midDigitizer, M_GRAB_TIMEOUT, 5000);

	// Joystick settings.
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	//MdigControl(mdig, M_LVDS + M_AUX_IO5, M_ENABLE);
	MdigControl(getImpl().midDigitizer, M_IO_MODE + M_AUX_IO11, M_INPUT);
	MdigControl(getImpl().midDigitizer, M_IO_INTERRUPT_STATE + M_AUX_IO11, M_ENABLE);
	MdigHookFunction(getImpl().midDigitizer, M_IO_CHANGE/* + M_AUX_IO5*/, hookWrapperJoystick, (void*)this);

	// Enable IO port for Eye Side.
	////////////////////////////////////////////////////////////////////////////////////////////////////////
	MdigControl(getImpl().midDigitizer, M_IO_MODE + M_AUX_IO13, M_INPUT);
	MdigControl(getImpl().midDigitizer, M_IO_INTERRUPT_STATE + M_AUX_IO13, M_ENABLE);

	MIL_INT val;
	MdigInquire(getImpl().midDigitizer, M_IO_STATUS_ALL, &val);
	//MdigControl(mdig, M_USER_BIT_MODE + 11, M_INPUT);
	//MdigControl(mdig, M_USER_BIT_INTERRUPT_STATE + 11, M_ENABLE);
	//MdigHookFunction(mdig, M_USER_BIT_CHANGE, hook_wrapper_joystick, (void*)this);
	return true;
}


bool OctGrab::FrameGrabber::updateCurrentEyeSide(void)
{
	if (!isInitiated()) {
		return false;
	}

	MIL_INT valDigi = 0;
	int result = 0;

	static int memDigi = -1;
	static int memSusi = -1;
	static bool useDigi = true;
	static bool useSusi = true;

	// OPTO_AUX_0 : 13
	// MdigInquire(getImpl().midDigitizer, M_USER_BIT_VALUE + 13, &val);
	MdigInquire(getImpl().midDigitizer, M_IO_STATUS + M_AUX_IO7, &valDigi);

	/*
	int valSusi = MainPlatform::getCurrentEyeSide();
	if (valSusi < 0) {
		result = valDigi;
	}
	else {
		if (valDigi != valSusi) {
			if (memDigi >= 0 && memSusi >= 0) {
				if (useDigi && useSusi) {
					if (memDigi != valDigi) {
						useDigi = true;
						useSusi = false;
					}
					if (memSusi != valSusi) {
						useDigi = false;
						useSusi = true;
					}
				}
			}
			result = (useDigi ? valDigi : valSusi);
		}
		else {
			result = valDigi;
		}
	}

	memDigi = valDigi;
	memSusi = valSusi;
	*/

	// if (result != LEFT_SIDE_IO_VALUE) {
	if (valDigi != LEFT_SIDE_IO_VALUE) {
		if (getImpl().isAtSideOd) {
			getImpl().isAtSideOd = false;
			if (getImpl().cbEyeSideEvent) {
				(*getImpl().cbEyeSideEvent)(false);
			}
		}
	}
	else {
		if (!getImpl().isAtSideOd) {
			getImpl().isAtSideOd = true;
			if (getImpl().cbEyeSideEvent) {
				(*getImpl().cbEyeSideEvent)(true);
			}
		}
	}
	return true;
}


bool OctGrab::FrameGrabber::updateDeviceStatus(void)
{
	if (!isInitiated()) {
		return false;
	}

	MIL_INT val;
	MdigInquire(getImpl().midDigitizer, M_IO_STATUS + M_AUX_IO6, &val);

	if (val != 0)
	{
		UsbComm& usbComm = d_ptr->board->getUsbComm();
		if (usbComm.UpdateDeviceStatus(1)) {
			uint32_t status;
			if (usbComm.Interrupt_ReadStatus(d_ptr->board->getBaseAddressOfGPStatus(), &status)) {
				if (status & 0x04) {
					DebugOut2() << "Optimized key pressed event occurred!";
					if (!MainPlatform::isBacklightOn()) {
						MainPlatform::turnOnBacklight();
						if (d_ptr->cbBacklightOnEvent) {
							(*d_ptr->cbBacklightOnEvent)();
						}
					}
					else {
						if (d_ptr->cbOptimizeEvent) {
							(*d_ptr->cbOptimizeEvent)();
						}
					}
				}
			}
		}
	}
	return true;
}


bool OctGrab::FrameGrabber::isCameraConnected(void) const
{
	if (!isInitiated()) {
		return false;
	}

	if (MdigInquire(getImpl().midDigitizer, M_CAMERA_PRESENT, M_NULL) == M_YES) {
		return true;
	}

	return false;
}


bool OctGrab::FrameGrabber::isAtSideOd(void)
{
	return getImpl().isAtSideOd;
}


EyeSide OctGrab::FrameGrabber::getEyeSide(void)
{
	return (isAtSideOd() ? EyeSide::OD : EyeSide::OS) ;
}


void OctGrab::FrameGrabber::testFunction(void)
{
	if (!isInitiated()) {
		return ;
	}

	MIL_UINT64 value;
	MdigInquire(getImpl().midDigitizer, M_EXPOSURE_TIME, &value);
	MdigInquire(getImpl().midDigitizer, M_EXPOSURE_DELAY, &value);
	MdigInquire(getImpl().midDigitizer, M_EXPOSURE_TIME + M_TIMER2, &value);
	MdigInquire(getImpl().midDigitizer, M_EXPOSURE_DELAY + M_TIMER2, &value);
	return;
}


bool OctGrab::FrameGrabber::prepareBuffersToPattern(std::vector<int> previews, std::vector<int> enfaces, std::vector<int> measures)
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


bool OctGrab::FrameGrabber::prepareBuffersToPreview(std::vector<int> sizes)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToPreview();

	int count = 0;
	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);
		MIL_ID buffId;

		MbufAlloc2d(getImpl().midSystem, sizeX, sizeY, 16L + M_UNSIGNED, M_IMAGE + M_GRAB  /*+ M_PROC +  M_HOST_MEMORY*/, &buffId);
		if (buffId == M_NULL) {
			LogE() << "Failed allocation of MIL buffer to preview, count: " << count ;
			return false;
		}

		LogD() << "MIL buffer to preview allocated, sizeX: " << sizeX << ", sizeY: " << sizeY << ", index: " << count;
		MbufClear(buffId, 0L);

		auto buffer = make_unique<unsigned short[]>(sizeX * sizeY);
		getImpl().previewBuffers.push_back(move(buffer));

		getImpl().previewBufferIds.push_back(buffId);
		getImpl().previewBufferXs.push_back(sizeX);
		getImpl().previewBufferYs.push_back(sizeY);
		count++;
	}

	return true;
}


bool OctGrab::FrameGrabber::prepareBuffersToMeasure(std::vector<int> sizes, int frameSize)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToMeasure();
	MIL_ID buffId;
	int count = 0;
	vector<MIL_ID> ids;

	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);

		if (frameSize == 0 || (count < frameSize)) {
			MbufAlloc2d(getImpl().midSystem, sizeX, sizeY, 16L + M_UNSIGNED, M_IMAGE + M_GRAB  /* + M_PROC +  M_HOST_MEMORY*/, &buffId);
			if (buffId == M_NULL) {
				LogE() << "Failed allocation of MIL buffer to measure, count: " << count;
				return false;
			}

			LogD() << "MIL buffer to measure allocated, sizeX: " << sizeX << ", sizeY: " << sizeY << ", buffId: " << buffId << ", index: " << count << ", frameSize: " << frameSize;
			MbufClear(buffId, 0L);
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


bool OctGrab::FrameGrabber::prepareBuffersToEnface(std::vector<int> sizes)
{
	if (!isInitiated()) {
		return false;
	}

	// releaseBuffersToEnface();

	int count = 0;
	for (auto size : sizes) {
		int sizeX = GRABBER_BUFFER_X_SIZE;
		int sizeY = size; // line.getNumberOfScanPoints(true);
		MIL_ID buffId;

		MbufAlloc2d(getImpl().midSystem, sizeX, sizeY, 16L + M_UNSIGNED, M_IMAGE + M_GRAB  /* + M_PROC +  M_HOST_MEMORY*/, &buffId);
		if (buffId == M_NULL) {
			LogE() << "Failed allocation of MIL buffer to enface, count: " << count;
			return false;
		}

		LogD() << "MIL buffer to enface allocated, sizeX: " << sizeX << ", sizeY: " << sizeY << ", index: " << count;
		MbufClear(buffId, 0L);

		auto buffer = make_unique<unsigned short[]>(sizeX * sizeY);
		getImpl().enfaceBuffers.push_back(move(buffer));

		getImpl().enfaceBufferIds.push_back(buffId);
		getImpl().enfaceBufferXs.push_back(sizeX);
		getImpl().enfaceBufferYs.push_back(sizeY);
		count++;
	}

	return true;
}


void OctGrab::FrameGrabber::releaseBuffersToPreview(void)
{
	for (auto buffId : getImpl().previewBufferIds) {
		if (buffId != M_NULL) {
			MbufFree(buffId);
			buffId = M_NULL;
		}
	}

	getImpl().previewBufferIds.clear();
	getImpl().previewBuffers.clear();
	getImpl().previewBufferXs.clear();
	getImpl().previewBufferYs.clear();

	DebugOut2() << "MIL buffer to preview released";
	return;
}


void OctGrab::FrameGrabber::releaseBuffersToMeasure(void)
{
	for (auto buffId : getImpl().measureBufferIds) {
		if (buffId != M_NULL) {
			MbufFree(buffId);
			buffId = M_NULL;
		}
	}

	getImpl().measureBufferIds.clear();
	getImpl().measureBuffers.clear();
	getImpl().measureBufferXs.clear();
	getImpl().measureBufferYs.clear();

	DebugOut2() << "MIL buffer to measure released";
	return;
}


void OctGrab::FrameGrabber::releaseBuffersToEnface(void)
{
	for (auto buffId : getImpl().enfaceBufferIds) {
		if (buffId != M_NULL) {
			MbufFree(buffId);
			buffId = M_NULL;
		}
	}

	getImpl().enfaceBufferIds.clear();
	getImpl().enfaceBuffers.clear();
	getImpl().enfaceBufferXs.clear();
	getImpl().enfaceBufferYs.clear();

	DebugOut2() << "MIL buffer to enface released";
	return;
}


void OctGrab::FrameGrabber::releaseBuffersToPattern(void)
{
	if (!isInitiated()) {
		return ;
	}

	releaseBuffersToPreview();
	releaseBuffersToMeasure();
	releaseBuffersToEnface();
	return;
}


void OctGrab::FrameGrabber::hookGrabPreviewBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabPreviewBuffer != nullptr) {
		(*getImpl().cbGrabPreviewBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}


void OctGrab::FrameGrabber::hookGrabMeasureBufferFunction(unsigned short* buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabMeasureBuffer != nullptr) {
		(*getImpl().cbGrabMeasureBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}


void OctGrab::FrameGrabber::hookGrabEnfaceBufferFunction(unsigned short * buff, int sizeX, int sizeY, int index)
{
	if (getImpl().cbGrabEnfaceBuffer != nullptr) {
		(*getImpl().cbGrabEnfaceBuffer)(buff, sizeX, sizeY, index);
	}
	return;
}


void OctGrab::FrameGrabber::clearGrabbedPreviewBuffer(void)
{
	setGrabbedPreviewBuffer(-1);
	return;
}


void OctGrab::FrameGrabber::clearGrabbedMeasureBuffer(void)
{
	setGrabbedMeasureBuffer(-1);
	return;
}


void OctGrab::FrameGrabber::clearGrabbedEnfaceBuffer(void)
{
	setGrabbedEnfaceBuffer(-1);
	return;
}


void OctGrab::FrameGrabber::setGrabbedPreviewBuffer(int index)
{
	getImpl().grabbedPreviewBufferIndex = index;
	return;
}


void OctGrab::FrameGrabber::setGrabbedMeasureBuffer(int index)
{
	getImpl().grabbedMeasureBufferIndex = index;
	return;
}


void OctGrab::FrameGrabber::setGrabbedEnfaceBuffer(int index)
{
	getImpl().grabbedEnfaceBufferIndex = index;
	return;
}


int OctGrab::FrameGrabber::getGrabbedPreviewBuffer(void)
{
	return getImpl().grabbedPreviewBufferIndex;
}


int OctGrab::FrameGrabber::getGrabbedMeasureBuffer(void)
{
	return getImpl().grabbedMeasureBufferIndex;
}


int OctGrab::FrameGrabber::getGrabbedEnfaceBuffer(void)
{
	return getImpl().grabbedEnfaceBufferIndex;
}

void OctGrab::FrameGrabber::setStartMeasureBuffer(int index)
{
	getImpl().startedMeasureBufferIndex = index;
	return;
}

int OctGrab::FrameGrabber::getStartMeasureBuffer(void)
{
	return getImpl().startedMeasureBufferIndex;
}


bool OctGrab::FrameGrabber::waitForFirstGrabOfPreviewBuffer(int firstIdx)
{
	for (int retry = 0; retry < GRAB_FIRST_RETRY_COUNT_MAX; retry++) {
		int grabbedIdx = getGrabbedPreviewBuffer();
		if (grabbedIdx >= firstIdx) {
			return true;
		}
		this_thread::sleep_for(chrono::milliseconds(GRAB_FIRST_RETRY_DELAY_TIME));
	}
	return false;
}


bool OctGrab::FrameGrabber::waitForFirstGrabOfMeasureBuffer(int firstIdx)
{
	for (int retry = 0; retry < (GRAB_FIRST_RETRY_COUNT_MAX*10); retry++) {
		int grabbedIdx = getGrabbedMeasureBuffer();
		if (grabbedIdx >= firstIdx) {
			return true;
		}
		this_thread::sleep_for(chrono::milliseconds(GRAB_FIRST_RETRY_DELAY_TIME));
	}
	return false;
}


bool OctGrab::FrameGrabber::waitForFirstGrabOfEnfaceBuffer(int firstIdx)
{
	for (int retry = 0; retry < GRAB_FIRST_RETRY_COUNT_MAX; retry++) {
		int grabbedIdx = getGrabbedEnfaceBuffer();
		if (grabbedIdx >= firstIdx) {
			return true;
		}
		this_thread::sleep_for(chrono::milliseconds(GRAB_FIRST_RETRY_DELAY_TIME));
	}
	return false;
}


MIL_INT MFTYPE hookGrabPreviewBuffer(MIL_INT hookType, MIL_ID hookId, void* hookDataPtr)
{
	MIL_ID bufferId;

	MdigGetHookInfo(hookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &bufferId);

	FrameGrabber *grab = (FrameGrabber*)hookDataPtr;
	int index = grab->getBufferIndexOfPreviewImage(bufferId);
	if (index >= 0)
	{
		grab->setGrabbedPreviewBuffer(index);

		int sizeX = grab->getBufferSizeXOfPreviewImage(index);
		int sizeY = grab->getBufferSizeYOfPreviewImage(index);
		unsigned short* bptr = grab->getBufferOfPreviewImage(index);

		if (bptr != nullptr) 
		{
			// DebugOut2() << "Hook grab preview buffer: " << index << ", buffer id: " << bufferId;
			MbufGet2d(bufferId, 0, 0, sizeX, sizeY, bptr);
			grab->hookGrabPreviewBufferFunction(bptr, sizeX, sizeY, index);
		}
	}

	// Upon successful completion
	return M_NULL;
}


bool OctGrab::FrameGrabber::grabPreviewProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().previewBufferIds.size()) {
		return false;
	}

	MIL_ID buffIds[256] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().previewBufferIds[index + i];
		if (id == M_NULL) {
			return false;
		}
		else {
			buffIds[i] = id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, hookGrabPreviewBuffer, (void*)this);
	// MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, M_NULL, M_NULL);
	return true;
}


bool OctGrab::FrameGrabber::grabPreviewProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count <= 0 || (index + count) > getImpl().previewBufferIds.size()) {
		return false;
	}

	MIL_ID buffIds[256] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().previewBufferIds[index + i];
		if (id == M_NULL) {
			return false;
		}
		else {
			buffIds[i] = id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	// Additional call to free internally allocated resources. 
	// MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, NULL, NULL);
	MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, hookGrabPreviewBuffer, (void*)this);
	return true;
}


MIL_INT MFTYPE hookGrabMeasureBuffer(MIL_INT hookType, MIL_ID hookId, void* hookDataPtr)
{
	MIL_ID bufferId;
	MdigGetHookInfo(hookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &bufferId);

	FrameGrabber *grab = (FrameGrabber*)hookDataPtr;
	int index = grab->getBufferIndexOfMeasureImage(bufferId);

	LogD() << "Hook grab buffer, bufferId: " << bufferId << ", buffIdx: " << index;

	if (index >= 0)
	{
		grab->setGrabbedMeasureBuffer(index);
		int sizeX = grab->getBufferSizeXOfMeasureImage(index);
		int sizeY = grab->getBufferSizeYOfMeasureImage(index);
		unsigned short* bptr = grab->getBufferOfMeasureImage(index);

		if (bptr != nullptr)
		{
			// DebugOut2() << "Hook grab measure buffer: " << index << ", buffer id: " << bufferId;
			MbufGet2d(bufferId, 0, 0, sizeX, sizeY, bptr);
			// ClockTimer::start();
			grab->hookGrabMeasureBufferFunction(bptr, sizeX, sizeY, index);
			// double msec = CppUtil::ClockTimer::elapsedMsec();
			// DebugOut2() << "Grab measure elapsed: " << msec;
		}
	}

	// Upon successful completion
	return M_NULL;
}



bool OctGrab::FrameGrabber::grabMeasureProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count < 0 || (index + count) > getImpl().measureBufferIds.size()) {
		return false;
	}

	static MIL_ID buffIds[4096] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().measureBufferIds[index + i];
		if (id == M_NULL) {
			LogD() << "Grab measure, invalid MILL buffer id at frame " << i;
			return false;
		}
		else {
			buffIds[i] = id;
			// DebugOut2() << "Grab MIL buffer id: " << id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	LogD() << "Grab measure, MIL buffer count: " << count;
	MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, hookGrabMeasureBuffer, (void*)this);
	// MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, M_NULL, M_NULL);
	return true;
}


bool OctGrab::FrameGrabber::grabMeasureProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count < 0 || (index + count) > getImpl().measureBufferIds.size()) {
		return false;
	}

	static MIL_ID buffIds[4096] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().measureBufferIds[index + i];
		if (id == M_NULL) {
			LogD() << "Grab measure, invalid MILL buffer id at frame " << i;
			return false;
		}
		else {
			buffIds[i] = id;
			// DebugOut2() << "Wait MIL buffer id: " << id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	// Additional call to free internally allocated resources. 
	// MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, NULL, NULL);
	MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, hookGrabMeasureBuffer, (void*)this);
	return true;
}


MIL_INT MFTYPE hookGrabEnfaceBuffer(MIL_INT hookType, MIL_ID hookId, void* hookDataPtr)
{
	MIL_ID bufferId;

	MdigGetHookInfo(hookId, M_MODIFIED_BUFFER + M_BUFFER_ID, &bufferId);

	FrameGrabber *grab = (FrameGrabber*)hookDataPtr;
	int index = grab->getBufferIndexOfEnfaceImage(bufferId);
	if (index >= 0)
	{
		grab->setGrabbedEnfaceBuffer(index);

		int sizeX = grab->getBufferSizeXOfEnfaceImage(index);
		int sizeY = grab->getBufferSizeYOfEnfaceImage(index);
		unsigned short* bptr = grab->getBufferOfEnfaceImage(index);

		if (bptr != nullptr)
		{
			// DebugOut2() << "Hook grab enface buffer: " << index << ", buffer id: " << bufferId;
			//CppUtil::ClockTimer::start();
			MbufGet2d(bufferId, 0, 0, sizeX, sizeY, bptr);
			//double msec = CppUtil::ClockTimer::elapsedMsec();
			//DebugOut2() << "get: " << msec;
			grab->hookGrabEnfaceBufferFunction(bptr, sizeX, sizeY, index);
			//msec = CppUtil::ClockTimer::elapsedMsec();
			//DebugOut2() << "hook: " << msec;
		}
	}

	// Upon successful completion
	return M_NULL;
}


bool OctGrab::FrameGrabber::grabEnfaceProcess(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count < 0 || (index + count) > getImpl().enfaceBufferIds.size()) {
		return false;
	}

	MIL_ID buffIds[256] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().enfaceBufferIds[index + i];
		if (id == M_NULL) {
			return false;
		}
		else {
			buffIds[i] = id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, hookGrabEnfaceBuffer, (void*)this);
	//MdigProcess(getImpl().midDigitizer, buffIds, count, M_SEQUENCE + M_COUNT(count), M_ASYNCHRONOUS, M_NULL, M_NULL);
	return true;
}


bool OctGrab::FrameGrabber::grabEnfaceProcessWait(int index, int count)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || count < 0 || (index + count) > getImpl().enfaceBufferIds.size()) {
		return false;
	}

	MIL_ID buffIds[256] = { M_NULL };
	for (int i = 0; i < count; i++) {
		MIL_ID id = getImpl().enfaceBufferIds[index + i];
		if (id == M_NULL) {
			return false;
		}
		else {
			buffIds[i] = id;
		}
	}

	if (getImpl().midDigitizer == M_NULL) {
		return false;
	}

	// Additional call to free internally allocated resources. 
	//MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, NULL, NULL);
	MdigProcess(getImpl().midDigitizer, buffIds, count, M_STOP + M_WAIT, M_DEFAULT, hookGrabEnfaceBuffer, (void*)this);
	return true;
}


bool OctGrab::FrameGrabber::copyBufferOfPreviewImage(int index)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || index > getImpl().previewBufferIds.size()) {
		return false;
	}

	MIL_ID buffId = getImpl().previewBufferIds[index];
	if (buffId == M_NULL) {
		return false;
	}

	int sizeX = getImpl().previewBufferXs[index];
	int sizeY = getImpl().previewBufferYs[index];
	MbufGet2d(buffId, 0, 0, sizeX, sizeY, getImpl().previewBuffers[index].get());
	return true;
}


bool OctGrab::FrameGrabber::copyBufferOfMeasureImage(int index)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || index > getImpl().measureBufferIds.size()) {
		return false;
	}

	MIL_ID buffId = getImpl().measureBufferIds[index];
	if (buffId == M_NULL) {
		return false;
	}
	
	int sizeX = getImpl().measureBufferXs[index];
	int sizeY = getImpl().measureBufferYs[index];
	MbufGet2d(buffId, 0, 0, sizeX, sizeY, getImpl().measureBuffers[index].get());
	return true;
}


bool OctGrab::FrameGrabber::copyBufferOfEnfaceImage(int index)
{
	if (!isInitiated()) {
		return false;
	}

	if (index < 0 || index > getImpl().enfaceBufferIds.size()) {
		return false;
	}

	MIL_ID buffId = getImpl().enfaceBufferIds[index];
	if (buffId == M_NULL) {
		return false;
	}

	int sizeX = getImpl().enfaceBufferXs[index];
	int sizeY = getImpl().enfaceBufferYs[index];
	MbufGet2d(buffId, 0, 0, sizeX, sizeY, getImpl().enfaceBuffers[index].get());
	return true;
}


bool OctGrab::FrameGrabber::setSourceImageSizeY(int size)
{
	if (!isInitiated()) {
		return false;
	}

	MdigControl(getImpl().midDigitizer, M_SOURCE_SIZE_Y, size);
	return true;
}


int OctGrab::FrameGrabber::getBufferIndexOfPreviewImage(std::uint64_t milBuffId)
{
	for (int i = 0; i < getImpl().previewBufferIds.size(); i++) {
		if (getImpl().previewBufferIds[i] == milBuffId) {
			return i;
		}
	}
	return -1;
}


int OctGrab::FrameGrabber::getBufferIndexOfMeasureImage(std::uint64_t milBuffId)
{
	int start = getStartMeasureBuffer();
	for (int i = start; i < getImpl().measureBufferIds.size(); i++) {
		if (getImpl().measureBufferIds[i] == milBuffId) {
			return i;
		}
	}
	return -1;
}


int OctGrab::FrameGrabber::getBufferIndexOfEnfaceImage(std::uint64_t milBuffId)
{
	for (int i = 0; i < getImpl().enfaceBufferIds.size(); i++) {
		if (getImpl().enfaceBufferIds[i] == milBuffId) {
			return i;
		}
	}
	return -1;
}


int OctGrab::FrameGrabber::getBufferSizeXOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBufferXs.size()) {
		return 0;
	}
	return getImpl().previewBufferXs[index];
}


int OctGrab::FrameGrabber::getBufferSizeXOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBufferXs.size()) {
		return 0;
	}
	return getImpl().measureBufferXs[index];
}


int OctGrab::FrameGrabber::getBufferSizeXOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBufferXs.size()) {
		return 0;
	}
	return getImpl().enfaceBufferXs[index];
}


int OctGrab::FrameGrabber::getBufferSizeYOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBufferYs.size()) {
		return 0;
	}
	return getImpl().previewBufferYs[index];
}


int OctGrab::FrameGrabber::getBufferSizeYOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBufferYs.size()) {
		return 0;
	}
	return getImpl().measureBufferYs[index];
}


int OctGrab::FrameGrabber::getBufferSizeYOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBufferYs.size()) {
		return 0;
	}
	return getImpl().enfaceBufferYs[index];
}


unsigned short * OctGrab::FrameGrabber::getBufferOfPreviewImage(int index)
{
	if (index < 0 || index >= getImpl().previewBuffers.size()) {
		return nullptr;
	}
	return getImpl().previewBuffers[index].get();
}


unsigned short * OctGrab::FrameGrabber::getBufferOfMeasureImage(int index)
{
	if (index < 0 || index >= getImpl().measureBuffers.size()) {
		return nullptr;
	}
	return getImpl().measureBuffers[index].get();
}


unsigned short * OctGrab::FrameGrabber::getBufferOfEnfaceImage(int index)
{
	if (index < 0 || index >= getImpl().enfaceBuffers.size()) {
		return nullptr;
	}
	return getImpl().enfaceBuffers[index].get();
}


void OctGrab::FrameGrabber::setJoystickEventCallback(JoystickEventCallback * callback)
{
	getImpl().cbJoystickEvent = callback;
	return;
}


void OctGrab::FrameGrabber::setEyeSideEventCallback(EyeSideEventCallback * callback)
{
	getImpl().cbEyeSideEvent = callback;
	return;
}


void OctGrab::FrameGrabber::setOptimizeKeyEventCallback(OptimizeKeyEventCallback * callback)
{
	getImpl().cbOptimizeEvent = callback;
	return;
}


void OctGrab::FrameGrabber::setBacklightOnEventCallback(BacklightOnEventCallback * callback)
{
	getImpl().cbBacklightOnEvent = callback;
	return;
}


void OctGrab::FrameGrabber::setGrabPreviewBufferCallback(GrabPreviewBufferCallback * callback)
{
	getImpl().cbGrabPreviewBuffer = callback;
	return;
}


void OctGrab::FrameGrabber::setGrabMeasureBufferCallback(GrabMeasureBufferCallback * callback)
{
	getImpl().cbGrabMeasureBuffer = callback;
	return;
}


void OctGrab::FrameGrabber::setGrabEnfaceBufferCallback(GrabEnfaceBufferCallback * callback)
{
	getImpl().cbGrabEnfaceBuffer = callback;
	return;
}


void OctGrab::FrameGrabber::startEventWorkerThread(void)
{
	closeEventWorkerThread();

	getImpl().eventWorking = true;
	getImpl().worker = thread{ &FrameGrabber::eventWorkerThreadFunction, this };
	return;
}


void OctGrab::FrameGrabber::closeEventWorkerThread(void)
{
	getImpl().eventWorking = false;
	if (getImpl().worker.joinable()) {
		getImpl().worker.join();
	}
	return;
}


void OctGrab::FrameGrabber::eventWorkerThreadFunction(void)
{
	while (true)
	{
		if (!getImpl().eventWorking) {
			break;
		}

		if (updateCurrentEyeSide()) {
		}

		if (updateDeviceStatus()) {

		}

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return;
}




FrameGrabber::FrameGrabberImpl & OctGrab::FrameGrabber::getImpl(void) const
{
	return *d_ptr;
}


